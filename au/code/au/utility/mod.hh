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

#include <cstdint>
#include <limits>

namespace au {
namespace detail {

// (a + b) % n
//
// Precondition: (a < n).
// Precondition: (b < n).
constexpr uint64_t add_mod(uint64_t a, uint64_t b, uint64_t n) {
    if (a >= n - b) {
        return a - (n - b);
    } else {
        return a + b;
    }
}

// (a - b) % n
//
// Precondition: (a < n).
// Precondition: (b < n).
constexpr uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t n) {
    if (a >= b) {
        return a - b;
    } else {
        return n - (b - a);
    }
}

// (a * b) % n
//
// Precondition: (a < n).
// Precondition: (b < n).
constexpr uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t n) {
    // Start by trying the simplest case, where everything "fits".
    if (b == 0u || a < std::numeric_limits<uint64_t>::max() / b) {
        return (a * b) % n;
    }

    // We know the "negative" result is smaller, because we've taken as many copies of `a` as will
    // fit into `n`.  So, do the reduced calculation in "negative space", and then transform the
    // result back at the end.
    uint64_t chunk_size = n / a;
    uint64_t num_chunks = b / chunk_size;
    uint64_t negative_chunk = n - (a * chunk_size);  // == n % a  (but this should be cheaper)
    uint64_t chunk_result = n - mul_mod(negative_chunk, num_chunks, n);

    // Compute the leftover.  (We don't need to recurse, because we know it will fit.)
    uint64_t leftover = b - num_chunks * chunk_size;
    uint64_t leftover_result = (a * leftover) % n;

    return add_mod(chunk_result, leftover_result, n);
}

// (a / 2) % n
//
// Precondition: (a < n).
// Precondition: (n is odd).
constexpr uint64_t half_mod_odd(uint64_t a, uint64_t n) {
    return (a / 2u) + ((a % 2u == 0u) ? 0u : (n + 1u) / 2u);
}

// (base ^ exp) % n
constexpr uint64_t pow_mod(uint64_t base, uint64_t exp, uint64_t n) {
    uint64_t result = 1u;
    base %= n;

    while (exp > 0u) {
        if (exp % 2u == 1u) {
            result = mul_mod(result, base, n);
        }

        exp /= 2u;
        base = mul_mod(base, base, n);
    }

    return result;
}

}  // namespace detail
}  // namespace au
