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
#include "au/units/lumens.hh"
#include "au/units/watts.hh"

namespace au {

namespace detail {
// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/0.5.0/howto/new-units/).
template <typename T>
struct LuminousEfficacy540TerahertzLabel {
    static constexpr const char label[] = "K_cd";
};
template <typename T>
constexpr const char LuminousEfficacy540TerahertzLabel<T>::label[];
struct LuminousEfficacy540TerahertzUnit : decltype((Lumens{} / Watts{}) * mag<683>()),
                                          LuminousEfficacy540TerahertzLabel<void> {
    using LuminousEfficacy540TerahertzLabel<void>::label;
};
}  // namespace detail

constexpr auto LUMINOUS_EFFICACY_540_TERAHERTZ =
    make_constant(detail::LuminousEfficacy540TerahertzUnit{});

}  // namespace au
