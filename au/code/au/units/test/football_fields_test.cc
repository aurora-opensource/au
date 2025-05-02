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

#include "au/units/football_fields.hh"

#include "au/testing.hh"
#include "au/units/meters.hh"
#include "au/units/yards.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;
using ::testing::Lt;

TEST(FootballFields, HasExpectedLabel) { expect_label<FootballFields>("ftbl_fld"); }

TEST(FootballFields, HasCorrectQuantityRelationshipWithYards) {
    EXPECT_THAT(football_fields(1), Eq(yards(100)));
}

TEST(FootballFields, FourFootballFieldsIsLessThanKnownFirstlightLidarRange) {
    // Sources:
    // https://blog.aurora.tech/progress/firstlight-lidar-on-a-chip
    // https://ir.aurora.tech/news-events/press-releases/detail/119/aurora-begins-commercial-driverless-trucking-in-texas
    EXPECT_THAT(football_fields(4), Lt(meters(400)));
}

TEST(FootballFields, HasExpectedSymbol) {
    using symbols::ftbl_fld;
    EXPECT_THAT(5 * ftbl_fld, SameTypeAndValue(football_fields(5)));
}

}  // namespace au
