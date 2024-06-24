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

namespace au {
namespace {
constexpr auto PI = Magnitude<Pi>{};

TEST(CommonSingleFile, HasExpectedUnits) {
    EXPECT_EQ(meters(1.23).in(meters), 1.23);
    EXPECT_EQ(seconds(1.23).in(seconds), 1.23);
    EXPECT_EQ(kilo(grams)(1.23).in(kilo(grams)), 1.23);
    EXPECT_EQ(kelvins(1.23).in(kelvins), 1.23);
    EXPECT_EQ(amperes(1.23).in(amperes), 1.23);
    EXPECT_EQ(moles(1.23).in(moles), 1.23);
    EXPECT_EQ(candelas(1.23).in(candelas), 1.23);
    EXPECT_EQ(radians(1.23).in(radians), 1.23);
    EXPECT_EQ(bits(1.23).in(bits), 1.23);
    EXPECT_EQ(unos(1.23).in(unos), 1.23);
}

TEST(CommonSingleFile, SupportsPrefixes) {
    EXPECT_EQ(kibi(bits)(1), bits(1024));
    EXPECT_EQ(centi(meters)(100), meters(1));
}

TEST(CommonSingleFile, SeamlesslyInteroperatesWithStdChronoDuration) {
    constexpr std::chrono::nanoseconds as_chrono = micro(seconds)(5);
    EXPECT_EQ(as_chrono, std::chrono::nanoseconds{5'000});
}

TEST(CommonSingleFile, IncludesMathFunctions) {
    EXPECT_EQ(round_as(meters, centi(meters)(187)), meters(2));
    EXPECT_DOUBLE_EQ(sin(radians(get_value<double>(PI / mag<2>()))), 1.0);
}

}  // namespace
}  // namespace au
