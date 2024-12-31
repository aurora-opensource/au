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

#include "au/constants/planck_constant.hh"

#include "au/testing.hh"
#include "au/units/joules.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using symbols::J;
using symbols::s;
using ::testing::StrEq;

TEST(PlanckConstant, HasExpectedValue) {
    // h = 6.62607015e-34 J s = 662607015e-41 J s

    // Test approximate value (guard against powers-of-10 type errors).
    constexpr auto defining_units = joule * seconds * pow<-34>(mag<10>());
    constexpr auto val = defining_units(6.62607015);
    constexpr auto err = defining_units(0.00000001);
    EXPECT_THAT(PLANCK_CONSTANT.as<double>(J * s), IsNear(val, err));

    // Test exact value.
    EXPECT_THAT(PLANCK_CONSTANT.in<int>(defining_units / pow<8>(mag<10>())),
                SameTypeAndValue(662'607'015));
}

TEST(PlanckConstant, HasExpectedLabel) { EXPECT_THAT(unit_label(PLANCK_CONSTANT), StrEq("h")); }

}  // namespace
}  // namespace au
