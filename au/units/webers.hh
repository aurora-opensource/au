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

#include "au/quantity.hh"
#include "au/unit_symbol.hh"
#include "au/units/seconds.hh"
#include "au/units/volts.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct WebersLabel {
    static constexpr const char label[] = "Wb";
};
template <typename T>
constexpr const char WebersLabel<T>::label[];
struct Webers : decltype(Volts{} * Seconds{}), WebersLabel<void> {
    using WebersLabel<void>::label;
};
constexpr auto weber = SingularNameFor<Webers>{};
constexpr auto webers = QuantityMaker<Webers>{};

namespace symbols {
constexpr auto Wb = SymbolFor<Webers>{};
}
}  // namespace au
