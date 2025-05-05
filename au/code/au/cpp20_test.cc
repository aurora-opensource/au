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

#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/testing.hh"
#include "au/units/meters.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using symbols::m;
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

}  // namespace au
