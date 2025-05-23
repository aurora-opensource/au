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

// This file is meant to be `#include`d in a test file, after all other files.
//
// The point is to contain all of the test cases which both ready-made
// single-file versions of the library (i.e., with and without <iostream>)
// should pass.

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::DoubleEq;
using ::testing::Eq;
using ::testing::Lt;

namespace {

constexpr auto PI = Magnitude<Pi>{};

TEST(CommonSingleFile, HasExpectedUnits) {
    EXPECT_THAT(meters(1.23).in(meters), Eq(1.23));
    EXPECT_THAT(seconds(1.23).in(seconds), Eq(1.23));
    EXPECT_THAT(kilo(grams)(1.23).in(kilo(grams)), Eq(1.23));
    EXPECT_THAT(kelvins(1.23).in(kelvins), Eq(1.23));
    EXPECT_THAT(amperes(1.23).in(amperes), Eq(1.23));
    EXPECT_THAT(moles(1.23).in(moles), Eq(1.23));
    EXPECT_THAT(candelas(1.23).in(candelas), Eq(1.23));
    EXPECT_THAT(radians(1.23).in(radians), Eq(1.23));
    EXPECT_THAT(bits(1.23).in(bits), Eq(1.23));
    EXPECT_THAT(unos(1.23).in(unos), Eq(1.23));
}

TEST(CommonSingleFile, SupportsPrefixes) {
    EXPECT_THAT(kibi(bits)(1), Eq(bits(1024)));
    EXPECT_THAT(centi(meters)(100), Eq(meters(1)));
}

TEST(CommonSingleFile, SeamlesslyInteroperatesWithStdChronoDuration) {
    constexpr std::chrono::nanoseconds as_chrono = micro(seconds)(5);
    EXPECT_THAT(as_chrono, Eq(std::chrono::nanoseconds{5'000}));
}

TEST(CommonSingleFile, IncludesMathFunctions) {
    EXPECT_THAT(round_as(meters, centi(meters)(187)), Eq(meters(2)));
    EXPECT_THAT(sin(radians(get_value<double>(PI / mag<2>()))), DoubleEq(1.0));
}

TEST(CommonSingleFile, MixedSignQuantityComparisonWorks) {
    EXPECT_THAT(meters(-1), Lt(meters(1u)));
}

}  // namespace
}  // namespace au
