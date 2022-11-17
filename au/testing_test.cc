// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

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
