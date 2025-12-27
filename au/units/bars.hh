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

#include "au/units/bars_fwd.hh"
// Keep corresponding `_fwd.hh` file on top.

#include "au/quantity.hh"
#include "au/unit_symbol.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct BarsLabel {
    static constexpr const char label[] = "bar";
};
template <typename T>
constexpr const char BarsLabel<T>::label[];
struct Bars
    // In particular, do NOT manually specify `Dimension<...>` and `Magnitude<...>` types.  The
    // ordering of the arguments is very particular, and could change out from under you in future
    // versions, making the program ill-formed.  Only units defined within the Au library itself can
    // safely use this pattern.
    : UnitImpl<Dimension<Pow<base_dim::Length, -1>, base_dim::Mass, Pow<base_dim::Time, -2>>,
               Magnitude<Pow<Prime<2>, 8>, Pow<Prime<5>, 8>>>,
      BarsLabel<void> {
    using BarsLabel<void>::label;
};
constexpr auto bar = SingularNameFor<Bars>{};
constexpr auto bars = QuantityMaker<Bars>{};

namespace symbols {
constexpr auto bar = SymbolFor<Bars>{};
}  // namespace symbols
}  // namespace au
