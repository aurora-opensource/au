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

#include "au/quantity_point.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::Not;
using ::testing::StaticAssertTypeEq;

namespace au {

struct Meters : UnitImpl<Length> {};
constexpr QuantityMaker<Meters> meters{};
constexpr QuantityPointMaker<Meters> meters_pt{};

struct Kelvins : UnitImpl<Temperature> {};
constexpr QuantityMaker<Kelvins> kelvins{};
constexpr QuantityPointMaker<Kelvins> kelvins_pt{};

struct Celsius : Kelvins {
    static constexpr auto origin() { return (kelvins / mag<20>())(273'15 / 5); }
};
constexpr QuantityMaker<Celsius> celsius_qty{};
constexpr QuantityPointMaker<Celsius> celsius_pt{};

TEST(QuantityPoint, HasExpectedDiffType) {
    StaticAssertTypeEq<QuantityPointI32<Kelvins>::Diff, QuantityI32<Kelvins>>();
    StaticAssertTypeEq<QuantityPointF<Celsius>::Diff, QuantityF<Celsius>>();
}

TEST(QuantityPoint, CanImplicitlyConstructDoubleFromIntButNotViceVersa) {
    EXPECT_TRUE((std::is_convertible<QuantityPointI32<Celsius>, QuantityPointD<Celsius>>::value));
    EXPECT_FALSE((std::is_convertible<QuantityPointD<Celsius>, QuantityPointI32<Celsius>>::value));
}

TEST(QuantityPoint, CanImplicitlyConstructWithNonintegerOffsetIffDestinationIsFloatingPoint) {
    EXPECT_FALSE(
        (std::is_convertible<QuantityPointI32<Celsius>, QuantityPointI32<Kelvins>>::value));

    EXPECT_TRUE((std::is_convertible<QuantityPointI32<Celsius>, QuantityPointD<Kelvins>>::value));

    // Also works for ints if the destination unit evenly divides both offset and initial diff.
    EXPECT_TRUE(
        (std::is_convertible<QuantityPointI32<Celsius>, QuantityPointI32<Milli<Kelvins>>>::value));
}

TEST(QuantityPoint, ImplicitConstructionsAreCorrect) {
    constexpr QuantityPointD<Celsius> temp = celsius_pt(20);
    EXPECT_THAT(temp, SameTypeAndValue(celsius_pt(20.)));

    constexpr QuantityPointI32<Milli<Kelvins>> zero_celsius_as_mK = celsius_pt(0);
    EXPECT_THAT(zero_celsius_as_mK, SameTypeAndValue(milli(kelvins_pt)(273'150)));
}

TEST(QuantityPoint, CanCreateAndRetrieveValue) {
    constexpr auto p = celsius_pt(3);
    EXPECT_THAT(p.in(Celsius{}), SameTypeAndValue(3));
}

TEST(QuantityPoint, CanGetValueInDifferentUnits) {
    constexpr auto p = meters_pt(3);
    EXPECT_THAT(p.in(centi(meters_pt)), SameTypeAndValue(300));
}

TEST(QuantityPoint, SupportsDirectAccessWithSameUnit) {
    auto p = celsius_pt(3);
    ++(p.data_in(Celsius{}));
    EXPECT_EQ(p, celsius_pt(4));
}

TEST(QuantityPoint, SupportsDirectConstAccessWithSameUnit) {
    const auto p = meters_pt(3.5);
    EXPECT_EQ(static_cast<const void *>(&p.data_in(Meters{})), static_cast<const void *>(&p));
}

TEST(QuantityPoint, SupportsDirectAccessWithEquivalentUnit) {
    auto p = kelvins_pt(3);
    ++(p.data_in(Micro<Mega<Kelvins>>{}));
    EXPECT_EQ(p, kelvins_pt(4));

    // Uncomment to test compile time failure:
    // ++(p.data_in(Celsius{}));
}

TEST(QuantityPoint, SupportsDirectConstAccessWithEquivalentUnit) {
    const auto p = milli(meters_pt)(3.5);
    EXPECT_EQ(static_cast<const void *>(&p.data_in(Micro<Kilo<Meters>>{})),
              static_cast<const void *>(&p));

    // Uncomment to test compile time failure:
    // EXPECT_EQ(static_cast<const void *>(&p.data_in(Micro<Meters>{})),
    //           static_cast<const void *>(&p));
}

TEST(QuantityPoint, SupportsDirectAccessWithQuantityMakerOfSameUnit) {
    auto p = meters_pt(3);
    ++(p.data_in(meters_pt));
    EXPECT_EQ(p, meters_pt(4));
}

TEST(QuantityPoint, SupportsDirectConstAccessWithQuantityMakerOfSameUnit) {
    const auto p = celsius_pt(3.5);
    EXPECT_EQ(static_cast<const void *>(&p.data_in(celsius_pt)), static_cast<const void *>(&p));
}

TEST(QuantityPoint, SupportsDirectAccessWithQuantityMakerOfEquivalentUnit) {
    auto p = kelvins_pt(3);
    ++(p.data_in(micro(mega(kelvins_pt))));
    EXPECT_EQ(p, kelvins_pt(4));

    // Uncomment to test compile time failure:
    // ++(p.data_in(micro(kelvins_pt)));
}

TEST(QuantityPoint, SupportsDirectConstAccessWithQuantityMakerOfEquivalentUnit) {
    const auto p = milli(meters_pt)(3.5);
    EXPECT_EQ(static_cast<const void *>(&p.data_in(micro(kilo(meters_pt)))),
              static_cast<const void *>(&p));

    // Uncomment to test compile time failure:
    // EXPECT_EQ(static_cast<const void*>(&p.data_in(meters_pt)), static_cast<const void*>(&p));
}

TEST(QuantityPoint, HasDefaultConstructor) {
    // All we are permitted to do with a default-constructed value is to assign to it.  However, the
    // default constructor must _exist_, so we can use it with, e.g., `std::atomic`.
    QuantityPointF<Celsius> qp;
    qp = celsius_pt(4.5f);
    EXPECT_EQ(qp.in(celsius_pt), 4.5f);
}

TEST(QuantityPoint, InHandlesUnitsWithNonzeroOffset) {
    constexpr auto room_temperature = kelvins_pt(293.15);
    EXPECT_NEAR(room_temperature.in(Celsius{}), 20, 1e-12);
}

TEST(QuantityPoint, InHandlesIntegerRepInUnitsWithNonzeroOffset) {
    constexpr auto room_temperature = celsius_pt(20);
    EXPECT_EQ(room_temperature.in(celsius_pt), 20);
}

TEST(QuantityPoint, CanRequestOutputRepWhenCallingIn) {
    EXPECT_EQ(celsius_pt(5.2).in<int>(Celsius{}), 5);
}

TEST(QuantityPoint, CanCastToUnitWithDifferentMagnitude) {
    EXPECT_THAT(centi(meters_pt)(75).as<int>(meters_pt), SameTypeAndValue(meters_pt(0)));

    EXPECT_THAT(centi(meters_pt)(75.0).as(meters_pt), SameTypeAndValue(meters_pt(0.75)));
}

TEST(QuantityPoint, CanCastToUnitWithDifferentOrigin) {
    EXPECT_THAT(celsius_pt(10.).as(kelvins_pt), IsNear(kelvins_pt(283.15), nano(kelvins)(1)));
    EXPECT_THAT(celsius_pt(10).as<int>(Kelvins{}), SameTypeAndValue(kelvins_pt(283)));
}

TEST(QuantityPoint, ComparisonsWorkAsExpected) {
    constexpr auto x = meters_pt(3);

    EXPECT_THAT(x, ConsistentlyGreaterThan(meters_pt(2)));
    EXPECT_THAT(x, ConsistentlyEqualTo(meters_pt(3)));
    EXPECT_THAT(x, ConsistentlyLessThan(meters_pt(4)));
}

TEST(QuantityPoint, SubtractionYieldsDiffT) {
    constexpr auto diff = kelvins_pt(5) - kelvins_pt(3);
    EXPECT_THAT(diff, SameTypeAndValue(kelvins(2)));
}

TEST(QuantityPoint, CanAddDiffTFromLeft) {
    constexpr auto two_kelvins_hotter = kelvins(2) + kelvins_pt(5);
    EXPECT_THAT(two_kelvins_hotter, SameTypeAndValue(kelvins_pt(7)));
}

TEST(QuantityPoint, CanAddDiffTFromRight) {
    constexpr auto two_kelvins_hotter = kelvins_pt(5) + kelvins(2);
    EXPECT_THAT(two_kelvins_hotter, SameTypeAndValue(kelvins_pt(7)));
}

TEST(QuantityPoint, CanSubtractDiffTFromRight) {
    constexpr auto two_kelvins_cooler = kelvins_pt(5) - kelvins(2);
    EXPECT_THAT(two_kelvins_cooler, SameTypeAndValue(kelvins_pt(3)));
}

TEST(QuantityPoint, ShortHandAdditionAssignmentWorks) {
    auto d = kelvins_pt(1.25);
    d += kelvins(2.75);
    EXPECT_EQ(d, kelvins_pt(4.));
}

TEST(QuantityPoint, ShortHandAdditionHasReferenceCharacter) {
    auto d = kelvins_pt(1);
    (d += kelvins(1234)) = kelvins_pt(3);
    EXPECT_EQ(d, (kelvins_pt(3)));
}

TEST(QuantityPoint, ShortHandSubtractionAssignmentWorks) {
    auto d = kelvins_pt(4.75);
    d -= kelvins(2.75);
    EXPECT_EQ(d, (kelvins_pt(2.)));
}

TEST(QuantityPoint, ShortHandSubtractionHasReferenceCharacter) {
    auto d = kelvins_pt(4);
    (d -= kelvins(1234)) = kelvins_pt(3);
    EXPECT_EQ(d, (kelvins_pt(3)));
}

TEST(QuantityPoint, MixedUnitAdditionUsesCommonDenominator) {
    EXPECT_THAT(meters_pt(2) + centi(meters)(3), PointEquivalent(centi(meters_pt)(203)));
    EXPECT_THAT(centi(meters)(2) + meters_pt(3), PointEquivalent(centi(meters_pt)(302)));
}

TEST(QuantityPoint, MixedUnitAdditionWithQuantityDoesNotConsiderOrigin) {
    EXPECT_THAT(celsius_pt(20) + kelvins(5), PointEquivalent(celsius_pt(25)));
    EXPECT_THAT(celsius_qty(20) + kelvins_pt(5), PointEquivalent(kelvins_pt(25)));
}

TEST(QuantityPoint, MixedUnitSubtractionUsesCommonDenominator) {
    EXPECT_THAT(meters_pt(2) - centi(meters)(3), PointEquivalent(centi(meters_pt)(197)));
    EXPECT_THAT(meters_pt(2) - centi(meters_pt)(3), QuantityEquivalent(centi(meters)(197)));
}

TEST(QuantityPoint, MixedUnitSubtractionWithQuantityDoesNotConsiderOrigin) {
    EXPECT_THAT(celsius_pt(20) - kelvins(5), PointEquivalent(celsius_pt(15)));
}

TEST(QuantityPoint, MixedUnitsWithIdenticalNonzeroOriginDontGetSubdivided) {
    constexpr auto diff = kilo(celsius_pt)(1) - celsius_qty(900);
    EXPECT_THAT(diff, PointEquivalent(celsius_pt(100)));

    // Just to leave no doubt: the centi-celsius units of the origin should _not_ influence the
    // units in which the result is expressed (although we _should_ compare _equal_ to that result).
    constexpr auto right_answer_wrong_units = centi(celsius_pt)(10000);
    ASSERT_EQ(diff, right_answer_wrong_units);
    EXPECT_THAT(diff, Not(PointEquivalent(right_answer_wrong_units)));
}

TEST(QuantityPoint, MixedUnitAndRepDifferenceUsesCommonPointType) {
    // The point of this test case is to construct two `QuantityPoint` types whose common type is
    // neither of them.
    //
    // It forces us to write a disambiguating overload for subtraction between two different
    // `QuantityPoint` types.
    constexpr auto rep_wins = kilo(meters_pt)(0.5);
    constexpr auto unit_wins = meters_pt(400);

    EXPECT_THAT(rep_wins - unit_wins, QuantityEquivalent(meters(100.0)));
}

TEST(QuantityPoint, CanCompareUnitsWithDifferentOrigins) {
    EXPECT_THAT(celsius_pt(0), ConsistentlyGreaterThan(kelvins_pt(273)));
    EXPECT_THAT(celsius_pt(0), ConsistentlyEqualTo(milli(kelvins_pt)(273'150)));
    EXPECT_THAT(celsius_pt(0), ConsistentlyLessThan(kelvins_pt(274)));
}

TEST(QuantityPoint, CanSubtractIntegralInputsWithNonintegralOriginDifference) {
    EXPECT_EQ(celsius_pt(0) - kelvins_pt(273), centi(kelvins)(15));
}

TEST(QuantityPoint, InheritsOverflowSafetySurfaceFromUnderlyingQuantityTypes) {
    // This should fail to compile with a warning about a "dangerous conversion".  Here is why.
    //
    // - The maximum `uint16_t` value is 65,535.
    // - The common point-unit of `Celsius` and `Kelvins` is _at most_ (1/20)K; so, we are
    //   guaranteed to be multiplying by at least 20.  (In fact, it's currently implemented as
    //   `Centi<Kelvins>`, so we're multiplying by 100.)
    // - The safety surface kicks in if we would overflow a value of 4,294.
    // - To be able to multiply any value of up to 4,294 by a number at least 20 without
    //   overflowing, we need to be able to store 85,580.  This can't fit inside of a `uint16_t`;
    //   hence, dangerous conversion.

    // UNCOMMENT THE FOLLOWING LINE TO TEST:
    // celsius_pt(static_cast<uint16_t>(20)) < kelvins_pt(static_cast<uint16_t>(293));

    // Note that this is explicitly due to the influence of the origin _difference_.  For
    // _quantities_, rather than quantity _points_, this would work just fine, as the following test
    // shows.
    ASSERT_TRUE(celsius_qty(static_cast<uint16_t>(20)) < kelvins(static_cast<uint16_t>(293)));
}

TEST(QuantityPointMaker, CanApplyPrefix) {
    EXPECT_THAT(centi(kelvins_pt)(12), SameTypeAndValue(make_quantity_point<Centi<Kelvins>>(12)));
}

TEST(QuantityPointMaker, CanScaleByMagnitude) {
    StaticAssertTypeEq<decltype(kelvins_pt * mag<5>()),
                       QuantityPointMaker<decltype(Kelvins{} * mag<5>())>>();
    StaticAssertTypeEq<decltype(kelvins_pt / mag<5>()),
                       QuantityPointMaker<decltype(Kelvins{} / mag<5>())>>();
}

namespace detail {
TEST(OriginDisplacementFitsIn, CanRetrieveValueInGivenRep) {
    EXPECT_TRUE((OriginDisplacementFitsIn<uint64_t, Kelvins, Celsius>::value));
    EXPECT_TRUE((OriginDisplacementFitsIn<int64_t, Kelvins, Celsius>::value));

    EXPECT_TRUE((OriginDisplacementFitsIn<uint32_t, Kelvins, Celsius>::value));
    EXPECT_TRUE((OriginDisplacementFitsIn<int32_t, Kelvins, Celsius>::value));

    EXPECT_TRUE((OriginDisplacementFitsIn<uint16_t, Kelvins, Celsius>::value));
    EXPECT_TRUE((OriginDisplacementFitsIn<int16_t, Kelvins, Celsius>::value));

    EXPECT_FALSE((OriginDisplacementFitsIn<uint8_t, Kelvins, Celsius>::value));
    EXPECT_FALSE((OriginDisplacementFitsIn<int8_t, Kelvins, Celsius>::value));
}

TEST(OriginDisplacementFitsIn, AlwaysTrueForZero) {
    EXPECT_TRUE((OriginDisplacementFitsIn<uint64_t, Celsius, Celsius>::value));
    EXPECT_TRUE((OriginDisplacementFitsIn<int64_t, Celsius, Celsius>::value));

    EXPECT_TRUE((OriginDisplacementFitsIn<uint32_t, Celsius, Celsius>::value));
    EXPECT_TRUE((OriginDisplacementFitsIn<int32_t, Celsius, Celsius>::value));

    EXPECT_TRUE((OriginDisplacementFitsIn<uint16_t, Celsius, Celsius>::value));
    EXPECT_TRUE((OriginDisplacementFitsIn<int16_t, Celsius, Celsius>::value));

    EXPECT_TRUE((OriginDisplacementFitsIn<uint8_t, Celsius, Celsius>::value));
    EXPECT_TRUE((OriginDisplacementFitsIn<int8_t, Celsius, Celsius>::value));
}
}  // namespace detail
}  // namespace au
