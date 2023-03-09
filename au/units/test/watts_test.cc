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

#include "au/units/watts.hh"

#include "au/testing.hh"
#include "au/units/ohms.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Watts, HasExpectedLabel) { expect_label<Watts>("W"); }

TEST(Watts, EquivalentToJoulesPerSecond) {
    EXPECT_THAT(watts(12.34), QuantityEquivalent((joules / second)(12.34)));
}

TEST(Watts, EquivalentToOhmicHeating) {
    constexpr auto i = amperes(3.0);
    constexpr auto r = ohms(4.0);
    constexpr auto v = i * r;
    constexpr auto p = v * i;
    EXPECT_THAT(v * i, QuantityEquivalent(p));
    EXPECT_THAT(v * v / r, QuantityEquivalent(p));
    EXPECT_THAT(i * i * r, QuantityEquivalent(p));
}

}  // namespace au
