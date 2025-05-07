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

#include "au/units/degrees.hh"

#include "au/testing.hh"
#include "au/units/revolutions.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Degrees, HasExpectedLabel) { expect_label<Degrees>("deg"); }

TEST(Degrees, RoughlyEquivalentToPiOver180Radians) {
    EXPECT_DOUBLE_EQ(degrees(1.0).in(radians), get_value<double>(Magnitude<Pi>{} / mag<180>()));
}

TEST(Degrees, One360thOfARevolution) { EXPECT_THAT(degrees(360), Eq(revolutions(1))); }

TEST(Degrees, HasExpectedSymbol) {
    using symbols::deg;
    EXPECT_THAT(5 * deg, SameTypeAndValue(degrees(5)));
}

}  // namespace au
