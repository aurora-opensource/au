// Copyright 2022 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

TEST(Slugs, HasExpectedSymbol) {
    using symbols::slug;
    EXPECT_THAT(5 * slug, SameTypeAndValue(slugs(5)));
}

}  // namespace au
