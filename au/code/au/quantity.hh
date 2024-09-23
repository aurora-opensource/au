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

#include <utility>

#include "au/apply_magnitude.hh"
#include "au/conversion_policy.hh"
#include "au/operators.hh"
#include "au/rep.hh"
#include "au/stdx/functional.hh"
#include "au/unit_of_measure.hh"
#include "au/utility/type_traits.hh"
#include "au/zero.hh"

namespace au {

template <typename UnitT>
struct QuantityMaker;

template <typename UnitT, typename RepT>
class Quantity;

//
// Make a Quantity of the given Unit, which has this value as measured in the Unit.
//
template <typename UnitT, typename T>
constexpr auto make_quantity(T value) {
    return QuantityMaker<UnitT>{}(value);
}

template <typename Unit, typename T>
constexpr auto make_quantity_unless_unitless(T value) {
    return std::conditional_t<IsUnitlessUnit<Unit>::value, stdx::identity, QuantityMaker<Unit>>{}(
        value);
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
template <typename T>
using CorrespondingQuantityT =
    Quantity<typename CorrespondingQuantity<T>::Unit, typename CorrespondingQuantity<T>::Rep>;

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
constexpr auto as_quantity(T &&x) -> CorrespondingQuantityT<T> {
    using Q = CorrespondingQuantity<T>;
    static_assert(IsUnit<typename Q::Unit>{}, "No Quantity corresponding to type");

    auto value = Q::extract_value(std::forward<T>(x));
    static_assert(std::is_same<decltype(value), typename Q::Rep>{},
                  "Inconsistent CorrespondingQuantity implementation");

    return make_quantity<typename Q::Unit>(value);
}

template <typename UnitT, typename RepT>
class Quantity {
    template <bool ImplicitOk, typename OtherUnit, typename OtherRep>
    using EnableIfImplicitOkIs = std::enable_if_t<
        ImplicitOk ==
        ConstructionPolicy<UnitT, RepT>::template PermitImplicitFrom<OtherUnit, OtherRep>::value>;

 public:
    using Rep = RepT;
    using Unit = UnitT;
    static constexpr auto unit = Unit{};

    static_assert(IsValidRep<Rep>::value, "Rep must meet our requirements for a rep");

    // IMPLICIT constructor for another Quantity of the same Dimension.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>>
    constexpr Quantity(Quantity<OtherUnit, OtherRep> other)  // NOLINT(runtime/explicit)
        : Quantity{other.template as<Rep>(UnitT{})} {}

    // EXPLICIT constructor for another Quantity of the same Dimension.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<false, OtherUnit, OtherRep>,
              typename ThisUnusedTemplateParameterDistinguishesUsFromTheAboveConstructor = void>
    // Deleted: use `.as<NewRep>(new_unit)` to force a cast.
    explicit constexpr Quantity(Quantity<OtherUnit, OtherRep> other) = delete;

    // Construct this Quantity with a value of exactly Zero.
    constexpr Quantity(Zero) : value_{0} {}

    constexpr Quantity() noexcept = default;

    // Implicit construction from any exactly-equivalent type.
    template <
        typename T,
        std::enable_if_t<std::is_convertible<CorrespondingQuantityT<T>, Quantity>::value, int> = 0>
    constexpr Quantity(T &&x) : Quantity{as_quantity(std::forward<T>(x))} {}

    template <typename NewRep,
              typename NewUnit,
              typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
    constexpr auto as(NewUnit) const {
        using Common = std::common_type_t<Rep, NewRep>;
        using Factor = UnitRatioT<AssociatedUnitT<Unit>, AssociatedUnitT<NewUnit>>;

        return make_quantity<AssociatedUnitT<NewUnit>>(
            static_cast<NewRep>(detail::apply_magnitude(static_cast<Common>(value_), Factor{})));
    }

    template <typename NewUnit,
              typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
    constexpr auto as(NewUnit u) const {
        constexpr bool IMPLICIT_OK =
            implicit_rep_permitted_from_source_to_target<Rep>(unit, NewUnit{});
        constexpr bool INTEGRAL_REP = std::is_integral<Rep>::value;
        static_assert(
            IMPLICIT_OK || INTEGRAL_REP,
            "Should never occur.  In the following static_assert, we assume that IMPLICIT_OK "
            "can never fail unless INTEGRAL_REP is true.");
        static_assert(
            IMPLICIT_OK,
            "Dangerous conversion for integer Rep!  See: "
            "https://aurora-opensource.github.io/au/main/troubleshooting/#dangerous-conversion");
        return as<Rep>(u);
    }

    template <typename NewRep,
              typename NewUnit,
              typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
    constexpr NewRep in(NewUnit u) const {
        if (are_units_quantity_equivalent(unit, u) && std::is_same<Rep, NewRep>::value) {
            return static_cast<NewRep>(value_);
        } else {
            return as<NewRep>(u).in(u);
        }
    }

    template <typename NewUnit,
              typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
    constexpr Rep in(NewUnit u) const {
        if (are_units_quantity_equivalent(unit, u)) {
            return value_;
        } else {
            // Since Rep was requested _implicitly_, delegate to `.as()` for its safety checks.
            return as(u).in(u);
        }
    }

    // "Old-style" overloads with <U, R> template parameters, and no function parameters.
    //
    // Matches the syntax from the CppCon 2021 talk, and legacy Aurora usage.
    template <typename U>
    [[deprecated(
        "Do not write `.as<YourUnits>()`; write `.as(your_units)` instead.")]] constexpr auto
    as() const -> decltype(as(U{})) {
        return as(U{});
    }
    template <typename U, typename R, typename = std::enable_if_t<IsUnit<U>::value>>
    [[deprecated(
        "Do not write `.as<YourUnits, T>()`; write `.as<T>(your_units)` instead.")]] constexpr auto
    as() const {
        return as<R>(U{});
    }
    template <typename U>
    [[deprecated(
        "Do not write `.in<YourUnits>()`; write `.in(your_units)` instead.")]] constexpr auto
    in() const -> decltype(in(U{})) {
        return in(U{});
    }
    template <typename U, typename R, typename = std::enable_if_t<IsUnit<U>::value>>
    [[deprecated(
        "Do not write `.in<YourUnits, T>()`; write `.in<T>(your_units)` instead.")]] constexpr auto
    in() const {
        return in<R>(U{});
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
    // Mutable access, QuantityMaker input.
    template <typename U>
    Rep &data_in(const QuantityMaker<U> &) {
        static_assert(AreUnitsQuantityEquivalent<U, Unit>::value,
                      "Can only access value via Quantity-equivalent unit");
        return value_;
    }
    // Mutable access, Unit input.
    template <typename U>
    Rep &data_in(const U &) {
        return data_in(QuantityMaker<U>{});
    }
    // Const access, QuantityMaker input.
    template <typename U>
    const Rep &data_in(const QuantityMaker<U> &) const {
        static_assert(AreUnitsQuantityEquivalent<U, Unit>::value,
                      "Can only access value via Quantity-equivalent unit");
        return value_;
    }
    // Const access, Unit input.
    template <typename U>
    const Rep &data_in(const U &) const {
        return data_in(QuantityMaker<U>{});
    }

    // Permit this factory functor to access our private constructor.
    //
    // We allow this because it explicitly names the unit at the callsite, even if people refer to
    // this present Quantity type by an alias that omits the unit.  This preserves Unit Safety and
    // promotes callsite readability.
    friend struct QuantityMaker<UnitT>;

    // Comparison operators.
    friend constexpr bool operator==(Quantity a, Quantity b) { return a.value_ == b.value_; }
    friend constexpr bool operator!=(Quantity a, Quantity b) { return a.value_ != b.value_; }
    friend constexpr bool operator<(Quantity a, Quantity b) { return a.value_ < b.value_; }
    friend constexpr bool operator<=(Quantity a, Quantity b) { return a.value_ <= b.value_; }
    friend constexpr bool operator>(Quantity a, Quantity b) { return a.value_ > b.value_; }
    friend constexpr bool operator>=(Quantity a, Quantity b) { return a.value_ >= b.value_; }

    // Addition and subtraction for like quantities.
    friend constexpr Quantity<UnitT, decltype(std::declval<RepT>() + std::declval<RepT>())>
    operator+(Quantity a, Quantity b) {
        return make_quantity<UnitT>(a.value_ + b.value_);
    }
    friend constexpr Quantity<UnitT, decltype(std::declval<RepT>() - std::declval<RepT>())>
    operator-(Quantity a, Quantity b) {
        return make_quantity<UnitT>(a.value_ - b.value_);
    }

    // Scalar multiplication.
    template <typename T, typename = std::enable_if_t<IsProductValidRep<RepT, T>::value>>
    friend constexpr auto operator*(Quantity a, T s) {
        return make_quantity<UnitT>(a.value_ * s);
    }
    template <typename T, typename = std::enable_if_t<IsProductValidRep<T, RepT>::value>>
    friend constexpr auto operator*(T s, Quantity a) {
        return make_quantity<UnitT>(s * a.value_);
    }

    // Scalar division.
    template <typename T, typename = std::enable_if_t<IsQuotientValidRep<RepT, T>::value>>
    friend constexpr auto operator/(Quantity a, T s) {
        return make_quantity<UnitT>(a.value_ / s);
    }
    template <typename T, typename = std::enable_if_t<IsQuotientValidRep<T, RepT>::value>>
    friend constexpr auto operator/(T s, Quantity a) {
        warn_if_integer_division<UnitProductT<>, T>();
        return make_quantity<decltype(pow<-1>(unit))>(s / a.value_);
    }

    // Multiplication for dimensioned quantities.
    template <typename OtherUnit, typename OtherRep>
    constexpr auto operator*(Quantity<OtherUnit, OtherRep> q) const {
        return make_quantity_unless_unitless<UnitProductT<Unit, OtherUnit>>(value_ *
                                                                            q.in(OtherUnit{}));
    }

    // Division for dimensioned quantities.
    template <typename OtherUnit, typename OtherRep>
    constexpr auto operator/(Quantity<OtherUnit, OtherRep> q) const {
        warn_if_integer_division<OtherUnit, OtherRep>();
        return make_quantity_unless_unitless<UnitQuotientT<Unit, OtherUnit>>(value_ /
                                                                             q.in(OtherUnit{}));
    }

    // Short-hand addition and subtraction assignment.
    constexpr Quantity &operator+=(Quantity other) {
        value_ += other.value_;
        return *this;
    }
    constexpr Quantity &operator-=(Quantity other) {
        value_ -= other.value_;
        return *this;
    }

    template <typename T>
    constexpr void perform_shorthand_checks() {
        static_assert(
            IsValidRep<T>::value,
            "This overload is only for scalar mult/div-assignment with raw numeric types");

        static_assert((!std::is_integral<detail::RealPart<Rep>>::value) ||
                          std::is_integral<detail::RealPart<T>>::value,
                      "We don't support compound mult/div of integral types by floating point");
    }

    // Short-hand multiplication assignment.
    template <typename T>
    constexpr Quantity &operator*=(T s) {
        perform_shorthand_checks<T>();

        value_ *= s;
        return *this;
    }

    // Short-hand division assignment.
    template <typename T>
    constexpr Quantity &operator/=(T s) {
        perform_shorthand_checks<T>();

        value_ /= s;
        return *this;
    }

    // Unary plus and minus.
    constexpr Quantity operator+() const { return {+value_}; }
    constexpr Quantity operator-() const { return {-value_}; }

    // Automatic conversion to Rep for Unitless type.
    template <typename U = UnitT, typename = std::enable_if_t<IsUnitlessUnit<U>::value>>
    constexpr operator Rep() const {
        return value_;
    }

    // Automatic conversion to any equivalent type that supports it.
    template <
        typename T,
        std::enable_if_t<std::is_convertible<Quantity, CorrespondingQuantityT<T>>::value, int> = 0>
    constexpr operator T() const {
        return CorrespondingQuantity<T>::construct_from_value(
            CorrespondingQuantityT<T>{*this}.in(typename CorrespondingQuantity<T>::Unit{}));
    }

 private:
    template <typename OtherUnit, typename OtherRep>
    static constexpr void warn_if_integer_division() {
        constexpr bool uses_integer_division =
            (std::is_integral<Rep>::value && std::is_integral<OtherRep>::value);
        constexpr bool are_units_quantity_equivalent =
            AreUnitsQuantityEquivalent<UnitT, OtherUnit>::value;
        static_assert(are_units_quantity_equivalent || !uses_integer_division,
                      "Integer division forbidden: use integer_quotient() if you really want it");
    }

    constexpr Quantity(Rep value) : value_{value} {}

    Rep value_{0};
};

// Force integer division beteween two integer Quantities, in a callsite-obvious way.
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto integer_quotient(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    static_assert(std::is_integral<R1>::value && std::is_integral<R2>::value,
                  "integer_quotient() can only be called with integral Rep");
    return make_quantity<UnitQuotientT<U1, U2>>(q1.in(U1{}) / q2.in(U2{}));
}

// Force integer division beteween an integer Quantity and a raw number.
template <typename U, typename R, typename T>
constexpr auto integer_quotient(Quantity<U, R> q, T x) {
    static_assert(std::is_integral<R>::value && std::is_integral<T>::value,
                  "integer_quotient() can only be called with integral Rep");
    return make_quantity<U>(q.in(U{}) / x);
}

// Force integer division beteween a raw number and an integer Quantity.
template <typename T, typename U, typename R>
constexpr auto integer_quotient(T x, Quantity<U, R> q) {
    static_assert(std::is_integral<T>::value && std::is_integral<R>::value,
                  "integer_quotient() can only be called with integral Rep");
    return make_quantity<UnitInverseT<U>>(x / q.in(U{}));
}

// The modulo operator (i.e., the remainder of an integer division).
//
// Only defined whenever (R1{} % R2{}) is defined (i.e., for integral Reps), _and_
// `CommonUnitT<U1, U2>` is also defined.  We convert to that common unit to perform the operation.
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto operator%(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
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
constexpr auto rep_cast(Quantity<Unit, Rep> q) {
    return q.template as<NewRep>(Unit{});
}

// Help Zero act more faithfully like a Quantity.
//
// Casting Zero to any "Rep" is trivial, because it has no Rep, and is already consistent with all.
template <typename NewRep>
constexpr auto rep_cast(Zero z) {
    return z;
}

//
// Quantity aliases to set a particular Rep.
//
// This presents a less cumbersome interface for end users.
//
template <typename UnitT>
using QuantityD = Quantity<UnitT, double>;
template <typename UnitT>
using QuantityF = Quantity<UnitT, float>;
template <typename UnitT>
using QuantityI = Quantity<UnitT, int>;
template <typename UnitT>
using QuantityU = Quantity<UnitT, unsigned int>;
template <typename UnitT>
using QuantityI32 = Quantity<UnitT, int32_t>;
template <typename UnitT>
using QuantityU32 = Quantity<UnitT, uint32_t>;
template <typename UnitT>
using QuantityI64 = Quantity<UnitT, int64_t>;
template <typename UnitT>
using QuantityU64 = Quantity<UnitT, uint64_t>;

// Forward declare `QuantityPoint` here, so that we can give better error messages when users try to
// make it into a quantity.
template <typename U, typename R>
class QuantityPoint;

template <typename UnitT>
struct QuantityMaker {
    using Unit = UnitT;
    static constexpr auto unit = Unit{};

    template <typename T>
    constexpr Quantity<Unit, T> operator()(T value) const {
        return {value};
    }

    template <typename U, typename R>
    constexpr void operator()(Quantity<U, R>) const {
        constexpr bool is_not_already_a_quantity = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_already_a_quantity, "Input to QuantityMaker is already a Quantity");
    }

    template <typename U, typename R>
    constexpr void operator()(QuantityPoint<U, R>) const {
        constexpr bool is_not_a_quantity_point = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_a_quantity_point, "Input to QuantityMaker is a QuantityPoint");
    }

    template <typename... BPs>
    constexpr auto operator*(Magnitude<BPs...> m) const {
        return QuantityMaker<decltype(unit * m)>{};
    }

    template <typename... BPs>
    constexpr auto operator/(Magnitude<BPs...> m) const {
        return QuantityMaker<decltype(unit / m)>{};
    }

    template <typename DivisorUnit>
    constexpr auto operator/(SingularNameFor<DivisorUnit>) const {
        return QuantityMaker<UnitQuotientT<Unit, DivisorUnit>>{};
    }

    template <typename MultiplierUnit>
    friend constexpr auto operator*(SingularNameFor<MultiplierUnit>, QuantityMaker) {
        return QuantityMaker<UnitProductT<MultiplierUnit, Unit>>{};
    }

    template <typename OtherUnit>
    constexpr auto operator*(QuantityMaker<OtherUnit>) const {
        return QuantityMaker<UnitProductT<Unit, OtherUnit>>{};
    }

    template <typename OtherUnit>
    constexpr auto operator/(QuantityMaker<OtherUnit>) const {
        return QuantityMaker<UnitQuotientT<Unit, OtherUnit>>{};
    }
};

template <typename U>
struct AssociatedUnit<QuantityMaker<U>> : stdx::type_identity<U> {};

template <int Exp, typename Unit>
constexpr auto pow(QuantityMaker<Unit>) {
    return QuantityMaker<UnitPowerT<Unit, Exp>>{};
}

template <int N, typename Unit>
constexpr auto root(QuantityMaker<Unit>) {
    return QuantityMaker<UnitPowerT<Unit, 1, N>>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Runtime conversion checkers

// Check conversion for overflow (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool will_conversion_overflow(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return detail::ApplyMagnitudeT<R, decltype(unit_ratio(U{}, target_unit))>::would_overflow(
        q.in(U{}));
}

// Check conversion for truncation (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool will_conversion_truncate(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return detail::ApplyMagnitudeT<R, decltype(unit_ratio(U{}, target_unit))>::would_truncate(
        q.in(U{}));
}

// Check for any lossiness in conversion (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool is_conversion_lossy(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return will_conversion_truncate(q, target_unit) || will_conversion_overflow(q, target_unit);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Comparing and/or combining Quantities of different types.

namespace detail {
// Helper to cast this Quantity to its common type with some other Quantity (explicitly supplied).
//
// Note that `TargetUnit` is supposed to be the common type of the input Quantity and some other
// Quantity.  This function should never be called directly; it should only be called by
// `using_common_type()`.  The program behaviour is undefined if anyone calls this function
// directly.  (In particular, we explicitly assume that the conversion to the Rep of TargetUnit is
// not narrowing for the input Quantity.)
//
// We would have liked this to just be a simple lambda, but some old compilers sometimes struggle
// with understanding that the lambda implementation of this can be constexpr.
template <typename TargetUnit, typename U, typename R>
constexpr auto cast_to_common_type(Quantity<U, R> q) {
    // When we perform a unit conversion to U, we need to make sure the library permits this
    // conversion *implicitly* for a rep R.  The form `rep_cast<R>(q).as(U{})` achieves
    // this.  First, we cast the Rep to R (which will typically be the wider of the input Reps).
    // Then, we use the *unit-only* form of the conversion operator: `as(U{})`, not
    // `as<R>(U{})`, because only the former actually checks the conversion policy.
    return rep_cast<typename TargetUnit::Rep>(q).as(TargetUnit::unit);
}

template <typename T, typename U, typename Func>
constexpr auto using_common_type(T t, U u, Func f) {
    using C = std::common_type_t<T, U>;
    static_assert(
        std::is_same<typename C::Rep, std::common_type_t<typename T::Rep, typename U::Rep>>::value,
        "Rep of common type is not common type of Reps (this should never occur)");

    return f(cast_to_common_type<C>(t), cast_to_common_type<C>(u));
}
}  // namespace detail

// Comparison functions for compatible Quantity types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator==(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator!=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::not_equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator<(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::less);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator<=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::less_equal);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator>(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::greater);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator>=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::greater_equal);
}

// Addition and subtraction functions for compatible Quantity types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator+(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::plus);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator-(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::minus);
}

// Mixed-type operations with a left-Quantity, and right-Quantity-equivalent.
template <typename U, typename R, typename QLike>
constexpr auto operator+(Quantity<U, R> q1, QLike q2) -> decltype(q1 + as_quantity(q2)) {
    return q1 + as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator-(Quantity<U, R> q1, QLike q2) -> decltype(q1 - as_quantity(q2)) {
    return q1 - as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator==(Quantity<U, R> q1, QLike q2) -> decltype(q1 == as_quantity(q2)) {
    return q1 == as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator!=(Quantity<U, R> q1, QLike q2) -> decltype(q1 != as_quantity(q2)) {
    return q1 != as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator<(Quantity<U, R> q1, QLike q2) -> decltype(q1 < as_quantity(q2)) {
    return q1 < as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator<=(Quantity<U, R> q1, QLike q2) -> decltype(q1 <= as_quantity(q2)) {
    return q1 <= as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator>(Quantity<U, R> q1, QLike q2) -> decltype(q1 > as_quantity(q2)) {
    return q1 > as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator>=(Quantity<U, R> q1, QLike q2) -> decltype(q1 >= as_quantity(q2)) {
    return q1 >= as_quantity(q2);
}

// Mixed-type operations with a left-Quantity-equivalent, and right-Quantity.
template <typename U, typename R, typename QLike>
constexpr auto operator+(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) + q2) {
    return as_quantity(q1) + q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator-(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) - q2) {
    return as_quantity(q1) - q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator==(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) == q2) {
    return as_quantity(q1) == q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator!=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) != q2) {
    return as_quantity(q1) != q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator<(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) < q2) {
    return as_quantity(q1) < q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator<=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) <= q2) {
    return as_quantity(q1) <= q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator>(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) > q2) {
    return as_quantity(q1) > q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator>=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) >= q2) {
    return as_quantity(q1) >= q2;
}

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
    : stdx::type_identity<Quantity<CommonUnitT<U1, U2>, std::common_type_t<R1, R2>>> {};
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
