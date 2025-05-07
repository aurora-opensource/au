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

namespace au {

using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Le;

namespace detail {
namespace {

std::uintmax_t cube(std::uintmax_t n) { return n * n * n; }

}  // namespace

TEST(FirstPrimes, HasOnlyPrimesInOrderAndDoesntSkipAny) {
    const auto &first_primes = FirstPrimes::values;
    auto i_prime = 0u;
    for (auto i = 2u; i <= first_primes.back(); ++i) {
        if (i == first_primes[i_prime]) {
            EXPECT_THAT(is_prime(i), IsTrue()) << i;
            ++i_prime;
        } else {
            EXPECT_THAT(is_prime(i), IsFalse()) << i;
        }
    }
}

TEST(FindFactor, ReturnsInputForPrimes) {
    EXPECT_EQ(find_prime_factor(2u), 2u);
    EXPECT_EQ(find_prime_factor(3u), 3u);
    EXPECT_EQ(find_prime_factor(5u), 5u);
    EXPECT_EQ(find_prime_factor(7u), 7u);
    EXPECT_EQ(find_prime_factor(11u), 11u);

    EXPECT_EQ(find_prime_factor(196961u), 196961u);
}

TEST(FindFactor, FindsFactorWhenFirstFactorIsSmall) {
    EXPECT_THAT(find_prime_factor(7u * 11u * 13u), AnyOf(Eq(7u), Eq(11u), Eq(13u)));
    EXPECT_THAT(find_prime_factor(cube(196961u)), 196961u);
}

TEST(FindFactor, CanFactorNumbersWithLargePrimeFactor) {
    // Small prime factors.
    EXPECT_THAT(find_prime_factor(2u * 9'007'199'254'740'881u),
                AnyOf(Eq(2u), Eq(9'007'199'254'740'881u)));
    EXPECT_THAT(find_prime_factor(3u * 9'007'199'254'740'881u),
                AnyOf(Eq(3u), Eq(9'007'199'254'740'881u)));

    constexpr auto LAST_TRIAL_PRIME = FirstPrimes::values.back();

    // Large prime factor, with a number that trial division would find.
    ASSERT_THAT(541u, Le(LAST_TRIAL_PRIME));
    EXPECT_THAT(find_prime_factor(541u * 9'007'199'254'740'881u),
                AnyOf(Eq(541u), Eq(9'007'199'254'740'881u)));

    // Large prime factor higher than what we use for trial division.
    ASSERT_THAT(1999u, Gt(LAST_TRIAL_PRIME));
    EXPECT_THAT(find_prime_factor(1999u * 9'007'199'254'740'881u),
                AnyOf(Eq(1999u), Eq(9'007'199'254'740'881u)));
}

TEST(FindFactor, CanFactorChallengingCompositeNumbers) {
    // For ideas, see numbers in the "best solution" column in the various tables in
    // <https://miller-rabin.appspot.com/>.
    {
        // Also passes for trial division.
        constexpr auto factor = find_prime_factor(7'999'252'175'582'851u);
        EXPECT_THAT(factor, AnyOf(Eq(9'227u), Eq(894'923u), Eq(968'731u)));
    }
    {
        // Fails for trial division: requires Pollard's rho.
        constexpr auto factor = find_prime_factor(55'245'642'489'451u);
        EXPECT_THAT(factor, AnyOf(Eq(3'716'371u), Eq(14'865'481u)));
    }
}

TEST(IsPrime, FalseForLessThan2) {
    EXPECT_THAT(is_prime(0u), IsFalse());
    EXPECT_THAT(is_prime(1u), IsFalse());
}

TEST(IsPrime, FindsPrimes) {
    EXPECT_THAT(is_prime(2u), IsTrue());
    EXPECT_THAT(is_prime(3u), IsTrue());
    EXPECT_THAT(is_prime(4u), IsFalse());
    EXPECT_THAT(is_prime(5u), IsTrue());
    EXPECT_THAT(is_prime(6u), IsFalse());
    EXPECT_THAT(is_prime(7u), IsTrue());
    EXPECT_THAT(is_prime(8u), IsFalse());
    EXPECT_THAT(is_prime(9u), IsFalse());
    EXPECT_THAT(is_prime(10u), IsFalse());
    EXPECT_THAT(is_prime(11u), IsTrue());

    EXPECT_THAT(is_prime(196959u), IsFalse());
    EXPECT_THAT(is_prime(196960u), IsFalse());
    EXPECT_THAT(is_prime(196961u), IsTrue());
    EXPECT_THAT(is_prime(196962u), IsFalse());
}

TEST(IsPrime, CanHandleVeryLargePrimes) {
    for (const auto &p : {
             uint64_t{225'653'407'801u},
             uint64_t{334'524'384'739u},
             uint64_t{9'007'199'254'740'881u},
             uint64_t{18'446'744'073'709'551'557u},
         }) {
        EXPECT_THAT(is_prime(p), IsTrue()) << p;
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
