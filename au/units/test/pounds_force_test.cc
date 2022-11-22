// Copyright 2022 Aurora Operations, Inc.

#include "au/units/pounds_force.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(PoundsForce, HasExpectedLabel) { expect_label<PoundsForce>("lbf"); }

TEST(PoundsForce, EquivalentToStandardGravityActingOnPoundMass) {
    EXPECT_THAT(pounds_force(123), QuantityEquivalent((pound_mass * standard_gravity)(123)));
}

}  // namespace au
