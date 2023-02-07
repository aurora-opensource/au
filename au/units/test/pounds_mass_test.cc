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

#include "au/units/pounds_mass.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(PoundsMass, HasExpectedLabel) { expect_label<PoundsMass>("lb"); }

TEST(PoundsMass, EquivalentToAppropriateQuantityOfKilograms) {
    EXPECT_EQ(pounds_mass(100'000'000L), (kilo(grams)(45'359'237L)));
}

}  // namespace au
