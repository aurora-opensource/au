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

#include "au/units/us_quarts_fwd.hh"
// Keep corresponding `_fwd.hh` file on top.

#include "au/units/inches.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct USQuartsLabel {
    static constexpr const char label[] = "US_qt";
};
template <typename T>
constexpr const char USQuartsLabel<T>::label[];
struct USQuarts : decltype(cubed(Inches{}) * (mag<231>() / mag<4>())), USQuartsLabel<void> {
    using USQuartsLabel<void>::label;
};
constexpr auto us_quart = SingularNameFor<USQuarts>{};
constexpr auto us_quarts = QuantityMaker<USQuarts>{};

namespace symbols {
constexpr auto US_qt = SymbolFor<USQuarts>{};
}

}  // namespace au
