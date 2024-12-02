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

#include "au/constants/luminous_efficacy_540_terahertz.hh"

#include "au/testing.hh"
#include "au/units/lumens.hh"
#include "au/units/watts.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using symbols::lm;
using symbols::W;
using ::testing::StrEq;

TEST(LuminousEfficacy540Terahertz, HasExpectedValue) {
    // K_cd = 683 lm/W

    // Test exact value.
    EXPECT_THAT(LUMINOUS_EFFICACY_540_TERAHERTZ.in<int>(lm / W), SameTypeAndValue(683));
}

TEST(LuminousEfficacy540Terahertz, HasExpectedLabel) {
    EXPECT_THAT(unit_label(LUMINOUS_EFFICACY_540_TERAHERTZ), StrEq("K_cd"));
}

}  // namespace
}  // namespace au
