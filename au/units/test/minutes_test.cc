// Copyright 2022 Aurora Operations, Inc.

#include "au/units/minutes.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Minutes, HasExpectedLabel) { expect_label<Minutes>("min"); }

TEST(Minutes, EquivalentTo60Seconds) { EXPECT_EQ(minutes(3), seconds(180)); }

}  // namespace au
