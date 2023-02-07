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

#include "au/testing.hh"
#include "au/units/celsius.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Fahrenheit, HasExpectedLabel) { expect_label<Fahrenheit>("F"); }

TEST(Fahrenheit, HasCorrectQuantityRelationshipWithCelsius) {
    EXPECT_EQ(fahrenheit_qty(9), celsius_qty(5));
}

TEST(Fahrenheit, HasCorrectRelationshipsWithCelsius) {
    EXPECT_THAT(fahrenheit_pt(32.0).as(celsius_pt), SameTypeAndValue(celsius_pt(0.0)));
    EXPECT_THAT(fahrenheit_pt(212.0).as(celsius_pt), SameTypeAndValue(celsius_pt(100.0)));
}

}  // namespace au
