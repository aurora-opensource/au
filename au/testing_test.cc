// Copyright 2022 Aurora Operations, Inc.

#include "au/testing.hh"

#include "gtest/gtest.h"

using ::testing::Not;

namespace au {

TEST(SameTypeAndValue, FailsUnlessBothAreSame) {
    EXPECT_THAT(312, SameTypeAndValue(312));
    EXPECT_THAT(312, Not(SameTypeAndValue(314)));
    EXPECT_THAT(312, Not(SameTypeAndValue(312u)));
}

}  // namespace au
