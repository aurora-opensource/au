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

#include "au/units/joules.hh"

#include "au/testing.hh"
#include "au/units/meters.hh"
#include "au/units/newtons.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Joules, HasExpectedLabel) { expect_label<Joules>("J"); }

TEST(Joules, EquivalentToNewtonMeters) { EXPECT_EQ(joules(18), (newton * meters)(18)); }

TEST(Joules, HasExpectedSymbol) {
    using symbols::J;
    EXPECT_THAT(5 * J, SameTypeAndValue(joules(5)));
}

}  // namespace au
