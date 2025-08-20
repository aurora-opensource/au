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

#include "au/constant.hh"
#include "au/units/moles.hh"

namespace au {

namespace detail {
// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/0.5.0/howto/new-units/).
template <typename T>
struct AvogadroConstantLabel {
    static constexpr const char label[] = "N_A";
};
template <typename T>
constexpr const char AvogadroConstantLabel<T>::label[];
struct AvogadroConstantUnit : decltype(inverse(Moles{}) * mag<602'214'076>() * pow<15>(mag<10>())),
                              AvogadroConstantLabel<void> {
    using AvogadroConstantLabel<void>::label;
};
}  // namespace detail

constexpr auto AVOGADRO_CONSTANT = make_constant(detail::AvogadroConstantUnit{});

}  // namespace au
