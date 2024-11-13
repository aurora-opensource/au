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

#include "au/utility/factoring.hh"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::Gt;
using ::testing::Le;

namespace au {
namespace detail {
namespace {
std::uintmax_t cube(std::uintmax_t n) { return n * n * n; }
}  // namespace

TEST(FirstPrimes, HasOnlyPrimesInOrderAndDoesntSkipAny) {
    const auto &first_primes = FirstPrimes::values;
    const auto &n_primes = FirstPrimes::N;
    auto i_prime = 0u;
    for (auto i = 2u; i <= first_primes[n_primes - 1u]; ++i) {
        if (i == first_primes[i_prime]) {
            EXPECT_TRUE(is_prime(i)) << i;
            ++i_prime;
        } else {
            EXPECT_FALSE(is_prime(i)) << i;
        }
    }
}

TEST(FindFirstFactor, ReturnsInputForPrimes) {
    EXPECT_EQ(find_first_factor(2u), 2u);
    EXPECT_EQ(find_first_factor(3u), 3u);
    EXPECT_EQ(find_first_factor(5u), 5u);
    EXPECT_EQ(find_first_factor(7u), 7u);
    EXPECT_EQ(find_first_factor(11u), 11u);

    EXPECT_EQ(find_first_factor(196961u), 196961u);
}

TEST(FindFirstFactor, FindsFirstFactor) {
    EXPECT_EQ(find_first_factor(7u * 11u * 13u), 7u);
    EXPECT_EQ(find_first_factor(cube(196961u)), 196961u);
}

TEST(FindFirstFactor, CanFactorNumbersWithLargePrimeFactor) {
    // Small prime factors.
    EXPECT_EQ(find_first_factor(2u * 9'007'199'254'740'881u), 2u);
    EXPECT_EQ(find_first_factor(3u * 9'007'199'254'740'881u), 3u);

    constexpr auto LAST_TRIAL_PRIME = FirstPrimes::values[FirstPrimes::N - 1u];

    // Large prime factor from trial division.
    ASSERT_THAT(541u, Le(LAST_TRIAL_PRIME));
    EXPECT_EQ(find_first_factor(541u * 9'007'199'254'740'881u), 541u);

    // Large prime factor higher than what we use for trial division.
    ASSERT_THAT(1999u, Gt(LAST_TRIAL_PRIME));
    EXPECT_EQ(find_first_factor(1999u * 9'007'199'254'740'881u), 1999u);
}

TEST(IsPrime, FalseForLessThan2) {
    EXPECT_FALSE(is_prime(0u));
    EXPECT_FALSE(is_prime(1u));
}

TEST(IsPrime, FindsPrimes) {
    EXPECT_TRUE(is_prime(2u));
    EXPECT_TRUE(is_prime(3u));
    EXPECT_FALSE(is_prime(4u));
    EXPECT_TRUE(is_prime(5u));
    EXPECT_FALSE(is_prime(6u));
    EXPECT_TRUE(is_prime(7u));
    EXPECT_FALSE(is_prime(8u));
    EXPECT_FALSE(is_prime(9u));
    EXPECT_FALSE(is_prime(10u));
    EXPECT_TRUE(is_prime(11u));

    EXPECT_FALSE(is_prime(196959u));
    EXPECT_FALSE(is_prime(196960u));
    EXPECT_TRUE(is_prime(196961u));
    EXPECT_FALSE(is_prime(196962u));
}

TEST(IsPrime, CanHandleVeryLargePrimes) {
    for (const auto &p : {
             uint64_t{225'653'407'801u},
             uint64_t{334'524'384'739u},
             uint64_t{9'007'199'254'740'881u},
             uint64_t{18'446'744'073'709'551'557u},
         }) {
        EXPECT_TRUE(is_prime(p)) << p;
    }
}

TEST(Multiplicity, CountsFactors) {
    constexpr std::uintmax_t n = (2u * 2u * 2u) * (3u) * (5u * 5u);
    EXPECT_EQ(multiplicity(2u, n), 3u);
    EXPECT_EQ(multiplicity(3u, n), 1u);
    EXPECT_EQ(multiplicity(5u, n), 2u);
    EXPECT_EQ(multiplicity(7u, n), 0u);
}

}  // namespace detail
}  // namespace au
