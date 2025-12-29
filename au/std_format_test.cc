// Copyright 2025 Aurora Operations, Inc.
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

#include "au/std_format.hh"

#include "au/prefix.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::StrEq;

using symbols::m;
using symbols::s;
constexpr auto cm = centi(m);
constexpr auto km = kilo(m);

TEST(StdFormat, PrintsQuantityAndUnitLabelByDefault) {
    EXPECT_THAT(std::format("{}", meters(8.5)), StrEq("8.5 m"));
}

TEST(StdFormat, DefaultFormatAppliesToNumberPart) {
    EXPECT_THAT(std::format("{:,<10}", meters(8.5)), StrEq("8.5,,,,,,, m"));
    EXPECT_THAT(std::format("{:,>10}", meters(8.5)), StrEq(",,,,,,,8.5 m"));
    EXPECT_THAT(std::format("{:,>8.2f}", meters(0.1234)), StrEq(",,,,0.12 m"));
}

TEST(StdFormat, CanFormatUnitLabelWithUPrefix) {
    EXPECT_THAT(std::format("{:U4}", meters(8.5)), StrEq("8.5 m   "));
    //                                             alignment: 1234
}

TEST(StdFormat, CanFormatBothParts) {
    // Overall width of 23: 10 for the number, 1 for the space, 12 for the label.

    EXPECT_THAT(std::format("{:U12;*>10.3f}", 123.456789 * cm / s),
                StrEq("***123.457 cm / s      "));
    //                 alignment: 123456789012
}

TEST(StdFormat, DocExamplesAreCorrect) {
    EXPECT_THAT(std::format("{}", meters(123.456)), StrEq("123.456 m"));
    EXPECT_THAT(std::format("{:~^10.2f}", meters(123.456)), StrEq("~~123.46~~ m"));
    EXPECT_THAT(std::format("{:U5}", meters(123.456)), StrEq("123.456 m    "));
    EXPECT_THAT(std::format("{:U5;~^10.2f}", meters(123.456)), StrEq("~~123.46~~ m    "));

    constexpr auto speed = (centi(meters) / second)(987.654321);
    EXPECT_THAT(std::format("{:.^31}", std::format("{:U10;,<8.2f}", speed)),
                StrEq("......987.65,, cm / s    ......"));

    constexpr auto c = 299'792.458 * km / s;
    EXPECT_THAT(std::format("{:,<12.2f}", c.data_in(km / s)), StrEq("299792.46,,,"));
}

}  // namespace
}  // namespace au
