// Copyright 2022 Aurora Operations, Inc.

#include "au/units/hertz.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Hertz, HasExpectedLabel) { expect_label<Hertz>("Hz"); }

TEST(Hertz, EquivalentToInverseSeconds) {
    EXPECT_THAT(hertz(5.5), QuantityEquivalent(inverse(seconds)(5.5)));
}

}  // namespace au
