// Copyright 2023 Aurora Operations, Inc.
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

#include "au/units/grays.hh"

#include "au/testing.hh"
#include "au/units/grams.hh"
#include "au/units/joules.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Grays, HasExpectedLabel) { expect_label<Grays>("Gy"); }

TEST(Grays, EquivalentToJoulesPerKilogram) {
    EXPECT_THAT(grays(4.0), QuantityEquivalent(joules(8.0) / kilo(grams)(2.0)));
}

TEST(Grays, HasExpectedSymbol) {
    using symbols::Gy;
    EXPECT_THAT(5 * Gy, SameTypeAndValue(grays(5)));
}

}  // namespace au
