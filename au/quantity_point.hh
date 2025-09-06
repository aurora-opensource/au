// Copyright 2022 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "au/constant.hh"
#include "au/fwd.hh"
#include "au/quantity.hh"
#include "au/stdx/type_traits.hh"
#include "au/utility/type_traits.hh"

namespace au {

// `QuantityPoint`: an _affine space type_ modeling points on a line.
//
// For a quick primer on affine space types, see: http://videocortex.io/2018/Affine-Space-Types/
//
// By "modeling points", we mean that `QuantityPoint` instances cannot be added to each other, and
// cannot be multiplied.  However, they can be subtracted: the difference between two
// `QuantityPoint` instances (of the same Unit) is a `Quantity` of that unit.  We can also add a
// `Quantity` to a `QuantityPoint`, and vice versa; the result is a new `QuantityPoint`.
//
// Key motivating examples include _mile markers_ (effectively `QuantityPoint<Miles, T>`), and
// _absolute temperature measurements_ (e.g., `QuantityPoint<Celsius, T>`).  This type is also
// analogous to `std::chrono::time_point`, in the same way that `Quantity` is analogous to
// `std::chrono::duration`.

// Make a Quantity of the given Unit, which has this value as measured in the Unit.
template <typename UnitT, typename T>
constexpr auto make_quantity_point(T value) {
    return QuantityPointMaker<UnitT>{}(value);
}

// Trait to check whether two QuantityPoint types are exactly equivalent.
template <typename P1, typename P2>
struct AreQuantityPointTypesEquivalent;

namespace detail {
template <typename FromRep, typename ToRep>
struct IntermediateRep;
}  // namespace detail

// Some units have an "origin".  This is not meaningful by itself, but its difference w.r.t. the
// "origin" of another unit of the same Dimension _is_ meaningful.  This type trait provides access
// to that difference.
template <typename U1, typename U2>
constexpr auto origin_displacement(U1, U2) {
    return make_constant(detail::ComputeOriginDisplacementUnit<AssociatedUnitForPointsT<U1>,
                                                               AssociatedUnitForPointsT<U2>>{});
}

template <typename U1, typename U2>
using OriginDisplacement = decltype(origin_displacement(U1{}, U2{}));

// QuantityPoint implementation and API elaboration.
template <typename UnitT, typename RepT>
class QuantityPoint {
    // Q: When should we enable IMPLICIT construction from another QuantityPoint type?
    // A: EXACTLY WHEN our own Diff type can be IMPLICITLY constructed from BOTH the target's Diff
    //    type AND the offset between our Units' zero points.
    //
    // In other words, there are two ways to fail implicit convertibility.
    //
    //   1. Their Diff type might not work with our Rep.  Examples:
    //      BAD: QuantityPoint<Milli<Meters>, int> -> QuantityPoint<Meters, int>
    //      OK : QuantityPoint<Kilo<Meters> , int> -> QuantityPoint<Meters, int>
    //
    //   2. Their zero point might be offset from ours by a non-representable amount.  Examples:
    //      BAD: QuantityPoint<Celsius, int> -> QuantityPoint<Kelvins, int>
    //      OK : QuantityPoint<Celsius, int> -> QuantityPoint<Kelvins, double>
    //      OK : QuantityPoint<Celsius, int> -> QuantityPoint<Milli<Kelvins>, int>
    template <typename OtherUnit, typename OtherRep>
    static constexpr bool should_enable_implicit_construction_from() {
        using Com = CommonUnitT<OtherUnit, detail::ComputeOriginDisplacementUnit<Unit, OtherUnit>>;
        return std::is_convertible<Quantity<Com, OtherRep>, QuantityPoint::Diff>::value;
    }

    // This machinery exists to give us a conditionally explicit constructor, using SFINAE to select
    // the explicit or implicit version (https://stackoverflow.com/a/26949793/15777264).  If we had
    // C++20, we could use the `explicit(bool)` feature, making this code simpler and faster.
    template <bool ImplicitOk, typename OtherUnit, typename OtherRep>
    using EnableIfImplicitOkIs = std::enable_if_t<
        ImplicitOk ==
        QuantityPoint::should_enable_implicit_construction_from<OtherUnit, OtherRep>()>;

 public:
    using Rep = RepT;
    using Unit = UnitT;
    static constexpr Unit unit{};
    using Diff = Quantity<Unit, Rep>;

    // The default constructor produces a QuantityPoint whose value is default constructed.  It
    // exists to give you an object you can assign to.  The main motivating factor for including
    // this is to support `std::atomic`, which requires its types to be default-constructible.
    constexpr QuantityPoint() noexcept : x_{} {}

    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>>
    constexpr QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other)  // NOLINT(runtime/explicit)
        : QuantityPoint{other.template as<Rep>(unit)} {}

    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<false, OtherUnit, OtherRep>,
              typename ThisUnusedTemplateParameterDistinguishesUsFromTheAboveConstructor = void>
    // Deleted: use `.as<NewRep>(new_unit)` to force a cast.
    constexpr explicit QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other) = delete;

    // Construct from another QuantityPoint with an explicit conversion risk policy.
    template <typename OtherUnit,
              typename OtherRep,
              typename RiskPolicyT,
              std::enable_if_t<IsConversionRiskPolicy<RiskPolicyT>::value, int> = 0>
    constexpr QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other, RiskPolicyT policy)
        : QuantityPoint{other.template as<Rep>(Unit{}, policy)} {}

    // The notion of "0" is *not* unambiguous for point types, because different scales can make
    // different decisions about what point is labeled as "0".
    constexpr QuantityPoint(Zero) = delete;

    template <typename NewRep, typename NewUnit, typename RiskPolicyT = decltype(ignore(ALL_RISKS))>
    constexpr auto as(NewUnit u, RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity_point<AssociatedUnitForPointsT<NewUnit>>(in_impl<NewRep>(u, policy));
    }

    template <typename NewUnit, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
    constexpr auto as(NewUnit u, RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity_point<AssociatedUnitForPointsT<NewUnit>>(in_impl<Rep>(u, policy));
    }

    template <typename NewRep, typename NewUnit, typename RiskPolicyT = decltype(ignore(ALL_RISKS))>
    constexpr NewRep in(NewUnit u, RiskPolicyT policy = RiskPolicyT{}) const {
        return in_impl<NewRep>(u, policy);
    }

    template <typename NewUnit, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
    constexpr Rep in(NewUnit u, RiskPolicyT policy = RiskPolicyT{}) const {
        return in_impl<Rep>(u, policy);
    }

    // "Forcing" conversions, which explicitly ignore safety checks for overflow and truncation.
    template <typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `p.coerce_as(new_units)`.
        return as<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `p.coerce_as<T>(new_units)`.
        return as<NewRep>(NewUnit{});
    }
    template <typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `p.coerce_in(new_units)`.
        return in<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `p.coerce_in<T>(new_units)`.
        return in<NewRep>(NewUnit{});
    }

    // Direct access to the underlying value member, with any Point-equivalent Unit.
    //
    // Mutable access, QuantityPointMaker input.
    template <typename U>
    Rep &data_in(const QuantityPointMaker<U> &) {
        static_assert(AreUnitsPointEquivalent<U, Unit>::value,
                      "Can only access value via Point-equivalent unit");
        return x_.data_in(QuantityMaker<U>{});
    }
    // Mutable access, Unit input.
    template <typename U>
    Rep &data_in(const U &) {
        return data_in(QuantityPointMaker<U>{});
    }
    // Const access, QuantityPointMaker input.
    template <typename U>
    const Rep &data_in(const QuantityPointMaker<U> &) const {
        static_assert(AreUnitsPointEquivalent<U, Unit>::value,
                      "Can only access value via Point-equivalent unit");
        return x_.data_in(QuantityMaker<U>{});
    }
    // Const access, Unit input.
    template <typename U>
    const Rep &data_in(const U &) const {
        return data_in(QuantityPointMaker<U>{});
    }

    // Comparison operators.
    constexpr friend bool operator==(QuantityPoint a, QuantityPoint b) { return a.x_ == b.x_; }
    constexpr friend bool operator!=(QuantityPoint a, QuantityPoint b) { return a.x_ != b.x_; }
    constexpr friend bool operator>=(QuantityPoint a, QuantityPoint b) { return a.x_ >= b.x_; }
    constexpr friend bool operator>(QuantityPoint a, QuantityPoint b) { return a.x_ > b.x_; }
    constexpr friend bool operator<=(QuantityPoint a, QuantityPoint b) { return a.x_ <= b.x_; }
    constexpr friend bool operator<(QuantityPoint a, QuantityPoint b) { return a.x_ < b.x_; }

    // Subtraction between two QuantityPoint types.
    constexpr friend Diff operator-(QuantityPoint a, QuantityPoint b) { return a.x_ - b.x_; }

    // Left and right addition of a Diff.
    constexpr friend auto operator+(Diff d, QuantityPoint p) { return QuantityPoint{d + p.x_}; }
    constexpr friend auto operator+(QuantityPoint p, Diff d) { return QuantityPoint{p.x_ + d}; }

    // Right subtraction of a Diff.
    constexpr friend auto operator-(QuantityPoint p, Diff d) { return QuantityPoint{p.x_ - d}; }

    // Short-hand addition assignment.
    constexpr QuantityPoint &operator+=(Diff diff) {
        x_ += diff;
        return *this;
    }

    // Short-hand subtraction assignment.
    constexpr QuantityPoint &operator-=(Diff diff) {
        x_ -= diff;
        return *this;
    }

    // Permit this factory functor to access our private constructor.
    //
    // We allow this because it explicitly names the unit at the callsite, even if people refer to
    // this present Quantity type by an alias that omits the unit.  This preserves Unit Safety and
    // promotes callsite readability.
    friend struct QuantityPointMaker<Unit>;

 private:
    template <typename OtherRep, typename OtherPointUnitSlot, typename RiskPolicyT>
    constexpr OtherRep in_impl(OtherPointUnitSlot, RiskPolicyT policy) const {
        using OtherUnit = AssociatedUnitForPointsT<OtherPointUnitSlot>;
        using OriginDisplacementUnit = detail::ComputeOriginDisplacementUnit<Unit, OtherUnit>;
        using Common = CommonUnitT<Unit, OtherUnit, OriginDisplacementUnit>;

        using CalcRep = typename detail::IntermediateRep<Rep, OtherRep>::type;

        Quantity<Common, CalcRep> intermediate_result =
            x_.template as<CalcRep>(Common{}, policy) + origin_displacement(OtherUnit{}, unit);
        return intermediate_result.template in<OtherRep>(OtherUnit{}, policy);
    }

    constexpr explicit QuantityPoint(Diff x) : x_{x} {}

    Diff x_;
};

template <typename Unit>
struct QuantityPointMaker {
    static constexpr auto unit = Unit{};

    template <typename T>
    constexpr auto operator()(T value) const {
        return QuantityPoint<Unit, T>{make_quantity<Unit>(value)};
    }

    template <typename U, typename R>
    constexpr void operator()(Quantity<U, R>) const {
        constexpr bool is_not_a_quantity = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_a_quantity, "Input to QuantityPointMaker is a Quantity");
    }

    template <typename U, typename R>
    constexpr void operator()(QuantityPoint<U, R>) const {
        constexpr bool is_not_already_a_quantity_point = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_already_a_quantity_point,
                      "Input to QuantityPointMaker is already a QuantityPoint");
    }

    template <typename... BPs>
    constexpr auto operator*(Magnitude<BPs...> m) const {
        return QuantityPointMaker<decltype(unit * m)>{};
    }

    template <typename... BPs>
    constexpr auto operator/(Magnitude<BPs...> m) const {
        return QuantityPointMaker<decltype(unit / m)>{};
    }
};

template <typename U>
struct AssociatedUnitForPoints<QuantityPointMaker<U>> : stdx::type_identity<U> {};

// Provide nicer error messages when users try passing a `QuantityPoint` to a unit slot.
template <typename U, typename R>
struct AssociatedUnit<QuantityPoint<U, R>> {
    static_assert(
        detail::AlwaysFalse<U, R>::value,
        "Cannot pass QuantityPoint to a unit slot (see: "
        "https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)");
};
template <typename U, typename R>
struct AssociatedUnitForPoints<QuantityPoint<U, R>> {
    static_assert(
        detail::AlwaysFalse<U, R>::value,
        "Cannot pass QuantityPoint to a unit slot (see: "
        "https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)");
};

// Type trait to detect whether two QuantityPoint types are equivalent.
//
// In this library, QuantityPoint types are "equivalent" exactly when they use the same Rep, and are
// based on point-equivalent units.
template <typename U1, typename U2, typename R1, typename R2>
struct AreQuantityPointTypesEquivalent<QuantityPoint<U1, R1>, QuantityPoint<U2, R2>>
    : stdx::conjunction<std::is_same<R1, R2>, AreUnitsPointEquivalent<U1, U2>> {};

// Cast QuantityPoint to a different underlying type.
template <typename NewRep, typename Unit, typename Rep>
constexpr auto rep_cast(QuantityPoint<Unit, Rep> q) {
    return q.template as<NewRep>(Unit{});
}

namespace detail {
template <typename X, typename Y, typename Func>
constexpr auto using_common_point_unit(X x, Y y, Func f) {
    using R = std::common_type_t<typename X::Rep, typename Y::Rep>;
    constexpr auto u = CommonPointUnitT<typename X::Unit, typename Y::Unit>{};
    return f(rep_cast<R>(x).as(u), rep_cast<R>(y).as(u));
}

template <typename Op, typename U1, typename U2, typename R1, typename R2>
constexpr auto convert_and_compare(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    using U = CommonPointUnitT<U1, U2>;
    using ComRep1 = detail::CommonTypeButPreserveIntSignedness<R1, R2>;
    using ComRep2 = detail::CommonTypeButPreserveIntSignedness<R2, R1>;
    return detail::SignAwareComparison<UnitSign<U>, Op>{}(
        p1.template in<ComRep1>(U{}, check_for(ALL_RISKS)),
        p2.template in<ComRep2>(U{}, check_for(ALL_RISKS)));
}
}  // namespace detail

// Comparison functions for compatible QuantityPoint types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator<(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::Less>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator>(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::Greater>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator<=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::LessEqual>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator>=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::GreaterEqual>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator==(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::Equal>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator!=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::NotEqual>(p1, p2);
}

namespace detail {
// Another subtlety arises when we mix QuantityPoint and Quantity in adding or subtracting.  We
// actually don't want to use `CommonPointUnitT`, because this is too restrictive if the units have
// different origins.  Imagine adding a `Quantity<Kelvins>` to a `QuantityPoint<Celsius>`---we
// wouldn't want this to subdivide the unit of measure to satisfy an additive relative offset which
// we will never actually use!
//
// The solution is to set the (unused!) origin of the `Quantity` unit to the same as the
// `QuantityPoint` unit.  Once we do, everything flows simply from there.
//
// This utility should be used for every overload below which combines a `QuantityPoint` with a
// `Quantity`.
template <typename Target, typename U>
constexpr auto borrow_origin(U u) {
    return Target{} * unit_ratio(u, Target{});
}
}  // namespace detail

// Addition and subtraction functions for compatible QuantityPoint types.
template <typename UnitP, typename UnitQ, typename RepP, typename RepQ>
constexpr auto operator+(QuantityPoint<UnitP, RepP> p, Quantity<UnitQ, RepQ> q) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(p, q.as(new_unit_q), detail::plus);
}
template <typename UnitQ, typename UnitP, typename RepQ, typename RepP>
constexpr auto operator+(Quantity<UnitQ, RepQ> q, QuantityPoint<UnitP, RepP> p) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(q.as(new_unit_q), p, detail::plus);
}
template <typename UnitP, typename UnitQ, typename R1, typename RepQ>
constexpr auto operator-(QuantityPoint<UnitP, R1> p, Quantity<UnitQ, RepQ> q) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(p, q.as(new_unit_q), detail::minus);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator-(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::minus);
}

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto operator<=>(const QuantityPoint<U1, R1> &lhs, const QuantityPoint<U2, R2> &rhs) {
    return detail::convert_and_compare<detail::ThreeWayCompare>(lhs, rhs);
}
#endif

namespace detail {

// We simply want a version of `std::make_signed_t` that won't choke on non-integral types.
template <typename T>
struct MakeSigned : std::conditional<std::is_integral<T>::value, std::make_signed_t<T>, T> {};

// If the destination is a signed integer, we want to ensure we do our
// computations in a signed type.  Otherwise, just use the common type for our
// intermediate computations.
template <typename CommonT, bool IsDestinationSigned>
struct IntermediateRepImpl
    : std::conditional_t<stdx::conjunction<std::is_integral<CommonT>,
                                           stdx::bool_constant<IsDestinationSigned>>::value,
                         MakeSigned<CommonT>,
                         stdx::type_identity<CommonT>> {};

template <typename FromRep, typename ToRep>
struct IntermediateRep
    : IntermediateRepImpl<std::common_type_t<FromRep, ToRep>, std::is_signed<ToRep>::value> {};

}  // namespace detail
}  // namespace au
