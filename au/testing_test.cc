// Copyright 2022 Aurora Operations, Inc.

#include "au/testing.hh"

#include "gtest/gtest.h"

using ::testing::Not;

namespace au {

struct Compound {
    int first;
    double second;
};

TEST(FirstSeemsLikeDataMemberOfSecond, TrueForEachMember) {
    constexpr auto compound = Compound{1, 2.3};

    EXPECT_TRUE(first_seems_like_data_member_of_second(compound.first, compound));
    EXPECT_FALSE(first_seems_like_data_member_of_second(compound, compound.first));

    EXPECT_TRUE(first_seems_like_data_member_of_second(compound.second, compound));
    EXPECT_FALSE(first_seems_like_data_member_of_second(compound, compound.second));
}

TEST(SameTypeAndValue, FailsUnlessBothAreSame) {
    EXPECT_THAT(312, SameTypeAndValue(312));
    EXPECT_THAT(312, Not(SameTypeAndValue(314)));
    EXPECT_THAT(312, Not(SameTypeAndValue(312u)));
}

}  // namespace au
