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

#include "au/units/percent.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/unos.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Percent, HasExpectedLabel) { expect_label<Percent>("%"); }

TEST(Percent, OneHundredthOfUnos) { EXPECT_EQ(percent(75.0), unos(0.75)); }

TEST(Percent, HasExpectedSymbol) {
    using symbols::pct;
    EXPECT_THAT(5 * pct, SameTypeAndValue(percent(5)));
}

}  // namespace au
