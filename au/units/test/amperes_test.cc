// Copyright 2022 Aurora Operations, Inc.

#include "au/units/amperes.hh"

#include "au/testing.hh"
#include "au/units/ohms.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Amperes, HasExpectedLabel) { expect_label<Amperes>("A"); }

TEST(Amperes, SatisfiesOhmsLaw) {
    EXPECT_THAT(amperes(2.0), QuantityEquivalent(volts(8.0) / ohms(4.0)));
}

TEST(Amperes, ProductWithVoltsGivesPower) {
    EXPECT_THAT(amperes(2.0), QuantityEquivalent(watts(8.0) / volts(4.0)));
}

}  // namespace au
