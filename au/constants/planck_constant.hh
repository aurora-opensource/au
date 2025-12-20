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
#include "au/units/seconds.hh"

namespace au {

namespace detail {
// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/0.5.1/howto/new-units/).
template <typename T>
struct PlanckConstantLabel {
    static constexpr const char label[] = "h";
};
template <typename T>
constexpr const char PlanckConstantLabel<T>::label[];
struct PlanckConstantUnit
    : decltype(Joules{} * Seconds{} * mag<662'607'015>() * pow<-42>(mag<10>())),
      PlanckConstantLabel<void> {
    using PlanckConstantLabel<void>::label;
};
}  // namespace detail

constexpr auto PLANCK_CONSTANT = make_constant(detail::PlanckConstantUnit{});

}  // namespace au
