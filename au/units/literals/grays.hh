// Copyright 2026 Aurora Operations, Inc.
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

#include "au/constant.hh"
#include "au/magnitude.hh"
#include "au/units/grays.hh"

namespace au {
namespace literals {

// `1.28e-4_Gy` is a `Constant` equivalent to `make_constant(1.28e-4_mag * grays)`.
template <char... Cs>
constexpr auto operator""_Gy() {
    return make_constant(grays * ::au::au_literals::operator""_mag < Cs... > ());
}

}  // namespace literals
}  // namespace au
