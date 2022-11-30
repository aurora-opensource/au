// Copyright 2022 Aurora Operations, Inc.

#include "au/units/pounds_mass.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(PoundsMass, HasExpectedLabel) { expect_label<PoundsMass>("lb"); }

TEST(PoundsMass, EquivalentToAppropriateQuantityOfKilograms) {
    EXPECT_EQ(pounds_mass(100'000'000L), (kilo(grams)(45'359'237L)));
}

}  // namespace au
