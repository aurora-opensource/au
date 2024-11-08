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

#pragma once

#include "au/utility/mod.hh"

namespace au {
namespace detail {

//
// The possible results of a probable prime test.
//
enum class PrimeResult {
    COMPOSITE,
    PROBABLY_PRIME,
    BAD_INPUT,
};

//
// Decompose a number by factoring out all powers of 2: `n = 2^power_of_two * odd_remainder`.
//
struct NumberDecomposition {
    uint64_t power_of_two;
    uint64_t odd_remainder;
};

//
// Express any positive `n` as `(2^s * d)`, where `d` is odd.
//
// Preconditions: `n` is positive.
constexpr NumberDecomposition decompose(uint64_t n) {
    NumberDecomposition result{0u, n};
    while (result.odd_remainder % 2u == 0u) {
        result.odd_remainder /= 2u;
        ++result.power_of_two;
    }
    return result;
}

//
// Perform a Miller-Rabin primality test on `n` using base `a`.
//
// Preconditions: `n` is odd, and at least as big as `a + 2`.  Also, `2` is the smallest allowable
// value for `a`.  We will return `BAD_INPUT` if these preconditions are violated.  Otherwise, we
// will return `PROBABLY_PRIME` for all prime inputs, and also all composite inputs which are
// pseudoprime to base `a`, returning `COMPOSITE` for all other inputs (which are definitely known
// to be composite).
//
constexpr PrimeResult miller_rabin(std::size_t a, uint64_t n) {
    if (a < 2u || n < a + 2u || n % 2u == 0u) {
        return PrimeResult::BAD_INPUT;
    }

    const auto params = decompose(n - 1u);
    const auto &s = params.power_of_two;
    const auto &d = params.odd_remainder;

    uint64_t x = pow_mod(a, d, n);
    if (x == 1u) {
        return PrimeResult::PROBABLY_PRIME;
    }

    const auto minus_one = n - 1u;
    for (auto r = 0u; r < s; ++r) {
        if (x == minus_one) {
            return PrimeResult::PROBABLY_PRIME;
        }
        x = mul_mod(x, x, n);
    }
    return PrimeResult::COMPOSITE;
}

constexpr uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0u) {
        const auto remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

// The conversions `true` -> `1` and `false` -> `0` are guaranteed by the standard.
//
// This is a branchless implementation, which should generally be faster.
constexpr int bool_sign(bool x) { return x - (!x); }

//
// The Jacobi symbol (a/n) is defined for odd positive `n` and any integer `a` as the product of the
// Legendre symbols (a/p) for all prime factors `p` of n.  There are several rules that make this
// easier to calculate, including:
//
//  1. (a/n) = (b/n) whenever (a % n) == (b % n).
//
//  2. (2a/n) = (a/n) if n is congruent to 1 or 7 (mod 8), and -(a/n) if n is congruent to 3 or 5.
//
//  3. (1/n) = 1 for all n.
//
//  4. (a/n) = 0 whenever a and n have a nontrivial common factor.
//
//  5. (a/n) = (n/a) * (-1)^x if a and n are both odd, positive, and coprime.  Here, x is 0 if
//     either a or n is congruent to 1 (mod 4), and 1 otherwise.
//
constexpr int jacobi_symbol_positive_numerator(uint64_t a, uint64_t n, int start) {
    int &result = start;

    while (a != 0u) {
        // Handle even numbers in the "numerator".
        const int sign_for_even = bool_sign(n % 8u == 1u || n % 8u == 7u);
        while (a % 2u == 0u) {
            a /= 2u;
            result *= sign_for_even;
        }

        // `jacobi_symbol(1, n)` is `1` for all `n`.
        if (a == 1u) {
            return result;
        }

        // `jacobi_symbol(a, n)` is `0` whenever `a` and `n` have a common factor.
        if (gcd(a, n) != 1u) {
            return 0;
        }

        // At this point, `a` and `n` are odd, positive, and coprime.  We can use the reciprocity
        // relationship to "flip" them, and modular arithmetic to reduce them.

        // First, compute the sign change from the flip.
        result *= bool_sign((a % 4u == 1u) || (n % 4u == 1u));

        // Now, do the flip-and-reduce.
        const uint64_t new_a = n % a;
        n = a;
        a = new_a;
    }
    return 0;
}
constexpr int jacobi_symbol(int64_t raw_a, uint64_t n) {
    // Degenerate case: n = 1.
    if (n == 1u) {
        return 1;
    }

    // Starting conditions: transform `a` to strictly non-negative values, setting `result` to the
    // sign we pick up from this operation (if any).
    int result = bool_sign((raw_a >= 0) || (n % 4u == 1u));
    auto a = static_cast<uint64_t>(raw_a * bool_sign(raw_a >= 0)) % n;

    // Delegate to an implementation which can only handle positive numbers.
    return jacobi_symbol_positive_numerator(a, n, result);
}

}  // namespace detail
}  // namespace au
