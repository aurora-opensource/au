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

#include <limits>

#include "au/magnitude.hh"
#include "au/stdx/type_traits.hh"
#include "au/stdx/utility.hh"
#include "au/unit_of_measure.hh"

namespace au {

// Check that this particular Magnitude won't cause this specific value to overflow its type.
template <typename Rep, typename... BPs>
constexpr bool can_scale_without_overflow(Magnitude<BPs...> m, Rep value) {
    // Scales that shrink don't cause overflow.
    constexpr bool mag_cannot_increase_values = get_value<double>(abs(m)) <= 1.0;
    return mag_cannot_increase_values ||
           (std::numeric_limits<Rep>::max() / get_value<Rep>(abs(m)) >= value);
}

namespace auimpl {
// Chosen so as to allow populating a `QuantityI32<Hertz>` with an input in MHz.
constexpr auto OVERFLOW_THRESHOLD = 2'147;

// This wrapper for `can_scale_without_overflow<...>(..., OVERFLOW_THRESHOLD)` can prevent an
// instantiation via short-circuiting, speeding up compile times.
template <typename Rep, typename ScaleFactor>
struct CanScaleThresholdWithoutOverflow
    : stdx::conjunction<
          stdx::bool_constant<stdx::in_range<Rep>(OVERFLOW_THRESHOLD)>,
          stdx::bool_constant<can_scale_without_overflow<Rep>(ScaleFactor{}, OVERFLOW_THRESHOLD)>> {
};

template <typename U1, typename U2>
struct SameDimension : stdx::bool_constant<U1::dim_ == U2::dim_> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct CoreImplicitConversionPolicyImplAssumingReal
    : stdx::disjunction<
          std::is_floating_point<Rep>,
          stdx::conjunction<std::is_integral<SourceRep>,
                            IsInteger<ScaleFactor>,
                            auimpl::CanScaleThresholdWithoutOverflow<Rep, ScaleFactor>>> {};

// Always permit the identity scaling.
template <typename Rep>
struct CoreImplicitConversionPolicyImplAssumingReal<Rep, Magnitude<>, Rep> : std::true_type {};

// `SettingPureRealFromMixedReal<A, B>` tests whether `A` is a pure real type, _and_ `B` is a type
// that has a real _part_, but is not purely real (call it a "mixed-real" type).
//
// The point is to guard against situations where we're _implicitly_ converting a "mixed-real" type
// (i.e., typically a complex number) to a pure real type.
template <typename Rep, typename SourceRep>
struct SettingPureRealFromMixedReal
    : stdx::conjunction<stdx::negation<std::is_same<SourceRep, RealPart<SourceRep>>>,
                        std::is_same<Rep, RealPart<Rep>>> {};

// `SettingUnsignedFromNegativeScaleFactor<Rep, ScaleFactor>` makes sure we're not applying a
// negative scale factor and then storing the result in an unsigned type.  This would only be OK if
// the stored value itself were also negative, which is either never true (unsigned source) or true
// only about half the time (signed source) --- in either case, not good enough for _implicit_
// conversion.
template <typename Rep, typename ScaleFactor>
struct SettingUnsignedFromNegativeScaleFactor
    : stdx::conjunction<std::is_unsigned<Rep>, stdx::negation<IsPositive<ScaleFactor>>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct CoreImplicitConversionPolicyImpl
    : stdx::conjunction<stdx::negation<SettingPureRealFromMixedReal<Rep, SourceRep>>,
                        stdx::negation<SettingUnsignedFromNegativeScaleFactor<Rep, ScaleFactor>>,
                        CoreImplicitConversionPolicyImplAssumingReal<RealPart<Rep>,
                                                                     ScaleFactor,
                                                                     RealPart<SourceRep>>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
using CoreImplicitConversionPolicy = CoreImplicitConversionPolicyImpl<Rep, ScaleFactor, SourceRep>;

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct PermitAsCarveOutForIntegerPromotion
    : stdx::conjunction<std::is_same<Abs<ScaleFactor>, Magnitude<>>,
                        stdx::disjunction<IsPositive<ScaleFactor>, std::is_signed<Rep>>,
                        std::is_integral<Rep>,
                        std::is_integral<SourceRep>,
                        std::is_assignable<Rep &, SourceRep>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
using ImplicitConversionPolicy =
    stdx::disjunction<CoreImplicitConversionPolicy<Rep, ScaleFactor, SourceRep>,
                      PermitAsCarveOutForIntegerPromotion<Rep, ScaleFactor, SourceRep>>;
}  // namespace auimpl

template <typename Rep, typename ScaleFactor>
struct ImplicitRepPermitted : auimpl::ImplicitConversionPolicy<Rep, ScaleFactor, Rep> {};

template <typename Rep, typename SourceUnitSlot, typename TargetUnitSlot>
constexpr bool implicit_rep_permitted_from_source_to_target(SourceUnitSlot, TargetUnitSlot) {
    using SourceUnit = AssociatedUnitT<SourceUnitSlot>;
    using TargetUnit = AssociatedUnitT<TargetUnitSlot>;
    static_assert(HasSameDimension<SourceUnit, TargetUnit>::value,
                  "Can only convert same-dimension units");

    return ImplicitRepPermitted<Rep, UnitRatioT<SourceUnit, TargetUnit>>::value;
}

template <typename Unit, typename Rep>
struct ConstructionPolicy {
    // Note: it's tempting to use the UnitRatioT trait here, but we can't, because it produces a
    // hard error for units with different dimensions.  This is for good reason: magnitude ratios
    // are meaningless unless the dimension is the same.  UnitRatioT is the user-facing tool, so we
    // build in this hard error for safety.  Here, we need a soft error, so we do the dimension
    // check manually below.
    template <typename SourceUnit>
    using ScaleFactor = MagQuotientT<auimpl::MagT<SourceUnit>, auimpl::MagT<Unit>>;

    template <typename SourceUnit, typename SourceRep>
    using PermitImplicitFrom = stdx::conjunction<
        HasSameDimension<Unit, SourceUnit>,
        auimpl::ImplicitConversionPolicy<Rep, ScaleFactor<SourceUnit>, SourceRep>>;
};

}  // namespace au
