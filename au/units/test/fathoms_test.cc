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

#include "au/units/fathoms.hh"

#include "au/testing.hh"
#include "au/units/feet.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Fathoms, HasExpectedLabel) { expect_label<Fathoms>("ftm"); }

TEST(Fathoms, EquivalentTo6Feet) { EXPECT_THAT(fathoms(1), Eq(feet(6))); }

TEST(Fathoms, HasExpectedSymbol) {
    using symbols::ftm;
    EXPECT_THAT(5 * ftm, SameTypeAndValue(fathoms(5)));
}

}  // namespace au
