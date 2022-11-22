// Copyright 2022 Aurora Operations, Inc.

#include "au/units/volts.hh"

#include "au/testing.hh"
#include "au/units/amperes.hh"
#include "au/units/ohms.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Volts, HasExpectedLabel) { expect_label<Volts>("V"); }

TEST(Volts, SatisfiesOhmsLaw) { EXPECT_THAT(volts(8), QuantityEquivalent(amperes(2) * ohms(4))); }

}  // namespace au
