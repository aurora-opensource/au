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

#include "au/units/us_pints.hh"

#include "au/testing.hh"
#include "au/units/us_gallons.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(USPints, HasExpectedLabel) { expect_label<USPints>("US_pt"); }

TEST(USPints, EightInAGallon) { EXPECT_THAT(us_pints(8), Eq(us_gallons(1))); }

TEST(USPints, HasExpectedSymbol) {
    using symbols::US_pt;
    EXPECT_THAT(5 * US_pt, SameTypeAndValue(us_pints(5)));
}

}  // namespace au
