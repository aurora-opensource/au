// Copyright 2022 Aurora Operations, Inc.

#include "au/units/slugs.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/grams.hh"
#include "au/units/pounds_mass.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Slugs, HasExpectedLabel) { expect_label<Slugs>("slug"); }

TEST(Slugs, ExactDefinitionIsCorrect) {
    // The automatic conversions to the common unit here will cause overflow.  However, _unsigned_
    // integer overflow is well defined.  And if these values both overflow to the same number, it
    // adds confidence that the definition is correct.
    EXPECT_EQ(slugs(609'600'000'000ULL), kilo(grams)(8'896'443'230'521ULL));

    // These test cases check for _approximate_ correctness of the definition, within some
    // tolerance.  They complement the overflowing-integer test case just above.
    EXPECT_THAT(slugs(1.0), IsNear(kilo(grams)(14.59390293720636), nano(grams)(1)));
    EXPECT_THAT(slugs(1.0), IsNear(pounds_mass(32.174), milli(pounds_mass)(1)));
}

}  // namespace au
