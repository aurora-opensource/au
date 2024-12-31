// Copyright 2022 Aurora Operations, Inc.
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

#include "au/units/standard_gravity.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {

TEST(StandardGravity, HasExpectedLabel) { expect_label<StandardGravity>("g_0"); }

TEST(StandardGravity, HasExpectedValue) {
    EXPECT_EQ(standard_gravity(1L), (micro(meters) / squared(second))(9'806'650L));
}

TEST(StandardGravity, HasExpectedSymbol) {
    using symbols::g_0;
    EXPECT_THAT(5 * g_0, SameTypeAndValue(standard_gravity(5)));
}

}  // namespace au
