// Copyright 2022 Aurora Operations, Inc.

#include "au/units/moles.hh"

#include "au/testing.hh"
#include "au/units/katals.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Moles, HasExpectedLabel) { expect_label<Moles>("mol"); }

TEST(Moles, EquivalentToKatalSeconds) {
    EXPECT_THAT(moles(6), QuantityEquivalent(katals(2) * seconds(3)));
}

}  // namespace au
