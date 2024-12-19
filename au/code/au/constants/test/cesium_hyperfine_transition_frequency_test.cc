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

#include "au/constants/cesium_hyperfine_transition_frequency.hh"

#include "au/testing.hh"
#include "au/units/hertz.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::StrEq;

TEST(CesiumHyperfineTransitionFrequency, HasExpectedValue) {
    // Delta_nu_Cs = 9'192'631'770 Hz

    // Test exact value.
    EXPECT_THAT(CESIUM_HYPERFINE_TRANSITION_FREQUENCY.in<uint64_t>(hertz),
                SameTypeAndValue(uint64_t{9'192'631'770}));
}

TEST(CesiumHyperfineTransitionFrequency, HasExpectedLabel) {
    EXPECT_THAT(unit_label(CESIUM_HYPERFINE_TRANSITION_FREQUENCY), StrEq("Delta_nu_Cs"));
}

}  // namespace
}  // namespace au
