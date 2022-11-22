// Copyright 2022 Aurora Operations, Inc.

#include "au/units/ohms.hh"

#include "au/testing.hh"
#include "au/units/ohms.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Ohms, HasExpectedLabel) { expect_label<Ohms>("ohm"); }

TEST(Ohms, SatisfiesOhmsLaw) {
    EXPECT_THAT(ohms(4.0), QuantityEquivalent(volts(8.0) / amperes(2.0)));
}

TEST(Ohms, SatisfiesOhmicHeatingEquation) {
    // P = I^2 R   -->  R = P / I^2
    EXPECT_EQ(ohms(10.0), watts(250.0) / (amperes(5.0) * amperes(5.0)));

    // P = V^2 / R -->  R = V^2 / P
    EXPECT_EQ(ohms(10.0), (volts(50.0) * volts(50.0)) / watts(250.0));
}

}  // namespace au
