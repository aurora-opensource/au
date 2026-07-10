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

#include "au/units/lumens.hh"

#include "au/constant.hh"
#include "au/magnitude.hh"
#include "au/testing.hh"
#include "au/units/candelas.hh"
#include "au/units/literals/lumens.hh"
#include "au/units/steradians.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

TEST(Lumens, HasExpectedLabel) { expect_label<Lumens>("lm"); }

TEST(Lumens, EquivalentToCandelaSteradians) {
    EXPECT_THAT(lumens(6), QuantityEquivalent(candelas(2) * steradians(3)));
}

TEST(Lumens, HasExpectedSymbol) {
    using symbols::lm;
    EXPECT_THAT(5 * lm, SameTypeAndValue(lumens(5)));
}

TEST(Lumens, LiteralMakesEquivalentConstant) {
    using namespace ::au::au_literals;
    EXPECT_THAT(1.28e-4_lm, SameTypeAndValue(make_constant(lumens * 1.28e-4_mag)));
}

}  // namespace au
