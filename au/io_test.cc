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

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "gtest/gtest.h"

namespace au {
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

struct Seconds : UnitImpl<Time> {
    static constexpr const char label[] = "s";
};
constexpr const char Seconds::label[];
constexpr auto second = SingularNameFor<Seconds>{};

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
    EXPECT_EQ(stream_to_string(feet(3)), "3 ft");
    EXPECT_EQ(stream_to_string((feet / milli(second))(1.25)), "1.25 ft / ms");
}

TEST(StreamingOutput, DistinguishesPointFromQuantityByAtSign) {
    EXPECT_EQ(stream_to_string(celsius_qty(20)), "20 deg C");
    EXPECT_EQ(stream_to_string(celsius_pt(20)), "@(20 deg C)");

    EXPECT_EQ(stream_to_string(kelvins(20)), "20 K");
    EXPECT_EQ(stream_to_string(kelvins_pt(20)), "@(20 K)");
}

TEST(StreamingOutput, PrintsZero) { EXPECT_EQ(stream_to_string(ZERO), "0"); }

}  // namespace au
