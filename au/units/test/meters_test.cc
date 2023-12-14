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

#include "au/units/meters.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/inches.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Meters, HasExpectedLabel) { expect_label<Meters>("m"); }

TEST(Meters, HasExpectedRelationshipsWithInches) { EXPECT_EQ(centi(meters)(254), inches(100)); }

TEST(Meters, HasExpectedSymbol) {
    using symbols::m;
    EXPECT_THAT(5 * m, SameTypeAndValue(meters(5)));
}

}  // namespace au
