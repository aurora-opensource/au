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

}  // namespace detail
}  // namespace au
