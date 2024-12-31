// Copyright 2024 Aurora Operations, Inc.
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

#include "au/constants/reduced_planck_constant.hh"

#include "au/constants/planck_constant.hh"
#include "au/testing.hh"
#include "au/units/joules.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using symbols::J;
using symbols::s;
using ::testing::StrEq;

TEST(ReducedPlanckConstant, HasExpectedValue) {
    // h_bar = (6.62607015e-34 / (2 pi)) J s ~= 1.054571817e-34 J s

    // Test approximate value (guard against powers-of-10 type errors).
    constexpr auto defining_units = joule * seconds * pow<-34>(mag<10>());
    constexpr auto val = defining_units(1.054571817);
    constexpr auto err = defining_units(0.000000001);
    EXPECT_THAT(REDUCED_PLANCK_CONSTANT.as<double>(J * s), IsNear(val, err));
}

TEST(ReducedPlanckConstant, ExactlyPlanckConstantDividedByTwoPi) {
    constexpr auto ratio = (REDUCED_PLANCK_CONSTANT * mag<2>() * Magnitude<Pi>{}) / PLANCK_CONSTANT;

    // We know that `.as<int>()` could not succeed unless the result were exactly expressible with
    // `int` as a rep, and we know that the comparison to `1` would not compile unless all
    // dimensions had cancelled out.
    constexpr int result = ratio.as<int>();
    EXPECT_EQ(result, 1);
}

TEST(ReducedPlanckConstant, HasExpectedLabel) {
    EXPECT_THAT(unit_label(REDUCED_PLANCK_CONSTANT), StrEq("h_bar"));
}

}  // namespace
}  // namespace au
