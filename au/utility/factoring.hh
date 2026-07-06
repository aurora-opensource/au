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

#include <array>
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

// Compute the next step for Pollard's rho algorithm factoring `n`, with parameter `t`.
constexpr std::uintmax_t x_squared_plus_t_mod_n(std::uintmax_t x,
                                                std::uintmax_t t,
                                                std::uintmax_t n) {
    return add_mod(mul_mod(x, x, n), t, n);
}

constexpr std::uintmax_t absolute_diff(std::uintmax_t a, std::uintmax_t b) {
    return a > b ? a - b : b - a;
}

// A single attempt at Pollard's rho, using Brent's cycle detection method, with the polynomial
// `x^2 + t`.  Returns a nontrivial factor of `n` on success, or `n` itself if this particular
// parameterization fails to find one (in which case the caller should retry with a different `t`).
//
// To keep the number of (relatively expensive) `gcd` calls small, we accumulate the product of the
// position differences modulo `n` across a batch of steps, and take only a single `gcd` per batch.
// This is the standard batched form of Brent's algorithm; it trades many `gcd` calls for many
// (much cheaper) `mul_mod` calls, which is what makes it tractable at compile time.  See
// <https://github.com/aurora-opensource/au/issues/328>.
//
// Precondition: `n` is known to be composite.
constexpr std::uintmax_t pollard_rho_attempt(std::uintmax_t n, std::uintmax_t t) {
    constexpr std::uintmax_t batch_size = 128u;

    std::uintmax_t anchor = 2u;
    std::uintmax_t cursor = 2u;
    std::uintmax_t batch_start = 2u;
    std::uintmax_t diff_product = 1u;
    std::uintmax_t factor = 1u;
    std::uintmax_t segment_length = 1u;

    do {
        anchor = cursor;
        for (std::uintmax_t i = 0u; i < segment_length; ++i) {
            cursor = x_squared_plus_t_mod_n(cursor, t, n);
        }

        std::uintmax_t offset_in_segment = 0u;
        while (offset_in_segment < segment_length && factor == 1u) {
            batch_start = cursor;
            const std::uintmax_t remaining = segment_length - offset_in_segment;
            const std::uintmax_t steps = (batch_size < remaining) ? batch_size : remaining;
            for (std::uintmax_t i = 0u; i < steps; ++i) {
                cursor = x_squared_plus_t_mod_n(cursor, t, n);
                diff_product = mul_mod(diff_product, absolute_diff(anchor, cursor), n);
            }
            factor = gcd(diff_product, n);
            offset_in_segment += batch_size;
        }

        segment_length *= 2u;
    } while (factor == 1u);

    // If the batched product happened to accumulate _all_ of `n`'s factors at once (`factor == n`),
    // the batch hid the real factor.  Recover it by walking the same steps one at a time, taking a
    // `gcd` after each, until we isolate a single nontrivial factor.
    if (factor == n) {
        factor = 1u;
        do {
            batch_start = x_squared_plus_t_mod_n(batch_start, t, n);
            factor = gcd(absolute_diff(anchor, batch_start), n);
        } while (factor == 1u);
    }

    return factor;
}

// Pollard's rho algorithm, using Brent's cycle detection method.
//
// Precondition: `n` is known to be composite.
constexpr std::uintmax_t find_pollard_rho_factor(std::uintmax_t n) {
    // The outer loop tries separate _parameterizations_ of Pollard's rho.  We try a finite number
    // of them just to guarantee that we terminate.  But in practice, the vast overwhelming majority
    // will succeed on the first iteration, and we don't expect that any will _ever_ come anywhere
    // _near_ to hitting this limit.
    for (std::uintmax_t t = 1u; t < n / 2u; ++t) {
        const std::uintmax_t factor = pollard_rho_attempt(n, t);
        if (factor != n) {
            return factor;
        }
    }
    // Failure case: we think this should be unreachable (in practice) with any composite `n`.
    return n;
}

template <typename T = void>
struct FirstPrimesImpl {
    static constexpr std::array<uint16_t, 100u> values = {
        2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,
        61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131, 137, 139,
        149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
        239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
        347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
        443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541};
};
template <typename T>
constexpr std::array<uint16_t, 100u> FirstPrimesImpl<T>::values;
using FirstPrimes = FirstPrimesImpl<>;

// Find the smallest factor which divides n.
//
// Undefined unless (n > 1).
constexpr std::uintmax_t find_prime_factor(std::uintmax_t n) {
    // First, do trial division against the first N primes.
    //
    // Note that range-for isn't supported until C++17, so we need to use an index.
    for (auto i = 0u; i < FirstPrimes::values.size(); ++i) {
        const std::uintmax_t p = FirstPrimes::values[i];

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

    auto factor = find_pollard_rho_factor(n);
    while (!is_prime(factor)) {
        factor = find_pollard_rho_factor(factor);
    }
    return factor;
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
