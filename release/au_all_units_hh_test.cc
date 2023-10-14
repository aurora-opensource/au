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

#include "docs/au_all_units.hh"
#include "gtest/gtest.h"

// clang-format off
//
// This file textually includes all of the test cases which every ready-made
// single-file release should pass.  It needs to go after all other includes.
#include "release/common_test_cases.hh"
// clang-format on

namespace au {

namespace {
template <typename T>
std::string stream_to_string(const T &x) {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}
}  // namespace

TEST(AuAllUnitsHh, PrintsValueAndUnitLabel) { EXPECT_EQ(stream_to_string(minutes(3)), "3 min"); }

}  // namespace au
