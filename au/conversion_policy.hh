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
    if (get_value<double>(m) <= 1.0) {
        (void)value;
        return true;
    } else {
        return std::numeric_limits<Rep>::max() / get_value<Rep>(m) >= value;
    }
}

namespace detail {
// Chosen so as to allow populating a `QuantityU32<Hertz>` with an input in MHz.
constexpr auto OVERFLOW_THRESHOLD = 4'294;

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
struct CoreImplicitConversionPolicyImpl
    : stdx::disjunction<
          std::is_floating_point<Rep>,
          stdx::conjunction<std::is_integral<SourceRep>,
                            IsInteger<ScaleFactor>,
                            detail::CanScaleThresholdWithoutOverflow<Rep, ScaleFactor>>> {};

// Always permit the identity scaling.
template <typename Rep>
struct CoreImplicitConversionPolicyImpl<Rep, Magnitude<>, Rep> : std::true_type {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
using CoreImplicitConversionPolicy = CoreImplicitConversionPolicyImpl<Rep, ScaleFactor, SourceRep>;

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct PermitAsCarveOutForIntegerPromotion
    : stdx::conjunction<std::is_same<ScaleFactor, Magnitude<>>,
                        std::is_integral<Rep>,
                        std::is_integral<SourceRep>,
                        std::is_assignable<Rep &, SourceRep>> {};
}  // namespace detail

template <typename Rep, typename ScaleFactor>
struct ImplicitRepPermitted : detail::CoreImplicitConversionPolicy<Rep, ScaleFactor, Rep> {};

template <typename Rep, typename SourceUnit, typename TargetUnit>
constexpr bool implicit_rep_permitted_from_source_to_target(SourceUnit, TargetUnit) {
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
        stdx::disjunction<
            detail::CoreImplicitConversionPolicy<Rep, ScaleFactor<SourceUnit>, SourceRep>,
            detail::PermitAsCarveOutForIntegerPromotion<Rep, ScaleFactor<SourceUnit>, SourceRep>>>;
};

}  // namespace au
