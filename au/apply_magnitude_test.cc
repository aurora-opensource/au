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

#include "au/magnitude.hh"
#include "au/overflow_boundary.hh"
#include "au/testing.hh"
#include "au/truncation_risk.hh"
#include "gtest/gtest.h"

using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Not;

namespace au {
namespace detail {
namespace {

//
// This file held the unit tests for an older library, `:apply_magnitude`, which we no longer need.
// We have retained the test file so that we can still get value out of all of the pre-existing test
// cases.  The first part of this file simply re-implements the functionality of the old library
// very concisely, in a few lines of code that use the replacement libraries.
//

// `NewOverflowChecker<Op>::would_product_overflow(x)` checks whether the value `x` would exceed the
// bounds of the operation `Op`.
template <typename Op>
struct NewOverflowChecker {
    static constexpr bool would_product_overflow(const OpInput<Op> &x) {
        return MinValueChecker<Op>::is_too_small(x) || MaxValueChecker<Op>::is_too_large(x);
    }
};

template <typename Mag, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl {
    using Op = ConversionForRepsAndFactor<T, T, Mag>;
    constexpr T operator()(const T &x) { return Op::apply_to(x); }

    static constexpr bool would_overflow(const T &x) {
        return NewOverflowChecker<Op>::would_product_overflow(x);
    }

    static constexpr bool would_truncate(const T &x) {
        return TruncationRiskFor<Op>::would_value_truncate(x);
    }
};

template <typename T, typename MagT>
using ApplyMagnitudeT = ApplyMagnitudeImpl<MagT, T, std::is_integral<T>::value>;

template <typename T, typename... BPs>
constexpr T apply_magnitude(const T &x, Magnitude<BPs...>) {
    return ApplyMagnitudeT<T, Magnitude<BPs...>>{}(x);
}

constexpr auto PI = Magnitude<Pi>{};

template <typename T>
std::vector<T> first_n_positive_values(std::size_t n) {
    std::vector<T> result;
    result.reserve(n);
    for (auto i = 1u; i <= n; ++i) {
        result.push_back(static_cast<T>(i));
    }
    return result;
}
}  // namespace

TEST(ApplyMagnitude, MultipliesForIntegerMultiply) {
    constexpr auto m = mag<25>();

    EXPECT_THAT(apply_magnitude(4, m), SameTypeAndValue(100));
    EXPECT_THAT(apply_magnitude(4.0f, m), SameTypeAndValue(100.0f));
}

TEST(ApplyMagnitude, MultipliesForNegativeIntegerMultiply) {
    constexpr auto m = -mag<20>();

    EXPECT_THAT(apply_magnitude(4, m), SameTypeAndValue(-80));
    EXPECT_THAT(apply_magnitude(4.0f, m), SameTypeAndValue(-80.0f));
}

TEST(ApplyMagnitude, DividesForIntegerDivide) {
    constexpr auto one_thirteenth = ONE / mag<13>();

    // This test would fail if our implementation multiplied by the float representation of (1/13),
    // instead of dividing by 13, under the hood.
    for (const auto &i : first_n_positive_values<float>(100u)) {
        EXPECT_THAT(apply_magnitude(i * 13, one_thirteenth), SameTypeAndValue(i));
    }
}

TEST(ApplyMagnitude, DividesForNegativeIntegerDivide) {
    constexpr auto minus_one_thirteenth = -ONE / mag<13>();

    // This test would fail if our implementation multiplied by the float representation of (-1/13),
    // instead of dividing by -13, under the hood.  (We'll use this `bool` variable to make sure
    // that this claim is true.  If the test fails because this variable is `false`, then it
    // probably means we don't have enough coverage, and we should check more numbers.)
    constexpr auto inverse = get_value<float>(minus_one_thirteenth);
    bool any_round_trip_failures = false;

    for (const auto &i : first_n_positive_values<float>(100u)) {
        const auto x = -13.0f * i;
        if (x * inverse != i) {
            any_round_trip_failures = true;
        }
        EXPECT_THAT(apply_magnitude(x, minus_one_thirteenth), SameTypeAndValue(i));
    }
    EXPECT_THAT(any_round_trip_failures, IsTrue());
}

TEST(ApplyMagnitude, MultipliesThenDividesForRationalMagnitudeOnInteger) {
    // Consider applying the magnitude (3/2) to the value 5.  The exact answer is the real number
    // 7.5, which becomes 7 when translated (via truncation) to the integer domain.
    //
    // If we multiply-then-divide, we get (5 * 3) / 2 = 7, which is correct.
    //
    // If we divide-then-multiply --- say, because we are trying to avoid overflow --- then we get
    // (5 / 2) * 3 = 2 * 3 = 6, which is wrong.
    constexpr auto three_halves = mag<3>() / mag<2>();

    EXPECT_THAT(apply_magnitude(5, three_halves), SameTypeAndValue(7));
}

TEST(ApplyMagnitude, MultipliesThenDividesForNegativeRationalMagnitudeOnInteger) {
    // Similar to the above test case, but with a negative number.
    constexpr auto minus_three_halves = -mag<3>() / mag<2>();

    EXPECT_THAT(apply_magnitude(5, minus_three_halves), SameTypeAndValue(-7));
}

TEST(ApplyMagnitude, SupportsNumeratorThatFitsInPromotedTypeButNotOriginalType) {
    using T = uint16_t;
    using P = PromotedType<T>;
    ASSERT_THAT((std::is_same<P, int32_t>::value), IsTrue())
        << "This test fails on architectures where `uint16_t` doesn't get promoted to `int32_t`";

    // Choose a magnitude whose effect will basically be to divide by 2.  (We make the denominator
    // slightly _smaller_ than twice the numerator, rather than slightly _larger_, so that the
    // division will end up on the "high" side of the target, and truncation will bring it down very
    // slightly instead of going down a full integer.)
    auto roughly_one_half = mag<100'000'000>() / mag<199'999'999>();

    // The whole point of this test case is to apply a magnitude whose numerator fits in the
    // promoted type, but does not fit in the target type itself.
    ASSERT_THAT(get_value_result<P>(numerator(roughly_one_half)).outcome,
                Eq(MagRepresentationOutcome::OK));
    ASSERT_THAT(get_value_result<T>(numerator(roughly_one_half)).outcome,
                Eq(MagRepresentationOutcome::ERR_CANNOT_FIT));

    EXPECT_THAT(apply_magnitude(T{18}, roughly_one_half), SameTypeAndValue(T{9}));
}

TEST(ApplyMagnitude, MultipliesSingleNumberForRationalMagnitudeOnFloatingPoint) {
    // Helper similar to `std::transform`, but with more convenient interfaces.
    auto apply = [](std::vector<float> vals, auto fun) {
        for (auto &v : vals) {
            v = fun(v);
        }
        return vals;
    };

    // Create our rational magnitude, (2 / 13).
    constexpr auto two_thirteenths = mag<2>() / mag<13>();

    // Test a bunch of values.  We are hoping that the two different strategies will yield different
    // results for at least some of these strategies (and we'll check that this is the case).
    const auto original_vals = first_n_positive_values<float>(10u);

    // Compute expected answers for each possible strategy.
    const auto if_we_multiply_and_divide =
        apply(original_vals, [](float v) { return v * 2.0f / 13.0f; });
    const auto if_we_use_one_factor =
        apply(original_vals, [](float v) { return v * (2.0f / 13.0f); });

    // The strategies must be different for at least some results!
    ASSERT_THAT(if_we_multiply_and_divide, Not(ElementsAreArray(if_we_use_one_factor)));

    // Make sure we follow the single-number strategy, every time.
    const auto results =
        apply(original_vals, [=](float v) { return apply_magnitude(v, two_thirteenths); });
    EXPECT_THAT(results, ElementsAreArray(if_we_use_one_factor));
    EXPECT_THAT(results, Not(ElementsAreArray(if_we_multiply_and_divide)));
}

TEST(ApplyMagnitude, MultipliesSingleNumberForIrrationalMagnitudeOnFloatingPoint) {
    EXPECT_THAT(apply_magnitude(2.0f, PI), SameTypeAndValue(2.0f * static_cast<float>(M_PI)));
}

TEST(WouldOverflow, HasCorrectBoundariesForIntegerMultiply) {
    auto ONE_BILLION = pow<9>(mag<10>());

    {
        using ApplyOneBillionToI32 = ApplyMagnitudeT<int32_t, decltype(ONE_BILLION)>;

        EXPECT_THAT(ApplyOneBillionToI32::would_overflow(3), IsTrue());

        EXPECT_THAT(ApplyOneBillionToI32::would_overflow(2), IsFalse());
        EXPECT_THAT(ApplyOneBillionToI32::would_overflow(0), IsFalse());
        EXPECT_THAT(ApplyOneBillionToI32::would_overflow(-2), IsFalse());

        EXPECT_THAT(ApplyOneBillionToI32::would_overflow(-3), IsTrue());
    }

    {
        using ApplyOneBillionToU8 = ApplyMagnitudeT<uint8_t, decltype(ONE_BILLION)>;

        EXPECT_THAT(ApplyOneBillionToU8::would_overflow(1), IsTrue());

        EXPECT_THAT(ApplyOneBillionToU8::would_overflow(0), IsFalse());
    }

    {
        using ApplyOneBillionToF = ApplyMagnitudeT<float, decltype(ONE_BILLION)>;

        EXPECT_THAT(ApplyOneBillionToF::would_overflow(3.403e29f), IsTrue());

        EXPECT_THAT(ApplyOneBillionToF::would_overflow(3.402e29f), IsFalse());
        EXPECT_THAT(ApplyOneBillionToF::would_overflow(-3.402e29f), IsFalse());

        EXPECT_THAT(ApplyOneBillionToF::would_overflow(-3.403e29f), IsTrue());
    }
}

TEST(WouldOverflow, AlwaysFalseForIntegerDivide) {
    auto ONE_BILLIONTH = ONE / pow<9>(mag<10>());

    {
        using ApplyOneBillionthToI32 = ApplyMagnitudeT<int32_t, decltype(ONE_BILLIONTH)>;

        EXPECT_THAT(ApplyOneBillionthToI32::would_overflow(2'147'483'647), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToI32::would_overflow(1), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToI32::would_overflow(0), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToI32::would_overflow(-1), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToI32::would_overflow(-2'147'483'648), IsFalse());
    }

    {
        using ApplyOneBillionthToU8 = ApplyMagnitudeT<uint8_t, decltype(ONE_BILLIONTH)>;

        EXPECT_THAT(ApplyOneBillionthToU8::would_overflow(255), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToU8::would_overflow(1), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToU8::would_overflow(0), IsFalse());
    }

    {
        using ApplyOneBillionthToF = ApplyMagnitudeT<float, decltype(ONE_BILLIONTH)>;

        EXPECT_THAT(ApplyOneBillionthToF::would_overflow(3.402e38f), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToF::would_overflow(1.0f), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToF::would_overflow(0.0f), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToF::would_overflow(-1.0f), IsFalse());
        EXPECT_THAT(ApplyOneBillionthToF::would_overflow(-3.402e38f), IsFalse());
    }
}

TEST(WouldOverflow, UsesNumeratorWhenApplyingRationalMagnitudeToIntegralType) {
    {
        using ApplyTwoThirdsToI32 = ApplyMagnitudeT<int32_t, decltype(mag<2>() / mag<3>())>;

        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(2'147'483'647), IsTrue());
        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(1'073'741'824), IsTrue());

        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(1'073'741'823), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(1), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(0), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(-1), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(-1'073'741'824), IsFalse());

        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(-1'073'741'825), IsTrue());
        EXPECT_THAT(ApplyTwoThirdsToI32::would_overflow(-2'147'483'648), IsTrue());
    }

    {
        using ApplyRoughlyOneThirdToU8 =
            ApplyMagnitudeT<uint8_t, decltype(mag<100'000'000>() / mag<300'000'001>())>;

        ASSERT_THAT((std::is_same<decltype(uint8_t{} * uint8_t{}), int32_t>::value), IsTrue())
            << "This test fails on architectures where `uint8_t` doesn't get promoted to `int32_t`";

        EXPECT_THAT(ApplyRoughlyOneThirdToU8::would_overflow(255), IsTrue());
        EXPECT_THAT(ApplyRoughlyOneThirdToU8::would_overflow(22), IsTrue());

        EXPECT_THAT(ApplyRoughlyOneThirdToU8::would_overflow(21), IsFalse());
        EXPECT_THAT(ApplyRoughlyOneThirdToU8::would_overflow(1), IsFalse());
        EXPECT_THAT(ApplyRoughlyOneThirdToU8::would_overflow(0), IsFalse());
    }
}

TEST(WouldOverflow, UsesFullValueWhenApplyingRationalMagnitudeToFloatingPointType) {
    {
        using ApplyThreeHalvesToF = ApplyMagnitudeT<float, decltype(mag<3>() / mag<2>())>;

        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(3.402e38f), IsTrue());
        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(2.269e38f), IsTrue());

        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(2.268e38f), IsFalse());
        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(1.0f), IsFalse());
        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(0.0f), IsFalse());
        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(-1.0f), IsFalse());
        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(-2.268e38f), IsFalse());

        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(-2.269e38f), IsTrue());
        EXPECT_THAT(ApplyThreeHalvesToF::would_overflow(-3.402e38f), IsTrue());
    }

    {
        using ApplyTwoThirdsToF = ApplyMagnitudeT<float, decltype(mag<2>() / mag<3>())>;

        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(3.402e38f), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(2.268e38f), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(2.267e38f), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(1.0f), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(0.0f), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(-1.0f), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(-2.267e38f), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(-2.268e38f), IsFalse());
        EXPECT_THAT(ApplyTwoThirdsToF::would_overflow(-3.402e38f), IsFalse());
    }
}

TEST(WouldOverflow, UsesFullValueWhenApplyingIrrationalMagnitude) {
    using ApplyPiByTwoToF = ApplyMagnitudeT<float, decltype(PI / mag<2>())>;

    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(3.402e38f), IsTrue());
    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(2.167e38f), IsTrue());

    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(2.166e38f), IsFalse());
    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(1.0f), IsFalse());
    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(0.0f), IsFalse());
    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(-1.0f), IsFalse());
    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(-2.166e38f), IsFalse());

    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(-2.167e38f), IsTrue());
    EXPECT_THAT(ApplyPiByTwoToF::would_overflow(-3.402e38f), IsTrue());
}

TEST(WouldTruncate, AlwaysFalseForIntegerMultiply) {
    auto ONE_BILLION = pow<9>(mag<10>());

    {
        using ApplyOneBillionToI32 = ApplyMagnitudeT<int32_t, decltype(ONE_BILLION)>;

        EXPECT_THAT(ApplyOneBillionToI32::would_truncate(2'147'483'647), IsFalse());
        EXPECT_THAT(ApplyOneBillionToI32::would_truncate(1), IsFalse());
        EXPECT_THAT(ApplyOneBillionToI32::would_truncate(0), IsFalse());
        EXPECT_THAT(ApplyOneBillionToI32::would_truncate(-1), IsFalse());
        EXPECT_THAT(ApplyOneBillionToI32::would_truncate(-2'147'483'648), IsFalse());
    }

    {
        using ApplyOneBillionToU8 = ApplyMagnitudeT<uint8_t, decltype(ONE_BILLION)>;

        EXPECT_THAT(ApplyOneBillionToU8::would_truncate(255), IsFalse());
        EXPECT_THAT(ApplyOneBillionToU8::would_truncate(1), IsFalse());
        EXPECT_THAT(ApplyOneBillionToU8::would_truncate(0), IsFalse());
    }

    {
        using ApplyOneBillionToF = ApplyMagnitudeT<float, decltype(ONE_BILLION)>;

        EXPECT_THAT(ApplyOneBillionToF::would_truncate(3.402e38f), IsFalse());
        EXPECT_THAT(ApplyOneBillionToF::would_truncate(1.0f), IsFalse());
        EXPECT_THAT(ApplyOneBillionToF::would_truncate(0.0f), IsFalse());
        EXPECT_THAT(ApplyOneBillionToF::would_truncate(-1.0f), IsFalse());
        EXPECT_THAT(ApplyOneBillionToF::would_truncate(-3.402e38f), IsFalse());
    }
}

TEST(WouldTruncate, UsesModWhenDividingIntegralTypeByInteger) {
    auto ONE_SEVEN_HUNDREDTH = ONE / mag<700>();

    {
        using ApplyOneSevenHundredthToI32 = ApplyMagnitudeT<int32_t, decltype(ONE_SEVEN_HUNDREDTH)>;

        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(701), IsTrue());
        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(700), IsFalse());
        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(699), IsTrue());

        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(1), IsTrue());
        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(0), IsFalse());
        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(-1), IsTrue());

        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(-699), IsTrue());
        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(-700), IsFalse());
        EXPECT_THAT(ApplyOneSevenHundredthToI32::would_truncate(-701), IsTrue());
    }

    {
        using ApplyOneSevenHundredthToU8 = ApplyMagnitudeT<uint8_t, decltype(ONE_SEVEN_HUNDREDTH)>;

        EXPECT_THAT(ApplyOneSevenHundredthToU8::would_truncate(255), IsTrue());
        EXPECT_THAT(ApplyOneSevenHundredthToU8::would_truncate(254), IsTrue());
        EXPECT_THAT(ApplyOneSevenHundredthToU8::would_truncate(1), IsTrue());

        EXPECT_THAT(ApplyOneSevenHundredthToU8::would_truncate(0), IsFalse());
    }
}

TEST(WouldTruncate, AlwaysFalseWhenDividingFloatingPointTypeByInteger) {
    using ApplyOneSevenHundredthToF = ApplyMagnitudeT<float, decltype(ONE / mag<700>())>;

    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(3.402e38f), IsFalse());

    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(701.0f), IsFalse());
    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(700.0f), IsFalse());
    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(699.0f), IsFalse());

    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(1.0f), IsFalse());
    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(0.0f), IsFalse());
    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(-1.0f), IsFalse());

    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(-699.0f), IsFalse());
    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(-700.0f), IsFalse());
    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(-701.0f), IsFalse());

    EXPECT_THAT(ApplyOneSevenHundredthToF::would_truncate(-3.402e38f), IsFalse());
}

TEST(WouldTruncate, UsesDenominatorWhenApplyingRationalMagnitudeToIntegralType) {
    auto TWO_FIFTHS = mag<2>() / mag<5>();

    {
        using ApplyTwoFifthsToI32 = ApplyMagnitudeT<int32_t, decltype(TWO_FIFTHS)>;

        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(2'147'483'646), IsTrue());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(2'147'483'645), IsFalse());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(2'147'483'644), IsTrue());

        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(6), IsTrue());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(5), IsFalse());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(4), IsTrue());

        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(1), IsTrue());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(0), IsFalse());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(-1), IsTrue());

        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(-4), IsTrue());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(-5), IsFalse());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(-6), IsTrue());

        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(-2'147'483'644), IsTrue());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(-2'147'483'645), IsFalse());
        EXPECT_THAT(ApplyTwoFifthsToI32::would_truncate(-2'147'483'646), IsTrue());
    }

    {
        using ApplyTwoFifthsToU8 = ApplyMagnitudeT<uint8_t, decltype(TWO_FIFTHS)>;

        EXPECT_THAT(ApplyTwoFifthsToU8::would_truncate(255), IsFalse());
        EXPECT_THAT(ApplyTwoFifthsToU8::would_truncate(254), IsTrue());

        EXPECT_THAT(ApplyTwoFifthsToU8::would_truncate(6), IsTrue());
        EXPECT_THAT(ApplyTwoFifthsToU8::would_truncate(5), IsFalse());
        EXPECT_THAT(ApplyTwoFifthsToU8::would_truncate(4), IsTrue());

        EXPECT_THAT(ApplyTwoFifthsToU8::would_truncate(1), IsTrue());
        EXPECT_THAT(ApplyTwoFifthsToU8::would_truncate(0), IsFalse());
    }
}

TEST(WouldTruncate, AlwaysFalseWhenApplyingRationalMagnitudeToFloatingPointType) {
    using ApplyTwoFifthsToF = ApplyMagnitudeT<float, decltype(mag<2>() / mag<5>())>;

    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(3.402e38f), IsFalse());

    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(6.0f), IsFalse());
    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(5.0f), IsFalse());
    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(4.0f), IsFalse());

    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(1.0f), IsFalse());
    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(0.0f), IsFalse());
    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(-1.0f), IsFalse());

    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(-4.0f), IsFalse());
    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(-5.0f), IsFalse());
    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(-6.0f), IsFalse());

    EXPECT_THAT(ApplyTwoFifthsToF::would_truncate(-3.402e38f), IsFalse());
}

TEST(WouldTruncate, AlwaysFalseWhenApplyingIrrationalMagnitude) {
    using ApplyPiByTwoToF = ApplyMagnitudeT<float, decltype(PI / mag<2>())>;

    EXPECT_THAT(ApplyPiByTwoToF::would_truncate(3.402e38f), IsFalse());

    EXPECT_THAT(ApplyPiByTwoToF::would_truncate(1.0f), IsFalse());
    EXPECT_THAT(ApplyPiByTwoToF::would_truncate(0.0f), IsFalse());
    EXPECT_THAT(ApplyPiByTwoToF::would_truncate(-1.0f), IsFalse());

    EXPECT_THAT(ApplyPiByTwoToF::would_truncate(-3.402e38f), IsFalse());
}
}  // namespace detail
}  // namespace au
