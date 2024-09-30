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

#include "au/units/celsius.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/fahrenheit.hh"
#include "au/units/kelvins.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Celsius, HasExpectedLabel) { expect_label<Celsius>("degC"); }

TEST(Celsius, QuantityEquivalentToKelvins) {
    EXPECT_THAT(celsius_qty(20), QuantityEquivalent(kelvins(20)));
}

TEST(Celsius, QuantityPointHasCorrectOffsetFromKelvins) {
    EXPECT_EQ(milli(kelvins_pt)(273'150).as(milli(celsius_pt)), milli(celsius_pt)(0));
}

TEST(Celsius, QuantityPointMatchesUpCorrectlyWithFahrenheit) {
    EXPECT_EQ(celsius_pt(0), fahrenheit_pt(32));
    EXPECT_EQ(celsius_pt(100), fahrenheit_pt(212));
}

TEST(Celsius, HasExpectedSymbol) {
    using symbols::degC_qty;
    EXPECT_THAT(5 * degC_qty, SameTypeAndValue(celsius_qty(5)));
}

}  // namespace au
