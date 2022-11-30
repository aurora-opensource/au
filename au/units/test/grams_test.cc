// Copyright 2022 Aurora Operations, Inc.

#include "au/units/grams.hh"

#include "au/testing.hh"
#include "au/units/pounds_mass.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Grams, HasExpectedLabel) { expect_label<Grams>("g"); }

TEST(Grams, HasCorrectRelationshipWithPoundsMass) {
    EXPECT_EQ(micro(grams)(453'592'370L), pounds_mass(1L));
}

}  // namespace au
