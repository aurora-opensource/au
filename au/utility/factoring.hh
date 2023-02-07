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

namespace au {
namespace detail {

// Find the smallest factor which divides n.
//
// Undefined unless (n > 1).
constexpr std::uintmax_t find_first_factor(std::uintmax_t n) {
    if (n % 2u == 0u) {
        return 2u;
    }

    std::uintmax_t factor = 3u;
    while (factor * factor <= n) {
        if (n % factor == 0u) {
            return factor;
        }
        factor += 2u;
    }

    return n;
}

// Check whether a number is prime.
constexpr bool is_prime(std::uintmax_t n) { return (n > 1) && (find_first_factor(n) == n); }

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
