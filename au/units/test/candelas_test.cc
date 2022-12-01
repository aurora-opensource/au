// Copyright 2022 Aurora Operations, Inc.

#include "au/units/candelas.hh"

#include "au/testing.hh"
#include "au/units/lumens.hh"
#include "au/units/steradians.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Candelas, HasExpectedLabel) { expect_label<Candelas>("cd"); }

TEST(Candelas, EquivalentToLumensPerSteradian) {
    EXPECT_THAT(candelas(2.0), QuantityEquivalent(lumens(6.0) / steradians(3.0)));
}

}  // namespace au
