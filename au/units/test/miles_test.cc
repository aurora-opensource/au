// Copyright 2022 Aurora Operations, Inc.

#include "au/units/miles.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Miles, HasExpectedLabel) { expect_label<Miles>("mi"); }

TEST(Miles, EquivalentTo5280Feet) { EXPECT_EQ(miles(1), feet(5280)); }

}  // namespace au
