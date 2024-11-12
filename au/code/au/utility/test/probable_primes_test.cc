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

#include "au/utility/probable_primes.hh"

#include <array>
#include <string>
#include <type_traits>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::Lt;

namespace au {
namespace detail {
// Make test output for `PrimeResult` easier to read.
std::ostream &operator<<(std::ostream &out, const PrimeResult &m) {
    switch (m) {
        case PrimeResult::COMPOSITE:
            return (out << "COMPOSITE");
        case PrimeResult::PROBABLY_PRIME:
            return (out << "PROBABLY_PRIME");
        case PrimeResult::BAD_INPUT:
            return (out << "BAD_INPUT");
    }
    return out;
}

// We don't need this equality operator in our "real" code, but it's useful for tests.
constexpr bool operator==(const NumberDecomposition &a, const NumberDecomposition &b) {
    return (a.power_of_two == b.power_of_two) && (a.odd_remainder == b.odd_remainder);
}

namespace {

//
// Compute a compile-time array of the first N prime numbers.
//
template <std::size_t N>
std::array<uint64_t, N> first_n_primes() {
    static std::array<uint64_t, N> result;
    result[0u] = 2u;

    uint64_t candidate = 3u;
    for (auto i = 1u; i < N; ++i) {
        for (auto j = 0u; j < i; ++j) {
            const auto &prime = result[j];

            // If we find the candidate is composite, start over with the next candidate.
            if (candidate % prime == 0u) {
                candidate += 2u;
                j = 0u;
                continue;
            }

            // If we've checked all of the primes and found no divisors, then this is the next prime
            // we were looking for.
            if (prime * prime > candidate) {
                result[i] = candidate;
                candidate += 2u;
                break;
            }
        }
    }

    return result;
}

TEST(FirstNPrimes, For1ReturnsSingleSizedArrayContainingTwo) {
    EXPECT_THAT(first_n_primes<1u>(), ElementsAre(2u));
}

TEST(FirstNPrimes, For10ReturnsFirst10Primes) {
    EXPECT_THAT(first_n_primes<10u>(), ElementsAre(2u, 3u, 5u, 7u, 11u, 13u, 17u, 19u, 23u, 29u));
}

TEST(Decompose, ReturnsCorrectValues) {
    EXPECT_THAT(decompose(126u), Eq(NumberDecomposition{1u, 63u}));
    EXPECT_THAT(decompose(127u), Eq(NumberDecomposition{0u, 127u}));
    EXPECT_THAT(decompose(128u), Eq(NumberDecomposition{7u, 1u}));
}

std::vector<uint64_t> miller_rabin_pseudoprimes_to_base_2() {
    // https://oeis.org/A001262
    return {2047u,   3277u,   4033u,   4681u,   8321u,   15841u,  29341u,  42799u,
            49141u,  52633u,  65281u,  74665u,  80581u,  85489u,  88357u,  90751u,
            104653u, 130561u, 196093u, 220729u, 233017u, 252601u, 253241u, 256999u,
            271951u, 280601u, 314821u, 357761u, 390937u, 458989u, 476971u, 486737u};
}

std::vector<uint64_t> miller_rabin_pseudoprimes_to_base_3() {
    // https://oeis.org/A020229
    return {121u,    703u,    1891u,   3281u,   8401u,   8911u,   10585u,  12403u,  16531u,
            18721u,  19345u,  23521u,  31621u,  44287u,  47197u,  55969u,  63139u,  74593u,
            79003u,  82513u,  87913u,  88573u,  97567u,  105163u, 111361u, 112141u, 148417u,
            152551u, 182527u, 188191u, 211411u, 218791u, 221761u, 226801u};
}

TEST(MillerRabin, EvenNumbersAreBadInput) {
    EXPECT_THAT(miller_rabin(2u, 0u), Eq(PrimeResult::BAD_INPUT));
    EXPECT_THAT(miller_rabin(2u, 2u), Eq(PrimeResult::BAD_INPUT));
    EXPECT_THAT(miller_rabin(2u, 4u), Eq(PrimeResult::BAD_INPUT));
    EXPECT_THAT(miller_rabin(2u, 6u), Eq(PrimeResult::BAD_INPUT));
    EXPECT_THAT(miller_rabin(2u, 8u), Eq(PrimeResult::BAD_INPUT));

    EXPECT_THAT(miller_rabin(2u, 123456u), Eq(PrimeResult::BAD_INPUT));
}

TEST(MillerRabin, NumbersLessThanAPlusTwoAreBadInput) {
    ASSERT_THAT(miller_rabin(9u, 11u), Eq(PrimeResult::PROBABLY_PRIME));

    EXPECT_THAT(miller_rabin(10u, 11u), Eq(PrimeResult::BAD_INPUT));
    EXPECT_THAT(miller_rabin(11u, 11u), Eq(PrimeResult::BAD_INPUT));
}

TEST(MillerRabin, MarksEveryPrimeAsProbablyPrime) {
    auto expect_miller_rabin_probably_prime = [](std::size_t a, uint64_t n) {
        const auto result = miller_rabin(a, n);
        const auto expected =
            (n < a + 2u || n % 2u == 0u) ? PrimeResult::BAD_INPUT : PrimeResult::PROBABLY_PRIME;

        EXPECT_THAT(result, Eq(expected)) << "a = " << a << ", n = " << n;
    };

    const auto primes = first_n_primes<3'000u>();
    for (const auto &p : primes) {
        expect_miller_rabin_probably_prime(2u, p);
        expect_miller_rabin_probably_prime(3u, p);
        expect_miller_rabin_probably_prime(4u, p);
        expect_miller_rabin_probably_prime(5u, p);

        expect_miller_rabin_probably_prime(88u, p);
    }
}

TEST(MillerRabin, OddNumberIsProbablyPrimeIffPrimeOrPseudoprime) {
    const auto primes = first_n_primes<3'000u>();
    const auto pseudoprimes = miller_rabin_pseudoprimes_to_base_2();

    // Make sure that we are both _into the regime_ of the pseudoprimes, and that we aren't off the
    // end of it.
    ASSERT_THAT(primes.back(), AllOf(Gt(pseudoprimes.front()), Lt(pseudoprimes.back())));

    std::size_t i_prime = 2u;  // Skip 2 and 3; they're too small for Miller-Rabin.
    std::size_t i_pseudoprime = 0u;
    for (uint64_t n = primes[i_prime]; n <= primes.back(); n += 2u) {
        const auto is_prime = (n == primes[i_prime]);
        if (is_prime) {
            ++i_prime;
        }

        const auto is_pseudoprime = (n == pseudoprimes[i_pseudoprime]);
        if (is_pseudoprime) {
            ++i_pseudoprime;
        }

        const auto expected =
            (is_prime || is_pseudoprime) ? PrimeResult::PROBABLY_PRIME : PrimeResult::COMPOSITE;
        EXPECT_THAT(miller_rabin(2u, n), Eq(expected)) << "n = " << n;
    }
}

TEST(MillerRabin, HasExpectedBase2Pseudoprimes) {
    for (const auto &n : miller_rabin_pseudoprimes_to_base_2()) {
        EXPECT_THAT(miller_rabin(2u, n), Eq(PrimeResult::PROBABLY_PRIME)) << n;
    }
}

TEST(MillerRabin, HasExpectedBase3Pseudoprimes) {
    for (const auto &n : miller_rabin_pseudoprimes_to_base_3()) {
        EXPECT_THAT(miller_rabin(3u, n), Eq(PrimeResult::PROBABLY_PRIME)) << n;
    }
}

TEST(MillerRabin, HandlesExtremelyLargePrimes) {
    for (const auto &base : {2ull, 3ull, 4ull, 5ull, 99ull, 12345ull, 9876543210123456789ull}) {
        EXPECT_THAT(miller_rabin(base, 18'446'744'073'709'551'557u),
                    Eq(PrimeResult::PROBABLY_PRIME));
    }
}

TEST(MillerRabin, SupportsConstexpr) {
    constexpr auto result = miller_rabin(2u, 997u);
    static_assert(result == PrimeResult::PROBABLY_PRIME, "997 is prime");
}

TEST(Gcd, ResultIsAlwaysAFactorAndGCDFindsNoLargerFactor) {
    for (auto i = 0u; i < 500u; ++i) {
        for (auto j = 1u; j < i; ++j) {
            const auto g = gcd(i, j);
            EXPECT_EQ(i % g, 0u);
            EXPECT_EQ(j % g, 0u);

            // Brute force: no larger factors.
            for (auto k = g + 1u; k < j / 2u; ++k) {
                EXPECT_FALSE((i % k == 0u) && (j % k == 0u));
            }
        }
    }
}

TEST(Gcd, HandlesZeroCorrectly) {
    // The usual convention: if one argument is 0, return the other argument.
    EXPECT_EQ(gcd(0u, 0u), 0u);
    EXPECT_EQ(gcd(10u, 0u), 10u);
    EXPECT_EQ(gcd(0u, 10u), 10u);
}

TEST(JacobiSymbol, ZeroWhenCommonFactorExists) {
    for (int i = -20; i <= 20; ++i) {
        for (auto j = 1u; j <= 19u; j += 2u) {
            for (auto factor = 3u; factor < 200u; factor += 2u) {
                // Make sure that `j * factor` is odd, or else the result is undefined.
                EXPECT_EQ(jacobi_symbol(i * static_cast<int>(factor), j * factor), 0)
                    << "jacobi(" << i * static_cast<int>(factor) << ", " << j * factor
                    << ") should be 0";
            }
        }
    }
}

TEST(JacobiSymbol, AlwaysOneWhenFirstInputIsOne) {
    for (auto i = 3u; i < 99u; i += 2u) {
        EXPECT_EQ(jacobi_symbol(1, i), 1) << "jacobi(1, " << i << ") should be 1";
    }
}

TEST(JacobiSymbol, ReproducesExamplesFromWikipedia) {
    // https://en.wikipedia.org/wiki/Jacobi_symbol#Example_of_calculations
    EXPECT_EQ(jacobi_symbol(1001, 9907), -1);

    // https://en.wikipedia.org/wiki/Jacobi_symbol#Primality_testing
    EXPECT_EQ(jacobi_symbol(19, 45), 1);
    EXPECT_EQ(jacobi_symbol(8, 21), -1);
    EXPECT_EQ(jacobi_symbol(5, 21), 1);
}

TEST(BoolSign, ReturnsCorrectValues) {
    EXPECT_EQ(bool_sign(true), 1);
    EXPECT_EQ(bool_sign(false), -1);
}

}  // namespace
}  // namespace detail
}  // namespace au
