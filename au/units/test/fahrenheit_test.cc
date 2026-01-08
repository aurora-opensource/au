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

#include "au/units/fahrenheit.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/celsius.hh"
#include "au/units/rankine.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Fahrenheit, HasExpectedLabel) { expect_label<Fahrenheit>("degF"); }

TEST(Fahrenheit, HasCorrectQuantityRelationshipWithCelsius) {
    EXPECT_THAT(fahrenheit_qty(9), Eq(celsius_qty(5)));
}

TEST(Fahrenheit, HasCorrectRelationshipsWithCelsius) {
    EXPECT_THAT(fahrenheit_pt(32.0).as(celsius_pt), SameTypeAndValue(celsius_pt(0.0)));
    EXPECT_THAT(fahrenheit_pt(212.0).as(celsius_pt), SameTypeAndValue(celsius_pt(100.0)));
}

TEST(Fahrenheit, HasExpectedSymbol) {
    using symbols::degF_qty;
    EXPECT_THAT(5 * degF_qty, SameTypeAndValue(fahrenheit_qty(5)));
}

TEST(Fahrenheit, QuantityEquivalentToRankine) {
    EXPECT_THAT(fahrenheit_qty(1), QuantityEquivalent(rankine(1)));
}

TEST(Fahrenheit, HasCorrectRelationshipWithRankine) {
    EXPECT_THAT(centi(fahrenheit_pt)(0), Eq(centi(rankine_pt)(45967)));
}

}  // namespace au
