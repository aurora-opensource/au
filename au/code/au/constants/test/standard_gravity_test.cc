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

#include "au/constants/standard_gravity.hh"

#include "au/testing.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::StrEq;

TEST(StandardGravity, HasExpectedValue) {
    // g_0 = 9.80665 m/s^2

    // Test approximate value (guard against powers-of-10 type errors).
    constexpr auto defining_units = meters / pow<2>(seconds);
    constexpr auto val = defining_units(9.80665);
    constexpr auto err = defining_units(0.00001);
    EXPECT_THAT(STANDARD_GRAVITY.as<double>(defining_units), IsNear(val, err));

    // Test exact value.
    EXPECT_THAT(STANDARD_GRAVITY.in<int>(defining_units / pow<5>(mag<10>())),
                SameTypeAndValue(980'665));
}

TEST(StandardGravity, HasExpectedLabel) { EXPECT_THAT(unit_label(STANDARD_GRAVITY), StrEq("g_0")); }

}  // namespace
}  // namespace au
