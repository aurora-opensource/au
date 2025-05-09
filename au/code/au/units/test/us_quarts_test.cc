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

#include "au/units/us_quarts.hh"

#include "au/testing.hh"
#include "au/units/us_gallons.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(USQuarts, HasExpectedLabel) { expect_label<USQuarts>("US_qt"); }

TEST(USQuarts, FourInAGallon) { EXPECT_THAT(us_quarts(4), Eq(us_gallons(1))); }

TEST(USQuarts, HasExpectedSymbol) {
    using symbols::US_qt;
    EXPECT_THAT(5 * US_qt, SameTypeAndValue(us_quarts(5)));
}

}  // namespace au
