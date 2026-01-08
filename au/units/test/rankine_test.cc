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

#include "au/units/rankine.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/fahrenheit.hh"
#include "au/units/kelvins.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Rankine, HasExpectedLabel) { expect_label<Rankine>("degR"); }

TEST(Rankine, HasCorrectRelationshipWithKelvins) { EXPECT_THAT(rankine(9), Eq(kelvins(5))); }

TEST(Rankine, HasExpectedSymbol) {
    using symbols::degR;
    EXPECT_THAT(5 * degR, SameTypeAndValue(rankine(5)));
}

TEST(Rankine, QuantityEquivalentToFahrenheit) {
    EXPECT_THAT(rankine(1), QuantityEquivalent(fahrenheit_qty(1)));
}

TEST(Rankine, HasCorrectRelationshipWithFahrenheit) {
    EXPECT_THAT(centi(rankine_pt)(45967), Eq(centi(fahrenheit_pt)(0)));
}

}  // namespace au
