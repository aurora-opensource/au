// Copyright 2026 Aurora Operations, Inc.
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

// This test exercises the C++ module interface (`au/au.cppm`) via `import au;`.

#include <chrono>
#include <limits>
#include <sstream>
#include <type_traits>
#include <version>

#ifdef __cpp_lib_format
#include <format>
#endif

#include "gmock/gmock.h"
#include "gtest/gtest.h"

import au;

using au::symbols::V;
using au::symbols::W;
using namespace au::au_literals;

using ::testing::StrEq;

namespace au {

TEST(AuModule, ProvidesQuantityMakersAndPrefixes) {
    constexpr auto distance = meters(1500.0);
    static_assert(distance.in(kilo(meters)) == 1.5, "");
}

TEST(AuModule, ProvidesZero) { static_assert(meters(1) > ZERO, ""); }

TEST(AuModule, ProvidesMagnitudes) { static_assert(get_value<int>(mag<360>()) == 360, ""); }

TEST(AuModule, ProvidesUnitSymbols) {
    constexpr auto voltage = 12.0 * V;
    static_assert(voltage == volts(12.0), "");
}

TEST(AuModule, ProvidesUnitLiterals) {
    constexpr auto current = 2.5_A;
    static_assert(current == amperes(2.5), "");
}

TEST(AuModule, SupportsQuantityAlgebra) {
    constexpr auto power = volts(12.0) * amperes(2.5);
    static_assert(power == watts(30.0), "");
}

TEST(AuModule, ProvidesQuantityPointMakers) {
    constexpr auto temperature = celsius_pt(20);
    static_assert(temperature.in(celsius_pt) == 20, "");
}

TEST(AuModule, ProvidesMathFunctions) {
    EXPECT_EQ(max(feet(3), feet(4)), feet(4));
    EXPECT_EQ(round_as(meters, centi(meters)(178)), meters(2.0));
}

TEST(AuModule, ProvidesConstants) {
    static_assert(SPEED_OF_LIGHT.as<int>(meters / second) ==
                      (meters / second)(299'792'458),
                  "");
}

TEST(AuModule, ConversionRiskPoliciesWorkViaArgumentDependentLookup) {
    // `check_for()` and `ignore()` are hidden friends of the risk-set types, so they need no
    // using-declarations in `au.cppm`; ADL must find them through `au::OVERFLOW_RISK`, etc.
    constexpr auto risky = meters(std::numeric_limits<int>::max());
    EXPECT_FALSE(will_conversion_overflow(risky, kilo(meters)));
    (void)check_for(OVERFLOW_RISK);
    (void)ignore(ALL_RISKS);
}

TEST(AuModule, SupportsChronoInterop) {
    // Exercises the `CorrespondingQuantity` specializations declared in the global module
    // fragment (both the generic `std::chrono::duration` one, and an explicit one).
    EXPECT_EQ(as_quantity(std::chrono::seconds{3}), seconds(3));
    EXPECT_EQ(as_chrono_duration(milli(seconds)(250)), std::chrono::milliseconds{250});
}

TEST(AuModule, SupportsStdSpecializations) {
    // Exercises the `std::numeric_limits` and `std::common_type` specializations declared in
    // the global module fragment.
    static_assert(std::numeric_limits<QuantityI32<Meters>>::max() ==
                      meters(std::numeric_limits<int32_t>::max()),
                  "");
    static_assert(
        std::is_same<std::common_type_t<QuantityD<Meters>, QuantityD<Meters>>,
                     QuantityD<Meters>>::value,
        "");
}

TEST(AuModule, SupportsStreamingOutput) {
    std::ostringstream oss;
    oss << milli(amperes)(15) << " and " << ZERO << " and " << W;
    EXPECT_EQ(oss.str(), "15 mA and 0 and W");
}

TEST(AuModule, ProvidesRepSpecificAliases) {
    static_assert(std::is_same<QuantityF<Hertz>, Quantity<Hertz, float>>::value,
                  "");
}

#ifdef __cpp_lib_format
TEST(AuModule, StdFormatExportedFromModule) {
    EXPECT_THAT(std::format("{}", meters(8.5)), StrEq("8.5 m"));
}
#endif

}  // namespace au
