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

#include <type_traits>
#include <utility>

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
#include <compare>
#endif

#include "au/config.hh"
#include "au/conversion_policy.hh"
#include "au/conversion_strategy.hh"
#include "au/fwd.hh"
#include "au/operators.hh"
#include "au/rep.hh"
#include "au/stdx/functional.hh"
#include "au/truncation_risk.hh"
#include "au/unit_of_measure.hh"
#include "au/utility/type_traits.hh"
#include "au/view.hh"
#include "au/zero.hh"

namespace au {

//
// Make a Quantity of the given Unit, which has this value as measured in the Unit.
//
// lvalue: copy.  (Never move something the caller still owns; also the only thing that works for a
// packed field, which can't bind to a reference.)
template <typename UnitT, typename T>
AU_DEVICE_FUNC constexpr auto make_quantity(const T &value) {
    return QuantityMaker<UnitT>{}(value);
}

// rvalue: move.
template <typename UnitT,
          typename T,
          typename = std::enable_if_t<!std::is_lvalue_reference<T>::value>>
AU_DEVICE_FUNC constexpr auto make_quantity(T &&value) {
    return QuantityMaker<UnitT>{}(std::move(value));
}

// lvalue: copy.  (See `make_quantity` above.)
template <typename Unit, typename T>
AU_DEVICE_FUNC constexpr auto make_quantity_unless_unitless(const T &value) {
    return std::conditional_t<IsUnitlessUnit<Unit>::value, stdx::identity, QuantityMaker<Unit>>{}(
        value);
}

// rvalue: move.
template <typename Unit,
          typename T,
          typename = std::enable_if_t<!std::is_lvalue_reference<T>::value>>
AU_DEVICE_FUNC constexpr auto make_quantity_unless_unitless(T &&value) {
    return std::conditional_t<IsUnitlessUnit<Unit>::value, stdx::identity, QuantityMaker<Unit>>{}(
        std::move(value));
}

// Trait to check whether two Quantity types are exactly equivalent.
//
// For purposes of our library, "equivalent" means that they have the same Dimension and Magnitude.
template <typename Q1, typename Q2>
struct AreQuantityTypesEquivalent;

// Trait for a type T which corresponds exactly to some Quantity type.
//
// "Correspondence" with a `Quantity<U, R>` means that T stores a value in a numeric datatype R, and
// this value represents a quantity whose unit of measure is quantity-equivalent to U.
//
// The canonical examples are the `duration` types from the `std::chrono::library`.  For example,
// `std::chrono::duration<double, std::nano>` exactly corresponds to `QuantityD<Nano<Seconds>>`, and
// it is always OK to convert back and forth between these types implicitly.
//
// To add support for a type T which is equivalent to Quantity<U, R>, define a specialization of
// `CorrespondingQuantity<T>` with a member alias `Unit` for `U`, and `Rep` for `R`.  You should
// then add static member functions as follows to add support for each direction of conversion.
//   - For T -> Quantity, define `R extract_value(T)`.
//   - For Quantity -> T, define `T construct_from_value(R)`.
template <typename T>
struct CorrespondingQuantity {};

namespace detail {
template <typename T>
using CorrespondingQuantityType =
    Quantity<typename CorrespondingQuantity<T>::Unit, typename CorrespondingQuantity<T>::Rep>;
}  // namespace detail

// Redirect various cvref-qualified specializations to the "main" specialization.
//
// We use this slightly counterintuitive approach, rather than a more conventional
// `remove_cvref_t`-based approach, because the latter causes an _internal compiler error_ on the
// ACI QNX build.
template <typename T>
struct CorrespondingQuantity<const T> : CorrespondingQuantity<T> {};
template <typename T>
struct CorrespondingQuantity<T &> : CorrespondingQuantity<T> {};
template <typename T>
struct CorrespondingQuantity<const T &> : CorrespondingQuantity<T> {};

// Request conversion of any type to its corresponding Quantity, if there is one.
//
// This is a way to explicitly and readably "enter the au Quantity domain" when we have some
// non-au-Quantity type which is nevertheless exactly and unambiguously equivalent to some Quantity.
//
// `as_quantity()` is SFINAE-friendly: we can use it to constrain templates to types `T` which are
// exactly equivalent to some Quantity type.
template <typename T>
AU_DEVICE_FUNC constexpr auto as_quantity(T &&x) -> detail::CorrespondingQuantityType<T> {
    using Q = CorrespondingQuantity<T>;
    static_assert(IsUnit<typename Q::Unit>{}, "No Quantity corresponding to type");

    auto value = Q::extract_value(std::forward<T>(x));
    static_assert(std::is_same<decltype(value), typename Q::Rep>{},
                  "Inconsistent CorrespondingQuantity implementation");

    return make_quantity<typename Q::Unit>(value);
}

// Callsite-readable way to convert a `Quantity` to a raw number.
//
// Only works for dimensionless `Quantities`; will return a compile-time error otherwise.
//
// Identity for non-`Quantity` types.
template <typename U, typename R, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
AU_DEVICE_FUNC constexpr R as_raw_number(Quantity<U, R> q, RiskPolicyT policy = RiskPolicyT{}) {
    return q.in(UnitProduct<>{}, policy);
}
template <typename T>
AU_DEVICE_FUNC constexpr T as_raw_number(T x) {
    return x;
}

namespace detail {
// We implement `Quantity` comparisons by converting to a common unit, and comparing the values
// stored in the underlying Rep types.  This means we need to know the _sign_ of that common unit,
// so we can know which order to pass those underlying values (it gets reversed for negative units).
template <typename SignMag, typename Op>
struct SignAwareComparison;
}  // namespace detail

template <typename UnitT, typename RepT>
class Quantity {
    template <bool ImplicitOk, typename OtherUnit, typename OtherRep>
    using EnableIfImplicitOkIs = std::enable_if_t<
        ImplicitOk ==
        ConstructionPolicy<UnitT, RepT>::template PermitImplicitFrom<OtherUnit, OtherRep>::value>;

    // We could consider making this public someday, if we had a use case.
    using Sign = UnitSign<UnitT>;

    // Not strictly necessary, but we want to keep each comparator implementation to one line.
    using Eq = detail::SignAwareComparison<Sign, detail::Equal>;
    using Ne = detail::SignAwareComparison<Sign, detail::NotEqual>;
    using Lt = detail::SignAwareComparison<Sign, detail::Less>;
    using Le = detail::SignAwareComparison<Sign, detail::LessEqual>;
    using Gt = detail::SignAwareComparison<Sign, detail::Greater>;
    using Ge = detail::SignAwareComparison<Sign, detail::GreaterEqual>;

 public:
    using Rep = RepT;
    using Unit = UnitT;
    static constexpr auto unit = Unit{};

    static_assert(IsValidRep<Rep>::value, "Rep must meet our requirements for a rep");

    // IMPLICIT constructor for another Quantity of the same Dimension.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>>
    AU_DEVICE_FUNC constexpr Quantity(
        const Quantity<OtherUnit, OtherRep> &other)  // NOLINT(runtime/explicit)
        : value_{other.template in_impl<detail::UseImplicitConversion, Rep>(
              UnitT{}, check_for(ALL_RISKS))} {}

    // EXPLICIT constructor for another Quantity of the same Dimension.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<false, OtherUnit, OtherRep>,
              typename ThisUnusedTemplateParameterDistinguishesUsFromTheAboveConstructor = void>
    // Deleted: use `.as<NewRep>(new_unit)` to force a cast.
    explicit constexpr Quantity(const Quantity<OtherUnit, OtherRep> &other) = delete;

    // Constructor for another Quantity with an explicit conversion risk policy.
    template <typename OtherUnit,
              typename OtherRep,
              typename RiskPolicyT,
              std::enable_if_t<IsConversionRiskPolicy<RiskPolicyT>::value, int> = 0>
    AU_DEVICE_FUNC constexpr Quantity(const Quantity<OtherUnit, OtherRep> &other,
                                      RiskPolicyT policy)
        : value_{other.template in<Rep>(UnitT{}, policy)} {}

    // Construct this Quantity with a value of exactly Zero.
    AU_DEVICE_FUNC constexpr Quantity(Zero) : value_{0} {}

    AU_DEVICE_FUNC constexpr Quantity() noexcept = default;
    AU_DEVICE_FUNC constexpr Quantity(const Quantity &) = default;
    AU_DEVICE_FUNC constexpr Quantity(Quantity &&) = default;

    // Implicit construction from any exactly-equivalent type.
    template <
        typename T,
        std::enable_if_t<std::is_convertible<detail::CorrespondingQuantityType<T>, Quantity>::value,
                         int> = 0>
    AU_DEVICE_FUNC constexpr Quantity(T &&x) : Quantity{as_quantity(std::forward<T>(x))} {}

    // `q.as<Rep>()`, or `q.as<Rep>(risk_policy)`
    template <typename NewRep,
              typename RiskPolicyT = decltype(check_for(ALL_RISKS)),
              std::enable_if_t<IsConversionRiskPolicy<RiskPolicyT>::value, int> = 0>
    AU_DEVICE_FUNC constexpr auto as(RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity<Unit>(in_impl<detail::UseStaticCast, NewRep>(Unit{}, policy));
    }

    // `q.as<Rep>(new_unit)`, or `q.as<Rep>(new_unit, risk_policy)`
    template <typename NewRep,
              typename NewUnitSlot,
              typename RiskPolicyT = decltype(ignore(ALL_RISKS)),
              std::enable_if_t<!IsConversionRiskPolicy<NewUnitSlot>::value, int> = 0>
    AU_DEVICE_FUNC constexpr auto as(NewUnitSlot u, RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity<AssociatedUnit<NewUnitSlot>>(
            in_impl<detail::UseStaticCast, NewRep>(u, policy));
    }

    // `q.as(new_unit)`, or `q.as(new_unit, risk_policy)`
    template <typename NewUnitSlot, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
    AU_DEVICE_FUNC constexpr auto as(NewUnitSlot u, RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity<AssociatedUnit<NewUnitSlot>>(
            in_impl<detail::UseStaticCast, void>(u, policy));
    }

    // `q.in<Rep>(new_unit)`, or `q.in<Rep>(new_unit, risk_policy)`
    template <typename NewRep,
              typename NewUnitSlot,
              typename RiskPolicyT = decltype(ignore(ALL_RISKS))>
    AU_DEVICE_FUNC constexpr auto in(NewUnitSlot u, RiskPolicyT policy = RiskPolicyT{}) const {
        return in_impl<detail::UseStaticCast, NewRep>(u, policy);
    }

    // `q.in(new_unit)`, or `q.in(new_unit, risk_policy)`
    template <typename NewUnitSlot, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
    AU_DEVICE_FUNC constexpr auto in(NewUnitSlot u, RiskPolicyT policy = RiskPolicyT{}) const {
        return in_impl<detail::UseStaticCast, void>(u, policy);
    }

    // "Forcing" conversions, which explicitly ignore safety checks for overflow and truncation.
    template <typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `q.coerce_as(new_units)`.
        return as<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `q.coerce_as<T>(new_units)`.
        return as<NewRep>(NewUnit{});
    }
    template <typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `q.coerce_in(new_units)`.
        return in<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `q.coerce_in<T>(new_units)`.
        return in<NewRep>(NewUnit{});
    }

    // Direct access to the underlying value member, with any Quantity-equivalent Unit.
    //
    // Mutable access:
    template <typename UnitSlot>
    AU_DEVICE_FUNC constexpr Rep &data_in(UnitSlot) {
        static_assert(AreUnitsQuantityEquivalent<AssociatedUnit<UnitSlot>, Unit>::value,
                      "Can only access value via Quantity-equivalent unit");
        return value_;
    }
    // Const access:
    template <typename UnitSlot>
    AU_DEVICE_FUNC constexpr const Rep &data_in(UnitSlot) const {
        static_assert(AreUnitsQuantityEquivalent<AssociatedUnit<UnitSlot>, Unit>::value,
                      "Can only access value via Quantity-equivalent unit");
        return value_;
    }

    // Passthrough element access for vector/matrix rep types.
    template <typename R = Rep, typename I>
    AU_DEVICE_FUNC constexpr auto operator[](I i) const
        -> decltype(make_quantity<UnitT>(std::declval<const R &>()[i])) {
        return make_quantity<UnitT>(value_[i]);
    }

    template <typename R = Rep, typename... Is>
    AU_DEVICE_FUNC constexpr auto operator()(Is... is) const
        -> decltype(make_quantity<UnitT>(std::declval<const R &>()(is...))) {
        return make_quantity<UnitT>(value_(is...));
    }

    // Return a mutable view of this Quantity.
    AU_DEVICE_FUNC constexpr Quantity<UnitT, View<RepT>> mutable_view() & {
        return make_quantity<UnitT>(make_view(value_));
    }

    // Permit this factory functor to access our private constructor.
    //
    // We allow this because it explicitly names the unit at the callsite, even if people refer to
    // this present Quantity type by an alias that omits the unit.  This preserves Unit Safety and
    // promotes callsite readability.
    friend struct QuantityMaker<UnitT>;

    // Comparison operators.
    friend AU_DEVICE_FUNC constexpr bool operator==(Quantity a, Quantity b) {
        return Eq{}(a.value_, b.value_);
    }
    friend AU_DEVICE_FUNC constexpr bool operator!=(Quantity a, Quantity b) {
        return Ne{}(a.value_, b.value_);
    }
    friend AU_DEVICE_FUNC constexpr bool operator<(Quantity a, Quantity b) {
        return Lt{}(a.value_, b.value_);
    }
    friend AU_DEVICE_FUNC constexpr bool operator<=(Quantity a, Quantity b) {
        return Le{}(a.value_, b.value_);
    }
    friend AU_DEVICE_FUNC constexpr bool operator>(Quantity a, Quantity b) {
        return Gt{}(a.value_, b.value_);
    }
    friend AU_DEVICE_FUNC constexpr bool operator>=(Quantity a, Quantity b) {
        return Ge{}(a.value_, b.value_);
    }

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
    using Twc = detail::SignAwareComparison<Sign, detail::ThreeWayCompare>;
    friend AU_DEVICE_FUNC constexpr auto operator<=>(Quantity a, Quantity b) {
        return Twc{}(a.value_, b.value_);
    }
#endif

    // Addition and subtraction for like quantities.
    friend AU_DEVICE_FUNC constexpr Quantity<UnitT,
                                             decltype(std::declval<RepT>() + std::declval<RepT>())>
    operator+(const Quantity &a, const Quantity &b) {
        return make_quantity<UnitT>(a.value_ + b.value_);
    }
    friend AU_DEVICE_FUNC constexpr Quantity<UnitT,
                                             decltype(std::declval<RepT>() - std::declval<RepT>())>
    operator-(const Quantity &a, const Quantity &b) {
        return make_quantity<UnitT>(a.value_ - b.value_);
    }

    // Scalar multiplication.
    template <typename T, typename = std::enable_if_t<IsProductValidRep<RepT, T>::value>>
    friend AU_DEVICE_FUNC constexpr auto operator*(const Quantity &a, T s) {
        return make_quantity<UnitT>(a.value_ * s);
    }
    template <typename T, typename = std::enable_if_t<IsProductValidRep<T, RepT>::value>>
    friend AU_DEVICE_FUNC constexpr auto operator*(T s, const Quantity &a) {
        return make_quantity<UnitT>(s * a.value_);
    }

    // Scalar division.
    template <typename T, typename = std::enable_if_t<IsQuotientValidRep<RepT, T>::value>>
    friend AU_DEVICE_FUNC constexpr auto operator/(const Quantity &a, T s) {
        return make_quantity<UnitT>(a.value_ / s);
    }
    template <typename T, typename = std::enable_if_t<IsQuotientValidRep<T, RepT>::value>>
    friend AU_DEVICE_FUNC constexpr auto operator/(T s, const Quantity &a) {
        warn_if_integer_division<UnitProduct<>, T>();
        return make_quantity<decltype(pow<-1>(unit))>(s / a.value_);
    }

    // Multiplication for dimensioned quantities.
    template <typename OtherUnit, typename OtherRep>
    AU_DEVICE_FUNC constexpr auto operator*(Quantity<OtherUnit, OtherRep> q) const {
        return make_quantity_unless_unitless<UnitProduct<Unit, OtherUnit>>(value_ *
                                                                           q.in(OtherUnit{}));
    }

    // Division for dimensioned quantities.
    template <typename OtherUnit, typename OtherRep>
    AU_DEVICE_FUNC constexpr auto operator/(Quantity<OtherUnit, OtherRep> q) const {
        warn_if_integer_division<OtherUnit, OtherRep>();
        return make_quantity_unless_unitless<UnitQuotient<Unit, OtherUnit>>(value_ /
                                                                            q.in(OtherUnit{}));
    }

    // Copy and move assignment: lvalue-only.
    //
    // Ref-qualifying as `&` prevents silent no-ops like `q[0] = meters(10)`, where operator[]
    // returns by value and the assignment would modify a temporary.  The `&&`-qualified overloads
    // below re-enable rvalue assignment specifically for View reps, where it writes through.
    AU_DEVICE_FUNC constexpr Quantity &operator=(const Quantity &) & = default;
    AU_DEVICE_FUNC constexpr Quantity &operator=(Quantity &&) & = default;

    // Cross-unit assignment: lvalue-only.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>>
    AU_DEVICE_FUNC constexpr Quantity &operator=(Quantity<OtherUnit, OtherRep> other) & {
        value_ = other.template in_impl<detail::UseImplicitConversion, detail::UnderlyingType<Rep>>(
            Unit{}, check_for(ALL_RISKS));
        return *this;
    }

    // Rvalue assignment overloads for View reps (write-through semantics).
    template <typename R = Rep, std::enable_if_t<IsView<R>::value, int> = 0>
    AU_DEVICE_FUNC constexpr Quantity &operator=(const Quantity &other) && {
        value_ = other.value_;
        return *this;
    }
    template <typename OtherUnit,
              typename OtherRep,
              typename R = Rep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>,
              std::enable_if_t<IsView<R>::value, int> = 0>
    AU_DEVICE_FUNC constexpr Quantity &operator=(Quantity<OtherUnit, OtherRep> other) && {
        value_ = other.template in_impl<detail::UseImplicitConversion, detail::UnderlyingType<Rep>>(
            Unit{}, check_for(ALL_RISKS));
        return *this;
    }

    // Short-hand addition and subtraction assignment: lvalue-only by default.
    AU_DEVICE_FUNC constexpr Quantity &operator+=(Quantity other) & {
        value_ += other.value_;
        return *this;
    }
    AU_DEVICE_FUNC constexpr Quantity &operator-=(Quantity other) & {
        value_ -= other.value_;
        return *this;
    }

    // Support short-hand addition and subtraction on rvalues for view rep types only.
    template <typename R = Rep, std::enable_if_t<IsView<R>::value, int> = 0>
    AU_DEVICE_FUNC constexpr Quantity &operator+=(Quantity other) && {
        value_ += other.value_;
        return *this;
    }
    template <typename R = Rep, std::enable_if_t<IsView<R>::value, int> = 0>
    AU_DEVICE_FUNC constexpr Quantity &operator-=(Quantity other) && {
        value_ -= other.value_;
        return *this;
    }

    template <typename T>
    AU_DEVICE_FUNC constexpr void perform_shorthand_checks() {
        static_assert(
            IsValidRep<T>::value,
            "This overload is only for scalar mult/div-assignment with raw numeric types");

        static_assert((!std::is_integral<detail::RealPart<Rep>>::value) ||
                          std::is_integral<detail::RealPart<T>>::value,
                      "We don't support compound mult/div of integral types by floating point");
    }

    // Short-hand multiplication assignment: most types lvalue-only; view types support rvalues.
    template <typename T>
    AU_DEVICE_FUNC constexpr Quantity &operator*=(T s) & {
        perform_shorthand_checks<T>();

        value_ *= s;
        return *this;
    }
    template <typename T, typename R = Rep, std::enable_if_t<IsView<R>::value, int> = 0>
    AU_DEVICE_FUNC constexpr Quantity &operator*=(T s) && {
        perform_shorthand_checks<T>();

        value_ *= s;
        return *this;
    }

    // Short-hand division assignment: most types lvalue-only; view types support rvalues.
    template <typename T>
    AU_DEVICE_FUNC constexpr Quantity &operator/=(T s) & {
        perform_shorthand_checks<T>();

        value_ /= s;
        return *this;
    }
    template <typename T, typename R = Rep, std::enable_if_t<IsView<R>::value, int> = 0>
    AU_DEVICE_FUNC constexpr Quantity &operator/=(T s) && {
        perform_shorthand_checks<T>();

        value_ /= s;
        return *this;
    }

    // Modulo operator (defined only for integral rep).
    friend AU_DEVICE_FUNC constexpr Quantity operator%(Quantity a, Quantity b) {
        return {a.value_ % b.value_};
    }

    // Unary plus and minus.
    AU_DEVICE_FUNC constexpr Quantity operator+() const { return {+value_}; }
    AU_DEVICE_FUNC constexpr Quantity operator-() const { return {-value_}; }

    // Automatic conversion to Rep for Unitless type.
    template <typename U = UnitT, typename = std::enable_if_t<IsUnitlessUnit<U>::value>>
    AU_DEVICE_FUNC constexpr operator Rep() const {
        return value_;
    }

    // Automatic conversion to any equivalent type that supports it.
    template <
        typename T,
        std::enable_if_t<std::is_convertible<Quantity, detail::CorrespondingQuantityType<T>>::value,
                         int> = 0>
    AU_DEVICE_FUNC constexpr operator T() const {
        return CorrespondingQuantity<T>::construct_from_value(
            detail::CorrespondingQuantityType<T>{*this}.in(
                typename CorrespondingQuantity<T>::Unit{}));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Pre-C++20 Non-Type Template Parameter (NTTP) functionality.
    //
    // If `Rep` is a built in integral type, then `Quantity::NTTP` can be used as a template
    // parameter.

    enum class NTTP : std::conditional_t<std::is_integral<Rep>::value, Rep, bool> {
        ENUM_VALUES_ARE_UNUSED
    };

    AU_DEVICE_FUNC constexpr Quantity(NTTP val) : value_{static_cast<Rep>(val)} {
        static_assert(std::is_integral<Rep>::value,
                      "NTTP functionality only works when rep is built-in integral type");
    }

    AU_DEVICE_FUNC constexpr operator NTTP() const {
        static_assert(std::is_integral<Rep>::value,
                      "NTTP functionality only works when rep is built-in integral type");
        return static_cast<NTTP>(value_);
    }

    template <typename C, C x = C::ENUM_VALUES_ARE_UNUSED>
    AU_DEVICE_FUNC constexpr operator C() const = delete;
    // If you got here ^^^, then you need to do your unit conversion **manually**.  Check the type
    // of the template parameter, and convert it to that same unit and rep.

    friend AU_DEVICE_FUNC constexpr Quantity from_nttp(NTTP val) { return val; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Hidden friends for select math functions.
    //
    // Moving the implementation here lets us effortlessly support callsites where any number of
    // arguments are "shapeshifter" types that are compatible with this Quantity (such as `ZERO`, or
    // various physical constant).
    //
    // Note that the min/max implementations return by _value_, for consistency with other Quantity
    // implementations (because in the general case, the return type can differ from the inputs).
    // Note, too, that we use the Walter Brown implementation for min/max, where min prefers `a`,
    // max prefers `b`, and they never return the same input (although this matters less when we're
    // returning by value).
    friend AU_DEVICE_FUNC constexpr Quantity min(Quantity a, Quantity b) { return b < a ? b : a; }
    friend AU_DEVICE_FUNC constexpr Quantity max(Quantity a, Quantity b) { return b < a ? a : b; }
    friend AU_DEVICE_FUNC constexpr Quantity clamp(Quantity v, Quantity lo, Quantity hi) {
        return (v < lo) ? lo : ((hi < v) ? hi : v);
    }

#if defined(__cpp_lib_interpolate) && __cpp_lib_interpolate >= 201902L
    // `std::lerp` requires C++20 support.
    template <typename T>
    friend AU_DEVICE_FUNC constexpr auto lerp(Quantity a, Quantity b, T t) {
        return make_quantity<UnitT>(std::lerp(a.in(unit), b.in(unit), as_raw_number(t)));
    }
#endif

    template <typename OtherUnit, typename OtherRep>
    friend class Quantity;

 private:
    template <typename OtherUnit, typename OtherRep>
    static AU_DEVICE_FUNC constexpr void warn_if_integer_division() {
        constexpr bool uses_integer_division =
            (std::is_integral<Rep>::value && std::is_integral<OtherRep>::value);
        constexpr bool are_units_quantity_equivalent =
            AreUnitsQuantityEquivalent<UnitT, OtherUnit>::value;
        static_assert(are_units_quantity_equivalent || !uses_integer_division,
                      "Integer division forbidden.  See "
                      "<https://aurora-opensource.github.io/au/main/troubleshooting/"
                      "#integer-division-forbidden> for more details about the risks, "
                      "and your options to resolve this error.");
    }

    template <typename CastStrategy,
              typename OtherRep,
              typename OtherUnitSlot,
              typename RiskPolicyT>
    AU_DEVICE_FUNC constexpr auto in_impl(OtherUnitSlot, RiskPolicyT) const {
        using OtherUnit = AssociatedUnit<OtherUnitSlot>;
        static_assert(IsUnit<OtherUnit>::value, "Invalid type passed to unit slot");

        using Op = detail::
            ConversionForRepsAndFactor<CastStrategy, Rep, OtherRep, UnitRatio<Unit, OtherUnit>>;

        constexpr bool should_check_overflow =
            RiskPolicyT{}.should_check(detail::ConversionRisk::Overflow);
        constexpr bool is_overflow_risk_ok = stdx::disjunction<
            detail::OverflowRiskAcceptablyLow<Op>,
            detail::PermitAsCarveOutForIntegerPromotion<OtherRep,
                                                        UnitRatio<Unit, OtherUnit>,
                                                        Rep>>::value;

        constexpr bool should_check_truncation =
            RiskPolicyT{}.should_check(detail::ConversionRisk::Truncation);
        constexpr bool is_truncation_risk_ok = detail::TruncationRiskAcceptablyLow<Op>::value;

        constexpr bool is_overflow_only_unacceptable_risk =
            (should_check_overflow && !is_overflow_risk_ok && is_truncation_risk_ok);
        static_assert(!is_overflow_only_unacceptable_risk,
                      "Overflow risk too high.  See "
                      "<https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>"
                      ".  Your \"risk set\" is `OVERFLOW_RISK`.");

        constexpr bool is_truncation_only_unacceptable_risk =
            (should_check_truncation && !is_truncation_risk_ok && is_overflow_risk_ok);
        static_assert(!is_truncation_only_unacceptable_risk,
                      "Truncation risk too high.  See "
                      "<https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>"
                      ".  Your \"risk set\" is `TRUNCATION_RISK`.");

        constexpr bool are_both_overflow_and_truncation_unacceptably_risky =
            (should_check_overflow || should_check_truncation) && !is_overflow_risk_ok &&
            !is_truncation_risk_ok;
        static_assert(!are_both_overflow_and_truncation_unacceptably_risky,
                      "Both truncation and overflow risk too high.  See "
                      "<https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>"
                      ".  Your \"risk set\" is `OVERFLOW_RISK | TRUNCATION_RISK`.");

        return Op::apply_to(value_);
    }

    AU_DEVICE_FUNC constexpr Quantity(Rep value) : value_{std::move(value)} {}

    Rep value_{};
};

// Give more readable error messages when passing `Quantity` to a unit slot.
template <typename U, typename R>
struct AssociatedUnitImpl<Quantity<U, R>> {
    static_assert(
        detail::AlwaysFalse<U, R>::value,
        "Can't pass `Quantity` to a unit slot (see: "
        "https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)");
};
template <typename U, typename R>
struct AssociatedUnitForPointsImpl<Quantity<U, R>> {
    static_assert(
        detail::AlwaysFalse<U, R>::value,
        "Can't pass `Quantity` to a unit slot for points (see: "
        "https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)");
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Machinery to explicitly unblock integer division.
//
// Dividing by `unblock_int_div(x)` will allow integer division for any `x`.  If the division would
// have been allowed anyway, then `unblock_int_div` is a no-op: this enables us to write templated
// code to handle template parameters that may or may not be integral.

template <typename U, typename R>
class AlwaysDivisibleQuantity;

// Unblock integer divisoin for a `Quantity`.
template <typename U, typename R>
AU_DEVICE_FUNC constexpr AlwaysDivisibleQuantity<U, R> unblock_int_div(Quantity<U, R> q) {
    return AlwaysDivisibleQuantity<U, R>{q};
}

// Unblock integer division for any non-`Quantity` type.
template <typename R>
AU_DEVICE_FUNC constexpr AlwaysDivisibleQuantity<UnitProduct<>, R> unblock_int_div(R x) {
    return AlwaysDivisibleQuantity<UnitProduct<>, R>{make_quantity<UnitProduct<>>(x)};
}

template <typename U, typename R>
class AlwaysDivisibleQuantity {
 public:
    // Divide a `Quantity` by this always-divisible quantity type.
    template <typename U2, typename R2>
    friend AU_DEVICE_FUNC constexpr auto operator/(Quantity<U2, R2> q2, AlwaysDivisibleQuantity q) {
        return make_quantity<UnitQuotient<U2, U>>(q2.in(U2{}) / q.q_.in(U{}));
    }

    // Divide any non-`Quantity` by this always-divisible quantity type.
    template <typename T>
    friend AU_DEVICE_FUNC constexpr auto operator/(T x, AlwaysDivisibleQuantity q) {
        return make_quantity<UnitInverse<U>>(x / q.q_.in(U{}));
    }

    template <typename UU, typename RR>
    friend AU_DEVICE_FUNC constexpr AlwaysDivisibleQuantity<UU, RR> unblock_int_div(
        Quantity<UU, RR> q);

    template <typename RR>
    friend AU_DEVICE_FUNC constexpr AlwaysDivisibleQuantity<UnitProduct<>, RR> unblock_int_div(
        RR x);

 private:
    AU_DEVICE_FUNC constexpr AlwaysDivisibleQuantity(Quantity<U, R> q) : q_{q} {}

    Quantity<U, R> q_;
};

// Perform division in the common unit of two inputs.
//
// When two quantities have the same dimension, this is what most people probably expect when
// dividing them.  When they have different dimension, the operation is undefined, and we'll get a
// compiler error.
template <typename U1, typename R1, typename U2, typename R2>
AU_DEVICE_FUNC constexpr auto divide_using_common_unit(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnit<U1, U2>;
    return q1.as(U{}) / q2.as(U{});
}

// The modulo operator (i.e., the remainder of an integer division).
//
// Only defined whenever (R1{} % R2{}) is defined (i.e., for integral Reps), _and_
// `CommonUnit<U1, U2>` is also defined.  We convert to that common unit to perform the operation.
template <typename U1, typename R1, typename U2, typename R2>
AU_DEVICE_FUNC constexpr auto operator%(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnit<U1, U2>;
    return make_quantity<U>(q1.in(U{}) % q2.in(U{}));
}

// Type trait to detect whether two Quantity types are equivalent.
//
// In this library, Quantity types are "equivalent" exactly when they use the same Rep, and are
// based on equivalent units.
template <typename U1, typename U2, typename R1, typename R2>
struct AreQuantityTypesEquivalent<Quantity<U1, R1>, Quantity<U2, R2>>
    : stdx::conjunction<std::is_same<R1, R2>, AreUnitsQuantityEquivalent<U1, U2>> {};

// Cast Quantity to a different underlying type.
template <typename NewRep, typename Unit, typename Rep>
AU_DEVICE_FUNC constexpr auto rep_cast(Quantity<Unit, Rep> q) {
    return q.template as<NewRep>(Unit{});
}

// Help Zero act more faithfully like a Quantity.
//
// Casting Zero to any "Rep" is trivial, because it has no Rep, and is already consistent with all.
template <typename NewRep>
AU_DEVICE_FUNC constexpr auto rep_cast(Zero z) {
    return z;
}

template <typename UnitT>
struct QuantityMaker {
    using Unit = UnitT;
    static constexpr auto unit = Unit{};

    // lvalue: copy. (See `make_quantity` above.)
    template <typename T>
    AU_DEVICE_FUNC constexpr Quantity<Unit, std::decay_t<T>> operator()(const T &value) const {
        return Quantity<Unit, std::decay_t<T>>{value};
    }

    // rvalue: move.
    template <typename T, typename = std::enable_if_t<!std::is_lvalue_reference<T>::value>>
    AU_DEVICE_FUNC constexpr Quantity<Unit, std::decay_t<T>> operator()(T &&value) const {
        return Quantity<Unit, std::decay_t<T>>{std::move(value)};
    }

    template <typename U, typename R>
    AU_DEVICE_FUNC constexpr void operator()(Quantity<U, R>) const {
        constexpr bool is_not_already_a_quantity = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_already_a_quantity, "Input to QuantityMaker is already a Quantity");
    }

    template <typename U, typename R>
    AU_DEVICE_FUNC constexpr void operator()(QuantityPoint<U, R>) const {
        constexpr bool is_not_a_quantity_point = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_a_quantity_point, "Input to QuantityMaker is a QuantityPoint");
    }

    template <typename... BPs>
    AU_DEVICE_FUNC constexpr auto operator*(Magnitude<BPs...> m) const {
        return QuantityMaker<decltype(unit * m)>{};
    }

    template <typename... BPs>
    AU_DEVICE_FUNC constexpr auto operator/(Magnitude<BPs...> m) const {
        return QuantityMaker<decltype(unit / m)>{};
    }

    template <typename DivisorUnit>
    AU_DEVICE_FUNC constexpr auto operator/(SingularNameFor<DivisorUnit>) const {
        return QuantityMaker<UnitQuotient<Unit, DivisorUnit>>{};
    }

    template <typename MultiplierUnit>
    friend AU_DEVICE_FUNC constexpr auto operator*(SingularNameFor<MultiplierUnit>, QuantityMaker) {
        return QuantityMaker<UnitProduct<MultiplierUnit, Unit>>{};
    }

    template <typename OtherUnit>
    AU_DEVICE_FUNC constexpr auto operator*(QuantityMaker<OtherUnit>) const {
        return QuantityMaker<UnitProduct<Unit, OtherUnit>>{};
    }

    template <typename OtherUnit>
    AU_DEVICE_FUNC constexpr auto operator/(QuantityMaker<OtherUnit>) const {
        return QuantityMaker<UnitQuotient<Unit, OtherUnit>>{};
    }
};

template <typename U>
struct AssociatedUnitImpl<QuantityMaker<U>> : stdx::type_identity<U> {};
template <typename U>
struct AppropriateAssociatedUnitImpl<Quantity, U> : AssociatedUnitImpl<U> {};
template <typename... Us>
struct AppropriateCommonUnitImpl<Quantity, Us...> : ComputeCommonUnit<Us...> {};

template <int Exp, typename Unit>
AU_DEVICE_FUNC constexpr auto pow(QuantityMaker<Unit>) {
    return QuantityMaker<UnitPower<Unit, Exp>>{};
}

template <int N, typename Unit>
AU_DEVICE_FUNC constexpr auto root(QuantityMaker<Unit>) {
    return QuantityMaker<UnitPower<Unit, 1, N>>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Runtime conversion checkers

// Check conversion for overflow (implicit rep).
template <typename U, typename R, typename TargetUnitSlot>
AU_DEVICE_FUNC constexpr bool will_conversion_overflow(Quantity<U, R> q, TargetUnitSlot) {
    using Op = detail::ConversionForRepsAndFactor<detail::UseStaticCast,
                                                  R,
                                                  void,
                                                  UnitRatio<U, AssociatedUnit<TargetUnitSlot>>>;
    return detail::would_value_overflow<Op>(q.in(U{}));
}

// Check conversion for overflow (explicit rep).
template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
AU_DEVICE_FUNC constexpr bool will_conversion_overflow(Quantity<U, R> q, TargetUnitSlot) {
    using Op = detail::ConversionForRepsAndFactor<detail::UseStaticCast,
                                                  R,
                                                  TargetRep,
                                                  UnitRatio<U, AssociatedUnit<TargetUnitSlot>>>;
    return detail::would_value_overflow<Op>(q.in(U{}));
}

// Check conversion for truncation (implicit rep).
template <typename U, typename R, typename TargetUnitSlot>
AU_DEVICE_FUNC constexpr bool will_conversion_truncate(Quantity<U, R> q, TargetUnitSlot) {
    using Op = detail::ConversionForRepsAndFactor<detail::UseStaticCast,
                                                  R,
                                                  void,
                                                  UnitRatio<U, AssociatedUnit<TargetUnitSlot>>>;
    return detail::TruncationRiskFor<Op>::would_value_truncate(q.in(U{}));
}

// Check conversion for truncation (explicit rep).
template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
AU_DEVICE_FUNC constexpr bool will_conversion_truncate(Quantity<U, R> q, TargetUnitSlot) {
    using Op = detail::ConversionForRepsAndFactor<detail::UseStaticCast,
                                                  R,
                                                  TargetRep,
                                                  UnitRatio<U, AssociatedUnit<TargetUnitSlot>>>;
    return detail::TruncationRiskFor<Op>::would_value_truncate(q.in(U{}));
}

// Check for any lossiness in conversion (implicit rep).
template <typename U, typename R, typename TargetUnitSlot>
AU_DEVICE_FUNC constexpr bool is_conversion_lossy(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return will_conversion_truncate(q, target_unit) || will_conversion_overflow(q, target_unit);
}

// Check for any lossiness in conversion (new rep).
template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
AU_DEVICE_FUNC constexpr bool is_conversion_lossy(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return will_conversion_truncate<TargetRep>(q, target_unit) ||
           will_conversion_overflow<TargetRep>(q, target_unit);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Comparing and/or combining Quantities of different types.

namespace detail {
template <typename Op, typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr auto convert_and_compare(const Quantity<U1, R1> &q1,
                                                  const Quantity<U2, R2> &q2) {
    using U = CommonUnit<U1, U2>;
    using ComRep1 = detail::CommonTypeButPreserveIntSignedness<R1, R2>;
    using ComRep2 = detail::CommonTypeButPreserveIntSignedness<R2, R1>;
    return detail::SignAwareComparison<UnitSign<U>, Op>{}(
        q1.template in<ComRep1>(U{}, check_for(ALL_RISKS)),
        q2.template in<ComRep2>(U{}, check_for(ALL_RISKS)));
}
}  // namespace detail

// Comparison functions for compatible Quantity types.
template <typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr bool operator==(const Quantity<U1, R1> &q1, const Quantity<U2, R2> &q2) {
    return detail::convert_and_compare<detail::Equal>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr bool operator!=(const Quantity<U1, R1> &q1, const Quantity<U2, R2> &q2) {
    return detail::convert_and_compare<detail::NotEqual>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr bool operator<(const Quantity<U1, R1> &q1, const Quantity<U2, R2> &q2) {
    return detail::convert_and_compare<detail::Less>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr bool operator<=(const Quantity<U1, R1> &q1, const Quantity<U2, R2> &q2) {
    return detail::convert_and_compare<detail::LessEqual>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr bool operator>(const Quantity<U1, R1> &q1, const Quantity<U2, R2> &q2) {
    return detail::convert_and_compare<detail::Greater>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr bool operator>=(const Quantity<U1, R1> &q1, const Quantity<U2, R2> &q2) {
    return detail::convert_and_compare<detail::GreaterEqual>(q1, q2);
}

namespace detail {

//
// The explicit rep in which to *host* an operand's unit conversion, or `void` for implicit rep.
//
// When two Quantities with different units are combined, one or both operands must be scaled by
// some conversion magnitude `M`.  That scaling has to be carried out in some rep.  By default we
// use the operand's own rep `R` (signaled by `void` here), which preserves expression-template
// laziness for reps such as Eigen.  We relocate to a different "host" rep only when that host is a
// *safer* place to apply `M` -- e.g. wide enough to avoid integer overflow, or able to represent a
// non-integer `M` that `R` cannot.
//
// The decision is fundamentally per-operand and asymmetric: it depends on the operand's own rep
// `R`, the conversion magnitude `M` it undergoes, and the other operand's rep `OtherR`.  Today, we
// simply use `std::common_type`.  `M` is not consulted yet; it is part of the signature because a
// future, rep-agnostic overflow/truncation risk model will need it to compare "risk of applying `M`
// in `R`" against "risk of applying `M` in the host".
//
// Today we can only reliably reason about this for arithmetic reps: we relocate to
// `std::common_type_t<R, OtherR>` when it differs from `R` (the usual arithmetic conversions yield
// a type with more headroom, and make a non-integer `M` representable).  For non-arithmetic reps we
// lack both a risk model and a meaningful common rep, so we never relocate yet.
//
template <typename R,
          typename M,
          typename OtherR,
          bool BothArithmetic =
              stdx::conjunction<std::is_arithmetic<R>, std::is_arithmetic<OtherR>>::value>
struct ExplicitRepForImpl : stdx::type_identity<void> {};
template <typename R, typename M, typename OtherR>
struct ExplicitRepForImpl<R, M, OtherR, true>
    : std::conditional<std::is_same<std::common_type_t<R, OtherR>, R>::value,
                       void,
                       std::common_type_t<R, OtherR>> {};
template <typename R, typename M, typename OtherR>
using ExplicitRepFor = typename ExplicitRepForImpl<R, M, OtherR>::type;

//
// `ref_or_scaled_copy<OtherR>(target, q)` converts `q` to `target`, choosing how to host the
// conversion:
//   - `q.data_in(target)` (a reference) when units are quantity-equivalent (no conversion);
//   - `q.in<Host>(target)` some explicit "host" rep, `Host`, for cases where we determine that
//     implicit-rep is not good enough (e.g., it needlessly increases overflow or truncation risk).
//   - `q.in(target)` (implicit) otherwise.  Note that this pathway preserves expression templates
//     (laziness) in the case of libraries, such as Eigen, which use this approach.
//
// `OtherR` is the rep of the *other* operand in the enclosing operation; see `ExplicitRepFor`.
//

// Scaled conversion, given the already-decided host rep (`void` => convert implicitly).
template <typename Host, typename TargetUnit, typename U, typename R>
struct ScaledCopy {  // Host is a concrete rep: materialize into it.
    AU_DEVICE_FUNC constexpr auto operator()(const Quantity<U, R> &q) const {
        return q.template in<Host>(TargetUnit{}, check_for(ALL_RISKS));
    }
};
template <typename TargetUnit, typename U, typename R>
struct ScaledCopy<void, TargetUnit, U, R> {  // No host rep: convert implicitly (lazy).
    AU_DEVICE_FUNC constexpr auto operator()(const Quantity<U, R> &q) const {
        return q.in(TargetUnit{});
    }
};

// Top level: return a reference when no conversion is needed; otherwise, delegate to ScaledCopy.
template <typename OtherR,
          typename TargetUnit,
          typename U,
          typename R,
          bool IsUnitEquivalent = are_units_quantity_equivalent(TargetUnit{}, U{})>
struct RefOrScaledCopy {
    static_assert(IsUnitEquivalent,
                  "Primary template should only be instantiated when units are equivalent");
    AU_DEVICE_FUNC constexpr decltype(auto) operator()(const Quantity<U, R> &q) const {
        return q.data_in(TargetUnit{});
    }
};
template <typename OtherR, typename TargetUnit, typename U, typename R>
struct RefOrScaledCopy<OtherR, TargetUnit, U, R, false>
    : ScaledCopy<ExplicitRepFor<R, UnitRatio<U, TargetUnit>, OtherR>, TargetUnit, U, R> {};

template <typename OtherR, typename TargetUnit, typename U, typename R>
AU_DEVICE_FUNC constexpr decltype(auto) ref_or_scaled_copy(TargetUnit, const Quantity<U, R> &q) {
    return RefOrScaledCopy<OtherR, TargetUnit, U, R>{}(q);
}

}  // namespace detail

// Addition and subtraction functions for compatible Quantity types.
template <typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr auto operator+(const Quantity<U1, R1> &q1, const Quantity<U2, R2> &q2) {
    using U = CommonUnit<U1, U2>;
    return make_quantity<U>(detail::ref_or_scaled_copy<R2>(U{}, q1) +
                            detail::ref_or_scaled_copy<R1>(U{}, q2));
}
template <typename U1, typename U2, typename R1, typename R2>
AU_DEVICE_FUNC constexpr auto operator-(const Quantity<U1, R1> &q1, const Quantity<U2, R2> &q2) {
    using U = CommonUnit<U1, U2>;
    return make_quantity<U>(detail::ref_or_scaled_copy<R2>(U{}, q1) -
                            detail::ref_or_scaled_copy<R1>(U{}, q2));
}

// Mixed-type operations with a left-Quantity, and right-Quantity-equivalent.
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator+(Quantity<U, R> q1, QLike q2)
    -> decltype(q1 + as_quantity(q2)) {
    return q1 + as_quantity(q2);
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator-(Quantity<U, R> q1, QLike q2)
    -> decltype(q1 - as_quantity(q2)) {
    return q1 - as_quantity(q2);
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator==(Quantity<U, R> q1, QLike q2)
    -> decltype(q1 == as_quantity(q2)) {
    return q1 == as_quantity(q2);
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator!=(Quantity<U, R> q1, QLike q2)
    -> decltype(q1 != as_quantity(q2)) {
    return q1 != as_quantity(q2);
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator<(Quantity<U, R> q1, QLike q2)
    -> decltype(q1 < as_quantity(q2)) {
    return q1 < as_quantity(q2);
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator<=(Quantity<U, R> q1, QLike q2)
    -> decltype(q1 <= as_quantity(q2)) {
    return q1 <= as_quantity(q2);
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator>(Quantity<U, R> q1, QLike q2)
    -> decltype(q1 > as_quantity(q2)) {
    return q1 > as_quantity(q2);
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator>=(Quantity<U, R> q1, QLike q2)
    -> decltype(q1 >= as_quantity(q2)) {
    return q1 >= as_quantity(q2);
}

// Mixed-type operations with a left-Quantity-equivalent, and right-Quantity.
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator+(QLike q1, Quantity<U, R> q2)
    -> decltype(as_quantity(q1) + q2) {
    return as_quantity(q1) + q2;
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator-(QLike q1, Quantity<U, R> q2)
    -> decltype(as_quantity(q1) - q2) {
    return as_quantity(q1) - q2;
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator==(QLike q1, Quantity<U, R> q2)
    -> decltype(as_quantity(q1) == q2) {
    return as_quantity(q1) == q2;
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator!=(QLike q1, Quantity<U, R> q2)
    -> decltype(as_quantity(q1) != q2) {
    return as_quantity(q1) != q2;
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator<(QLike q1, Quantity<U, R> q2)
    -> decltype(as_quantity(q1) < q2) {
    return as_quantity(q1) < q2;
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator<=(QLike q1, Quantity<U, R> q2)
    -> decltype(as_quantity(q1) <= q2) {
    return as_quantity(q1) <= q2;
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator>(QLike q1, Quantity<U, R> q2)
    -> decltype(as_quantity(q1) > q2) {
    return as_quantity(q1) > q2;
}
template <typename U, typename R, typename QLike>
AU_DEVICE_FUNC constexpr auto operator>=(QLike q1, Quantity<U, R> q2)
    -> decltype(as_quantity(q1) >= q2) {
    return as_quantity(q1) >= q2;
}

namespace detail {
template <typename Op>
struct SignAwareComparison<Magnitude<>, Op> {
    template <typename T1, typename T2>
    AU_DEVICE_FUNC constexpr auto operator()(const T1 &lhs, const T2 &rhs) const {
        return Op{}(lhs, rhs);
    }
};

template <typename Op>
struct SignAwareComparison<Magnitude<Negative>, Op> {
    template <typename T1, typename T2>
    AU_DEVICE_FUNC constexpr auto operator()(const T1 &lhs, const T2 &rhs) const {
        return Op{}(rhs, lhs);
    }
};
}  // namespace detail

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
template <typename U1, typename R1, typename U2, typename R2>
AU_DEVICE_FUNC constexpr auto operator<=>(const Quantity<U1, R1> &lhs,
                                          const Quantity<U2, R2> &rhs) {
    return detail::convert_and_compare<detail::ThreeWayCompare>(lhs, rhs);
}
#endif

// Helper to compute the `std::common_type_t` of two `Quantity` types.
//
// `std::common_type` requires its specializations to be SFINAE-friendly, meaning that the `type`
// member should not exist for specializations with no common type.  Unfortunately, we can't
// directly use SFINAE on `std::common_type`.  What we can do is inherit our specialization's
// implementation from a different structure which we fully control, and which either has or doesn't
// have a `type` member as appropriate.
template <typename Q1, typename Q2, typename Enable = void>
struct CommonQuantity {};
template <typename U1, typename U2, typename R1, typename R2>
struct CommonQuantity<Quantity<U1, R1>,
                      Quantity<U2, R2>,
                      std::enable_if_t<HasSameDimension<U1, U2>::value>>
    : stdx::type_identity<Quantity<CommonUnit<U1, U2>, std::common_type_t<R1, R2>>> {};

//
// Formatter implementation for fmtlib or `std::format`.
//
// To use with fmtlib, add this template specialization to a file that includes both
// `"au/quantity.hh"`, and `"fmt/format.h"`:
//
//    namespace fmt {
//    template <typename U, typename R>
//    struct formatter<::au::Quantity<U, R>> : ::au::QuantityFormatter<U, R, ::fmt::formatter> {};
//    }  // namespace fmt
//
// Then, include that file any time you want to format a `Quantity`.
//
template <typename U, typename R, template <class...> class Formatter>
struct QuantityFormatter {
    template <typename FormatParseContext>
    constexpr auto parse_unit_label_part(FormatParseContext &ctx) {
        auto it = ctx.begin();

        if (it == ctx.end()) {
            return it;
        }

        if (*it != 'U') {
            return it;
        }
        // Consume the 'U'.
        ++it;

        // Parse the total width.
        while (it != ctx.end() && *it >= '0' && *it <= '9') {
            min_label_width_ = (min_label_width_ * 10) + static_cast<std::size_t>(*it++ - '0');
        }

        if (it == ctx.end() || *it == '}') {
            return it;
        }

        if (*it++ != ';') {
            // Cause an error condition in further parsing.
            it = ctx.end();
        }
        return it;
    }

    template <typename FormatParseContext>
    constexpr auto parse(FormatParseContext &ctx) {
        ctx.advance_to(parse_unit_label_part(ctx));
        return value_format.parse(ctx);
    }

    template <typename FormatContext>
    constexpr auto format(const au::Quantity<U, R> &q, FormatContext &ctx) const {
        value_format.format(q.data_in(U{}), ctx);
        Formatter<const char *>{}.format(" ", ctx);
        return write_and_pad(unit_label(U{}), sizeof(unit_label(U{})), ctx);
    }

    template <typename FormatContext>
    constexpr auto write_and_pad(const char *data,
                                 std::size_t data_size,
                                 FormatContext &ctx,
                                 char suffix = '\0') const {
        Formatter<const char *> unit_label_formatter{};
        unit_label_formatter.format(data, ctx);
        auto out = ctx.out();
        while (data_size <= min_label_width_) {
            *out++ = ' ';
            ++data_size;
        }
        if (suffix != '\0') {
            *out++ = suffix;
        }
        return out;
    }

    Formatter<R> value_format{};
    std::size_t min_label_width_{0};
};

}  // namespace au

namespace std {
// Note: we would prefer not to reopen `namespace std` [1].  However, some older compilers (which we
// still want to support) incorrectly treat the preferred syntax recommended in [1] as an error.
// This usage does not encounter any of the pitfalls described in that link, so we use it.
//
// [1] https://quuxplusone.github.io/blog/2021/10/27/dont-reopen-namespace-std/
template <typename U1, typename U2, typename R1, typename R2>
struct common_type<au::Quantity<U1, R1>, au::Quantity<U2, R2>>
    : au::CommonQuantity<au::Quantity<U1, R1>, au::Quantity<U2, R2>> {};
}  // namespace std
