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

#include "au/units/lux.hh"

#include "au/testing.hh"
#include "au/units/lumens.hh"
#include "au/units/meters.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Lux, HasExpectedLabel) { expect_label<Lux>("lx"); }

TEST(Lux, ProductWithAreaGivesLumens) {
    EXPECT_THAT(lux(2.0), QuantityEquivalent(lumens(8.0) / squared(meters)(4.0)));
}

TEST(Lux, HasExpectedSymbol) {
    using symbols::lx;
    EXPECT_THAT(5 * lx, SameTypeAndValue(lux(5)));
}

}  // namespace au
