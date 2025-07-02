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

#include "au/units/liters.hh"

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Liters, HasExpectedLabel) { expect_label<Liters>("L"); }

TEST(Liters, HasExpectedRelationshipsWithLinearUnits) {
    EXPECT_THAT(liters(1), Eq(cubed(deci(meters))(1)));

    // 1 mL == 1 c.c.
    EXPECT_THAT(milli(liters)(1), Eq(cubed(centi(meters))(1)));
}

TEST(Liters, HasExpectedSymbol) {
    using symbols::L;
    EXPECT_THAT(5 * L, SameTypeAndValue(liters(5)));
}

}  // namespace au
