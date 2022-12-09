// Copyright 2022 Aurora Operations, Inc.

#include "au/units/celsius.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/fahrenheit.hh"
#include "au/units/kelvins.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Celsius, HasExpectedLabel) { expect_label<Celsius>("degC"); }

TEST(Celsius, QuantityEquivalentToKelvins) {
    EXPECT_THAT(celsius_qty(20), QuantityEquivalent(kelvins(20)));
}

TEST(Celsius, QuantityPointHasCorrectOffsetFromKelvins) {
    EXPECT_EQ(milli(kelvins_pt)(273'150).as(milli(celsius_pt)), milli(celsius_pt)(0));
}

TEST(Celsius, QuantityPointMatchesUpCorrectlyWithFahrenheit) {
    EXPECT_EQ(celsius_pt(0), fahrenheit_pt(32));
    EXPECT_EQ(celsius_pt(100), fahrenheit_pt(212));
}

}  // namespace au
