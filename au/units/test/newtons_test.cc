// Copyright 2022 Aurora Operations, Inc.

#include "au/units/newtons.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Newtons, HasExpectedLabel) { expect_label<Newtons>("N"); }

TEST(Newtons, EquivalentToKilogramMetersPerSquaredSecond) {
    EXPECT_THAT(newtons(8), QuantityEquivalent((kilo(gram) * meters / squared(second))(8)));
}

}  // namespace au
