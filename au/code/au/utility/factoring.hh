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

#pragma once

#include <cstdint>

#include "au/utility/probable_primes.hh"

namespace au {
namespace detail {

// Check whether a number is prime.
constexpr bool is_prime(std::uintmax_t n) {
    static_assert(sizeof(std::uintmax_t) <= sizeof(std::uint64_t),
                  "Baillie-PSW only strictly guaranteed for 64-bit numbers");

    return baillie_psw(n) == PrimeResult::PROBABLY_PRIME;
}

template <typename T = void>
struct FirstPrimesImpl {
    static constexpr uint16_t values[] = {
        2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,
        61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131, 137, 139,
        149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
        239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
        347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
        443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541};
    static constexpr std::size_t N = sizeof(values) / sizeof(values[0]);
};
template <typename T>
constexpr uint16_t FirstPrimesImpl<T>::values[];
template <typename T>
constexpr std::size_t FirstPrimesImpl<T>::N;
using FirstPrimes = FirstPrimesImpl<>;

// Find the smallest factor which divides n.
//
// Undefined unless (n > 1).
constexpr std::uintmax_t find_prime_factor(std::uintmax_t n) {
    const auto &first_primes = FirstPrimes::values;
    const auto &n_primes = FirstPrimes::N;

    // First, do trial division against the first N primes.
    for (const auto &p : first_primes) {
        if (n % p == 0u) {
            return p;
        }

        if (p * p > n) {
            return n;
        }
    }

    // If we got this far, and haven't found a factor nor terminated, do a fast primality check.
    if (is_prime(n)) {
        return n;
    }

    // If we're here, we know `n` is composite, so continue with trial division for all odd numbers.
    std::uintmax_t factor = first_primes[n_primes - 1u] + 2u;
    while (factor * factor <= n) {
        if (n % factor == 0u) {
            return factor;
        }
        factor += 2u;
    }

    return n;
}

// Find the largest power of `factor` which divides `n`.
//
// Undefined unless n > 0, and factor > 1.
constexpr std::uintmax_t multiplicity(std::uintmax_t factor, std::uintmax_t n) {
    std::uintmax_t m = 0u;
    while (n % factor == 0u) {
        ++m;
        n /= factor;
    }
    return m;
}

template <typename T>
constexpr T square(T n) {
    return n * n;
}

// Raise a base to an integer power.
//
// Undefined behavior if base^exp overflows T.
template <typename T>
constexpr T int_pow(T base, std::uintmax_t exp) {
    if (exp == 0u) {
        return T{1};
    }

    if (exp % 2u == 1u) {
        return base * int_pow(base, exp - 1u);
    }

    return square(int_pow(base, exp / 2u));
}

}  // namespace detail
}  // namespace au
