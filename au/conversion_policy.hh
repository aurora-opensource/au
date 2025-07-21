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

#include "au/conversion_strategy.hh"
#include "au/magnitude.hh"
#include "au/operators.hh"
#include "au/overflow_boundary.hh"
#include "au/stdx/type_traits.hh"
#include "au/stdx/utility.hh"
#include "au/truncation_risk.hh"
#include "au/unit_of_measure.hh"

namespace au {
namespace detail {

// Chosen so as to allow populating a `QuantityI32<Hertz>` with an input in MHz.
constexpr auto OVERFLOW_THRESHOLD = mag<2'147>();

// `SettingPureRealFromMixedReal<A, B>` tests whether `A` is a pure real type, _and_ `B` is a type
// that has a real _part_, but is not purely real (call it a "mixed-real" type).
//
// The point is to guard against situations where we're _implicitly_ converting a "mixed-real" type
// (i.e., typically a complex number) to a pure real type.
template <typename Rep, typename SourceRep>
struct SettingPureRealFromMixedReal
    : stdx::conjunction<stdx::negation<std::is_same<SourceRep, RealPart<SourceRep>>>,
                        std::is_same<Rep, RealPart<Rep>>> {};

template <typename T>
constexpr bool meets_threshold(T x) {
    constexpr auto threshold_result = get_value_result<T>(OVERFLOW_THRESHOLD);
    static_assert(threshold_result.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT ||
                      threshold_result.outcome == MagRepresentationOutcome::OK,
                  "Overflow threshold must be a valid representation");
    const auto threshold = (threshold_result.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT)
                               ? std::numeric_limits<T>::max()
                               : threshold_result.value;
    if (Less{}(x, T{0})) {
        x = T{0} - x;
    }
    return x >= threshold;
}

// Check overflow risk from above.
template <bool CanOverflowAbove, typename Op>
struct OverflowAboveRiskAcceptablyLowImpl
    : stdx::bool_constant<meets_threshold(MaxGood<Op>::value())> {};
template <typename Op>
struct OverflowAboveRiskAcceptablyLowImpl<false, Op> : std::true_type {};

template <typename Op>
struct OverflowAboveRiskAcceptablyLow
    : OverflowAboveRiskAcceptablyLowImpl<CanOverflowAbove<Op>::value, Op> {};

// Check overflow risk, using "overflow above" risk only.
//
// We currently do not check the risk for overflowing _below_, because it is overwhelmingly common
// in practice for people to initialize an unsigned integer variable with a constant of a signed
// type whose value is known to be positive.  While we would love to be able to prevent implicit
// signed to unsigned conversions --- and, while our overflow detection machinery can easily do so
// --- we simply cannot afford to break that many _valid_ use cases to catch those invalid ones.
//
// That said, the _runtime_ overflow checkers _do_ check both above and below.
template <typename Op>
struct OverflowRiskAcceptablyLow : OverflowAboveRiskAcceptablyLow<Op> {};

// Check truncation risk.
template <typename Op>
struct TruncationRiskAcceptablyLow
    : std::is_same<TruncationRiskFor<Op>, NoTruncationRisk<RealPart<OpInput<Op>>>> {};

template <typename Op>
struct ConversionRiskAcceptablyLow
    : stdx::conjunction<OverflowRiskAcceptablyLow<Op>, TruncationRiskAcceptablyLow<Op>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct PermitAsCarveOutForIntegerPromotion
    : stdx::conjunction<std::is_same<Abs<ScaleFactor>, Magnitude<>>,
                        std::is_same<SourceRep, PromotedType<Rep>>,
                        stdx::disjunction<IsPositive<ScaleFactor>, std::is_signed<Rep>>,
                        std::is_integral<Rep>,
                        std::is_integral<SourceRep>,
                        std::is_assignable<Rep &, SourceRep>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct PassesConversionRiskCheck
    : stdx::disjunction<
          PermitAsCarveOutForIntegerPromotion<Rep, ScaleFactor, SourceRep>,
          ConversionRiskAcceptablyLow<ConversionForRepsAndFactor<SourceRep, Rep, ScaleFactor>>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
using ImplicitConversionPolicy =
    stdx::conjunction<PassesConversionRiskCheck<Rep, ScaleFactor, SourceRep>,
                      stdx::negation<SettingPureRealFromMixedReal<Rep, SourceRep>>>;

}  // namespace detail

template <typename Rep, typename ScaleFactor>
struct ImplicitRepPermitted : detail::ImplicitConversionPolicy<Rep, ScaleFactor, Rep> {};

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
    using ScaleFactor = MagQuotientT<detail::MagT<SourceUnit>, detail::MagT<Unit>>;

    template <typename SourceUnit, typename SourceRep>
    using PermitImplicitFrom = stdx::conjunction<
        HasSameDimension<Unit, SourceUnit>,
        detail::ImplicitConversionPolicy<Rep, ScaleFactor<SourceUnit>, SourceRep>>;
};

}  // namespace au
