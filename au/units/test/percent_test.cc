// Copyright 2022 Aurora Operations, Inc.

#include "au/units/percent.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/unos.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Percent, HasExpectedLabel) { expect_label<Percent>("%"); }

TEST(Percent, OneHundredthOfUnos) { EXPECT_EQ(percent(75.0), unos(0.75)); }

}  // namespace au
