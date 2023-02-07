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

#include "au/quantity.hh"
#include "au/units/feet.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct YardsLabel {
    static constexpr const char label[] = "yd";
};
template <typename T>
constexpr const char YardsLabel<T>::label[];
struct Yards : decltype(Feet{} * mag<3>()), YardsLabel<void> {
    using YardsLabel<void>::label;
};
constexpr auto yard = SingularNameFor<Yards>{};
constexpr auto yards = QuantityMaker<Yards>{};

}  // namespace au
