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

#include "au/units/minutes.hh"

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Minutes, HasExpectedLabel) { expect_label<Minutes>("min"); }

TEST(Minutes, EquivalentTo60Seconds) { EXPECT_THAT(minutes(3), Eq(seconds(180))); }

TEST(Minutes, HasExpectedSymbol) {
    using symbols::min;
    EXPECT_THAT(5 * min, SameTypeAndValue(minutes(5)));
}

}  // namespace au
