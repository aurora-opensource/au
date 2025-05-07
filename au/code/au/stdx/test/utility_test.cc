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

#include "au/stdx/utility.hh"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::IsFalse;
using ::testing::IsTrue;

namespace stdx {

TEST(CmpLess, HandlesMixedSignedUnsigned) {
    EXPECT_THAT(cmp_less(-1, 1u), IsTrue());
    EXPECT_THAT(cmp_less(1u, -1), IsFalse());
    EXPECT_THAT(cmp_less(1u, 1), IsFalse());
    EXPECT_THAT(cmp_less(1u, 2), IsTrue());
}

TEST(CmpEqual, HandlesMixedSignedUnsigned) {
    EXPECT_THAT(cmp_equal(-1, 1u), IsFalse());
    EXPECT_THAT(cmp_equal(1u, -1), IsFalse());
    EXPECT_THAT(cmp_equal(1u, 1), IsTrue());
    EXPECT_THAT(cmp_equal(1u, 2), IsFalse());
}

}  // namespace stdx
}  // namespace au
