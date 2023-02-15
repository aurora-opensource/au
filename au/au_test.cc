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

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/fathoms.hh"
#include "au/units/furlongs.hh"
#include "au/units/hertz.hh"
#include "au/units/hours.hh"
#include "au/units/knots.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/yards.hh"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

using namespace std::chrono_literals;

namespace au {

TEST(DurationQuantity, InterconvertsWithExactlyEquivalentChronoDuration) {
    constexpr QuantityD<Seconds> from_chrono = std::chrono::duration<double>{1.23};
    EXPECT_THAT(from_chrono, SameTypeAndValue(seconds(1.23)));

    constexpr auto val = std::chrono::nanoseconds::rep{456};
    constexpr std::chrono::nanoseconds from_au = nano(seconds)(val);
    EXPECT_THAT(from_au.count(), SameTypeAndValue(val));
}

TEST(DurationQuantity, InterconvertsWithIndirectlyEquivalentChronoDuration) {
    constexpr QuantityD<Seconds> from_chrono = as_quantity(1234ms);
    EXPECT_THAT(from_chrono, SameTypeAndValue(seconds(1.234)));
}

TEST(Conversions, SupportIntMHzToU32Hz) {
    constexpr QuantityU32<Hertz> freq = mega(hertz)(40);
    EXPECT_THAT(freq, SameTypeAndValue(hertz(40'000'000u)));
}

TEST(CommonUnit, HandlesPrefixesReasonably) {
    StaticAssertTypeEq<CommonUnitT<Kilo<Meters>, Meters>, Meters>();
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

TEST(RoundAs, ReproducesXkcd2585) {
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
    EXPECT_EQ((miles / hour)(45), rounded_speed);
}

}  // namespace au
