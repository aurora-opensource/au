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

#include "au/units/fahrenheit_fwd.hh"
// Keep corresponding `_fwd.hh` file on top.

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/unit_symbol.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct FahrenheitLabel {
    static constexpr const char label[] = "degF";
};
template <typename T>
constexpr const char FahrenheitLabel<T>::label[];
struct Rankines
    // In particular, do NOT manually specify `Dimension<...>` and `Magnitude<...>` types.  The
    // ordering of the arguments is very particular, and could change out from under you in future
    // versions, making the program ill-formed.  Only units defined within the Au library itself can
    // safely use this pattern.
    : UnitImpl<Temperature, Magnitude<Pow<Prime<3>, -2>, Prime<5>>> {};
constexpr auto rankines = QuantityMaker<Rankines>{};
struct Fahrenheit : Rankines, FahrenheitLabel<void> {
    using FahrenheitLabel<void>::label;
    static constexpr auto origin() {
        // 459.67 Rankines = 45967 centi-rankines
        return make_quantity<Centi<Rankines>>(45967);
    }
};
constexpr auto fahrenheit_qty = QuantityMaker<Fahrenheit>{};
constexpr auto fahrenheit_pt = QuantityPointMaker<Fahrenheit>{};

[[deprecated(
    "`fahrenheit()` is ambiguous.  Use `fahrenheit_pt()` for _points_, or `fahrenheit_qty()` for "
    "_quantities_")]] constexpr auto fahrenheit = QuantityMaker<Fahrenheit>{};

namespace symbols {
constexpr auto degF_qty = SymbolFor<Fahrenheit>{};
}
}  // namespace au
