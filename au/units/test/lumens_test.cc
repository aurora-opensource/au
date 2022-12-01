// Copyright 2022 Aurora Operations, Inc.

#include "au/units/lumens.hh"

#include "au/testing.hh"
#include "au/units/candelas.hh"
#include "au/units/steradians.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Lumens, HasExpectedLabel) { expect_label<Lumens>("lm"); }

TEST(Lumens, EquivalentToCandelaSteradians) {
    EXPECT_THAT(lumens(6), QuantityEquivalent(candelas(2) * steradians(3)));
}

}  // namespace au
