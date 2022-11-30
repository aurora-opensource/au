// Copyright 2022 Aurora Operations, Inc.

#include "au/units/seconds.hh"

#include "au/testing.hh"
#include "au/units/minutes.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Seconds, HasExpectedLabel) { expect_label<Seconds>("s"); }

TEST(Seconds, SixtyPerMinute) { EXPECT_EQ(seconds(60), minutes(1)); }

}  // namespace au
