// Copyright 2024 Aurora Operations, Inc.
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

#include <compare>

#include "au/constants/speed_of_light.hh"
#include "au/math.hh"
#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/testing.hh"
#include "au/units/celsius.hh"
#include "au/units/kelvins.hh"
#include "au/units/meters.hh"
#include "au/units/percent.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using symbols::m;
using symbols::s;

constexpr auto mm = milli(m);

using ::testing::Eq;
using ::testing::Lt;

struct Foo {
    auto operator<=>(const Foo &) const = default;

    QuantityD<Meters> thickness;
};

struct FooPt {
    auto operator<=>(const FooPt &) const = default;

    QuantityPointD<Meters> position;
};

TEST(Quantity, SupportsSpaceship) { EXPECT_THAT(Foo{5 * m}, Lt(Foo{6 * m})); }

TEST(Quantity, SpaceshipCorrectForMixedSignUnits) {
    constexpr auto negm = m * (-mag<1>());
    EXPECT_THAT((1 * m) <=> (0 * negm), Eq((1 * m) <=> (0 * m)));
    EXPECT_THAT((1 * m) <=> (-1 * negm), Eq((1 * m) <=> (1 * m)));
    EXPECT_THAT((1 * m) <=> (-2 * negm), Eq((1 * m) <=> (2 * m)));
}

TEST(QuantityPoint, SupportsSpaceship) {
    EXPECT_THAT(FooPt{meters_pt(5)}, Lt(FooPt{meters_pt(6)}));
}

TEST(Lerp, QuantityConsistentWithStdLerpWhenTypesAreIdentical) {
    auto expect_consistent_with_std_lerp = [](auto a, auto b, auto t) {
        const auto expected = meters(std::lerp(a, b, t));
        const auto actual = lerp(meters(a), meters(b), t);
        EXPECT_THAT(actual, SameTypeAndValue(expected));
    };

    // Rep: `int`; a < b.
    expect_consistent_with_std_lerp(0, 10, 0.0);
    expect_consistent_with_std_lerp(0, 10, 0.5);
    expect_consistent_with_std_lerp(0, 10, 1.0);
    expect_consistent_with_std_lerp(0, 10, 2.0);

    // Rep: `float`; a > b.
    expect_consistent_with_std_lerp(10.0f, 0.0f, 0.0f);
    expect_consistent_with_std_lerp(10.0f, 0.0f, 0.5f);
    expect_consistent_with_std_lerp(10.0f, 0.0f, 1.0f);
    expect_consistent_with_std_lerp(10.0f, 0.0f, 2.0f);
}

TEST(Lerp, QuantityProducesResultsInCommonUnitOfInputs) {
    EXPECT_THAT(lerp(meters(1.0), centi(meters)(200.0), 0.75),
                SameTypeAndValue(centi(meters)(175.0)));
}

TEST(Lerp, SupportsZero) {
    EXPECT_THAT(lerp(ZERO, 10.0 * m, 0.75), SameTypeAndValue(7.5 * m));
    EXPECT_THAT(lerp(10.0 * m, ZERO, 0.75), SameTypeAndValue(2.5 * m));
}

TEST(Lerp, SupportsConstant) {
    constexpr auto c = SPEED_OF_LIGHT;
    EXPECT_THAT(lerp(0 * m / s, c, 0.75), SameTypeAndValue(c.as<double>(m / s) * 0.75));
    EXPECT_THAT(lerp(c, 0 * m / s, 0.75), SameTypeAndValue(c.as<double>(m / s) * 0.25));
}

TEST(Lerp, SupportsPercentForThirdArgument) {
    // `Quantity`, same type.
    EXPECT_THAT(lerp(0 * m, 10 * m, percent(75.0)), SameTypeAndValue(7.5 * m));

    // Mixed `Quantity` types.
    EXPECT_THAT(lerp(0 * m, 10 * mm, percent(35.0)), SameTypeAndValue(3.5 * mm));

    // `Quantity` with a shapeshifter argument.
    EXPECT_THAT(lerp(ZERO, 10 * m, percent(37.5)), SameTypeAndValue(3.75 * m));

    // `QuantityPoint`, same type.
    EXPECT_THAT(lerp(meters_pt(0), meters_pt(10), percent(75.0)), SameTypeAndValue(meters_pt(7.5)));
}

TEST(Lerp, QuantityPointConsistentWithStdLerpWhenTypesAreIdentical) {
    auto expect_consistent_with_std_lerp = [](auto a, auto b, auto t) {
        const auto expected = meters_pt(std::lerp(a, b, t));
        const auto actual = lerp(meters_pt(a), meters_pt(b), t);
        EXPECT_THAT(actual, SameTypeAndValue(expected));
    };

    // Rep: `int`; a < b.
    expect_consistent_with_std_lerp(0, 10, 0.0);
    expect_consistent_with_std_lerp(0, 10, 0.5);
    expect_consistent_with_std_lerp(0, 10, 1.0);
    expect_consistent_with_std_lerp(0, 10, 2.0);

    // Rep: `float`; a > b.
    expect_consistent_with_std_lerp(10.0f, 0.0f, 0.0f);
    expect_consistent_with_std_lerp(10.0f, 0.0f, 0.5f);
    expect_consistent_with_std_lerp(10.0f, 0.0f, 1.0f);
    expect_consistent_with_std_lerp(10.0f, 0.0f, 2.0f);
}

TEST(Lerp, QuantityPointProducesResultsInCommonUnitOfInputs) {
    EXPECT_THAT(lerp(centi(kelvins_pt)(293'15.0), milli(celsius_pt)(0.0), 0.75),
                PointEquivalent(milli(kelvins_pt)(278'150.0)));
}

}  // namespace au
