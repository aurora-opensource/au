// Copyright 2023 Aurora Operations, Inc.

#include "au/testing.hh"
#include "au/units/degrees.hh"
#include "au/units/hours.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/radians.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

// For testing/tutorial purposes.
using namespace au;

// Replace this constant with an appropriate conversion function, wherever it occurs.
constexpr auto PLACEHOLDER = ZERO;

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 1
//
// Make the following test cases pass by making ad hoc conversions using Au.  Wherever you see
// `PLACEHOLDER`, you should replace it with the conversion.
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(AdHocConversions, DegreesToRadians) {
    constexpr double angle_deg = 65.0;

    // Old-style, manual conversion.
    constexpr double RAD_PER_DEG = M_PI / 180.0;
    constexpr double angle_rad_manual = angle_deg * RAD_PER_DEG;

    // TODO: make an ad hoc conversion, using Au:
    constexpr double angle_rad_au = PLACEHOLDER;
    EXPECT_DOUBLE_EQ(angle_rad_au, angle_rad_manual);
}

TEST(AdHocConversions, MilesPerHourToMetersPerSecond) {
    constexpr double speed_mph = 65.0;

    // Old-style, manual conversion.
    constexpr double M_PER_CM = 0.01;
    constexpr double CM_PER_INCH = 2.54;
    constexpr double INCHES_PER_FEET = 12.0;
    constexpr double FEET_PER_MILE = 5280.0;
    constexpr double M_PER_MILE = M_PER_CM * CM_PER_INCH * INCHES_PER_FEET * FEET_PER_MILE;

    constexpr double S_PER_H = 3600.0;

    constexpr double MPS_PER_MPH = M_PER_MILE / S_PER_H;

    constexpr double speed_mps_manual = speed_mph * MPS_PER_MPH;

    // TODO: make an ad hoc conversion, using Au:
    constexpr double speed_mps_au = PLACEHOLDER;
    EXPECT_DOUBLE_EQ(speed_mps_au, speed_mps_manual);
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
    EXPECT_EQ(decompose_height(inches(60)), (Height{feet(5), inches(0)}));
    EXPECT_EQ(decompose_height(inches(83)), (Height{feet(6), inches(11)}));
}
