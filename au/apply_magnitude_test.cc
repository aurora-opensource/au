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

#include "au/apply_magnitude.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

using ::testing::ElementsAreArray;
using ::testing::Not;

namespace au {
namespace detail {
namespace {
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

TEST(CategorizeMagnitude, FindsIntegerMultiplyInstances) {
    EXPECT_EQ(categorize_magnitude(mag<2>()), ApplyAs::INTEGER_MULTIPLY);
    EXPECT_EQ(categorize_magnitude(mag<35>()), ApplyAs::INTEGER_MULTIPLY);

    EXPECT_EQ(categorize_magnitude(mag<35>() / mag<7>()), ApplyAs::INTEGER_MULTIPLY);
}

TEST(CategorizeMagnitude, FindsIntegerDivideInstances) {
    EXPECT_EQ(categorize_magnitude(ONE / mag<2>()), ApplyAs::INTEGER_DIVIDE);
    EXPECT_EQ(categorize_magnitude(ONE / mag<35>()), ApplyAs::INTEGER_DIVIDE);

    EXPECT_EQ(categorize_magnitude(mag<7>() / mag<35>()), ApplyAs::INTEGER_DIVIDE);
}

TEST(CategorizeMagnitude, FindsRationalMultiplyInstances) {
    EXPECT_EQ(categorize_magnitude(mag<5>() / mag<2>()), ApplyAs::RATIONAL_MULTIPLY);
}

TEST(CategorizeMagnitude, FindsIrrationalMultiplyInstances) {
    EXPECT_EQ(categorize_magnitude(sqrt(mag<2>())), ApplyAs::IRRATIONAL_MULTIPLY);
    EXPECT_EQ(categorize_magnitude(PI), ApplyAs::IRRATIONAL_MULTIPLY);
}

TEST(ApplyMagnitude, MultipliesForIntegerMultiply) {
    constexpr auto m = mag<25>();
    ASSERT_EQ(categorize_magnitude(m), ApplyAs::INTEGER_MULTIPLY);

    EXPECT_THAT(apply_magnitude(4, m), SameTypeAndValue(100));
    EXPECT_THAT(apply_magnitude(4.0f, m), SameTypeAndValue(100.0f));
}

TEST(ApplyMagnitude, DividesForIntegerDivide) {
    constexpr auto one_thirteenth = ONE / mag<13>();
    ASSERT_EQ(categorize_magnitude(one_thirteenth), ApplyAs::INTEGER_DIVIDE);

    // This test would fail if our implementation multiplied by the float representation of (1/13),
    // instead of dividing by 13, under the hood.
    for (const auto &i : first_n_positive_values<float>(100u)) {
        EXPECT_THAT(apply_magnitude(i * 13, one_thirteenth), SameTypeAndValue(i));
    }
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
    ASSERT_EQ(categorize_magnitude(three_halves), ApplyAs::RATIONAL_MULTIPLY);

    EXPECT_THAT(apply_magnitude(5, three_halves), SameTypeAndValue(7));
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
    ASSERT_EQ(categorize_magnitude(two_thirteenths), ApplyAs::RATIONAL_MULTIPLY);

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
    ASSERT_EQ(categorize_magnitude(PI), ApplyAs::IRRATIONAL_MULTIPLY);
    EXPECT_THAT(apply_magnitude(2.0f, PI), SameTypeAndValue(2.0f * static_cast<float>(M_PI)));
}

TEST(WouldOverflow, HasCorrectBoundariesForIntegerMultiply) {
    auto ONE_BILLION = pow<9>(mag<10>());

    {
        using ApplyOneBillionToI32 = ApplyMagnitudeT<int32_t, decltype(ONE_BILLION)>;

        EXPECT_TRUE(ApplyOneBillionToI32::would_overflow(3));

        EXPECT_FALSE(ApplyOneBillionToI32::would_overflow(2));
        EXPECT_FALSE(ApplyOneBillionToI32::would_overflow(0));
        EXPECT_FALSE(ApplyOneBillionToI32::would_overflow(-2));

        EXPECT_TRUE(ApplyOneBillionToI32::would_overflow(-3));
    }

    {
        using ApplyOneBillionToU8 = ApplyMagnitudeT<uint8_t, decltype(ONE_BILLION)>;

        EXPECT_TRUE(ApplyOneBillionToU8::would_overflow(1));

        EXPECT_FALSE(ApplyOneBillionToU8::would_overflow(0));
    }

    {
        using ApplyOneBillionToF = ApplyMagnitudeT<float, decltype(ONE_BILLION)>;

        EXPECT_TRUE(ApplyOneBillionToF::would_overflow(3.403e29f));

        EXPECT_FALSE(ApplyOneBillionToF::would_overflow(3.402e29f));
        EXPECT_FALSE(ApplyOneBillionToF::would_overflow(-3.402e29f));

        EXPECT_TRUE(ApplyOneBillionToF::would_overflow(-3.403e29f));
    }
}

TEST(WouldOverflow, AlwaysFalseForIntegerDivide) {
    auto ONE_BILLIONTH = ONE / pow<9>(mag<10>());

    {
        using ApplyOneBillionthToI32 = ApplyMagnitudeT<int32_t, decltype(ONE_BILLIONTH)>;

        EXPECT_FALSE(ApplyOneBillionthToI32::would_overflow(2'147'483'647));
        EXPECT_FALSE(ApplyOneBillionthToI32::would_overflow(1));
        EXPECT_FALSE(ApplyOneBillionthToI32::would_overflow(0));
        EXPECT_FALSE(ApplyOneBillionthToI32::would_overflow(-1));
        EXPECT_FALSE(ApplyOneBillionthToI32::would_overflow(-2'147'483'648));
    }

    {
        using ApplyOneBillionthToU8 = ApplyMagnitudeT<uint8_t, decltype(ONE_BILLIONTH)>;

        EXPECT_FALSE(ApplyOneBillionthToU8::would_overflow(255));
        EXPECT_FALSE(ApplyOneBillionthToU8::would_overflow(1));
        EXPECT_FALSE(ApplyOneBillionthToU8::would_overflow(0));
    }

    {
        using ApplyOneBillionthToF = ApplyMagnitudeT<float, decltype(ONE_BILLIONTH)>;

        EXPECT_FALSE(ApplyOneBillionthToF::would_overflow(3.402e38f));
        EXPECT_FALSE(ApplyOneBillionthToF::would_overflow(1.0f));
        EXPECT_FALSE(ApplyOneBillionthToF::would_overflow(0.0f));
        EXPECT_FALSE(ApplyOneBillionthToF::would_overflow(-1.0f));
        EXPECT_FALSE(ApplyOneBillionthToF::would_overflow(-3.402e38f));
    }
}

TEST(WouldOverflow, UsesNumeratorWhenApplyingRationalMagnitudeToIntegralType) {
    {
        using ApplyTwoThirdsToI32 = ApplyMagnitudeT<int32_t, decltype(mag<2>() / mag<3>())>;

        EXPECT_TRUE(ApplyTwoThirdsToI32::would_overflow(2'147'483'647));
        EXPECT_TRUE(ApplyTwoThirdsToI32::would_overflow(1'073'741'824));

        EXPECT_FALSE(ApplyTwoThirdsToI32::would_overflow(1'073'741'823));
        EXPECT_FALSE(ApplyTwoThirdsToI32::would_overflow(1));
        EXPECT_FALSE(ApplyTwoThirdsToI32::would_overflow(0));
        EXPECT_FALSE(ApplyTwoThirdsToI32::would_overflow(-1));
        EXPECT_FALSE(ApplyTwoThirdsToI32::would_overflow(-1'073'741'824));

        EXPECT_TRUE(ApplyTwoThirdsToI32::would_overflow(-1'073'741'825));
        EXPECT_TRUE(ApplyTwoThirdsToI32::would_overflow(-2'147'483'648));
    }

    {
        using ApplyRoughlyOneThirdToU8 =
            ApplyMagnitudeT<uint8_t, decltype(mag<100'000'000>() / mag<300'000'001>())>;

        ASSERT_TRUE((std::is_same<decltype(uint8_t{} * uint8_t{}), int32_t>::value))
            << "This test fails on architectures where `uint8_t` doesn't get promoted to `int32_t`";

        EXPECT_TRUE(ApplyRoughlyOneThirdToU8::would_overflow(255));
        EXPECT_TRUE(ApplyRoughlyOneThirdToU8::would_overflow(22));

        EXPECT_FALSE(ApplyRoughlyOneThirdToU8::would_overflow(21));
        EXPECT_FALSE(ApplyRoughlyOneThirdToU8::would_overflow(1));
        EXPECT_FALSE(ApplyRoughlyOneThirdToU8::would_overflow(0));
    }
}

TEST(WouldOverflow, UsesFullValueWhenApplyingRationalMagnitudeToFloatingPointType) {
    {
        using ApplyThreeHalvesToF = ApplyMagnitudeT<float, decltype(mag<3>() / mag<2>())>;

        EXPECT_TRUE(ApplyThreeHalvesToF::would_overflow(3.402e38f));
        EXPECT_TRUE(ApplyThreeHalvesToF::would_overflow(2.269e38f));

        EXPECT_FALSE(ApplyThreeHalvesToF::would_overflow(2.268e38f));
        EXPECT_FALSE(ApplyThreeHalvesToF::would_overflow(1.0f));
        EXPECT_FALSE(ApplyThreeHalvesToF::would_overflow(0.0f));
        EXPECT_FALSE(ApplyThreeHalvesToF::would_overflow(-1.0f));
        EXPECT_FALSE(ApplyThreeHalvesToF::would_overflow(-2.268e38f));

        EXPECT_TRUE(ApplyThreeHalvesToF::would_overflow(-2.269e38f));
        EXPECT_TRUE(ApplyThreeHalvesToF::would_overflow(-3.402e38f));
    }

    {
        using ApplyTwoThirdsToF = ApplyMagnitudeT<float, decltype(mag<2>() / mag<3>())>;

        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(3.402e38f));
        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(2.268e38f));
        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(2.267e38f));
        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(1.0f));
        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(0.0f));
        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(-1.0f));
        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(-2.267e38f));
        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(-2.268e38f));
        EXPECT_FALSE(ApplyTwoThirdsToF::would_overflow(-3.402e38f));
    }
}

TEST(WouldOverflow, UsesFullValueWhenApplyingIrrationalMagnitude) {
    using ApplyPiByTwoToF = ApplyMagnitudeT<float, decltype(PI / mag<2>())>;

    EXPECT_TRUE(ApplyPiByTwoToF::would_overflow(3.402e38f));
    EXPECT_TRUE(ApplyPiByTwoToF::would_overflow(2.167e38f));

    EXPECT_FALSE(ApplyPiByTwoToF::would_overflow(2.166e38f));
    EXPECT_FALSE(ApplyPiByTwoToF::would_overflow(1.0f));
    EXPECT_FALSE(ApplyPiByTwoToF::would_overflow(0.0f));
    EXPECT_FALSE(ApplyPiByTwoToF::would_overflow(-1.0f));
    EXPECT_FALSE(ApplyPiByTwoToF::would_overflow(-2.166e38f));

    EXPECT_TRUE(ApplyPiByTwoToF::would_overflow(-2.167e38f));
    EXPECT_TRUE(ApplyPiByTwoToF::would_overflow(-3.402e38f));
}

TEST(WouldTruncate, AlwaysFalseForIntegerMultiply) {
    auto ONE_BILLION = pow<9>(mag<10>());

    {
        using ApplyOneBillionToI32 = ApplyMagnitudeT<int32_t, decltype(ONE_BILLION)>;

        EXPECT_FALSE(ApplyOneBillionToI32::would_truncate(2'147'483'647));
        EXPECT_FALSE(ApplyOneBillionToI32::would_truncate(1));
        EXPECT_FALSE(ApplyOneBillionToI32::would_truncate(0));
        EXPECT_FALSE(ApplyOneBillionToI32::would_truncate(-1));
        EXPECT_FALSE(ApplyOneBillionToI32::would_truncate(-2'147'483'648));
    }

    {
        using ApplyOneBillionToU8 = ApplyMagnitudeT<uint8_t, decltype(ONE_BILLION)>;

        EXPECT_FALSE(ApplyOneBillionToU8::would_truncate(255));
        EXPECT_FALSE(ApplyOneBillionToU8::would_truncate(1));
        EXPECT_FALSE(ApplyOneBillionToU8::would_truncate(0));
    }

    {
        using ApplyOneBillionToF = ApplyMagnitudeT<float, decltype(ONE_BILLION)>;

        EXPECT_FALSE(ApplyOneBillionToF::would_truncate(3.402e38f));
        EXPECT_FALSE(ApplyOneBillionToF::would_truncate(1.0f));
        EXPECT_FALSE(ApplyOneBillionToF::would_truncate(0.0f));
        EXPECT_FALSE(ApplyOneBillionToF::would_truncate(-1.0f));
        EXPECT_FALSE(ApplyOneBillionToF::would_truncate(-3.402e38f));
    }
}

TEST(WouldTruncate, UsesModWhenDividingIntegralTypeByInteger) {
    auto ONE_SEVEN_HUNDREDTH = ONE / mag<700>();

    {
        using ApplyOneSevenHundredthToI32 = ApplyMagnitudeT<int32_t, decltype(ONE_SEVEN_HUNDREDTH)>;

        EXPECT_TRUE(ApplyOneSevenHundredthToI32::would_truncate(701));
        EXPECT_FALSE(ApplyOneSevenHundredthToI32::would_truncate(700));
        EXPECT_TRUE(ApplyOneSevenHundredthToI32::would_truncate(699));

        EXPECT_TRUE(ApplyOneSevenHundredthToI32::would_truncate(1));
        EXPECT_FALSE(ApplyOneSevenHundredthToI32::would_truncate(0));
        EXPECT_TRUE(ApplyOneSevenHundredthToI32::would_truncate(-1));

        EXPECT_TRUE(ApplyOneSevenHundredthToI32::would_truncate(-699));
        EXPECT_FALSE(ApplyOneSevenHundredthToI32::would_truncate(-700));
        EXPECT_TRUE(ApplyOneSevenHundredthToI32::would_truncate(-701));
    }

    {
        using ApplyOneSevenHundredthToU8 = ApplyMagnitudeT<uint8_t, decltype(ONE_SEVEN_HUNDREDTH)>;

        EXPECT_TRUE(ApplyOneSevenHundredthToU8::would_truncate(255));
        EXPECT_TRUE(ApplyOneSevenHundredthToU8::would_truncate(254));
        EXPECT_TRUE(ApplyOneSevenHundredthToU8::would_truncate(1));

        EXPECT_FALSE(ApplyOneSevenHundredthToU8::would_truncate(0));
    }
}

TEST(WouldTruncate, AlwaysFalseWhenDividingFloatingPointTypeByInteger) {
    using ApplyOneSevenHundredthToF = ApplyMagnitudeT<float, decltype(ONE / mag<700>())>;

    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(3.402e38f));

    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(701.0f));
    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(700.0f));
    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(699.0f));

    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(1.0f));
    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(0.0f));
    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(-1.0f));

    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(-699.0f));
    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(-700.0f));
    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(-701.0f));

    EXPECT_FALSE(ApplyOneSevenHundredthToF::would_truncate(-3.402e38f));
}

TEST(WouldTruncate, UsesDenominatorWhenApplyingRationalMagnitudeToIntegralType) {
    auto TWO_FIFTHS = mag<2>() / mag<5>();

    {
        using ApplyTwoFifthsToI32 = ApplyMagnitudeT<int32_t, decltype(TWO_FIFTHS)>;

        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(2'147'483'646));
        EXPECT_FALSE(ApplyTwoFifthsToI32::would_truncate(2'147'483'645));
        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(2'147'483'644));

        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(6));
        EXPECT_FALSE(ApplyTwoFifthsToI32::would_truncate(5));
        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(4));

        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(1));
        EXPECT_FALSE(ApplyTwoFifthsToI32::would_truncate(0));
        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(-1));

        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(-4));
        EXPECT_FALSE(ApplyTwoFifthsToI32::would_truncate(-5));
        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(-6));

        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(-2'147'483'644));
        EXPECT_FALSE(ApplyTwoFifthsToI32::would_truncate(-2'147'483'645));
        EXPECT_TRUE(ApplyTwoFifthsToI32::would_truncate(-2'147'483'646));
    }

    {
        using ApplyTwoFifthsToU8 = ApplyMagnitudeT<uint8_t, decltype(TWO_FIFTHS)>;

        EXPECT_FALSE(ApplyTwoFifthsToU8::would_truncate(255));
        EXPECT_TRUE(ApplyTwoFifthsToU8::would_truncate(254));

        EXPECT_TRUE(ApplyTwoFifthsToU8::would_truncate(6));
        EXPECT_FALSE(ApplyTwoFifthsToU8::would_truncate(5));
        EXPECT_TRUE(ApplyTwoFifthsToU8::would_truncate(4));

        EXPECT_TRUE(ApplyTwoFifthsToU8::would_truncate(1));
        EXPECT_FALSE(ApplyTwoFifthsToU8::would_truncate(0));
    }
}

TEST(WouldTruncate, AlwaysFalseWhenApplyingRationalMagnitudeToFloatingPointType) {
    using ApplyTwoFifthsToF = ApplyMagnitudeT<float, decltype(mag<2>() / mag<5>())>;

    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(3.402e38f));

    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(6.0f));
    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(5.0f));
    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(4.0f));

    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(1.0f));
    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(0.0f));
    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(-1.0f));

    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(-4.0f));
    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(-5.0f));
    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(-6.0f));

    EXPECT_FALSE(ApplyTwoFifthsToF::would_truncate(-3.402e38f));
}

TEST(WouldTruncate, AlwaysFalseWhenApplyingIrrationalMagnitude) {
    using ApplyPiByTwoToF = ApplyMagnitudeT<float, decltype(PI / mag<2>())>;

    EXPECT_FALSE(ApplyPiByTwoToF::would_truncate(3.402e38f));

    EXPECT_FALSE(ApplyPiByTwoToF::would_truncate(1.0f));
    EXPECT_FALSE(ApplyPiByTwoToF::would_truncate(0.0f));
    EXPECT_FALSE(ApplyPiByTwoToF::would_truncate(-1.0f));

    EXPECT_FALSE(ApplyPiByTwoToF::would_truncate(-3.402e38f));
}
}  // namespace detail
}  // namespace au
