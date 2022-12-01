// Copyright 2022 Aurora Operations, Inc.

#include "au/units/katals.hh"

#include "au/testing.hh"
#include "au/units/moles.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Katals, HasExpectedLabel) { expect_label<Katals>("kat"); }

TEST(Katals, EquivalentToMolesPerSecond) {
    EXPECT_THAT(katals(2.0), QuantityEquivalent(moles(6.0) / seconds(3.0)));
}

}  // namespace au
