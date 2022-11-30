// Copyright 2022 Aurora Operations, Inc.

#include "au/units/inches.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Inches, HasExpectedLabel) { expect_label<Inches>("in"); }

TEST(Inches, EquivalentTo2Point54CentiMeters) { EXPECT_EQ(centi(meters)(254), inches(100)); }

}  // namespace au
