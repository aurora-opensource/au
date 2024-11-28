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

#include "au/constants/avogadro_constant.hh"

#include "au/testing.hh"
#include "au/units/moles.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::StrEq;

TEST(AvogadroConstant, HasExpectedValue) {
    // N_A = 6.02214076e23 mol^(-1)

    // Test approximate value (guard against powers-of-10 type errors).
    constexpr auto defining_units = inverse(moles) * pow<23>(mag<10>());
    constexpr auto val = defining_units(6.02214076);
    constexpr auto err = defining_units(0.00000001);
    EXPECT_THAT(AVOGADRO_CONSTANT.as<double>(inverse(moles)), IsNear(val, err));

    // Test exact value.
    EXPECT_THAT(AVOGADRO_CONSTANT.in<int>(defining_units / pow<8>(mag<10>())),
                SameTypeAndValue(602'214'076));
}

TEST(AvogadroConstant, HasExpectedLabel) {
    EXPECT_THAT(unit_label(AVOGADRO_CONSTANT), StrEq("N_A"));
}

}  // namespace
}  // namespace au
