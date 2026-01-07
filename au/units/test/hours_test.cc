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

#include "au/units/hours.hh"

#include "au/testing.hh"
#include "au/units/minutes.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Hours, HasExpectedLabel) { expect_label<Hours>("h"); }

TEST(Hours, EquivalentTo60Minutes) { EXPECT_THAT(hours(3), Eq(minutes(180))); }

TEST(Hours, HasExpectedSymbol) {
    using symbols::h;
    EXPECT_THAT(5 * h, SameTypeAndValue(hours(5)));
}

}  // namespace au
