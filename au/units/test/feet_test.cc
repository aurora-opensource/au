// Copyright 2022 Aurora Operations, Inc.

#include "au/units/feet.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Feet, HasExpectedLabel) { expect_label<Feet>("ft"); }

TEST(Feet, EquivalentTo12Inches) { EXPECT_EQ(feet(1), inches(12)); }

}  // namespace au
