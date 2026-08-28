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

// This test exercises the C++ module interface (`au/au.cppm`): it deliberately includes no Au
// headers, and gets the entire library via `import au;`.

#include <chrono>
#include <limits>
#include <sstream>
#include <type_traits>

#include "gtest/gtest.h"

import au;

using namespace au::au_literals;

namespace {

TEST(AuModule, ProvidesQuantityMakersAndPrefixes) {
    constexpr auto distance = au::meters(1500.0);
    static_assert(distance.in(au::kilo(au::meters)) == 1.5, "");
}

TEST(AuModule, ProvidesZero) { static_assert(au::meters(1) > au::ZERO, ""); }

TEST(AuModule, ProvidesMagnitudes) { static_assert(au::get_value<int>(au::mag<360>()) == 360, ""); }

TEST(AuModule, ProvidesUnitSymbols) {
    constexpr auto voltage = 12.0 * au::symbols::V;
    static_assert(voltage == au::volts(12.0), "");
}

TEST(AuModule, ProvidesUnitLiterals) {
    constexpr auto current = 2.5_A;
    static_assert(current == au::amperes(2.5), "");
}

TEST(AuModule, SupportsQuantityAlgebra) {
    constexpr auto power = au::volts(12.0) * au::amperes(2.5);
    static_assert(power == au::watts(30.0), "");
}

TEST(AuModule, ProvidesQuantityPointMakers) {
    constexpr auto temperature = au::celsius_pt(20);
    static_assert(temperature.in(au::celsius_pt) == 20, "");
}

TEST(AuModule, ProvidesMathFunctions) {
    EXPECT_EQ(au::max(au::feet(3), au::feet(4)), au::feet(4));
    EXPECT_EQ(au::round_as(au::meters, au::centi(au::meters)(178)), au::meters(2.0));
}

TEST(AuModule, ProvidesConstants) {
    static_assert(au::SPEED_OF_LIGHT.as<int>(au::meters / au::second) ==
                      (au::meters / au::second)(299'792'458),
                  "");
}

TEST(AuModule, ConversionRiskPoliciesWorkViaArgumentDependentLookup) {
    // `check_for()` and `ignore()` are hidden friends of the risk-set types, so they need no
    // using-declarations in `au.cppm`; ADL must find them through `au::OVERFLOW_RISK`, etc.
    constexpr auto risky = au::meters(std::numeric_limits<int>::max());
    EXPECT_FALSE(au::will_conversion_overflow(risky, au::kilo(au::meters)));
    (void)check_for(au::OVERFLOW_RISK);
    (void)ignore(au::ALL_RISKS);
}

TEST(AuModule, SupportsChronoInterop) {
    // Exercises the `CorrespondingQuantity` specializations declared in the global module
    // fragment (both the generic `std::chrono::duration` one, and an explicit one).
    EXPECT_EQ(au::as_quantity(std::chrono::seconds{3}), au::seconds(3));
    EXPECT_EQ(au::as_chrono_duration(au::milli(au::seconds)(250)), std::chrono::milliseconds{250});
}

TEST(AuModule, SupportsStdSpecializations) {
    // Exercises the `std::numeric_limits` and `std::common_type` specializations declared in
    // the global module fragment.
    static_assert(std::numeric_limits<au::QuantityI32<au::Meters>>::max() ==
                      au::meters(std::numeric_limits<int32_t>::max()),
                  "");
    static_assert(
        std::is_same<std::common_type_t<au::QuantityD<au::Meters>, au::QuantityD<au::Meters>>,
                     au::QuantityD<au::Meters>>::value,
        "");
}

TEST(AuModule, SupportsStreamingOutput) {
    std::ostringstream oss;
    oss << au::milli(au::amperes)(15) << " and " << au::ZERO << " and " << au::symbols::W;
    EXPECT_EQ(oss.str(), "15 mA and 0 and W");
}

TEST(AuModule, ProvidesRepSpecificAliases) {
    static_assert(std::is_same<au::QuantityF<au::Hertz>, au::Quantity<au::Hertz, float>>::value,
                  "");
}

}  // namespace
