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

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "fmt/format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace fmt {
struct FmtFormatterImpl {
    template <typename OutIter, typename... Args>
    static auto format_to(OutIter out, const char *fmt_str, Args &&...args) {
        return fmt::format_to(out, fmt_str, std::forward<Args>(args)...);
    }
};

template <typename U, typename R>
struct formatter<::au::Quantity<U, R>>
    : ::au::QuantityFormatter<U, R, ::fmt::formatter, FmtFormatterImpl> {};
}  // namespace fmt

namespace au {
namespace {

using symbols::m;
using symbols::s;
constexpr auto cm = centi(m);
constexpr auto km = kilo(m);

using ::testing::StrEq;

TEST(Fmt, PrintsQuantityAndUnitLabelByDefault) {
    EXPECT_THAT(fmt::format("{}", meters(8.5)), StrEq("8.5 m"));
}

TEST(Fmt, DefaultFormatAppliesToNumberPart) {
    EXPECT_THAT(fmt::format("{:,<10}", meters(8.5)), StrEq("8.5,,,,,,, m"));
    EXPECT_THAT(fmt::format("{:,>10}", meters(8.5)), StrEq(",,,,,,,8.5 m"));
    EXPECT_THAT(fmt::format("{:,>8.2f}", meters(0.1234)), StrEq(",,,,0.12 m"));
}

TEST(Fmt, CanFormatUnitLabelWithUPrefix) {
    EXPECT_THAT(fmt::format("{:U,^5}", meters(8.5)), StrEq("8.5 ,,m,,"));
}

TEST(Fmt, CanFormatBothParts) {
    // Overall width of 20: 10 for the number, 10 for the label.

    EXPECT_THAT(fmt::format("{:*>10.3f;U,<10}", 123.456789 * cm / s),
                StrEq("***123.457 cm / s,,,,"));
}

TEST(Fmt, DocExamplesAreCorrect) {
    EXPECT_THAT(fmt::format("{}", meters(123.456)), StrEq("123.456 m"));
    EXPECT_THAT(fmt::format("{:~^10.2f}", meters(123.456)), StrEq("~~123.46~~ m"));
    EXPECT_THAT(fmt::format("{:U.>5}", meters(123.456)), StrEq("123.456 ....m"));
    EXPECT_THAT(fmt::format("{:~^10.2f;U.>5}", meters(123.456)), StrEq("~~123.46~~ ....m"));

    constexpr auto speed = (centi(meters) / second)(987.654321);
    EXPECT_THAT(fmt::format("{:.^31}", fmt::format("{:,<8.2f;U*>10}", speed)),
                StrEq("......987.65,, ****cm / s......"));

    constexpr auto c = 299'792.458 * km / s;
    EXPECT_THAT(fmt::format("{:,<12.2f}", c.data_in(km / s)), StrEq("299792.46,,,"));
}

}  // namespace
}  // namespace au
