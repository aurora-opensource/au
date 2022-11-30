// Copyright 2022 Aurora Operations, Inc.

#include "au/units/fahrenheit.hh"

#include "au/testing.hh"
#include "au/units/celsius.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Fahrenheit, HasExpectedLabel) { expect_label<Fahrenheit>("F"); }

TEST(Fahrenheit, HasCorrectQuantityRelationshipWithCelsius) {
    EXPECT_EQ(fahrenheit(9), celsius(5));
}

TEST(Fahrenheit, HasCorrectRelationshipsWithCelsius) {
    EXPECT_THAT(fahrenheit_pt(32.0).as(celsius_pt), SameTypeAndValue(celsius_pt(0.0)));
    EXPECT_THAT(fahrenheit_pt(212.0).as(celsius_pt), SameTypeAndValue(celsius_pt(100.0)));
}

}  // namespace au
