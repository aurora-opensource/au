// Copyright 2022 Aurora Operations, Inc.

#include "au/units/yards.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Yards, HasExpectedLabel) { expect_label<Yards>("yd"); }

TEST(Yards, EquivalentTo3Feet) { EXPECT_EQ(yards(1), feet(3)); }

}  // namespace au
