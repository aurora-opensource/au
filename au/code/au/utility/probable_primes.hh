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
// Express any odd `n` as `(2^s + d) + 1`, where `d` is odd.
//
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

}  // namespace detail
}  // namespace au
