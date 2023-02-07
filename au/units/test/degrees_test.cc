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

#include "au/units/degrees.hh"

#include "au/testing.hh"
#include "au/units/revolutions.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Degrees, HasExpectedLabel) { expect_label<Degrees>("deg"); }

TEST(Degrees, RoughlyEquivalentToPiOver180Radians) {
    EXPECT_DOUBLE_EQ(degrees(1.0).in(radians), get_value<double>(PI / mag<180>()));
}

TEST(Degrees, One360thOfARevolution) { EXPECT_EQ(degrees(360), revolutions(1)); }

}  // namespace au
