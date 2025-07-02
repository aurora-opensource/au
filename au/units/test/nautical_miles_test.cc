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

#include "au/units/nautical_miles.hh"

#include "au/testing.hh"
#include "au/units/hours.hh"
#include "au/units/knots.hh"
#include "au/units/meters.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(NauticalMiles, HasExpectedLabel) { expect_label<NauticalMiles>("nmi"); }

TEST(NauticalMiles, EquivalentTo1852Meters) { EXPECT_THAT(nautical_miles(1), Eq(meters(1'852))); }

TEST(NauticalMiles, EquivalentToKnotHours) {
    EXPECT_THAT(nautical_miles(1), Eq((knot * hours)(1)));
}

TEST(NauticalMiles, HasExpectedSymbol) {
    using symbols::nmi;
    EXPECT_THAT(5 * nmi, SameTypeAndValue(nautical_miles(5)));
}

}  // namespace au
