// Copyright 2023 Aurora Operations, Inc.
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

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
#include <compare>
#endif

#include "au/fwd.hh"
#include "au/quantity.hh"
#include "au/stdx/type_traits.hh"
#include "au/unit_of_measure.hh"
#include "au/wrapper_operations.hh"

namespace au {

//
// A monovalue type to represent a constant value, including its units, if any.
//
// Users can multiply or divide `Constant` instances by raw numbers or `Quantity` instances, and it
// will perform symbolic arithmetic at compile time without affecting the stored numeric value.
// `Constant` also composes with other constants, and with `QuantityMaker` and other related types.
//
// Although `Constant` does not have any specific numeric type associated with it (as opposed to
// `Quantity`), it can easily convert to any appropriate `Quantity` type, with any rep.  Unlike
// `Quantity`, these conversions support _exact_ safety checks, so that every conversion producing a
// correctly representable value will succeed, and every unrepresentable conversion will fail.
//
template <typename Unit>
struct Constant : detail::MakesQuantityFromNumber<Constant, Unit>,
                  detail::ScalesQuantity<Constant, Unit>,
                  detail::ComposesWith<Constant, Unit, Constant, Constant>,
                  detail::ComposesWith<Constant, Unit, QuantityMaker, QuantityMaker>,
                  detail::ComposesWith<Constant, Unit, SingularNameFor, SingularNameFor>,
                  detail::SupportsRationalPowers<Constant, Unit>,
                  detail::CanScaleByMagnitude<Constant, Unit> {
    // Convert this constant to a Quantity of the given rep.
    template <typename T>
    constexpr auto as() const {
        return make_quantity<Unit>(static_cast<T>(1));
    }

    // Convert this constant to a Quantity of the given unit and rep, ignoring safety checks.
    template <typename T, typename OtherUnit>
    constexpr auto coerce_as(OtherUnit u) const {
        return as<T>().coerce_as(u);
    }

    // Convert this constant to a Quantity of the given unit and rep.
    template <typename T, typename OtherUnit>
    constexpr auto as(OtherUnit u) const {
        return as<T>(u, check_for(ALL_RISKS));
    }

    // Convert this constant to a Quantity of the given unit and rep, following this risk policy.
    template <typename T, typename OtherUnit, typename RiskPolicyT>
    constexpr auto as(OtherUnit, RiskPolicyT) const {
        constexpr auto this_value = make_quantity<Unit>(static_cast<T>(1));

        constexpr bool has_unacceptable_overflow =
            RiskPolicyT{}.should_check(detail::ConversionRisk::Overflow) &&
            will_conversion_overflow(this_value, OtherUnit{});
        static_assert(!has_unacceptable_overflow, "Constant conversion known to overflow");

        constexpr bool has_unacceptable_truncation =
            RiskPolicyT{}.should_check(detail::ConversionRisk::Truncation) &&
            will_conversion_truncate(this_value, OtherUnit{});
        static_assert(!has_unacceptable_truncation, "Constant conversion known to truncate");

        return this_value.as(OtherUnit{}, ignore(ALL_RISKS));
    }

    // Get the value of this constant in the given unit and rep, ignoring safety checks.
    template <typename T, typename OtherUnit>
    constexpr auto coerce_in(OtherUnit u) const {
        return as<T>().coerce_in(u);
    }

    // Get the value of this constant in the given unit and rep.
    template <typename T, typename OtherUnit>
    constexpr auto in(OtherUnit u) const {
        return in<T>(u, check_for(ALL_RISKS));
    }

    // Get the value of this constant in the given unit and rep, following this risk policy.
    template <typename T, typename OtherUnit, typename RiskPolicyT>
    constexpr auto in(OtherUnit u, RiskPolicyT policy) const {
        return as<T>(u, policy).in(u);
    }

    // Implicitly convert to any quantity type which passes safety checks.
    template <typename U, typename R>
    constexpr operator Quantity<U, R>() const {
        return as<R>(U{});
    }

    // Static function to check whether this constant can be exactly-represented in the given rep
    // `T` and unit `OtherUnit`.
    template <typename T, typename OtherUnit>
    static constexpr bool can_store_value_in(OtherUnit other) {
        return representable_in<T>(unit_ratio(Unit{}, other));
    }

    // Implicitly convert to type with an exactly corresponding quantity that passes safety checks.
    template <
        typename T,
        typename = std::enable_if_t<can_store_value_in<typename CorrespondingQuantity<T>::Rep>(
            typename CorrespondingQuantity<T>::Unit{})>>
    constexpr operator T() const {
        return as<typename CorrespondingQuantity<T>::Rep>(
            typename CorrespondingQuantity<T>::Unit{});
    }
};

// Make a constant from the given unit.
//
// Note that the argument is a _unit slot_, and thus can also accept things like `QuantityMaker` and
// `SymbolFor` in addition to regular units.
template <typename UnitSlot>
constexpr Constant<AssociatedUnit<UnitSlot>> make_constant(UnitSlot) {
    return {};
}

constexpr Zero make_constant(Zero) { return {}; }

// Support using `Constant` in a unit slot.
template <typename Unit>
struct AssociatedUnitImpl<Constant<Unit>> : stdx::type_identity<Unit> {};

// Relational operators.
//
// Note that these inherit the limitations of the Magnitude comparisons: they will not work for
// every combination of Constant.  We decided that supporting many common use cases was worth this
// tradeoff.
template <typename U1, typename U2>
constexpr bool operator==(Constant<U1>, Constant<U2>) {
    return UnitRatio<U1, U2>{} == mag<1>();
}
template <typename U1, typename U2>
constexpr bool operator<(Constant<U1>, Constant<U2>) {
    using SignU2 = Sign<detail::MagT<U2>>;
    using AbsU2 = decltype(U2{} * SignU2{});
    return UnitRatio<U1, AbsU2>{} < SignU2{};
}
template <typename U1, typename U2>
constexpr bool operator!=(Constant<U1> lhs, Constant<U2> rhs) {
    return !(lhs == rhs);
}
template <typename U1, typename U2>
constexpr bool operator<=(Constant<U1> lhs, Constant<U2> rhs) {
    return (lhs < rhs) || (lhs == rhs);
}
template <typename U1, typename U2>
constexpr bool operator>(Constant<U1> lhs, Constant<U2> rhs) {
    return !(lhs <= rhs);
}
template <typename U1, typename U2>
constexpr bool operator>=(Constant<U1> lhs, Constant<U2> rhs) {
    return !(lhs < rhs);
}

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
template <typename U1, typename U2>
constexpr std::strong_ordering operator<=>(Constant<U1>, Constant<U2>) {
    using SignU2 = Sign<detail::MagT<U2>>;
    using AbsU2 = decltype(U2{} * SignU2{});
    return UnitRatio<U1, AbsU2>{} <=> SignU2{};
}
#endif

}  // namespace au
