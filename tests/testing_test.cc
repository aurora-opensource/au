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

#include "au/testing.hh"

#include "gtest/gtest.h"

using ::testing::Not;

namespace au {

TEST(SameTypeAndValue, FailsUnlessBothAreSame) {
    EXPECT_THAT(312, SameTypeAndValue(312));
    EXPECT_THAT(312, Not(SameTypeAndValue(314)));
    EXPECT_THAT(312, Not(SameTypeAndValue(312u)));
}

}  // namespace au
