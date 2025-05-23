// Copyright 2023 Aurora Operations, Inc.
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

#include "au/testing.hh"
#include "au/units/degrees.hh"
#include "au/units/hours.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/radians.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// For testing/tutorial purposes.
using namespace au;

using ::testing::DoubleEq;
using ::testing::Eq;

// Replace this constant with an appropriate conversion function, wherever it occurs.
constexpr auto PLACEHOLDER = ZERO;

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 1
//
// Migrate the following unit conversions to an inline Au-based solution.  Clean up the parts you no
// longer need.
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(AdHocConversions, DegreesToRadians) {
    constexpr double angle_deg = 135.0;

    constexpr double RAD_PER_DEG = M_PI / 180.0;

    // TODO: replace `angle_rad` computation with an ad hoc conversion, using Au.
    constexpr double angle_rad = angle_deg * RAD_PER_DEG;

    EXPECT_THAT(angle_rad, DoubleEq(3.0 * M_PI / 4.0));
}

TEST(AdHocConversions, MilesPerHourToMetersPerSecond) {
    constexpr double speed_mph = 65.0;

    // Carefully compute conversion factor manually.
    constexpr double M_PER_CM = 0.01;
    constexpr double CM_PER_INCH = 2.54;
    constexpr double INCHES_PER_FEET = 12.0;
    constexpr double FEET_PER_MILE = 5280.0;
    constexpr double M_PER_MILE = M_PER_CM * CM_PER_INCH * INCHES_PER_FEET * FEET_PER_MILE;

    constexpr double S_PER_H = 3600.0;

    constexpr double MPS_PER_MPH = M_PER_MILE / S_PER_H;

    // TODO: replace `speed_mps` computation with an ad hoc conversion, using Au.
    constexpr double speed_mps = speed_mph * MPS_PER_MPH;

    EXPECT_THAT(speed_mps, DoubleEq(29.0576));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 2
//
// Scroll down to the function `decompose_height()`, and fill in the `PLACEHOLDER` instances with
// correct expressions that will make the test below pass.
//
// You may find it useful to use the explicit-Rep overload to force a conversion that is _generally_
// risky, but in _this_ instance is known to be OK.  You should not need to use floating point
// numbers at all.
////////////////////////////////////////////////////////////////////////////////////////////////////

struct Height {
    QuantityU32<Feet> feet;
    QuantityU32<Inches> inches;
};

// This makes the tests easier to write.
bool operator==(Height a, Height b) { return (a.feet == b.feet) && (a.inches == b.inches); }

// This makes the tests easier to read when they fail.
std::ostream &operator<<(std::ostream &out, const Height &h) {
    out << h.feet << " " << h.inches;
    return out;
}

// Decompose a height, given in inches, into the largest whole number of feet, plus the leftover
// inches.  For example, `inches(17)` would be decomposed into `Height{feet(1), inches(5)}`.
Height decompose_height(QuantityU32<Inches> total_height) {
    Height h;
    h.feet = PLACEHOLDER;
    h.inches = PLACEHOLDER;
    return h;
}

TEST(Height, DecomposesCorrectly) {
    EXPECT_THAT(decompose_height(inches(60)), Eq(Height{feet(5), inches(0)}));
    EXPECT_THAT(decompose_height(inches(83)), Eq(Height{feet(6), inches(11)}));
}
