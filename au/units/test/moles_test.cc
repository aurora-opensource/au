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

#include "au/units/moles.hh"

#include "au/testing.hh"
#include "au/units/katals.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Moles, HasExpectedLabel) { expect_label<Moles>("mol"); }

TEST(Moles, EquivalentToKatalSeconds) {
    EXPECT_THAT(moles(6), QuantityEquivalent(katals(2) * seconds(3)));
}

TEST(Moles, HasExpectedSymbol) {
    using symbols::mol;
    EXPECT_THAT(5 * mol, SameTypeAndValue(moles(5)));
}

}  // namespace au
