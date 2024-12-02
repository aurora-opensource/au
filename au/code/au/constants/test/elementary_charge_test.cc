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

#include "au/constants/elementary_charge.hh"

#include "au/testing.hh"
#include "au/units/coulombs.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using symbols::C;
using ::testing::StrEq;

TEST(ElementaryCharge, HasExpectedValue) {
    // e = 1.602176634e-19 C

    // Test approximate value (guard against powers-of-10 type errors).
    constexpr auto defining_units = coulombs * pow<-19>(mag<10>());
    constexpr auto val = defining_units(1.602176634);
    constexpr auto err = defining_units(0.000000001);
    EXPECT_THAT(ELEMENTARY_CHARGE.as<double>(C), IsNear(val, err));

    // Test exact value.
    EXPECT_THAT(ELEMENTARY_CHARGE.in<int>(defining_units / pow<9>(mag<10>())),
                SameTypeAndValue(1'602'176'634));
}

TEST(PlanckConstant, HasExpectedLabel) { EXPECT_THAT(unit_label(ELEMENTARY_CHARGE), StrEq("e")); }

}  // namespace
}  // namespace au
