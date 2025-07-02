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

#include "au/units/us_gallons.hh"

#include "au/testing.hh"
#include "au/units/inches.hh"
#include "au/units/liters.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(USGallons, HasExpectedLabel) { expect_label<USGallons>("US_gal"); }

TEST(USGallons, EquivalentTo231CubicInches) { EXPECT_THAT(us_gallons(1), Eq(cubed(inches)(231))); }

TEST(USGallons, WithinExpectationComparedToLiters) {
    EXPECT_THAT(us_gallons(1), IsNear(liters(3.785), milli(liters)(1)));
}

TEST(USGallons, HasExpectedSymbol) {
    using symbols::US_gal;
    EXPECT_THAT(5 * US_gal, SameTypeAndValue(us_gallons(5)));
}

}  // namespace au
