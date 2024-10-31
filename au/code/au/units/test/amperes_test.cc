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

#include "au/units/amperes.hh"

#include "au/testing.hh"
#include "au/units/ohms.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Amperes, HasExpectedLabel) { expect_label<Amperes>("A"); }

TEST(Amperes, SatisfiesOhmsLaw) {
    EXPECT_THAT(amperes(2.0), QuantityEquivalent(volts(8.0) / ohms(4.0)));
}

TEST(Amperes, ProductWithVoltsGivesPower) {
    EXPECT_THAT(amperes(2.0), QuantityEquivalent(watts(8.0) / volts(4.0)));
}

TEST(Amperes, HasExpectedSymbol) {
    using symbols::A;
    EXPECT_THAT(5 * A, SameTypeAndValue(amperes(5)));
}

}  // namespace au
