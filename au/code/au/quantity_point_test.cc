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

namespace au {

using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Not;
using ::testing::StaticAssertTypeEq;
using ::testing::StrEq;

namespace {

template <typename T>
std::string stream_to_string(const T &t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

}  // namespace

struct Meters : UnitImpl<Length> {};
constexpr QuantityMaker<Meters> meters{};
constexpr QuantityPointMaker<Meters> meters_pt{};

struct Inches : decltype(Centi<Meters>{} * mag<254>() / mag<100>()) {};
constexpr auto inches_pt = QuantityPointMaker<Inches>{};

struct Feet : decltype(Inches{} * mag<12>()) {};
constexpr auto feet_pt = QuantityPointMaker<Feet>{};

struct Kelvins : UnitImpl<Temperature> {
    static constexpr const char label[] = "K";
};
constexpr const char Kelvins::label[];
constexpr QuantityMaker<Kelvins> kelvins{};
constexpr QuantityPointMaker<Kelvins> kelvins_pt{};

struct Celsius : Kelvins {
    // We must divide by 100 to turn the integer value of `273'15` into the decimal `273.15`.  We
    // split that division between the unit and the value.  The goal is to divide the unit by as
    // little as possible (while still keeping the value an integer), because that will make the
    // conversion factor as small as possible in converting to the common-point-unit.
    static constexpr auto origin() { return (kelvins / mag<20>())(273'15 / 5); }

    static constexpr const char label[] = "degC";
};
constexpr const char Celsius::label[];
constexpr QuantityMaker<Celsius> celsius_qty{};
constexpr QuantityPointMaker<Celsius> celsius_pt{};

TEST(Quantity, HasCorrectRepNamedAliases) {
    StaticAssertTypeEq<QuantityPointD<Meters>, QuantityPoint<Meters, double>>();
    StaticAssertTypeEq<QuantityPointF<Meters>, QuantityPoint<Meters, float>>();
    StaticAssertTypeEq<QuantityPointI<Meters>, QuantityPoint<Meters, int>>();
    StaticAssertTypeEq<QuantityPointU<Meters>, QuantityPoint<Meters, unsigned int>>();
    StaticAssertTypeEq<QuantityPointI32<Meters>, QuantityPoint<Meters, int32_t>>();
    StaticAssertTypeEq<QuantityPointU32<Meters>, QuantityPoint<Meters, uint32_t>>();
    StaticAssertTypeEq<QuantityPointI64<Meters>, QuantityPoint<Meters, int64_t>>();
    StaticAssertTypeEq<QuantityPointU64<Meters>, QuantityPoint<Meters, uint64_t>>();
}

TEST(QuantityPoint, HasExpectedDiffType) {
    StaticAssertTypeEq<QuantityPointI32<Kelvins>::Diff, QuantityI32<Kelvins>>();
    StaticAssertTypeEq<QuantityPointF<Celsius>::Diff, QuantityF<Celsius>>();
}

TEST(QuantityPoint, CanImplicitlyConstructDoubleFromIntButNotViceVersa) {
    EXPECT_THAT((std::is_convertible<QuantityPointI32<Celsius>, QuantityPointD<Celsius>>::value),
                IsTrue());
    EXPECT_THAT((std::is_convertible<QuantityPointD<Celsius>, QuantityPointI32<Celsius>>::value),
                IsFalse());
}

TEST(QuantityPoint, CanImplicitlyConstructWithNonintegerOffsetIffDestinationIsFloatingPoint) {
    EXPECT_THAT((std::is_convertible<QuantityPointI32<Celsius>, QuantityPointI32<Kelvins>>::value),
                IsFalse());

    EXPECT_THAT((std::is_convertible<QuantityPointI32<Celsius>, QuantityPointD<Kelvins>>::value),
                IsTrue());

    // Also works for ints if the destination unit evenly divides both offset and initial diff.
    EXPECT_THAT(
        (std::is_convertible<QuantityPointI32<Celsius>, QuantityPointI32<Milli<Kelvins>>>::value),
        IsTrue());
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

TEST(QuantityPoint, IntermediateTypeIsSignedIfExplicitRepIsSigned) {
    EXPECT_THAT(milli(kelvins_pt)(0u).coerce_as<int>(celsius_pt),
                SameTypeAndValue(celsius_pt(-273)));
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
    EXPECT_THAT(centi(meters_pt)(75).coerce_as(meters_pt), SameTypeAndValue(meters_pt(0)));

    EXPECT_THAT(centi(meters_pt)(75.0).as(meters_pt), SameTypeAndValue(meters_pt(0.75)));
}

TEST(QuantityPoint, CanCastToUnitWithDifferentOrigin) {
    EXPECT_THAT(celsius_pt(10.).as(kelvins_pt), IsNear(kelvins_pt(283.15), nano(kelvins)(1)));
    EXPECT_THAT(celsius_pt(10).coerce_as(Kelvins{}), SameTypeAndValue(kelvins_pt(283)));
}

TEST(QuantityPoint, HandlesConversionWithSignedSourceAndUnsignedDestination) {
    EXPECT_THAT(celsius_pt(int16_t{-5}).coerce_as<uint16_t>(kelvins_pt),
                SameTypeAndValue(kelvins_pt(uint16_t{268})));
}

TEST(QuantityPoint, CoerceAsWillForceLossyConversion) {
    // Truncation.
    EXPECT_THAT(inches_pt(30).coerce_as(feet_pt), SameTypeAndValue(feet_pt(2)));

    // Unsigned overflow.
    ASSERT_EQ(static_cast<uint8_t>(30 * 12), 104);
    EXPECT_THAT(feet_pt(uint8_t{30}).coerce_as(inches_pt),
                SameTypeAndValue(inches_pt(uint8_t{104})));
}

TEST(QuantityPoint, CoerceAsExplicitRepSetsOutputType) {
    // Coerced truncation.
    EXPECT_THAT(inches_pt(30).coerce_as<std::size_t>(feet_pt),
                SameTypeAndValue(feet_pt(std::size_t{2})));

    // Exact answer for floating point destination type.
    EXPECT_THAT(inches_pt(30).coerce_as<float>(feet_pt), SameTypeAndValue(feet_pt(2.5f)));

    // Coerced unsigned overflow.
    ASSERT_EQ(static_cast<uint8_t>(30 * 12), 104);
    EXPECT_THAT(feet_pt(30).coerce_as<uint8_t>(inches_pt),
                SameTypeAndValue(inches_pt(uint8_t{104})));
}

TEST(QuantityPoint, CoerceInWillForceLossyConversion) {
    // Truncation.
    EXPECT_THAT(inches_pt(30).coerce_in(feet_pt), SameTypeAndValue(2));

    // Unsigned overflow.
    ASSERT_EQ(static_cast<uint8_t>(30 * 12), 104);
    EXPECT_THAT(feet_pt(uint8_t{30}).coerce_in(inches_pt), SameTypeAndValue(uint8_t{104}));
}

TEST(QuantityPoint, CoerceInExplicitRepSetsOutputType) {
    // Coerced truncation.
    EXPECT_THAT(inches_pt(30).coerce_in<std::size_t>(feet_pt), SameTypeAndValue(std::size_t{2}));

    // Exact answer for floating point destination type.
    EXPECT_THAT(inches_pt(30).coerce_in<float>(feet_pt), SameTypeAndValue(2.5f));

    // Coerced unsigned overflow.
    ASSERT_EQ(static_cast<uint8_t>(30 * 12), 104);
    EXPECT_THAT(feet_pt(30).coerce_in<uint8_t>(inches_pt), SameTypeAndValue(uint8_t{104}));
}

TEST(QuantityPoint, CoerceAsPerformsConversionInWidestType) {
    constexpr QuantityPointU32<Milli<Kelvins>> temp = milli(kelvins_pt)(313'150u);
    EXPECT_THAT(temp.coerce_as<uint16_t>(deci(kelvins_pt)),
                SameTypeAndValue(deci(kelvins_pt)(uint16_t{3131})));
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

TEST(QuantityPoint, CommonPointUnitLabel) {
    EXPECT_THAT(stream_to_string(celsius_pt(0) - kelvins_pt(0)),
                AnyOf(StrEq("5463 EQUIV{[(1 / 20) K], [(1 / 20) degC]}"),
                      StrEq("5463 EQUIV{[(1 / 20) degC], [(1 / 20) K]}")));
}

TEST(QuantityPoint, CanCompareUnitsWithDifferentOrigins) {
    EXPECT_THAT(celsius_pt(0), ConsistentlyGreaterThan(kelvins_pt(273)));
    EXPECT_THAT(celsius_pt(0), ConsistentlyEqualTo(milli(kelvins_pt)(273'150)));
    EXPECT_THAT(celsius_pt(0), ConsistentlyLessThan(kelvins_pt(274)));
}

TEST(QuantityPoint, ComparisonsWithNegativeUnitHaveAppropriatelyReversedResults) {
    constexpr auto neg_celsius_pt = celsius_pt * (-mag<1>());
    constexpr auto neg_kelvins_pt = kelvins_pt * (-mag<1>());

    EXPECT_THAT(neg_celsius_pt(1), ConsistentlyLessThan(neg_celsius_pt(0)));

    EXPECT_THAT(celsius_pt(0), ConsistentlyGreaterThan(neg_kelvins_pt(-273)));
    EXPECT_THAT(celsius_pt(0), ConsistentlyLessThan(neg_kelvins_pt(-274)));

    EXPECT_THAT(neg_celsius_pt(1), ConsistentlyEqualTo(milli(neg_kelvins_pt)(-272'150)));
}

TEST(QuantityPoint, AddingPosUnitQuantityToNegUnitPointGivesPosUnitPoint) {
    constexpr auto neg_celsius_pt = celsius_pt * (-mag<1>());
    EXPECT_THAT(neg_celsius_pt(40) + celsius_qty(15), PointEquivalent(celsius_pt(-25)));
}

TEST(QuantityPoint, CanSubtractIntegralInputsWithNonintegralOriginDifference) {
    EXPECT_EQ(celsius_pt(0) - kelvins_pt(273), centi(kelvins)(15));
}

TEST(QuantityPoint, InheritsOverflowSafetySurfaceFromUnderlyingQuantityTypes) {
    // This should fail to compile with a warning about a "dangerous conversion".  Here is why.
    //
    // - The maximum `int16_t` value is 32,767.
    // - The common point-unit of `Celsius` and `Kelvins` is _at most_ (1/20)K; so, we are
    //   guaranteed to be multiplying by at least 20.
    // - The safety surface kicks in if we would overflow a value of 2,147.
    // - To be able to multiply any value of up to 2,147 by a number at least 20 without
    //   overflowing, we need to be able to store 42,940.  This can't fit inside of a `int16_t`;
    //   hence, dangerous conversion.

    // UNCOMMENT THE FOLLOWING LINE TO TEST:
    // ASSERT_THAT(celsius_pt(static_cast<int16_t>(20)) < kelvins_pt(static_cast<int16_t>(293)),
    // IsFalse());

    // It so happens that moving from `int16_t` to `uint16_t` would give us enough room to make the
    // test compile (and pass).
    ASSERT_THAT(celsius_pt(static_cast<uint16_t>(20)) < kelvins_pt(static_cast<uint16_t>(293)),
                IsFalse());

    // Note also that the failure is explicitly due to the influence of the origin _difference_.
    // For _quantities_, rather than quantity _points_, this would work just fine, as the following
    // test shows.
    ASSERT_THAT(celsius_qty(static_cast<int16_t>(20)) < kelvins(static_cast<int16_t>(293)),
                IsTrue());
}

TEST(QuantityPoint, PreservesRep) {
    EXPECT_THAT(celsius_pt(static_cast<uint16_t>(0)).in(kelvins_pt / mag<20>()),
                SameTypeAndValue(static_cast<uint16_t>(27'315 / 5)));
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
    EXPECT_THAT((OriginDisplacementFitsIn<uint64_t, Kelvins, Celsius>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int64_t, Kelvins, Celsius>::value), IsTrue());

    EXPECT_THAT((OriginDisplacementFitsIn<uint32_t, Kelvins, Celsius>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int32_t, Kelvins, Celsius>::value), IsTrue());

    EXPECT_THAT((OriginDisplacementFitsIn<uint16_t, Kelvins, Celsius>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int16_t, Kelvins, Celsius>::value), IsTrue());

    EXPECT_THAT((OriginDisplacementFitsIn<uint8_t, Kelvins, Celsius>::value), IsFalse());
    EXPECT_THAT((OriginDisplacementFitsIn<int8_t, Kelvins, Celsius>::value), IsFalse());
}

TEST(OriginDisplacementFitsIn, AlwaysTrueForZero) {
    EXPECT_THAT((OriginDisplacementFitsIn<uint64_t, Celsius, Celsius>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int64_t, Celsius, Celsius>::value), IsTrue());

    EXPECT_THAT((OriginDisplacementFitsIn<uint32_t, Celsius, Celsius>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int32_t, Celsius, Celsius>::value), IsTrue());

    EXPECT_THAT((OriginDisplacementFitsIn<uint16_t, Celsius, Celsius>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int16_t, Celsius, Celsius>::value), IsTrue());

    EXPECT_THAT((OriginDisplacementFitsIn<uint8_t, Celsius, Celsius>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int8_t, Celsius, Celsius>::value), IsTrue());
}

TEST(OriginDisplacementFitsIn, FailsNegativeDisplacementForUnsignedRep) {
    EXPECT_THAT((OriginDisplacementFitsIn<uint64_t, Celsius, Kelvins>::value), IsFalse());
    EXPECT_THAT((OriginDisplacementFitsIn<uint32_t, Celsius, Kelvins>::value), IsFalse());
    EXPECT_THAT((OriginDisplacementFitsIn<uint16_t, Celsius, Kelvins>::value), IsFalse());

    EXPECT_THAT((OriginDisplacementFitsIn<int64_t, Celsius, Kelvins>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int32_t, Celsius, Kelvins>::value), IsTrue());
    EXPECT_THAT((OriginDisplacementFitsIn<int16_t, Celsius, Kelvins>::value), IsTrue());
}

}  // namespace detail
}  // namespace au
