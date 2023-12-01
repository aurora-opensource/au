// Copyright 2023 Aurora Operations, Inc.
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

#include <chrono>
#include <cstdint>

#include "au/quantity.hh"
#include "au/units/seconds.hh"

namespace au {

// Define 1:1 mapping between duration types of chrono library and our library.
template <typename RepT, typename Period>
struct CorrespondingQuantity<std::chrono::duration<RepT, Period>> {
    using Unit = decltype(Seconds{} * (mag<Period::num>() / mag<Period::den>()));
    using Rep = RepT;

    using ChronoDuration = std::chrono::duration<Rep, Period>;

    static constexpr Rep extract_value(ChronoDuration d) { return d.count(); }
    static constexpr ChronoDuration construct_from_value(Rep x) { return ChronoDuration{x}; }
};

// Convert any Au duration quantity to an equivalent `std::chrono::duration`.
template <typename U, typename R>
constexpr auto as_chrono_duration(Quantity<U, R> dt) {
    constexpr auto ratio = unit_ratio(U{}, seconds);
    static_assert(is_rational(ratio), "Cannot convert to chrono::duration with non-rational ratio");
    return std::chrono::duration<R,
                                 std::ratio<get_value<std::intmax_t>(numerator(ratio)),
                                            get_value<std::intmax_t>(denominator(ratio))>>{dt};
}

}  // namespace au
