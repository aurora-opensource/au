// Copyright 2022 Aurora Operations, Inc.

#include "au/units/bars.hh"

#include "au/testing.hh"
#include "au/units/pascals.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Bars, HasExpectedLabel) { expect_label<Bars>("bar"); }

TEST(Bars, HasCorrectRelationshipWithPascals) { EXPECT_EQ(bars(1), kilo(pascals)(100)); }

}  // namespace au
