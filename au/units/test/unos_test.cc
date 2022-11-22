// Copyright 2022 Aurora Operations, Inc.

#include "au/units/unos.hh"

#include "au/testing.hh"
#include "au/units/percent.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Unos, HasExpectedLabel) { expect_label<Unos>("U"); }

TEST(Unos, OneHundredPercent) { EXPECT_EQ(unos(2), percent(200)); }

TEST(Unos, ImplicitlyConvertToRawNumbers) {
    constexpr double x = unos(1.23);
    EXPECT_THAT(x, SameTypeAndValue(1.23));
}

}  // namespace au
