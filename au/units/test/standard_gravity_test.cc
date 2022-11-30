// Copyright 2022 Aurora Operations, Inc.

#include "au/units/standard_gravity.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {

TEST(StandardGravity, HasExpectedLabel) { expect_label<StandardGravity>("g_0"); }

TEST(StandardGravity, HasExpectedValue) {
    EXPECT_EQ(standard_gravity(1L), (micro(meters) / squared(second))(9'806'650L));
}

}  // namespace au
