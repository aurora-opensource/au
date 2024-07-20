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

#include "au/units/candelas.hh"

#include "au/testing.hh"
#include "au/units/lumens.hh"
#include "au/units/steradians.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Candelas, HasExpectedLabel) { expect_label<Candelas>("cd"); }

TEST(Candelas, EquivalentToLumensPerSteradian) {
    EXPECT_THAT(candelas(2.0), QuantityEquivalent(lumens(6.0) / steradians(3.0)));
}

TEST(Candelas, HasExpectedSymbol) {
    using symbols::cd;
    EXPECT_THAT(5 * cd, SameTypeAndValue(candelas(5)));
}

}  // namespace au
