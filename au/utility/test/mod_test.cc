// Copyright 2024 Aurora Operations, Inc.
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

#include "au/utility/mod.hh"

#include <limits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

namespace detail {
namespace {

constexpr auto MAX = std::numeric_limits<uint64_t>::max();

TEST(AddMod, HandlesSimpleCases) {
    EXPECT_THAT(add_mod(1u, 2u, 5u), Eq(3u));
    EXPECT_THAT(add_mod(4u, 4u, 5u), Eq(3u));
}

TEST(AddMod, HandlesVeryLargeNumbers) {
    EXPECT_THAT(add_mod(MAX - 1u, MAX - 2u, MAX), Eq(MAX - 3u));
}

TEST(SubMod, HandlesSimpleCases) {
    EXPECT_THAT(sub_mod(2u, 1u, 5u), Eq(1u));
    EXPECT_THAT(sub_mod(1u, 2u, 5u), Eq(4u));
}

TEST(SubMod, HandlesVeryLargeNumbers) {
    EXPECT_THAT(sub_mod(MAX - 2u, MAX - 1u, MAX), Eq(MAX - 1u));
    EXPECT_THAT(sub_mod(1u, MAX - 1u, MAX), Eq(2u));
}

TEST(MulMod, HandlesSimpleCases) {
    EXPECT_THAT(mul_mod(6u, 7u, 10u), Eq(2u));
    EXPECT_THAT(mul_mod(13u, 11u, 50u), Eq(43u));
}

TEST(MulMod, HandlesHugeNumbers) {
    constexpr auto JUST_UNDER_HALF = MAX / 2u;
    ASSERT_THAT(JUST_UNDER_HALF * 2u + 1u, Eq(MAX));

    EXPECT_THAT(mul_mod(JUST_UNDER_HALF, 10u, MAX), Eq(MAX - 5u));
}

TEST(HalfModOdd, HalvesEvenNumbers) {
    EXPECT_THAT(half_mod_odd(0u, 11u), Eq(0u));
    EXPECT_THAT(half_mod_odd(10u, 11u), Eq(5u));
}

TEST(HalfModOdd, HalvesSumWithNForOddNumbers) {
    EXPECT_THAT(half_mod_odd(1u, 11u), Eq(6u));
    EXPECT_THAT(half_mod_odd(9u, 11u), Eq(10u));
}

TEST(HalfModOdd, SameAsMultiplyingByCeilOfNOver2WhenNIsOdd) {
    // An interesting test case, which helps us make sense of the operation of "dividing by 2" in
    // modular arithmetic.  When `n` is odd, `2` has a multiplicative inverse, so we can understand
    // "dividing by two" in terms of multiplying by this inverse.
    //
    // This fails when `n` is even, but so does dividing by 2 generally.
    //
    // In principle, we could replace our `half_mod_odd` implementation with this, and it would have
    // the same preconditions, but there's a chance it would be less efficient (because `mul_mod`
    // may recurse multiple times).  Also, keeping them separate lets us use this test case as an
    // independent check.
    std::vector<uint64_t> n_values{9u, 11u, 8723493u, MAX};
    for (const auto &n : n_values) {
        const auto half_n = n / 2u + 1u;

        std::vector<uint64_t> x_values{0u, 1u, 2u, (n / 2u), (n / 2u + 1u), (n - 2u), (n - 1u)};
        for (const auto &x : x_values) {
            EXPECT_THAT(half_mod_odd(x, n), Eq(mul_mod(x, half_n, n)));
        }
    }
}

TEST(PowMod, HandlesSimpleCases) {
    auto to_the_eighth = [](uint64_t x) {
        x *= x;
        x *= x;
        x *= x;
        return x;
    };
    EXPECT_THAT(pow_mod(5u, 8u, 9u), Eq(to_the_eighth(5u) % 9u));
}

TEST(PowMod, HandlesNumbersThatWouldOverflow) { EXPECT_THAT(pow_mod(2u, 64u, MAX), Eq(1u)); }

TEST(PowMod, ProducesSameAnswerAsRepeatedModMulForLargeNumbers) {
    const auto x = MAX / 3u * 2u;
    const auto to_pow_2 = mul_mod(x, x, MAX);
    const auto to_pow_4 = mul_mod(to_pow_2, to_pow_2, MAX);
    const auto to_pow_5 = mul_mod(x, to_pow_4, MAX);
    const auto to_pow_10 = mul_mod(to_pow_5, to_pow_5, MAX);
    const auto to_pow_11 = mul_mod(x, to_pow_10, MAX);
    const auto to_pow_22 = mul_mod(to_pow_11, to_pow_11, MAX);
    EXPECT_THAT(pow_mod(x, 22u, MAX), Eq(to_pow_22));
}

}  // namespace
}  // namespace detail
}  // namespace au
