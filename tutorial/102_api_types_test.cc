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

#include "tutorial/102_api_types.hh"

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::DoubleNear;

TEST(StoppingDistanceM, GivesCorrectAnswerForZeroSpeed) {
    const double speed_mps = 0.0;
    const double acceleration_mpss = -5.0;
    const double distance_m = stopping_distance_m(speed_mps, acceleration_mpss);

    // If we are stopped already, the stopping distance should be zero.
    EXPECT_THAT(distance_m, DoubleNear(0.0, 1e-9));
}

TEST(StoppingDistanceM, GivesCorrectAnswerForNonzeroSpeed) {
    const double speed_mps = 5.0;
    const double acceleration_mpss = -5.0;
    const double distance_m = stopping_distance_m(speed_mps, acceleration_mpss);

    // If we slam on the brakes at low speed, we can stop in a very short distance.
    EXPECT_THAT(distance_m, DoubleNear(2.5, 1e-9));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 1(a)
//
// Uncomment the following test cases.  Make them pass by declaring a new function in
// `102_api_types.hh` with this signature:
//
//     AAA stopping_distance(BBB speed, CCC acceleration);
//
// The types AAA, BBB, and CCC are _placeholders_ for quantity types.  The "Rep" for each should be
// `double`, since that's what the function we're replacing uses.  As for the Unit, you may find it
// useful to define aliases for, say, `MetersPerSecond` and `MetersPerSecondSquared`.
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 1(b)
//
// You should now be getting a linker error, instead of a compiler error.  To fix this, go to
// `102_api_types.cc`, and implement the function you had just declared.
//
// Your function implementation should amount to a thin wrapper on the existing function.  It should
// "unwrap" the input quantity parameters to get raw `double` values, and then "wrap" the answer
// with the appropriate quantity maker.
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(StoppingDistance, GivesCorrectAnswerForZeroSpeed) {
    // const auto speed = (meters / second)(0.0);
    // const auto acceleration = (meters / squared(second))(-5.0);
    // const auto distance = stopping_distance(speed, acceleration);

    // // If we are stopped already, the stopping distance should be zero.
    // EXPECT_THAT(distance, IsNear(meters(0), meters(1e-9)));
}

TEST(StoppingDistance, GivesCorrectAnswerForNonzeroSpeed) {
    // const auto speed = (meters / second)(5.0);
    // const auto acceleration = (meters / squared(second))(-5.0);
    // const auto distance = stopping_distance(speed, acceleration);

    // // If we slam on the brakes at low speed, we can stop in a very short distance.
    // EXPECT_THAT(distance, IsNear(meters(2.5), meters(1e-9)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 2
//
// Reverse the roles of the functions.  Use quantity types for the core logic, and turn the raw
// numeric version into the thin wrapper.
//
// All existing tests should pass without modification.
////////////////////////////////////////////////////////////////////////////////////////////////////
