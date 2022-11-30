// Copyright 2022 Aurora Operations, Inc.

#include "au/units/watts.hh"

#include "au/testing.hh"
#include "au/units/ohms.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Watts, HasExpectedLabel) { expect_label<Watts>("W"); }

TEST(Watts, EquivalentToJoulesPerSecond) {
    EXPECT_THAT(watts(12.34), QuantityEquivalent((joules / second)(12.34)));
}

TEST(Watts, EquivalentToOhmicHeating) {
    constexpr auto i = amperes(3.0);
    constexpr auto r = ohms(4.0);
    constexpr auto v = i * r;
    constexpr auto p = v * i;
    EXPECT_THAT(v * i, QuantityEquivalent(p));
    EXPECT_THAT(v * v / r, QuantityEquivalent(p));
    EXPECT_THAT(i * i * r, QuantityEquivalent(p));
}

}  // namespace au
