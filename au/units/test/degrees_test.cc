// Copyright 2022 Aurora Operations, Inc.

#include "au/units/degrees.hh"

#include "au/testing.hh"
#include "au/units/revolutions.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Degrees, HasExpectedLabel) { expect_label<Degrees>("deg"); }

TEST(Degrees, RoughlyEquivalentToPiOver180Radians) {
    EXPECT_DOUBLE_EQ(degrees(1.0).in(radians), get_value<double>(PI / mag<180>()));
}

TEST(Degrees, One360thOfARevolution) { EXPECT_EQ(degrees(360), revolutions(1)); }

}  // namespace au
