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
#include "au/units/joules.hh"
#include "au/units/kelvins.hh"

namespace au {

namespace detail {
// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/0.5.1/howto/new-units/).
template <typename T>
struct BoltzmannConstantLabel {
    static constexpr const char label[] = "k_B";
};
template <typename T>
constexpr const char BoltzmannConstantLabel<T>::label[];
struct BoltzmannConstantUnit
    : decltype((Joules{} / Kelvins{}) * mag<1'380'649>() * pow<-29>(mag<10>())),
      BoltzmannConstantLabel<void> {
    using BoltzmannConstantLabel<void>::label;
};
}  // namespace detail

constexpr auto BOLTZMANN_CONSTANT = make_constant(detail::BoltzmannConstantUnit{});

}  // namespace au
