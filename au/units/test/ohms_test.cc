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

#include "au/units/ohms.hh"

#include "au/testing.hh"
#include "au/units/ohms.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Ohms, HasExpectedLabel) { expect_label<Ohms>("ohm"); }

TEST(Ohms, SatisfiesOhmsLaw) {
    EXPECT_THAT(ohms(4.0), QuantityEquivalent(volts(8.0) / amperes(2.0)));
}

TEST(Ohms, SatisfiesOhmicHeatingEquation) {
    // P = I^2 R   -->  R = P / I^2
    EXPECT_EQ(ohms(10.0), watts(250.0) / (amperes(5.0) * amperes(5.0)));

    // P = V^2 / R -->  R = V^2 / P
    EXPECT_EQ(ohms(10.0), (volts(50.0) * volts(50.0)) / watts(250.0));
}

}  // namespace au
