// Copyright 2022 Aurora Operations, Inc.

#include "au/units/hours.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Hours, HasExpectedLabel) { expect_label<Hours>("h"); }

TEST(Hours, EquivalentTo60Minutes) { EXPECT_EQ(hours(3), minutes(180)); }

}  // namespace au
