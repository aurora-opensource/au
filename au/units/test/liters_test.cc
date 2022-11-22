// Copyright 2022 Aurora Operations, Inc.

#include "au/units/liters.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Liters, HasExpectedLabel) { expect_label<Liters>("L"); }

TEST(Liters, HasExpectedRelationshipsWithLinearUnits) {
    EXPECT_EQ(liters(1), cubed(deci(meters))(1));

    // 1 mL == 1 c.c.
    EXPECT_EQ(milli(liters)(1), cubed(centi(meters))(1));
}

}  // namespace au
