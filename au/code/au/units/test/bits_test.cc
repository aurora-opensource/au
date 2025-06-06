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

#include "au/units/bits.hh"

#include "au/testing.hh"
#include "au/units/bytes.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Bits, HasExpectedLabel) { expect_label<Bits>("b"); }

TEST(Bits, OneEighthOfAByte) { EXPECT_THAT(bits(1.0), Eq(bytes(1.0 / 8.0))); }

TEST(Bits, HasExpectedSymbol) {
    using symbols::b;
    EXPECT_THAT(5 * b, SameTypeAndValue(bits(5)));
}

}  // namespace au
