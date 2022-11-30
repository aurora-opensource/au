// Copyright 2022 Aurora Operations, Inc.

#include "au/units/pascals.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Pascals, HasExpectedLabel) { expect_label<Pascals>("Pa"); }

TEST(Pascals, EquivalentToForcePerArea) {
    EXPECT_THAT(pascals(12), QuantityEquivalent((newtons / squared(meter))(12)));
}

}  // namespace au
