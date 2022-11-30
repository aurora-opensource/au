// Copyright 2022 Aurora Operations, Inc.

#include "au/units/coulombs.hh"

#include "au/testing.hh"
#include "au/units/amperes.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Coulombs, HasExpectedLabel) { expect_label<Coulombs>("C"); }

TEST(Coulombs, EquivalentToAmpereSeconds) {
    EXPECT_THAT(coulombs(10), QuantityEquivalent(amperes(2) * seconds(5)));
}

}  // namespace au
