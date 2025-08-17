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

#include "au/constants/speed_of_light.hh"

#include "au/testing.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using symbols::m;
using symbols::s;
using ::testing::StrEq;

TEST(SpeedOfLight, HasExpectedValue) {
    EXPECT_THAT(SPEED_OF_LIGHT.as<int>(m / s), SameTypeAndValue(299'792'458 * m / s));
}

TEST(SpeedOfLight, HasExpectedLabel) { EXPECT_THAT(unit_label(SPEED_OF_LIGHT), StrEq("c")); }

}  // namespace
}  // namespace au
