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

#include "au/io.hh"

#include <cstdint>

#include "au/constants/speed_of_light.hh"
#include "au/prefix.hh"
#include "au/quantity.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::StrEq;

namespace au {

using ::testing::StrEq;

namespace {

template <typename T>
std::string stream_to_string(const T &x) {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}

struct Feet : UnitImpl<Length> {
    static constexpr const char label[] = "ft";
};
constexpr const char Feet::label[];
constexpr auto feet = QuantityMaker<Feet>{};

struct Kelvins : UnitImpl<Temperature> {
    static constexpr const char label[] = "K";
};
constexpr const char Kelvins::label[];
constexpr auto kelvins = QuantityMaker<Kelvins>{};
constexpr auto kelvins_pt = QuantityPointMaker<Kelvins>{};

struct Celsius : Kelvins {
    static constexpr const char label[] = "deg C";
    static constexpr auto origin() { return centi(kelvins)(273'15); }
};
constexpr const char Celsius::label[];
constexpr auto celsius_qty = QuantityMaker<Celsius>{};
constexpr auto celsius_pt = QuantityPointMaker<Celsius>{};

}  // namespace

TEST(StreamingOutput, PrintsValueAndUnitLabel) {
    EXPECT_THAT(stream_to_string(feet(3)), StrEq("3 ft"));
    EXPECT_THAT(stream_to_string((feet / milli(second))(1.25)), StrEq("1.25 ft / ms"));
}

TEST(StreamingOutput, PrintValueRepChar) {
    // If the Rep resolves to a char, we sill want the number '65' to be output,
    // not the character literal that corresponds to 65 ('A').
    static_assert(std::is_same<int8_t, signed char>::value,
                  "Expected 'int8_t' to resolve to 'char'");
    EXPECT_THAT(stream_to_string(feet(int8_t{65})), StrEq("65 ft"));
}

TEST(StreamingOutput, DistinguishesPointFromQuantityByAtSign) {
    EXPECT_THAT(stream_to_string(celsius_qty(20)), StrEq("20 deg C"));
    EXPECT_THAT(stream_to_string(celsius_pt(20)), StrEq("@(20 deg C)"));

    EXPECT_THAT(stream_to_string(kelvins(20)), StrEq("20 K"));
    EXPECT_THAT(stream_to_string(kelvins_pt(20)), StrEq("@(20 K)"));
}

TEST(StreamingOutput, PrintsZero) { EXPECT_THAT(stream_to_string(ZERO), StrEq("0")); }

TEST(StreamingOutput, PrintsMagnitude) {
    EXPECT_THAT(stream_to_string(mag<289374>()), StrEq("289374"));
    EXPECT_THAT(stream_to_string(mag<22>() / mag<7>()), StrEq("22 / 7"));
}

TEST(StreamingOutput, PrintsDefaultLabelForMagnitudeWeCantLabelYet) {
    EXPECT_THAT(stream_to_string(cbrt(Magnitude<Pi>{})), StrEq("(UNLABELED SCALE FACTOR)"));
}

TEST(StreamingOutput, PrintsUnitLabelForConstant) {
    EXPECT_THAT(stream_to_string(SPEED_OF_LIGHT), StrEq("c"));
    EXPECT_THAT(stream_to_string(SPEED_OF_LIGHT * mag<3>() / mag<4>()), StrEq("[(3 / 4) c]"));
}

TEST(StreamingOutput, PrintsUnitLabelForSymbol) {
    EXPECT_THAT(stream_to_string(symbol_for(feet)), StrEq("ft"));
}

}  // namespace au
