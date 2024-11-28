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

#include "au/constants/boltzmann_constant.hh"

#include "au/testing.hh"
#include "au/units/joules.hh"
#include "au/units/kelvins.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using symbols::J;
using symbols::K;
using ::testing::StrEq;

TEST(BoltzmannConstant, HasExpectedValue) {
    // k_B = 1.380649e-23 J/K

    // Test approximate value (guard against powers-of-10 type errors).
    constexpr auto defining_units = (joules / kelvin) * pow<-23>(mag<10>());
    constexpr auto val = defining_units(1.380649);
    constexpr auto err = defining_units(0.000001);
    EXPECT_THAT(BOLTZMANN_CONSTANT.as<double>(J / K), IsNear(val, err));

    // Test exact value.
    EXPECT_THAT(BOLTZMANN_CONSTANT.in<int>(defining_units / pow<6>(mag<10>())),
                SameTypeAndValue(1'380'649));
}

TEST(BoltzmannConstant, HasExpectedLabel) {
    EXPECT_THAT(unit_label(BOLTZMANN_CONSTANT), StrEq("k_B"));
}

}  // namespace
}  // namespace au
