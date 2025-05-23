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

#include <iostream>

#include "au/io.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tutorial/utils.hh"

// For testing/tutorial purposes.
using namespace au;

using ::testing::DoubleEq;
using ::testing::StrEq;

// An API we'll upgrade later, along with some tests.
double stopping_accel_mpss(double initial_speed_mps, double stopping_distance_m);

TEST(StoppingAccelMpss, ReturnsZeroIfAlreadyStopped) {
    constexpr double speed_mps = 0.0;
    constexpr double stopping_distance_m = 1.0;
    EXPECT_THAT(stopping_accel_mpss(speed_mps, stopping_distance_m), DoubleEq(0.0));
}

TEST(StoppingAccelMpss, ReturnsCorrectAnswerForNonzeroValues) {
    constexpr double speed_mps = 20.0;
    constexpr double stopping_distance_m = 100.0;

    // Double-check that the distance covered is exactly the stopping distance.
    constexpr double expected_accel_mpss = -2.0;
    constexpr double t_s = -speed_mps / expected_accel_mpss;
    constexpr double distance_m = speed_mps * t_s + 0.5 * expected_accel_mpss * t_s * t_s;
    ASSERT_THAT(distance_m, DoubleEq(stopping_distance_m));

    EXPECT_THAT(stopping_accel_mpss(speed_mps, stopping_distance_m), DoubleEq(expected_accel_mpss));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 1(a)
//
// Uncomment the final two lines of the next function, and run the tutorial target, to see how the
// unit information is attached to the quantity.
//
//     bazel run //tutorial:101_quantity_makers
//
// What do you _expect_ to see?
//
// What _do_ you see?
////////////////////////////////////////////////////////////////////////////////////////////////////

void print_raw_number_and_quantity() {
    // Let's start by holding our physical quantity in a raw numeric type.
    // The suffix on the variable name is what keeps track of the units.
    constexpr double track_length_m = 100.0;

    // Create a _quantity type_ with a unit-safe handoff.
    // Now, the _type itself_ keeps track of the units.
    constexpr auto track_length = meters(track_length_m);

    // std::cout << "track_length_m: " << track_length_m << std::endl;
    // std::cout << "track_length: " << track_length << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 1(b)
//
// Uncomment the test assertions in the following test case.  Each one contains the empty string,
// "", as the expected answer.  This is just a placeholder.  Replace it with the answer you expect.
// For example, if you see this:
//
//     EXPECT_THAT(stream_to_string(squared(meters)(100)), StrEq(""));
//
// then you would replace it with this:
//
//     EXPECT_THAT(stream_to_string(squared(meters)(100)), StrEq("100 m^2"));
//
// TIP: you may find it useful to uncomment the lines one at a time.
////////////////////////////////////////////////////////////////////////////////////////////////////

TEST(Quantity, PrintsAsExpected) {
    // EXPECT_THAT(stream_to_string(meters(100)), StrEq(""));

    // EXPECT_THAT(stream_to_string(meters(100.0) / seconds(8.0)), StrEq(""));
    // EXPECT_THAT(stream_to_string((meters / second)(12.5)), StrEq(""));

    // EXPECT_THAT(stream_to_string((meters / second)(10.0) / seconds(8.0)), StrEq(""));
    // EXPECT_THAT(stream_to_string((meters / second)(10.0) * seconds(8.0)), StrEq(""));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// EXERCISE 2
//
// Replace the implementation of the following function with quantities, instead of raw doubles.
// You will probably want to do this in three stages.
//
//    1. Create a new quantity variable for each parameter.  It should have the same name, minus the
//       unit suffix.  For example, for a parameter `double duration_s`, you would write:
//
//           const auto duration = seconds(duration_s);
//
//    2. Replace the raw doubles in the core computation with their corresponding quantity
//       variables.  Note that you'll need to change both the type (to auto), and the name (to
//       eliminate the suffix).
//
//    3. Use `.in(...)` to extract the raw double to return.  You'll need to form the correct
//       quantity maker to pass to it.
//
// This first example is just a baby step, which doesn't show the real power or utility of the
// library.  We've made the computation safer, yes, but only internally to the function, and at the
// cost of some boilerplate.  In future lessons, we'll see how receiving and returning quantity
// types directly is much more powerful: it can make our implementation code simpler, and our
// callsite code safer and more readable.
////////////////////////////////////////////////////////////////////////////////////////////////////

double stopping_accel_mpss(double initial_speed_mps, double stopping_distance_m) {
    const double accel_mpss =
        -(initial_speed_mps * initial_speed_mps) / (2.0 * stopping_distance_m);

    return accel_mpss;
}

int main(int argc, char **argv) {
    print_raw_number_and_quantity();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
