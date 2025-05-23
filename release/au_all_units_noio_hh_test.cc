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

#include <chrono>

#include "docs/au_all_units_noio.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// clang-format off
//
// This file textually includes all of the test cases which every ready-made
// single-file release should pass.  It needs to go after all other includes.
#include "release/common_test_cases.hh"
// clang-format on

namespace au {

using ::testing::Eq;

TEST(AuAllUnitsNoioHh, IncludesMoreObscureUnits) {
    EXPECT_THAT(minutes(120), Eq(hours(2)));
    EXPECT_THAT(fahrenheit_pt(32), Eq(celsius_pt(0)));
}

}  // namespace au
