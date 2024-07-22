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

#include <chrono>
#include <type_traits>

namespace au {

// A type representing a quantity of "zero" in any units.
//
// Zero is special: it's the only number that we can meaningfully compare or assign to a Quantity of
// _any_ dimension.  Giving it a special type (and a predefined constant of that type, `ZERO`,
// defined below) lets our code be both concise and readable.
//
// For example, we can zero-initialize any arbitrary Quantity, even if it doesn't have a
// user-defined literal, and even if it's in a header file so we couldn't use the literals anyway:
//
//   struct PathPoint {
//       QuantityD<RadiansPerMeter> curvature = ZERO;
//   };
struct Zero {
    // Implicit conversion to arithmetic types.
    template <typename T, typename Enable = std::enable_if_t<std::is_arithmetic<T>::value>>
    constexpr operator T() const {
        return 0;
    }

    // Implicit conversion to chrono durations.
    template <typename Rep, typename Period>
    constexpr operator std::chrono::duration<Rep, Period>() const {
        return std::chrono::duration<Rep, Period>{0};
    }
};

// A value of Zero.
//
// This exists purely for convenience, so people don't have to call the initializer.  i.e., it lets
// us write `ZERO` instead of `Zero{}`.
static constexpr auto ZERO = Zero{};

// Addition, subtraction, and comparison of Zero are well defined.
inline constexpr Zero operator+(Zero, Zero) { return ZERO; }
inline constexpr Zero operator-(Zero, Zero) { return ZERO; }
inline constexpr bool operator==(Zero, Zero) { return true; }
inline constexpr bool operator>=(Zero, Zero) { return true; }
inline constexpr bool operator<=(Zero, Zero) { return true; }
inline constexpr bool operator!=(Zero, Zero) { return false; }
inline constexpr bool operator>(Zero, Zero) { return false; }
inline constexpr bool operator<(Zero, Zero) { return false; }

}  // namespace au
