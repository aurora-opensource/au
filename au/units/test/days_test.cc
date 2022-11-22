// Copyright 2022 Aurora Operations, Inc.

#include "au/units/days.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Days, EquivalentTo24Hours) { EXPECT_EQ(days(1), hours(24)); }

}  // namespace au
