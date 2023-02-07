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

#include "au/units/katals.hh"

#include "au/testing.hh"
#include "au/units/moles.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Katals, HasExpectedLabel) { expect_label<Katals>("kat"); }

TEST(Katals, EquivalentToMolesPerSecond) {
    EXPECT_THAT(katals(2.0), QuantityEquivalent(moles(6.0) / seconds(3.0)));
}

}  // namespace au
