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
#include "au/units/hertz.hh"

namespace au {

namespace auimpl {
// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct CesiumHyperfineTransitionFrequencyLabel {
    static constexpr const char label[] = "Delta_nu_Cs";
};
template <typename T>
constexpr const char CesiumHyperfineTransitionFrequencyLabel<T>::label[];
struct CesiumHyperfineTransitionFrequencyUnit : decltype(Hertz{} * mag<9'192'631'770>()),
                                                CesiumHyperfineTransitionFrequencyLabel<void> {
    using CesiumHyperfineTransitionFrequencyLabel<void>::label;
};
}  // namespace auimpl

constexpr auto CESIUM_HYPERFINE_TRANSITION_FREQUENCY =
    make_constant(auimpl::CesiumHyperfineTransitionFrequencyUnit{});

}  // namespace au
