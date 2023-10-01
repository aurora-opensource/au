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

#include "au/magnitude.hh"

namespace au {
namespace detail {

// The various categories by which a magnitude can be applied to a numeric quantity.
enum class ApplyAs {
    INTEGER_MULTIPLY,
    INTEGER_DIVIDE,
    RATIONAL_MULTIPLY,
    IRRATIONAL_MULTIPLY,
};

template <typename... BPs>
constexpr ApplyAs categorize_magnitude(Magnitude<BPs...>) {
    if (IsInteger<Magnitude<BPs...>>::value) {
        return ApplyAs::INTEGER_MULTIPLY;
    }

    if (IsInteger<MagInverseT<Magnitude<BPs...>>>::value) {
        return ApplyAs::INTEGER_DIVIDE;
    }

    return IsRational<Magnitude<BPs...>>::value ? ApplyAs::RATIONAL_MULTIPLY
                                                : ApplyAs::IRRATIONAL_MULTIPLY;
}

}  // namespace detail
}  // namespace au
