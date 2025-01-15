// Copyright 2025 Aurora Operations, Inc.
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

#include "au/units/arcminutes.hh"

#include "au/testing.hh"
#include "au/units/degrees.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Arcseconds, HasExpectedLabel) { expect_label<Arcseconds>("'"); }

TEST(Arcseconds, RelatesCorrectlyToDegrees) {
    EXPECT_THAT(arcseconds(120.0), QuantityEquivalent(degrees(2.0)));
}

TEST(Arcseconds, HasExpectedSymbol) {
    using symbols::am;
    EXPECT_THAT(5.f * am, SameTypeAndValue(arcminutes(5.f)));
}

}  // namespace au
