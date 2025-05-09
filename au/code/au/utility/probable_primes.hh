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

#include <cmath>

#include "au/utility/mod.hh"

namespace au {
namespace auimpl {

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

//
// Test whether the number is a perfect square.
//
constexpr bool is_perfect_square(uint64_t n) {
    if (n < 2u) {
        return true;
    }

    uint64_t prev = n / 2u;
    while (true) {
        const uint64_t curr = (prev + n / prev) / 2u;
        if (curr * curr == n) {
            return true;
        }
        if (curr >= prev) {
            return false;
        }
        prev = curr;
    }
}

constexpr uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0u) {
        const auto remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

// Map `true` onto `1`, and `false` onto `0`.
//
// The conversions `true` -> `1` and `false` -> `0` are guaranteed by the standard.  This is a
// branchless implementation, which should generally be faster.
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
    int result = start;

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

// The "D" parameter in the Strong Lucas probable prime test.
//
// Default construction produces the first value to try according to Selfridge's parameter
// selection.  Calling `increment()` on this will successively produce the next parameter to try.
struct LucasDParameter {
    uint64_t mag = 5u;
    bool is_positive = true;

    friend constexpr int as_int(const LucasDParameter &D) {
        return bool_sign(D.is_positive) * static_cast<int>(D.mag);
    }
    friend constexpr void increment(LucasDParameter &D) {
        D.mag += 2u;
        D.is_positive = !D.is_positive;
    }
};

//
// The first `D` in the infinite sequence {5, -7, 9, -11, ...} whose Jacobi symbol is (-1) is the
// `D` we want to use for the Strong Lucas Probable Prime test.
//
// Requires that `n` is *not* a perfect square.
//
constexpr LucasDParameter find_first_D_with_jacobi_symbol_neg_one(uint64_t n) {
    LucasDParameter D{};
    while (jacobi_symbol(as_int(D), n) != -1) {
        increment(D);
    }
    return D;
}

//
// Elements of the Lucas sequence.
//
// The default values give the first element (i.e., k=1) of the sequence.
//
struct LucasSequenceElement {
    uint64_t U = 1u;
    uint64_t V = 1u;
};

// Produce the Lucas element whose index is twice the input element's index.
constexpr LucasSequenceElement double_strong_lucas_index(const LucasSequenceElement &element,
                                                         uint64_t n,
                                                         LucasDParameter D) {
    const auto &U = element.U;
    const auto &V = element.V;

    uint64_t V_squared = mul_mod(V, V, n);
    uint64_t D_U_squared = mul_mod(D.mag, mul_mod(U, U, n), n);
    uint64_t V2 =
        D.is_positive ? add_mod(V_squared, D_U_squared, n) : sub_mod(V_squared, D_U_squared, n);
    V2 = half_mod_odd(V2, n);

    return LucasSequenceElement{
        mul_mod(U, V, n),
        V2,
    };
}

// Find the next element in the Lucas sequence, using parameters for strong Lucas probable primes.
constexpr LucasSequenceElement increment_strong_lucas_index(const LucasSequenceElement &element,
                                                            uint64_t n,
                                                            LucasDParameter D) {
    const auto &U = element.U;
    const auto &V = element.V;

    auto U2 = half_mod_odd(add_mod(U, V, n), n);

    const auto D_U = mul_mod(D.mag, U, n);
    auto V2 = D.is_positive ? add_mod(V, D_U, n) : sub_mod(V, D_U, n);
    V2 = half_mod_odd(V2, n);

    return LucasSequenceElement{U2, V2};
}

// Compute the strong Lucas sequence element at index `i`.
constexpr LucasSequenceElement find_strong_lucas_element(uint64_t i,
                                                         uint64_t n,
                                                         LucasDParameter D) {
    LucasSequenceElement element{};

    bool bits[64] = {};
    std::size_t n_bits = 0u;
    while (i > 1u) {
        bits[n_bits++] = (i & 1u);
        i >>= 1;
    }

    for (std::size_t j = n_bits; j > 0u; --j) {
        element = double_strong_lucas_index(element, n, D);
        if (bits[j - 1u]) {
            element = increment_strong_lucas_index(element, n, D);
        }
    }

    return element;
}

//
// Perform a strong Lucas primality test on `n`.
//
constexpr PrimeResult strong_lucas(uint64_t n) {
    if (n < 2u || n % 2u == 0u) {
        return PrimeResult::BAD_INPUT;
    }

    if (is_perfect_square(n)) {
        return PrimeResult::COMPOSITE;
    }

    const auto D = find_first_D_with_jacobi_symbol_neg_one(n);

    const auto params = decompose(n + 1u);
    const auto &s = params.power_of_two;
    const auto &d = params.odd_remainder;

    auto element = find_strong_lucas_element(d, n, D);
    if (element.U == 0u) {
        return PrimeResult::PROBABLY_PRIME;
    }

    for (std::size_t i = 0u; i < s; ++i) {
        if (element.V == 0u) {
            return PrimeResult::PROBABLY_PRIME;
        }
        element = double_strong_lucas_index(element, n, D);
    }

    return PrimeResult::COMPOSITE;
}

//
// Perform the Baillie-PSW test for primality.
//
// Returns `BAD_INPUT` for any number less than 2, `COMPOSITE` for any larger number that is _known_
// to be prime, and `PROBABLY_PRIME` for any larger number that is deemed "probably prime", which
// includes all prime numbers.
//
// Actually, the Baillie-PSW test is known to be completely accurate for all 64-bit numbers;
// therefore, since our input type is `uint64_t`, the output will be `PROBABLY_PRIME` if and only if
// the input is prime.
//
constexpr PrimeResult baillie_psw(uint64_t n) {
    if (n < 2u) {
        return PrimeResult::BAD_INPUT;
    }
    if (n < 4u) {
        return PrimeResult::PROBABLY_PRIME;
    }
    if (n % 2u == 0u) {
        return PrimeResult::COMPOSITE;
    }

    if (miller_rabin(2u, n) == PrimeResult::COMPOSITE) {
        return PrimeResult::COMPOSITE;
    }

    return strong_lucas(n);
}

}  // namespace auimpl
}  // namespace au
