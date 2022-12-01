// Copyright 2022 Aurora Operations, Inc.

#include "au/units/steradians.hh"

#include "au/testing.hh"
#include "au/units/radians.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Steradians, HasExpectedLabel) { expect_label<Steradians>("sr"); }

TEST(Steradians, EquivalentToSquaredRadians) {
    EXPECT_THAT(steradians(6), QuantityEquivalent(radians(2) * radians(3)));
}

}  // namespace au
