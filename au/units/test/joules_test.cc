// Copyright 2022 Aurora Operations, Inc.

#include "au/units/joules.hh"

#include "au/testing.hh"
#include "au/units/meters.hh"
#include "au/units/newtons.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Joules, HasExpectedLabel) { expect_label<Joules>("J"); }

TEST(Joules, EquivalentToNewtonMeters) { EXPECT_EQ(joules(18), (newton * meters)(18)); }

}  // namespace au
