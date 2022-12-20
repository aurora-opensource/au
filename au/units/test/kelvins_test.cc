// Copyright 2022 Aurora Operations, Inc.

#include "au/units/kelvins.hh"

#include "au/testing.hh"
#include "au/units/celsius.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Kelvins, HasExpectedLabel) { expect_label<Kelvins>("K"); }

TEST(Kelvins, QuantityEquivalentToCelsius) {
    EXPECT_THAT(kelvins(10), QuantityEquivalent(celsius_qty(10)));
}

}  // namespace au
