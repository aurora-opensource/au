// Copyright 2023 Aurora Operations, Inc.
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

#include "au/units/webers.hh"

#include "au/testing.hh"
#include "au/units/amperes.hh"
#include "au/units/henries.hh"
#include "au/units/seconds.hh"
#include "au/units/volts.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Webers, HasExpectedLabel) { expect_label<Webers>("Wb"); }

TEST(Webers, EquivalentToVoltSeconds) {
    EXPECT_THAT(webers(8.0), QuantityEquivalent(volts(4.0) * seconds(2.0)));
}

TEST(Webers, EquivalentToHenryAmperes) {
    EXPECT_THAT(webers(8.0), QuantityEquivalent(henries(4.0) * amperes(2.0)));
}

}  // namespace au
