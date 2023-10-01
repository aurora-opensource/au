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

namespace au {
namespace detail {

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
    for (int i = 1; i < 100; ++i) {
        EXPECT_THAT(apply_magnitude(static_cast<float>(i * 13), one_thirteenth),
                    SameTypeAndValue(static_cast<float>(i)));
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

}  // namespace detail
}  // namespace au
