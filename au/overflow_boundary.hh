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
#include "au/operators.hh"
#include "au/stdx/type_traits.hh"

// These utilities help assess overflow risk for an operation `Op` by finding the minimum and
// maximum values in the "scalar type" of `OpInput<Op>` that are guaranteed to not overflow.
//
// The "scalar type" of `T` is usually just `T`, but if `T` is something like `std::complex<U>`, or
// `Eigen::Vector<U, N>`, then it would be `U`.

namespace au {
namespace detail {

//
// `MinPossible<Op>::value()` is the smallest representable value in the "scalar type" for
// `OpInput<Op>` (see above comments for definition of "scalar type").
//
// This exists to give us an interface for `numeric_limits<T>::lowest()` that is as easy as possible
// to use with `MinGood<Op, Limits>`.  That means it automatically applies to the scalar type, and
// that it stores the result behind a `::value()` interface.
//
template <typename Op>
struct MinPossibleImpl;
template <typename Op>
using MinPossible = typename MinPossibleImpl<Op>::type;

//
// `MaxPossible<Op>::value()` is the largest representable value in the "scalar type" for
// `OpInput<Op>` (see above comments for definition of "scalar type").
//
template <typename Op>
struct MaxPossibleImpl;
template <typename Op>
using MaxPossible = typename MaxPossibleImpl<Op>::type;

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

//
// `CanOverflowBelow<Op>::value` is `true` if there is any value in `OpInput<Op>` that can cause the
// operation to exceed its bounds.
//
template <typename Op>
struct CanOverflowBelow;

//
// `CanOverflowAbove<Op>::value` is `true` if there is any value in `OpInput<Op>` that can cause the
// operation to exceed its bounds.
//
template <typename Op>
struct CanOverflowAbove;

// `MinValueChecker<Op>::is_too_small(x)` checks whether the value `x` is small enough to overflow
// the bounds of the operation.
template <typename Op>
struct MinValueChecker;

// `MaxValueChecker<Op>::is_too_large(x)` checks whether the value `x` is large enough to overflow
// the bounds of the operation.
template <typename Op>
struct MaxValueChecker;

// `would_value_overflow<Op>(x)` checks whether the value `x` would exceed the bounds of the
// operation at any stage.
template <typename Op>
constexpr bool would_value_overflow(const OpInput<Op> &x) {
    return MinValueChecker<Op>::is_too_small(x) || MaxValueChecker<Op>::is_too_large(x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS
////////////////////////////////////////////////////////////////////////////////////////////////////

// General note:
//
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// Predicate helpers

//
// `IsDefinitelyBounded<T>::value` is `true` if `T` is known to have specific min/max values.
//
template <typename T>
using IsDefinitelyBounded =
    stdx::conjunction<stdx::bool_constant<(std::numeric_limits<T>::is_specialized)>,
                      stdx::bool_constant<(std::numeric_limits<T>::is_bounded)>>;

//
// `IsDefinitelyUnsigned<T>::value` is `true` if `T` is known to be an unsigned type.
//
template <typename T>
using IsDefinitelyUnsigned =
    stdx::conjunction<stdx::bool_constant<std::numeric_limits<T>::is_specialized>,
                      stdx::bool_constant<!std::numeric_limits<T>::is_signed>>;

//
// `IsAbsProbablyBiggerThanOne<T, M>::value` is `true` if `Abs<M>` is bigger than 1.
//
template <typename T, typename M, MagRepresentationOutcome Outcome>
struct IsAbsProbablyBiggerThanOneHelper : std::false_type {};

template <typename T, typename M>
struct IsAbsProbablyBiggerThanOneHelper<T, M, MagRepresentationOutcome::OK>
    : stdx::bool_constant<(get_value<T>(Abs<M>{}) >= T{1})> {};

template <typename T, typename M>
struct IsAbsProbablyBiggerThanOneHelper<T, M, MagRepresentationOutcome::ERR_CANNOT_FIT>
    : std::true_type {};

template <typename T, typename M>
struct IsAbsProbablyBiggerThanOne
    : IsAbsProbablyBiggerThanOneHelper<T, M, get_value_result<T>(Abs<M>{}).outcome> {};

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

template <typename T>
constexpr T clamped_negate(T x) {
    if (Less{}(x, T{0}) && Less{}(x, -std::numeric_limits<T>::max())) {
        return std::numeric_limits<T>::max();
    }
    if (Greater{}(x, T{0}) && Greater{}(x, clamped_negate(std::numeric_limits<T>::lowest()))) {
        return std::numeric_limits<T>::lowest();
    }
    return -x;
}

// `LimitsFor<Op, Limits>` produces a type which can be the `Limits` argument for some other op.
template <typename Op, typename Limits>
struct LimitsFor {
    static constexpr RealPart<OpInput<Op>> lower() { return MinGood<Op, Limits>::value(); }
    static constexpr RealPart<OpInput<Op>> upper() { return MaxGood<Op, Limits>::value(); }
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

        return (LIMIT <= MAX_MANTISSA) ? LIMIT : double_first_until_second(MAX_MANTISSA, LIMIT);
    }

    static constexpr Float double_first_until_second(Float x, Float limit) {
        while (x + x < limit) {
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

template <typename T, typename MagT, MagRepresentationOutcome Outcome>
struct MagHelper {
    static constexpr bool equal(const T &, const T &) { return false; }
    static constexpr T div(const T &, const T &) {
        static_assert(Outcome == MagRepresentationOutcome::ERR_CANNOT_FIT,
                      "Internal library error");

        // Dividing by a number that is too big to fit in the type implies a result of 0.
        return T{0};
    }
};

template <typename T, typename MagT>
struct MagHelper<T, MagT, MagRepresentationOutcome::OK> {
    static constexpr bool equal(const T &x, const T &value) { return x == value; }
    static constexpr T div(const T &a, const T &b) { return a / b; }
};

template <typename T, typename... BPs>
constexpr T divide_by_mag(const T &x, Magnitude<BPs...> m) {
    constexpr auto result = get_value_result<T>(m);
    return MagHelper<T, Magnitude<BPs...>, result.outcome>::div(x, result.value);
}

// Name reads as "lowest of (limits divided by value)".  Remember that the value can be negative, so
// we just take whichever limit is smaller _after_ dividing.
//
// This utility should only be called when `Abs<M>` is greater than 1.  (We can't easily check this
// condition, so we simply assume it; all callers are library-internal anyway, and we have unit
// tests.)  Since `Abs<M>` can be assumed to be greater than one, we know that dividing by `M` will
// shrink values, so we don't risk overflow.
template <typename T, typename M, typename Limits>
struct LowestOfLimitsDividedByValue {
    static constexpr T value() {
        constexpr auto RELEVANT_LIMIT =
            IsPositive<M>::value ? LowerLimit<T, Limits>::value() : UpperLimit<T, Limits>::value();

        return divide_by_mag(RELEVANT_LIMIT, M{});
    }
};

// Name reads as "clamp lowest of (limits times inverse value)".  First, remember that the value can
// be negative, so multiplying can sometimes switch the sign: we want whichever is smaller _after_
// that operation.  Next, if clamping is relevant, that means both that the type is bounded (so
// overflow is _possible_), and that `Abs<M>` is _smaller_ than 1 (implying that its _inverse_ can
// _grow_ values, so we risk overflow).  Therefore, we have to start from the bounds of the type,
// and back out the most extreme value for the limit that will _not_ overflow.
template <typename T, typename M, typename Limits>
struct ClampLowestOfLimitsTimesInverseValue {
    static constexpr T value() {
        constexpr auto ABS_DIVISOR = MagInverse<Abs<M>>{};

        constexpr T RELEVANT_LIMIT = IsPositive<M>::value
                                         ? LowerLimit<T, Limits>::value()
                                         : clamped_negate(UpperLimit<T, Limits>::value());

        constexpr T RELEVANT_BOUND =
            IsPositive<M>::value
                ? divide_by_mag(std::numeric_limits<T>::lowest(), ABS_DIVISOR)
                : clamped_negate(divide_by_mag(std::numeric_limits<T>::max(), ABS_DIVISOR));
        constexpr bool SHOULD_CLAMP = RELEVANT_BOUND >= RELEVANT_LIMIT;

        // This value will be meaningless if `get_value_result<T>(ABS_DIVISOR).outcome` is not `OK`,
        // but we won't end up actually using the value in those cases.
        constexpr auto ABS_DIVISOR_AS_T = get_value_result<T>(ABS_DIVISOR).value;

        return SHOULD_CLAMP ? std::numeric_limits<T>::lowest() : RELEVANT_LIMIT * ABS_DIVISOR_AS_T;
    }
};

template <typename T, typename... BPs>
constexpr bool mag_representation_equals(const T &x, Magnitude<BPs...> m) {
    constexpr auto result = get_value_result<T>(m);
    return MagHelper<T, Magnitude<BPs...>, result.outcome>::equal(x, result.value);
}

// Name reads as "highest of (limits divided by value)".  Of course, normally this is just the
// higher limit divided by the value.  But if the value is negative, then the _lower limit_ will
// give the higher result _after_ we divide.
//
// Also, `Abs<M>` can be assumed to be greater than one, or else we would have been shunted into the
// clamping variant.  This means that dividing by `M` will shrink values, so we don't risk overflow.
template <typename T, typename M, typename Limits>
struct HighestOfLimitsDividedByValue {
    static constexpr T value() {
        if (mag_representation_equals(LowerLimit<T, Limits>::value(), M{})) {
            return T{1};
        }

        return (IsPositive<M>::value)
                   ? divide_by_mag(UpperLimit<T, Limits>::value(), M{})
                   : clamped_negate(divide_by_mag(LowerLimit<T, Limits>::value(), Abs<M>{}));
    }
};

// Name reads as "clamp highest of (limits times inverse value)".  See comments for
// `ClampLowestOfLimitsTimesInverseValue` for more details on the motivation and logic.
template <typename T, typename M, typename Limits>
struct ClampHighestOfLimitsTimesInverseValue {
    static constexpr T value() {
        constexpr auto ABS_DIVISOR = MagInverse<Abs<M>>{};

        constexpr T RELEVANT_LIMIT = IsPositive<M>::value
                                         ? UpperLimit<T, Limits>::value()
                                         : clamped_negate(LowerLimit<T, Limits>::value());

        constexpr T RELEVANT_BOUND =
            IsPositive<M>::value
                ? divide_by_mag(std::numeric_limits<T>::max(), ABS_DIVISOR)
                : clamped_negate(divide_by_mag(std::numeric_limits<T>::lowest(), ABS_DIVISOR));
        constexpr bool SHOULD_CLAMP = RELEVANT_BOUND <= RELEVANT_LIMIT;

        // This value will be meaningless if `get_value_result<T>(ABS_DIVISOR).outcome` is not `OK`,
        // but we won't end up actually using the value in those cases.
        constexpr auto ABS_DIVISOR_AS_T = get_value_result<T>(ABS_DIVISOR).value;

        return SHOULD_CLAMP ? std::numeric_limits<T>::max() : RELEVANT_LIMIT * ABS_DIVISOR_AS_T;
    }
};

constexpr bool is_ok_or_err_cannot_fit(MagRepresentationOutcome outcome) {
    return outcome == MagRepresentationOutcome::OK ||
           outcome == MagRepresentationOutcome::ERR_CANNOT_FIT;
}

template <typename T, typename M>
struct IsCompatibleApartFromMaybeOverflow
    : stdx::bool_constant<is_ok_or_err_cannot_fit(get_value_result<T>(M{}).outcome)> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MinPossible<Op>` implementation.

// Why this lazy implementation, instead of using `std::numeric_limits` directly?  Simply because we
// need a _type_ whose _`value()` method_ returns the given value.  We already built that for more
// complicated use cases (it's called `LowestOfLimitsDividedByValue`), so we can just reuse it here.
template <typename Op>
struct MinPossibleImpl
    : stdx::type_identity<LowestOfLimitsDividedByValue<RealPart<OpInput<Op>>, Magnitude<>, void>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MaxPossible<Op>` implementation.

// See `MinPossibleImpl` comments above for explanation of this lazy approach.
template <typename Op>
struct MaxPossibleImpl
    : stdx::type_identity<HighestOfLimitsDividedByValue<RealPart<OpInput<Op>>, Magnitude<>, void>> {
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

template <typename T, typename M>
using IsClampingRequired =
    stdx::conjunction<stdx::negation<IsAbsProbablyBiggerThanOne<T, M>>, IsDefinitelyBounded<T>>;

//
// `MinGood<MultiplyTypeBy<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyCompatibleTypeBy
    : std::conditional<IsClampingRequired<T, M>::value,
                       ClampLowestOfLimitsTimesInverseValue<T, M, Limits>,
                       LowestOfLimitsDividedByValue<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByAssumingSigned
    : std::conditional_t<IsCompatibleApartFromMaybeOverflow<T, M>::value,
                         MinGoodImplForMultiplyCompatibleTypeBy<T, M, Limits>,
                         stdx::type_identity<ValueOfZero<T>>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByUsingRealPart
    : std::conditional_t<IsDefinitelyUnsigned<T>::value,
                         stdx::type_identity<ValueOfZero<T>>,
                         MinGoodImplForMultiplyTypeByAssumingSigned<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImpl<MultiplyTypeBy<T, M>, Limits>
    : MinGoodImplForMultiplyTypeByUsingRealPart<RealPart<T>, M, Limits> {};

//
// `MaxGood<MultiplyTypeBy<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyCompatibleTypeBy
    : std::conditional<IsClampingRequired<T, M>::value,
                       ClampHighestOfLimitsTimesInverseValue<T, M, Limits>,
                       HighestOfLimitsDividedByValue<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyTypeByAssumingSignedTypeOrPositiveFactor
    : std::conditional_t<IsCompatibleApartFromMaybeOverflow<T, M>::value,
                         MaxGoodImplForMultiplyCompatibleTypeBy<T, M, Limits>,
                         stdx::type_identity<ValueOfZero<T>>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyTypeByUsingRealPart
    : std::conditional_t<
          stdx::conjunction<IsDefinitelyUnsigned<T>, stdx::negation<IsPositive<M>>>::value,
          stdx::type_identity<ValueOfZero<T>>,
          MaxGoodImplForMultiplyTypeByAssumingSignedTypeOrPositiveFactor<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImpl<MultiplyTypeBy<T, M>, Limits>
    : MaxGoodImplForMultiplyTypeByUsingRealPart<RealPart<T>, M, Limits> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger<T, M>` implementation.

//
// `MinGood<DivideTypeByInteger<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MinGoodImplForDivideTypeByIntegerAssumingSigned
    : stdx::type_identity<ClampLowestOfLimitsTimesInverseValue<T, MagInverse<M>, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForDivideTypeByIntegerUsingRealPart
    : std::conditional_t<IsDefinitelyUnsigned<T>::value,
                         stdx::type_identity<ValueOfZero<T>>,
                         MinGoodImplForDivideTypeByIntegerAssumingSigned<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImpl<DivideTypeByInteger<T, M>, Limits>
    : MinGoodImplForDivideTypeByIntegerUsingRealPart<RealPart<T>, M, Limits> {};

//
// `MaxGood<DivideTypeByInteger<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MaxGoodImplForDivideTypeByIntegerAssumingSignedTypeOrPositiveFactor
    : stdx::type_identity<ClampHighestOfLimitsTimesInverseValue<T, MagInverse<M>, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForDivideTypeByIntegerUsingRealPart
    : std::conditional_t<
          stdx::conjunction<IsDefinitelyUnsigned<T>, stdx::negation<IsPositive<M>>>::value,
          stdx::type_identity<ValueOfZero<T>>,
          MaxGoodImplForDivideTypeByIntegerAssumingSignedTypeOrPositiveFactor<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImpl<DivideTypeByInteger<T, M>, Limits>
    : MaxGoodImplForDivideTypeByIntegerUsingRealPart<RealPart<T>, M, Limits> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<Ops...>` implementation.

//
// `MinGood<OpSequence<Ops...>>` implementation cluster.
//

template <typename OnlyOp, typename Limits>
struct MinGoodImpl<OpSequenceImpl<OnlyOp>, Limits> : MinGoodImpl<OnlyOp, Limits> {};

template <typename Op1, typename Op2, typename... Ops, typename Limits>
struct MinGoodImpl<OpSequenceImpl<Op1, Op2, Ops...>, Limits>
    : MinGoodImpl<Op1, LimitsFor<OpSequenceImpl<Op2, Ops...>, Limits>> {
    static_assert(std::is_same<OpOutput<Op1>, OpInput<Op2>>::value,
                  "Output of each op in sequence must match input of next op");
};

//
// `MaxGood<OpSequence<Ops...>>` implementation cluster.
//

template <typename OnlyOp, typename Limits>
struct MaxGoodImpl<OpSequenceImpl<OnlyOp>, Limits> : MaxGoodImpl<OnlyOp, Limits> {};

template <typename Op1, typename Op2, typename... Ops, typename Limits>
struct MaxGoodImpl<OpSequenceImpl<Op1, Op2, Ops...>, Limits>
    : MaxGoodImpl<Op1, LimitsFor<OpSequenceImpl<Op2, Ops...>, Limits>> {
    static_assert(std::is_same<OpOutput<Op1>, OpInput<Op2>>::value,
                  "Output of each op in sequence must match input of next op");
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanOverflowBelow<Op>` implementation.

template <typename Op>
struct CanOverflowBelow : stdx::bool_constant<(MinGood<Op>::value() > MinPossible<Op>::value())> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanOverflowAbove<Op>` implementation.

template <typename Op>
struct CanOverflowAbove : stdx::bool_constant<(MaxGood<Op>::value() < MaxPossible<Op>::value())> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MinValueChecker<Op>` and `MaxValueChecker<Op>` implementation.

template <typename Op, bool IsOverflowPossible>
struct MinValueCheckerImpl {
    static constexpr bool is_too_small(const OpInput<Op> &x) { return x < MinGood<Op>::value(); }
};
template <typename Op>
struct MinValueCheckerImpl<Op, false> {
    static constexpr bool is_too_small(const OpInput<Op> &) { return false; }
};
template <typename Op>
struct MinValueChecker : MinValueCheckerImpl<Op, CanOverflowBelow<Op>::value> {};

template <typename Op, bool IsOverflowPossible>
struct MaxValueCheckerImpl {
    static constexpr bool is_too_large(const OpInput<Op> &x) { return x > MaxGood<Op>::value(); }
};
template <typename Op>
struct MaxValueCheckerImpl<Op, false> {
    static constexpr bool is_too_large(const OpInput<Op> &) { return false; }
};
template <typename Op>
struct MaxValueChecker : MaxValueCheckerImpl<Op, CanOverflowAbove<Op>::value> {};

}  // namespace detail
}  // namespace au
