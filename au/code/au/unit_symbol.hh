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

#include "au/fwd.hh"
#include "au/wrapper_operations.hh"

namespace au {

//
// A representation of the symbol for a unit.
//
// To use, create an instance variable templated on a unit, and make the instance variable's name
// the symbol to represent.  For example:
//
//     constexpr auto m = SymbolFor<Meters>{};
//
template <typename Unit>
struct SymbolFor : detail::MakesQuantityFromNumber<SymbolFor, Unit>,
                   detail::ScalesQuantity<SymbolFor, Unit>,
                   detail::ComposesWith<SymbolFor, Unit, SymbolFor, SymbolFor> {};

//
// Create a unit symbol using the more fluent APIs that unit slots make possible.  For example:
//
//     constexpr auto mps = symbol_for(meters / second);
//
// This is generally easier to work with and makes code that is easier to read, at the cost of being
// (very slightly) slower to compile.
//
template <typename UnitSlot>
constexpr auto symbol_for(UnitSlot) {
    return SymbolFor<AssociatedUnitT<UnitSlot>>{};
}

// Support using symbols in unit slot APIs (e.g., `v.in(m / s)`).
template <typename U>
struct AssociatedUnit<SymbolFor<U>> : stdx::type_identity<U> {};

}  // namespace au
