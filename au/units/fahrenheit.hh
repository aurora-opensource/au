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

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/units/kelvins.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct FahrenheitLabel {
    static constexpr const char label[] = "degF";
};
template <typename T>
constexpr const char FahrenheitLabel<T>::label[];
struct Rankines : decltype(Kelvins{} * mag<5>() / mag<9>()) {};
constexpr auto rankines = QuantityMaker<Rankines>{};
struct Fahrenheit : Rankines, FahrenheitLabel<void> {
    using FahrenheitLabel<void>::label;
    static constexpr auto origin() { return centi(rankines)(459'67); }
};
constexpr auto fahrenheit_qty = QuantityMaker<Fahrenheit>{};
constexpr auto fahrenheit_pt = QuantityPointMaker<Fahrenheit>{};

[[deprecated(
    "`fahrenheit()` is ambiguous.  Use `fahrenheit_pt()` for _points_, or `fahrenheit_qty()` for "
    "_quantities_")]] constexpr auto fahrenheit = QuantityMaker<Fahrenheit>{};

}  // namespace au
