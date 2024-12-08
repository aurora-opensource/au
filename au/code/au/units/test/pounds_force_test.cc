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

#include "au/units/pounds_force.hh"

#include "au/constants/standard_gravity.hh"
#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

constexpr auto g0 = STANDARD_GRAVITY;

TEST(PoundsForce, HasExpectedLabel) { expect_label<PoundsForce>("lbf"); }

TEST(PoundsForce, EquivalentToStandardGravityActingOnPoundMass) {
    EXPECT_THAT(pounds_force(123), QuantityEquivalent(pounds_mass(123) * g0));
}

TEST(PoundsForce, HasExpectedSymbol) {
    using symbols::lbf;
    EXPECT_THAT(5 * lbf, SameTypeAndValue(pounds_force(5)));
}

}  // namespace au
