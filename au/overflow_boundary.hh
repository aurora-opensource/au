// Copyright 2025 Aurora Operations, Inc.
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
#include <type_traits>

#include "au/abstract_operations.hh"
#include "au/magnitude.hh"
#include "au/stdx/type_traits.hh"

// These utilities help assess overflow risk for an operation `Op` by finding the minimum and
// maximum values in the "scalar type" of `OpInput<Op>` that are guaranteed to not overflow.
//
// The "scalar type" of `T` is usually just `T`, but if `T` is something like `std::complex<U>`, or
// `Eigen::Vector<U, N>`, then it would be `U`.

namespace au {
namespace detail {

//
// `MinGood<Op>::value()` is a constexpr constant of the "scalar type" for `OpInput<Op>` that is the
// minimum value that does not overflow.
//
// IMPORTANT: the result must always be non-positive.  The code is structured on this assumption.
//
template <typename Op, typename Limits>
struct MinGoodImpl;
template <typename Op, typename Limits = void>
using MinGood = typename MinGoodImpl<Op, Limits>::type;

//
// `MaxGood<Op>::value()` is a constexpr constant of the "scalar type" for `OpInput<Op>` that is the
// maximum value that does not overflow.
//
// IMPORTANT: the result must always be non-negative.  The code is structured on this assumption.
//
template <typename Op, typename Limits = void>
struct MaxGoodImpl;
template <typename Op, typename Limits = void>
using MaxGood = typename MaxGoodImpl<Op, Limits>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS
////////////////////////////////////////////////////////////////////////////////////////////////////

// The implementation strategy will be to decompose to increasingly specific cases, using
// `std::conditional` constructs that are _at most one layer deep_.  This should keep every
// individual piece as easy to understand as possible, although it does mean we'll tend to be
// navigating many layers deep from the top-level API to the ultimate implementation.
//
// It's easier to navigate these helpers if we put a shorthand comment at the top of each.  Here's
// the key:
//
// (A) = arithmetic (integral or floating point)
// (F) = floating point
// (I) = integral (signed or unsigned)
// (N) = non-arithmetic
// (S) = signed integral
// (U) = unsigned integral
// (X) = any type

// `UpperLimit<T, Limits>::value()` returns `Limits::upper()` (assumed to be of type `T`), unless
// `Limits` is `void`, in which case it means "no limit" and we return the highest possible value.
template <typename T, typename Limits>
struct UpperLimit {
    static constexpr T value() { return Limits::upper(); }
};
template <typename T>
struct UpperLimit<T, void> {
    static constexpr T value() { return std::numeric_limits<T>::max(); }
};

// `LowerLimit<T, Limits>::value()` returns `Limits::lower()` (assumed to be of type `T`), unless
// `Limits` is `void`, in which case it means "no limit" and we return the lowest possible value.
template <typename T, typename Limits>
struct LowerLimit {
    static constexpr T value() { return Limits::lower(); }
};
template <typename T>
struct LowerLimit<T, void> {
    static constexpr T value() { return std::numeric_limits<T>::lowest(); }
};

// Inherit from this struct to produce a compiler error in case we try to use a combination of types
// that isn't yet supported.
template <typename T>
struct OverflowBoundaryNotYetImplemented {
    struct NotYetImplemented {};
    static_assert(std::is_same<T, NotYetImplemented>::value,
                  "Overflow boundary not yet implemented for this type.");
};

// A type whose `::value()` function returns the higher of `std::numeric_limits<T>::lowest()`, or
// `LowerLimit<U, ULimit>` expressed in `T`.  Assumes that `U` is more expansive than `T`, so that
// we can cast everything to `U` to do the comparisons.
template <typename T, typename U, typename ULimit>
struct ValueOfSourceLowestUnlessDestLimitIsHigher {
    static constexpr T value() {
        constexpr auto LOWEST_T_IN_U = static_cast<U>(std::numeric_limits<T>::lowest());
        constexpr auto U_LIMIT = LowerLimit<U, ULimit>::value();
        return (LOWEST_T_IN_U <= U_LIMIT) ? static_cast<T>(U_LIMIT)
                                          : std::numeric_limits<T>::lowest();
    }
};

// A type whose `::value()` function returns the lower of `std::numeric_limits<T>::max()`, or
// `UpperLimit<U, ULimit>` expressed in `T`.  Assumes that `U` is more expansive than `T`, so that
// we can cast everything to `U` to do the comparisons.
template <typename T, typename U, typename ULimit>
struct ValueOfSourceHighestUnlessDestLimitIsLower {
    static constexpr T value() {
        constexpr auto HIGHEST_T_IN_U = static_cast<U>(std::numeric_limits<T>::max());
        constexpr auto U_LIMIT = UpperLimit<U, ULimit>::value();
        return (HIGHEST_T_IN_U >= U_LIMIT) ? static_cast<T>(U_LIMIT)
                                           : std::numeric_limits<T>::max();
    }
};

// A type whose `::value()` function returns the lowest value of `U`, expressed in `T`.
template <typename T, typename U = T, typename ULimit = void>
struct ValueOfLowestInDestination {
    static constexpr T value() { return static_cast<T>(LowerLimit<U, ULimit>::value()); }

    static_assert(static_cast<U>(value()) == LowerLimit<U, ULimit>::value(),
                  "This utility assumes lossless round trips");
};

// A type whose `::value()` function returns the highest value of `U`, expressed in `T`.
template <typename T, typename U = T, typename ULimit = void>
struct ValueOfHighestInDestination {
    static constexpr T value() { return static_cast<T>(UpperLimit<U, ULimit>::value()); }

    static_assert(static_cast<U>(value()) == UpperLimit<U, ULimit>::value(),
                  "This utility assumes lossless round trips");
};

// A type whose `::value()` function is capped at the highest value in `Float` (assumed to be a
// floating point type) that can be cast to `Int` (assumed to be an integral type).  We need to be
// really careful in how we express this, because max int values tend not to be nice powers of 2.
// Therefore, even though we can cast the `Int` max to `Float` successfully, casting back to `Int`
// will produce a compile time error because the closest representable integer in `Float` is
// slightly _higher_ than that max.
//
// On the implementation side, keep in mind that our library supports C++14, and most common
// floating point utilities (such as `std::nextafter`) are not `constexpr` compatible in C++14.
// Therefore, we need to use alternative strategies to explore the floating point type.  These are
// always evaluated at compile time, so we are not especially concerned about the efficiency: it
// should have no runtime effect at all, and we expect even the compile time impact --- which we
// measure regularly as we land commits --- to be too small to measure.
template <typename Float, typename Int, typename IntLimit>
struct ValueOfMaxFloatNotExceedingMaxInt {
    // The `Float` value where all mantissa bits are set to `1`, and the exponent is `0`.
    static constexpr Float max_mantissa() {
        constexpr Float ONE = Float{1};
        Float x = ONE;
        Float last = x;
        while (x + ONE > x) {
            last = x;
            x += x + ONE;
        }
        return last;
    }

    // Function to do the actual computation of the value.
    static constexpr Float compute_value() {
        constexpr Float LIMIT = static_cast<Float>(std::numeric_limits<Int>::max());
        constexpr Float MAX_MANTISSA = max_mantissa();

        if (LIMIT <= MAX_MANTISSA) {
            return LIMIT;
        }

        Float x = MAX_MANTISSA;
        while (x + x < LIMIT) {
            x += x;
        }
        return x;
    }

    // `value()` implementation simply computes the result _once_ (caching it), and then returns it.
    static constexpr Float value() {
        constexpr Float FLOAT_LIMIT = compute_value();
        constexpr Float EXPLICIT_LIMIT = static_cast<Float>(UpperLimit<Int, IntLimit>::value());
        constexpr Float RESULT = (FLOAT_LIMIT <= EXPLICIT_LIMIT) ? FLOAT_LIMIT : EXPLICIT_LIMIT;
        return RESULT;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` implementation.

//
// `MinGood<StaticCast<T, U>>` implementation cluster.
//
// See comment above for meanings of (N), (X), (A), etc.
//

// (N) -> (X) (placeholder)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (A) -> (N) (placeholder)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmeticToNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (S) -> (S)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToSigned
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueOfSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>,
                       ValueOfLowestInDestination<T, U, ULimit>> {};

// (S) -> (I)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToIntegral
    : std::conditional_t<std::is_unsigned<U>::value,
                         stdx::type_identity<ValueOfZero<T>>,
                         MinGoodImplForStaticCastFromSignedToSigned<T, U, ULimit>> {};

// (S) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToArithmetic
    : std::conditional_t<
          std::is_floating_point<U>::value,
          stdx::type_identity<ValueOfSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>>,
          MinGoodImplForStaticCastFromSignedToIntegral<T, U, ULimit>> {};

// (I) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromIntegralToArithmetic
    : std::conditional_t<
          std::is_unsigned<T>::value,
          stdx::type_identity<ValueOfSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>>,
          MinGoodImplForStaticCastFromSignedToArithmetic<T, U, ULimit>> {};

// (F) -> (F)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromFloatingPointToFloatingPoint
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueOfSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>,
                       ValueOfLowestInDestination<T, U, ULimit>> {};

// (F) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromFloatingPointToArithmetic
    : std::conditional_t<std::is_floating_point<U>::value,
                         MinGoodImplForStaticCastFromFloatingPointToFloatingPoint<T, U, ULimit>,
                         stdx::type_identity<ValueOfLowestInDestination<T, U, ULimit>>> {};

// (A) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmeticToArithmetic
    : std::conditional_t<std::is_integral<T>::value,
                         MinGoodImplForStaticCastFromIntegralToArithmetic<T, U, ULimit>,
                         MinGoodImplForStaticCastFromFloatingPointToArithmetic<T, U, ULimit>> {};

// (A) -> (X)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         MinGoodImplForStaticCastFromArithmeticToArithmetic<T, U, ULimit>,
                         MinGoodImplForStaticCastFromArithmeticToNonArithmetic<T, U, ULimit>> {};

// (X) -> (X)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastUsingRealPart
    : std::conditional_t<
          std::is_arithmetic<RealPart<T>>::value,
          MinGoodImplForStaticCastFromArithmetic<RealPart<T>, RealPart<U>, ULimit>,
          MinGoodImplForStaticCastFromNonArithmetic<RealPart<T>, RealPart<U>, ULimit>> {};

template <typename T, typename U, typename ULimit>
struct MinGoodImpl<StaticCast<T, U>, ULimit> : MinGoodImplForStaticCastUsingRealPart<T, U, ULimit> {
};

//
// `MaxGood<StaticCast<T, U>>` implementation cluster.
//
// See comment above for meanings of (N), (X), (A), etc.
//

// (N) -> (X) (placeholder)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (A) -> (N) (placeholder)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmeticToNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (I) -> (I)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromIntegralToIntegral
    : std::conditional<(static_cast<std::common_type_t<T, U>>(std::numeric_limits<T>::max()) <=
                        static_cast<std::common_type_t<T, U>>(std::numeric_limits<U>::max())),
                       ValueOfSourceHighestUnlessDestLimitIsLower<T, U, ULimit>,
                       ValueOfHighestInDestination<T, U, ULimit>> {};

// (I) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromIntegralToArithmetic
    : std::conditional_t<
          std::is_integral<U>::value,
          MaxGoodImplForStaticCastFromIntegralToIntegral<T, U, ULimit>,
          stdx::type_identity<ValueOfSourceHighestUnlessDestLimitIsLower<T, U, ULimit>>> {};

// (F) -> (F)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromFloatingPointToFloatingPoint
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueOfSourceHighestUnlessDestLimitIsLower<T, U, ULimit>,
                       ValueOfHighestInDestination<T, U, ULimit>> {};

// (F) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromFloatingPointToArithmetic
    : std::conditional_t<std::is_floating_point<U>::value,
                         MaxGoodImplForStaticCastFromFloatingPointToFloatingPoint<T, U, ULimit>,
                         stdx::type_identity<ValueOfMaxFloatNotExceedingMaxInt<T, U, ULimit>>> {};

// (A) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmeticToArithmetic
    : std::conditional_t<std::is_integral<T>::value,
                         MaxGoodImplForStaticCastFromIntegralToArithmetic<T, U, ULimit>,
                         MaxGoodImplForStaticCastFromFloatingPointToArithmetic<T, U, ULimit>> {};

// (A) -> (X)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         MaxGoodImplForStaticCastFromArithmeticToArithmetic<T, U, ULimit>,
                         MaxGoodImplForStaticCastFromArithmeticToNonArithmetic<T, U, ULimit>> {};

// (X) -> (X)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastUsingRealPart
    : std::conditional_t<
          std::is_arithmetic<RealPart<T>>::value,
          MaxGoodImplForStaticCastFromArithmetic<RealPart<T>, RealPart<U>, ULimit>,
          MaxGoodImplForStaticCastFromNonArithmetic<RealPart<T>, RealPart<U>, ULimit>> {};

template <typename T, typename U, typename ULimit>
struct MaxGoodImpl<StaticCast<T, U>, ULimit> : MaxGoodImplForStaticCastUsingRealPart<T, U, ULimit> {
};

}  // namespace detail
}  // namespace au
