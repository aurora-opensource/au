// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#include "au/unit_of_measure.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/utility/type_traits.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::au::detail::DimT;
using ::au::detail::MagT;
using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::Not;
using ::testing::StaticAssertTypeEq;
using ::testing::StrEq;

namespace au {

struct Feet : UnitImpl<Length> {
    static constexpr const char label[] = "ft";
};
constexpr const char Feet::label[];

struct Yards : decltype(Feet{} * mag<3>()) {
    static constexpr const char label[] = "yd";
};
constexpr const char Yards::label[];

struct Inches : decltype(Feet{} / mag<12>()) {
    static constexpr const char label[] = "in";
};
constexpr const char Inches::label[];

struct Meters : decltype(Inches{} * (mag<100>() / mag<254>() * mag<100>())) {
    static constexpr const char label[] = "m";
};
constexpr const char Meters::label[];

struct Minutes : UnitImpl<Time> {
    static constexpr const char label[] = "min";
};
constexpr const char Minutes::label[];

struct Kelvins : UnitImpl<Temperature> {};
constexpr auto kelvins = QuantityMaker<Kelvins>{};

struct Celsius : Kelvins {
    static constexpr auto origin() { return milli(kelvins)(273'150); }
};

struct AlternateCelsius : Kelvins {
    static constexpr auto origin() { return micro(kelvins)(273'150'000); }
};

constexpr auto F_PER_C = mag<5>() / mag<9>();
struct Fahrenheit : decltype(Kelvins{} * F_PER_C) {
    static constexpr auto origin() { return milli(kelvins * F_PER_C)(459'670); }
};

struct One : UnitImpl<Dimension<>> {};

// Test the ability to create labels non-intrusively, via a trait.
//
// This would let us label aliases.
struct TraitLabeledUnit : One {};
template <>
struct UnitLabel<TraitLabeledUnit> {
    static constexpr const char value[] = "TLU";
};
constexpr const char UnitLabel<TraitLabeledUnit>::value[];

struct UnitWithOrigin : decltype(One{} * mag<123>()) {
    static constexpr double origin() { return 3.14; }
};

struct AdHocSpeedUnit {
    using Dim = DimQuotientT<Length, Time>;
    using Mag = decltype(mag<1234>());
};

struct InvalidWrongDimType {
    using Dim = int;
    using Mag = Magnitude<>;
};

struct InvalidWrongMagType {
    using Dim = Length;
    using Mag = char;
};

struct UnlabeledUnit : decltype(Feet{} * mag<9>()) {};

MATCHER_P(QuantityEquivalentToUnit, target, "") {
    return are_units_quantity_equivalent(arg, target);
}

MATCHER_P(PointEquivalentToUnit, target, "") { return are_units_point_equivalent(arg, target); }

TEST(Unit, ProductWithMagnitudeGivesSameDimensionAndMultipliesMagnitude) {
    StaticAssertTypeEq<UnitRatioT<Yards, Feet>, decltype(mag<3>())>();
    StaticAssertTypeEq<UnitRatioT<Yards, Inches>, decltype(mag<36>())>();
}

TEST(Unit, OriginRetainedForProductWithMagnitudeButNotWithUnit) {
    constexpr auto scaled_by_mag = UnitWithOrigin{} * mag<8>();
    EXPECT_THAT(decltype(scaled_by_mag)::origin(), SameTypeAndValue(UnitWithOrigin::origin()));

    constexpr auto scaled_by_unit = UnitWithOrigin{} * One{};
    EXPECT_FALSE(
        (stdx::experimental::is_detected<detail::OriginMemberType, decltype(scaled_by_unit)>{}));
}

TEST(IsUnit, TrueForUnitImpl) { EXPECT_TRUE(IsUnit<UnitImpl<Length>>::value); }

TEST(IsUnit, TrueForOpaqueTypedef) { EXPECT_TRUE(IsUnit<Feet>::value); }

TEST(IsUnit, TrueForPowerOfUnit) {
    EXPECT_TRUE((IsUnit<Pow<Feet, 3>>::value));
    EXPECT_TRUE((IsUnit<RatioPow<Feet, 1, 2>>::value));
}

TEST(IsUnit, SupportsDeductionFromInstance) {
    EXPECT_TRUE(is_unit(Feet{}));
    EXPECT_FALSE(is_unit(3));
}

TEST(IsUnit, TrueForUnrelatedTypeWithDimAndMag) { EXPECT_TRUE(IsUnit<AdHocSpeedUnit>::value); }

TEST(IsUnit, FalseIfDimOrMagHasWrongType) {
    EXPECT_FALSE(is_unit(InvalidWrongDimType{}));
    EXPECT_FALSE(is_unit(InvalidWrongMagType{}));
}

TEST(Product, IsUnitWithProductOfMagnitudesAndDimensions) {
    constexpr auto foot_yards = Feet{} * Yards{};
    EXPECT_TRUE(is_unit(foot_yards));

    StaticAssertTypeEq<MagT<decltype(foot_yards)>, MagProductT<MagT<Feet>, MagT<Yards>>>();
    StaticAssertTypeEq<DimT<decltype(foot_yards)>, DimProductT<DimT<Feet>, DimT<Yards>>>();
}

TEST(Quotient, IsUnitWithQuotientOfMagnitudesAndDimensions) {
    constexpr auto in_per_min = Inches{} / Minutes{};
    EXPECT_TRUE(is_unit(in_per_min));
    StaticAssertTypeEq<MagT<decltype(in_per_min)>, MagQuotientT<MagT<Inches>, MagT<Minutes>>>();
    StaticAssertTypeEq<DimT<decltype(in_per_min)>, DimQuotientT<DimT<Inches>, DimT<Minutes>>>();
}

TEST(Pow, FunctionGivesUnitWhichIsEquivalentToManuallyComputedPower) {
    constexpr auto in = Inches{};
    constexpr auto cubic_inches = pow<3>(in);

    EXPECT_TRUE(is_unit(cubic_inches));
    EXPECT_TRUE(are_units_quantity_equivalent(cubic_inches, in * in * in));
}

TEST(UnitProduct, IsUnitlessUnitForNoInputs) {
    StaticAssertTypeEq<DimT<UnitProduct<>>, Dimension<>>();
    StaticAssertTypeEq<MagT<UnitProduct<>>, Magnitude<>>();
}

TEST(UnitProduct, ExactlyCancellingInstancesYieldsNullPack) {
    StaticAssertTypeEq<decltype(Feet{} * Inches{} / Minutes{} / Inches{} * Minutes{} / Feet{}),
                       UnitProduct<>>();
}

TEST(UnitProductT, IdentityForSingleUnit) {
    StaticAssertTypeEq<UnitProductT<Feet>, Feet>();
    StaticAssertTypeEq<UnitProductT<Minutes>, Minutes>();
}

TEST(UnitProductT, ProductOfTwoUnitsIsUnitWithProductsOfDimAndMag) {
    using FootInches = UnitProductT<Feet, Inches>;
    EXPECT_TRUE(IsUnit<FootInches>::value);
    StaticAssertTypeEq<DimT<FootInches>, DimProductT<DimT<Feet>, DimT<Inches>>>();
    StaticAssertTypeEq<MagT<FootInches>, MagProductT<MagT<Feet>, MagT<Inches>>>();
}

TEST(UnitProductT, AchievesExactCancellations) {
    using FootInchesPerFoot = UnitProductT<Feet, Inches, UnitInverseT<Feet>>;
    StaticAssertTypeEq<FootInchesPerFoot, Inches>();
}

TEST(UnitProductT, CreatesPowForIntegerPowers) {
    using FeetSquared = UnitProductT<Feet, Feet>;
    StaticAssertTypeEq<FeetSquared, Pow<Feet, 2>>();

    StaticAssertTypeEq<UnitProductT<FeetSquared, UnitInverseT<Feet>>, Feet>();
}

TEST(UnitPowerT, ProducesSimplifiedPowersOfAllExponents) {
    using Input = UnitProductT<Feet, Pow<Minutes, 3>, Pow<Inches, -6>, RatioPow<Yards, 3, 2>>;

    using ExpectedCbrtInput =
        UnitProductT<RatioPow<Feet, 1, 3>, Minutes, Pow<Inches, -2>, RatioPow<Yards, 1, 2>>;

    StaticAssertTypeEq<UnitPowerT<Input, 1, 3>, ExpectedCbrtInput>();
}

TEST(UnitQuotientT, InteractsAndCancelsAsExpected) {
    StaticAssertTypeEq<UnitProductT<UnitQuotientT<Feet, Minutes>, Minutes>, Feet>();
}

TEST(UnitInverseT, CreatesAppropriateUnitPower) {
    StaticAssertTypeEq<UnitInverseT<Feet>, Pow<Feet, -1>>();
    StaticAssertTypeEq<UnitInverseT<UnitInverseT<Minutes>>, Minutes>();
}

TEST(AssociatedUnitT, IsIdentityForUnits) { StaticAssertTypeEq<AssociatedUnitT<Feet>, Feet>(); }

TEST(AssociatedUnitT, FunctionalInterfaceHandlesInstancesCorrectly) {
    StaticAssertTypeEq<decltype(associated_unit(Feet{} / Minutes{})),
                       decltype(Feet{} / Minutes{})>();
}

TEST(UnitInverseT, CommutesWithProduct) {
    StaticAssertTypeEq<UnitInverseT<UnitProductT<Feet, Minutes>>,
                       UnitProductT<UnitInverseT<Feet>, UnitInverseT<Minutes>>>();
}

TEST(Root, FunctionalInterfaceHandlesInstancesCorrectly) {
    constexpr auto cubic_inches = pow<3>(Inches{});
    constexpr auto cbrt_cubic_inches = root<3>(cubic_inches);

    EXPECT_TRUE(is_unit(cbrt_cubic_inches));
    EXPECT_TRUE(are_units_quantity_equivalent(cbrt_cubic_inches, Inches{}));
}

TEST(IsDimensionless, PicksOutDimensionlessUnit) {
    EXPECT_FALSE((IsDimensionless<Feet>::value));

    EXPECT_TRUE((IsDimensionless<UnitQuotientT<Inches, Yards>>::value));

    EXPECT_TRUE(
        (IsDimensionless<UnitQuotientT<AdHocSpeedUnit, UnitQuotientT<Feet, Minutes>>>::value));
}

TEST(IsDimensionless, FunctionalInterfaceHandlesInstancesCorrectly) {
    EXPECT_FALSE(is_dimensionless(Feet{}));
    EXPECT_TRUE(is_dimensionless(Inches{} / Yards{}));
    EXPECT_TRUE(is_dimensionless(AdHocSpeedUnit{} / Feet{} * Minutes{}));
}

TEST(IsUnitlessUnit, PicksOutUnitlessUnit) {
    EXPECT_FALSE(is_unitless_unit(Inches{} / Yards{}));
    EXPECT_TRUE(is_unitless_unit(Inches{} / Inches{}));
}

TEST(HasSameDimension, TrueForAnySingleDimension) {
    EXPECT_TRUE(HasSameDimension<Feet>::value);
    EXPECT_TRUE(HasSameDimension<Minutes>::value);
    EXPECT_TRUE(HasSameDimension<AdHocSpeedUnit>::value);
}

TEST(HasSameDimension, CorrectlyIdentifiesEquivalenceClass) {
    EXPECT_TRUE((HasSameDimension<Feet, Feet>::value));
    EXPECT_TRUE((HasSameDimension<Feet, Yards>::value));
    EXPECT_TRUE((HasSameDimension<Feet, Yards, Inches>::value));

    EXPECT_FALSE((HasSameDimension<Minutes, Yards>::value));
}

TEST(HasSameDimension, FunctionalInterfaceHandlesInstancesCorrectly) {
    EXPECT_TRUE(has_same_dimension(Feet{}, Feet{}));
    EXPECT_TRUE(has_same_dimension(Feet{}, Yards{}));

    EXPECT_TRUE(has_same_dimension(Feet{}, Yards{}, Inches{}, Yards{}));
    EXPECT_FALSE(has_same_dimension(Feet{}, Yards{}, Inches{}, Minutes{}));
}

TEST(UnitRatio, ComputesRatioForSameDimensionedUnits) {
    StaticAssertTypeEq<UnitRatioT<Yards, Inches>, decltype(mag<36>())>();
    StaticAssertTypeEq<UnitRatioT<Inches, Inches>, decltype(mag<1>())>();
    StaticAssertTypeEq<UnitRatioT<Inches, Yards>, decltype(mag<1>() / mag<36>())>();
}

TEST(UnitRatio, FunctionalInterfaceHandlesInstancesCorrectly) {
    EXPECT_EQ(unit_ratio(Yards{}, Inches{}), mag<36>());
    EXPECT_EQ(unit_ratio(Inches{}, Inches{}), mag<1>());
    EXPECT_EQ(unit_ratio(Inches{}, Yards{}), mag<1>() / mag<36>());
}

TEST(AreUnitsQuantityEquivalent, UnitIsEquivalentToItself) {
    EXPECT_TRUE((AreUnitsQuantityEquivalent<Feet, Feet>::value));
    EXPECT_TRUE((AreUnitsQuantityEquivalent<Minutes, Minutes>::value));
}

TEST(AreUnitsQuantityEquivalent, InequivalentUnitCanBeMadeEquivalentByAppropriateScaling) {
    EXPECT_FALSE((AreUnitsQuantityEquivalent<Feet, Inches>::value));

    using Stretchedinches = decltype(Inches{} * unit_ratio(Feet{}, Inches{}));
    ASSERT_FALSE((detail::SameTypeIgnoringCvref<Feet, Stretchedinches>::value));
    EXPECT_TRUE((AreUnitsQuantityEquivalent<Feet, Stretchedinches>::value));
}

TEST(AreUnitsQuantityEquivalent, DifferentDimensionedUnitsAreNotEquivalent) {
    EXPECT_FALSE((AreUnitsQuantityEquivalent<Feet, Minutes>::value));
}

TEST(AreUnitsPointEquivalent, UnitIsEquivalentToItself) {
    EXPECT_TRUE((AreUnitsPointEquivalent<Feet, Feet>::value));
    EXPECT_TRUE((AreUnitsPointEquivalent<Celsius, Celsius>::value));
}

TEST(AreUnitsPointEquivalent, InequivalentUnitCanBeMadeEquivalentByAppropriateScaling) {
    EXPECT_FALSE((AreUnitsPointEquivalent<Feet, Inches>::value));

    using Stretchedinches = decltype(Inches{} * unit_ratio(Feet{}, Inches{}));
    ASSERT_FALSE((detail::SameTypeIgnoringCvref<Feet, Stretchedinches>::value));
    EXPECT_TRUE((AreUnitsPointEquivalent<Feet, Stretchedinches>::value));
}

TEST(AreUnitsPointEquivalent, DifferentDimensionedUnitsAreNotEquivalent) {
    EXPECT_FALSE((AreUnitsPointEquivalent<Feet, Minutes>::value));
}

TEST(AreUnitsPointEquivalent, UnitsWithDifferentOriginsAreNotPointEquivalent) {
    ASSERT_TRUE(are_units_quantity_equivalent(Celsius{}, Kelvins{}));
    EXPECT_FALSE(are_units_point_equivalent(Celsius{}, Kelvins{}));
}

TEST(AreUnitsPointEquivalent, DifferentUnitsWithDifferentButEquivalentOriginsArePointEquivalent) {
    // The origins of these units represent the same conceptual Quantity, although they are
    // represented in quantity-inequivalent units.
    ASSERT_THAT(Celsius::origin(), Eq(AlternateCelsius::origin()));
    ASSERT_FALSE(are_units_quantity_equivalent(decltype(Celsius::origin())::unit,
                                               decltype(AlternateCelsius::origin())::unit));

    // We check that this difference isn't enough to prevent them from being point-equivalent.  We
    // definitely want to be able to freely convert between `QuantityPoint` instances in either of
    // these point-equivalent units.
    EXPECT_TRUE(are_units_point_equivalent(Celsius{}, AlternateCelsius{}));
}

TEST(OriginDisplacement, IdenticallyZeroForOriginsThatCompareEqual) {
    ASSERT_THAT(detail::OriginOf<Celsius>::value(),
                Not(SameTypeAndValue(detail::OriginOf<AlternateCelsius>::value())));
    EXPECT_THAT(origin_displacement(Celsius{}, AlternateCelsius{}), SameTypeAndValue(ZERO));
}

TEST(OriginDisplacement, GivesDisplacementFromFirstToSecond) {
    EXPECT_EQ(origin_displacement(Kelvins{}, Celsius{}), milli(kelvins)(273'150));
    EXPECT_EQ(origin_displacement(Celsius{}, Kelvins{}), milli(kelvins)(-273'150));
}

struct OffsetCelsius : Celsius {
    static constexpr auto origin() { return detail::OriginOf<Celsius>::value() + kelvins(10); }
};

TEST(DisplaceOrigin, DisplacesOrigin) {
    EXPECT_EQ(origin_displacement(Celsius{}, OffsetCelsius{}), kelvins(10));
}

TEST(CommonUnit, FindsCommonMagnitude) {
    EXPECT_THAT((CommonUnitT<Feet, Feet>{}), QuantityEquivalentToUnit(Feet{}));
    EXPECT_THAT((CommonUnitT<Feet, Inches>{}), QuantityEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonUnitT<Inches, Feet>{}), QuantityEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonUnitT<Inches, Inches>{}), QuantityEquivalentToUnit(Inches{}));
}

TEST(CommonUnit, IndependentOfOrderingAndRepetitions) {
    using U = CommonUnitT<Feet, Yards, Inches>;
    StaticAssertTypeEq<U, CommonUnitT<Yards, Feet, Inches>>();
    StaticAssertTypeEq<U, CommonUnitT<Inches, Yards, Feet>>();
    StaticAssertTypeEq<U, CommonUnitT<Feet, Yards, Feet, Feet, Inches, Yards>>();
}

TEST(CommonUnit, PrefersUnitFromListIfAnyIdentical) {
    StaticAssertTypeEq<CommonUnitT<Feet, Feet>, Feet>();
    StaticAssertTypeEq<CommonUnitT<Feet, Inches, Yards>, Inches>();
}

TEST(CommonUnit, DownranksAnonymousScaledUnits) {
    StaticAssertTypeEq<CommonUnitT<Feet, decltype(Feet{} * mag<1>())>, Feet>();
    StaticAssertTypeEq<CommonUnitT<Feet, UnitImpl<Length>>, Feet>();

    using OpaqueFeetSquared = decltype(pow<2>(Feet{}) * ONE);
    using OpaqueFeet = UnitProductT<OpaqueFeetSquared, UnitInverseT<Feet>>;
    ASSERT_FALSE((std::is_same<OpaqueFeet, Feet>::value));
    ASSERT_TRUE((AreUnitsQuantityEquivalent<OpaqueFeet, Feet>::value));
    ASSERT_TRUE((InOrderFor<CommonUnit, Feet, OpaqueFeet>::value));
    StaticAssertTypeEq<CommonUnitT<Feet, OpaqueFeet>, Feet>();
}

// Four coprime units of the same dimension.
struct W : decltype(Inches{} * mag<2>()) {};
struct X : decltype(Inches{} * mag<3>()) {};
struct Y : decltype(Inches{} * mag<5>()) {};
struct Z : decltype(Inches{} * mag<7>()) {};

TEST(CommonUnit, UnpacksTypesInNestedCommonUnit) {
    using C1 = CommonUnitT<W, X>;
    ASSERT_TRUE((detail::IsPackOf<CommonUnit, C1>{}));

    using C2 = CommonUnitT<Y, Z>;
    ASSERT_TRUE((detail::IsPackOf<CommonUnit, C2>{}));

    using Common = CommonUnitT<C1, C2>;
    ASSERT_TRUE((detail::IsPackOf<CommonUnit, Common>{}));

    // Check that `c(c(w, x), c(y, z))` is the same as `c(w, x, y, z)`.
    StaticAssertTypeEq<Common, CommonUnitT<W, X, Y, Z>>();
}

TEST(CommonPointUnit, FindsCommonMagnitude) {
    EXPECT_THAT((CommonPointUnitT<Feet, Feet>{}), PointEquivalentToUnit(Feet{}));
    EXPECT_THAT((CommonPointUnitT<Feet, Inches>{}), PointEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonPointUnitT<Inches, Feet>{}), PointEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonPointUnitT<Inches, Inches>{}), PointEquivalentToUnit(Inches{}));
}

TEST(CommonPointUnit, HasMinimumOrigin) {
    EXPECT_EQ(origin_displacement(CommonPointUnitT<Kelvins, Celsius>{}, Kelvins{}), ZERO);
    EXPECT_EQ(origin_displacement(CommonPointUnitT<Fahrenheit, Celsius>{}, Fahrenheit{}), ZERO);
}

TEST(CommonPointUnit, TakesOriginMagnitudeIntoAccount) {
    using CommonByQuantity = CommonUnitT<Kelvins, Celsius>;
    using CommonByPoint = CommonPointUnitT<Kelvins, Celsius>;

    // The definition of Celsius in this file uses millikelvin to define its constant.
    EXPECT_EQ(unit_ratio(CommonByQuantity{}, CommonByPoint{}), mag<1000>());

    // The common point-unit should not be the _same_ as mK (since we never named the latter, and
    // thus can't "conjure it up").  However, it _should_ be _point-equivalent_ to mK.
    ASSERT_FALSE((std::is_same<CommonByPoint, Milli<Kelvins>>::value));
    EXPECT_THAT(CommonByPoint{}, PointEquivalentToUnit(milli(Kelvins{})));
}

TEST(CommonPointUnit, IndependentOfOrderingAndRepetitions) {
    using U = CommonPointUnitT<Celsius, Kelvins, Fahrenheit>;
    StaticAssertTypeEq<U, CommonPointUnitT<Kelvins, Celsius, Fahrenheit>>();
    StaticAssertTypeEq<U, CommonPointUnitT<Fahrenheit, Kelvins, Celsius>>();
    StaticAssertTypeEq<U, CommonPointUnitT<Kelvins, Celsius, Celsius, Fahrenheit, Kelvins>>();
}

TEST(CommonPointUnit, PrefersUnitFromListIfAnyIdentical) {
    StaticAssertTypeEq<CommonPointUnitT<Celsius, Celsius>, Celsius>();
    StaticAssertTypeEq<CommonPointUnitT<Celsius, Milli<Kelvins>, Milli<Celsius>>, Milli<Kelvins>>();
}

TEST(CommonPointUnit, UnpacksTypesInNestedCommonUnit) {
    using C1 = CommonPointUnitT<W, X>;
    ASSERT_TRUE((detail::IsPackOf<CommonPointUnit, C1>{}));

    using C2 = CommonPointUnitT<Y, Z>;
    ASSERT_TRUE((detail::IsPackOf<CommonPointUnit, C2>{}));

    using Common = CommonPointUnitT<C1, C2>;
    ASSERT_TRUE((detail::IsPackOf<CommonPointUnit, Common>{}));

    // Check that `c(c(w, x), c(y, z))` is the same as `c(w, x, y, z)`.
    StaticAssertTypeEq<Common, CommonPointUnitT<W, X, Y, Z>>();
}

TEST(UnitLabel, DefaultsToUnlabeledUnit) {
    EXPECT_THAT(unit_label<UnlabeledUnit>(), StrEq("[UNLABELED UNIT]"));
    EXPECT_EQ(sizeof(unit_label<UnlabeledUnit>()), 17);
}

TEST(UnitLabel, PicksUpLabelForLabeledUnit) {
    EXPECT_THAT(unit_label<Feet>(), StrEq("ft"));
    EXPECT_EQ(sizeof(unit_label<Feet>()), 3);
}

TEST(UnitLabel, PrintsExponentForUnitPower) {
    EXPECT_THAT(unit_label(Pow<Minutes, 2>{}), StrEq("min^2"));
    EXPECT_THAT(unit_label(Pow<Feet, 33>{}), StrEq("ft^33"));
    EXPECT_THAT(unit_label(Pow<TraitLabeledUnit, -4321>{}), StrEq("TLU^(-4321)"));

    EXPECT_THAT(unit_label(RatioPow<Minutes, 1, 2>{}), StrEq("min^(1/2)"));
    EXPECT_THAT(unit_label(RatioPow<Feet, -22, 7>{}), StrEq("ft^(-22/7)"));
}

TEST(UnitLabel, EmptyForNullProduct) { EXPECT_THAT(unit_label<UnitProduct<>>(), StrEq("")); }

TEST(UnitLabel, NonintrusivelyLabelableByTrait) {
    EXPECT_THAT(unit_label<TraitLabeledUnit>(), StrEq("TLU"));
    EXPECT_EQ(sizeof(unit_label<TraitLabeledUnit>()), 4);
}

TEST(UnitLabel, ProductComposesLabelsOfInputUnits) {
    EXPECT_THAT(unit_label(Feet{} * Minutes{}), AnyOf(StrEq("ft * min"), StrEq("min * ft")));
}

TEST(UnitLabel, QuotientComposesLabelsOfInputUnits) {
    EXPECT_THAT(unit_label(Feet{} / Minutes{}), StrEq("ft / min"));
}

TEST(UnitLabel, NumeratorGetsParensIfMultipleInputs) {
    EXPECT_THAT(unit_label(Feet{} * Minutes{} / Inches{}),
                AnyOf(StrEq("(ft * min) / in"), StrEq("(min * ft) / in")));
}

TEST(UnitLabel, NumeratorIsOneIfAllPowersNegative) {
    EXPECT_THAT(unit_label(UnitInverseT<decltype(Feet{} * Minutes{})>{}),
                AnyOf(StrEq("1 / (ft * min)"), StrEq("1 / (min * ft)")));
}

TEST(UnitLabel, NumeratorAndDenominatorBothGetParensIfMultipleItems) {
    EXPECT_THAT(unit_label((Inches{} * Yards{}) / (Feet{} * Minutes{})),
                AnyOf(StrEq("(in * yd) / (ft * min)"),
                      StrEq("(yd * in) / (ft * min)"),
                      StrEq("(in * yd) / (min * ft)"),
                      StrEq("(yd * in) / (min * ft)")));
}

TEST(UnitLabel, PowersAndProductsComposeNicely) {
    const auto unit = Inches{} * Pow<Yards, 2>{} * RatioPow<Minutes, -1, 2>{};
    EXPECT_THAT(unit_label(unit),
                AnyOf(StrEq("(in * yd^2) / min^(1/2)"), StrEq("(yd^2 * in) / min^(1/2)")));
}

TEST(UnitLabel, LabelsCommonUnitCorrectly) {
    using U = CommonUnitT<Inches, Meters>;
    EXPECT_THAT(unit_label(U{}), AnyOf(StrEq("COM[in, m]"), StrEq("COM[m, in]")));
}

TEST(UnitLabel, CommonUnitLabelWorksWithUnitProduct) {
    using U = CommonUnitT<UnitQuotientT<Meters, Minutes>, UnitQuotientT<Inches, Minutes>>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("COM[m / min, in / min]"), StrEq("COM[in / min, m / min]")));
}

TEST(UnitLabel, LabelsCommonPointUnitCorrectly) {
    using U = CommonPointUnitT<Inches, Meters>;
    EXPECT_THAT(unit_label(U{}), AnyOf(StrEq("COM_PT[in, m]"), StrEq("COM_PT[m, in]")));
}

TEST(UnitLabel, CommonPointUnitLabelWorksWithUnitProduct) {
    using U = CommonPointUnitT<UnitQuotientT<Meters, Minutes>, UnitQuotientT<Inches, Minutes>>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("COM_PT[m / min, in / min]"), StrEq("COM_PT[in / min, m / min]")));
}

namespace detail {

TEST(Origin, ZeroForUnitWithNoSpecifiedOrigin) {
    EXPECT_THAT(OriginOf<Kelvins>::value(), SameTypeAndValue(ZERO));
}

TEST(Origin, ValueOfOriginDataMemberIfAppropriate) {
    EXPECT_THAT(OriginOf<Celsius>::value(), SameTypeAndValue(milli(kelvins)(273'150)));
}

TEST(Origin, InheritedUnderScaling) {
    EXPECT_THAT(OriginOf<Milli<Celsius>>::value(), SameTypeAndValue(OriginOf<Celsius>::value()));
    EXPECT_THAT(OriginOf<decltype(Celsius{} * mag<5>() / mag<9>())>::value(),
                SameTypeAndValue(OriginOf<Celsius>::value()));
}

TEST(CommonOrigin, SymmetricUnderReordering) {
    // Rearrange the order; the result shouldn't change.
    StaticAssertTypeEq<decltype(CommonOrigin<Celsius, AlternateCelsius>::value()),
                       decltype(CommonOrigin<AlternateCelsius, Celsius>::value())>();

    // The bigger-Magnitude unit should "win".
    constexpr auto common_origin_value = CommonOrigin<Celsius, AlternateCelsius>::value();
    EXPECT_THAT(common_origin_value, SameTypeAndValue(Celsius::origin()));
    EXPECT_THAT(common_origin_value, Not(SameTypeAndValue(AlternateCelsius::origin())));
}

}  // namespace detail
}  // namespace au
