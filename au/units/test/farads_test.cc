// Copyright 2023 Aurora Operations, Inc.
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

#include "au/units/farads.hh"

#include "au/testing.hh"
#include "au/units/coulombs.hh"
#include "au/units/volts.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Farads, HasExpectedLabel) { expect_label<Farads>("F"); }

TEST(Farads, EquivalentToCoulombsPerVolt) {
    EXPECT_THAT(farads(4.0), QuantityEquivalent(coulombs(8.0) / volts(2.0)));
}

TEST(Farads, HasExpectedSymbol) {
    using symbols::F;
    EXPECT_THAT(5 * F, SameTypeAndValue(farads(5)));
}

}  // namespace au
