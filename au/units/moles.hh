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

#include "au/units/moles_fwd.hh"
// Keep corresponding `_fwd.hh` file on top.
#include "au/config.hh"
#include "au/quantity.hh"
#include "au/unit_symbol.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct MolesLabel {
    static constexpr const char label[] = "mol";
};
template <typename T>
constexpr const char MolesLabel<T>::label[];
struct Moles : UnitImpl<AmountOfSubstance>, MolesLabel<void> {
    using MolesLabel<void>::label;
};
AU_DEVICE_VAR constexpr auto mole = SingularNameFor<Moles>{};
AU_DEVICE_VAR constexpr auto moles = QuantityMaker<Moles>{};

namespace symbols {
AU_DEVICE_VAR constexpr auto mol = SymbolFor<Moles>{};
}
}  // namespace au
