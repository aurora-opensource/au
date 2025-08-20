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
#include "au/units/coulombs.hh"

namespace au {

namespace detail {
// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/0.5.0/howto/new-units/).
template <typename T>
struct ElementaryChargeLabel {
    static constexpr const char label[] = "e";
};
template <typename T>
constexpr const char ElementaryChargeLabel<T>::label[];
struct ElementaryChargeUnit : decltype(Coulombs{} * mag<1'602'176'634>() * pow<-28>(mag<10>())),
                              ElementaryChargeLabel<void> {
    using ElementaryChargeLabel<void>::label;
};
}  // namespace detail

constexpr auto ELEMENTARY_CHARGE = make_constant(detail::ElementaryChargeUnit{});

}  // namespace au
