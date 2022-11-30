// Copyright 2022 Aurora Operations, Inc.

#include "au/units/radians.hh"

#include "au/testing.hh"
#include "au/units/revolutions.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Radians, HasExpectedLabel) { expect_label<Radians>("rad"); }

TEST(Radians, TwoPiPerRevolution) {
    EXPECT_DOUBLE_EQ(radians(get_value<double>(mag<2>() * PI)).in(revolutions), 1.0);
}

}  // namespace au
