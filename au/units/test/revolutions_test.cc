// Copyright 2022 Aurora Operations, Inc.

#include "au/units/revolutions.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Revolutions, HasExpectedLabel) { expect_label<Revolutions>("rev"); }

TEST(Revolutions, ExactlyEquivalentTo360Degrees) { EXPECT_EQ(revolutions(1), degrees(360)); }

}  // namespace au
