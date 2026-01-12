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

#include "au/au.hh"

#include "au/constants/speed_of_light.hh"
#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/arcminutes.hh"
#include "au/units/celsius.hh"
#include "au/units/fahrenheit.hh"
#include "au/units/fathoms.hh"
#include "au/units/furlongs.hh"
#include "au/units/hertz.hh"
#include "au/units/hours.hh"
#include "au/units/kelvins.hh"
#include "au/units/knots.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/steradians.hh"
#include "au/units/yards.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;
using ::testing::Lt;
using ::testing::StaticAssertTypeEq;

namespace {

template <typename T, typename U>
auto IsBetween(T lower, U upper) {
    return ::testing::AllOf(::testing::Ge(lower), ::testing::Le(upper));
}

}  // namespace

TEST(Conversions, SupportIntMHzToU32Hz) {
    constexpr QuantityU32<Hertz> freq = mega(hertz)(40);
    EXPECT_THAT(freq, SameTypeAndValue(hertz(40'000'000u)));
}

TEST(CommonUnit, HandlesPrefixesReasonably) {
    StaticAssertTypeEq<CommonUnit<Kilo<Meters>, Meters>, Meters>();
}

template <typename U, typename R>
constexpr auto round_sequentially(Quantity<U, R> q) {
    std::cout << q << std::endl;
    return q;
}

template <typename U, typename R, typename FirstUnit, typename... NextUnits>
constexpr auto round_sequentially(Quantity<U, R> q, FirstUnit first_unit, NextUnits... next_units) {
    std::cout << q << "\n `-> ";
    return round_sequentially(round_as(first_unit, q), next_units...);
}

TEST(Xkcd, RoundAsReproducesXkcd2585) {
    constexpr auto true_speed = (miles / hour)(17);

    const auto rounded_speed = round_sequentially(true_speed,
                                                  meters / second,
                                                  knots,
                                                  fathoms / second,
                                                  furlongs / minute,
                                                  fathoms / second,
                                                  kilo(meters) / hour,
                                                  knots,
                                                  kilo(meters) / hour,
                                                  furlongs / hour,
                                                  miles / hour,
                                                  meters / second,
                                                  furlongs / minute,
                                                  yards / second,
                                                  fathoms / second,
                                                  meters / second,
                                                  miles / hour,
                                                  furlongs / minute,
                                                  knots,
                                                  yards / second,
                                                  fathoms / second,
                                                  knots,
                                                  furlongs / minute,
                                                  miles / hour);

    // Authoritative reference: https://xkcd.com/2585/
    EXPECT_THAT((miles / hour)(45), Eq(rounded_speed));
}

TEST(Xkcd, Xkcd3038GivesReasonableSpeedLimit) {
    using symbols::h;
    using symbols::mi;

    constexpr auto c = SPEED_OF_LIGHT;
    constexpr auto SPEED_LIMIT = make_constant(c * squared(arcminutes) / steradian);

    EXPECT_THAT(SPEED_LIMIT, IsBetween(25.0 * mi / h, 75.0 * mi / h))
        << SPEED_LIMIT.as<double>(mi / h);
}

TEST(QuantityPoint, DocumentationExampleIsCorrect) {
    EXPECT_THAT(fahrenheit_pt(-40) + celsius_qty(60), Lt(kelvins_pt(300)));
}

}  // namespace au
