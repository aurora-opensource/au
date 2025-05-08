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

#include "au/unit_of_measure.hh"

#include "au/constant.hh"
#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/fahrenheit.hh"
#include "au/units/feet.hh"
#include "au/units/inches.hh"
#include "au/units/kelvins.hh"
#include "au/units/meters.hh"
#include "au/units/minutes.hh"
#include "au/units/yards.hh"
#include "au/utility/type_traits.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::au::detail::DimT;
using ::au::detail::MagT;
using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Not;
using ::testing::StaticAssertTypeEq;
using ::testing::StrEq;

struct Celsius : Kelvins {
    static constexpr auto origin() { return milli(kelvins)(273'150); }
    static constexpr const char label[] = "deg_C";
};
constexpr const char Celsius::label[];
constexpr auto celsius = QuantityMaker<Celsius>{};
constexpr auto celsius_pt = QuantityPointMaker<Celsius>{};

struct AlternateCelsius : Kelvins {
    static constexpr auto origin() { return micro(kelvins)(273'150'000); }
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

// Useful for testing "unit slot" compatibility in APIs.
template <typename UnitT>
struct SomeUnitWrapper {};
template <typename UnitT>
struct AssociatedUnit<SomeUnitWrapper<UnitT>> : stdx::type_identity<UnitT> {};

// Useful for testing parameter pack logic.
template <typename... Units>
struct SomePack {};
template <typename A, typename B>
struct InOrderFor<SomePack, A, B> : InOrderFor<CommonUnit, A, B> {};

struct UnlabeledUnit : UnitImpl<Length> {};

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
    EXPECT_THAT(
        (stdx::experimental::is_detected<detail::OriginMemberType, decltype(scaled_by_unit)>{}),
        IsFalse());
}

TEST(ScaledUnit, IsTypeIdentityWhenScalingByOne) {
    StaticAssertTypeEq<decltype(Feet{} * mag<1>()), Feet>();
    StaticAssertTypeEq<decltype((Feet{} * mag<3>()) / mag<3>()), Feet>();
}

TEST(IsUnit, TrueForUnitImpl) { EXPECT_THAT(IsUnit<UnitImpl<Length>>::value, IsTrue()); }

TEST(IsUnit, TrueForOpaqueTypedef) { EXPECT_THAT(IsUnit<Feet>::value, IsTrue()); }

TEST(IsUnit, TrueForPowerOfUnit) {
    EXPECT_THAT((IsUnit<Pow<Feet, 3>>::value), IsTrue());
    EXPECT_THAT((IsUnit<RatioPow<Feet, 1, 2>>::value), IsTrue());
}

TEST(IsUnit, SupportsDeductionFromInstance) {
    EXPECT_THAT(is_unit(Feet{}), IsTrue());
    EXPECT_THAT(is_unit(3), IsFalse());
}

TEST(IsUnit, TrueForUnrelatedTypeWithDimAndMag) {
    EXPECT_THAT(IsUnit<AdHocSpeedUnit>::value, IsTrue());
}

TEST(IsUnit, FalseIfDimOrMagHasWrongType) {
    EXPECT_THAT(is_unit(InvalidWrongDimType{}), IsFalse());
    EXPECT_THAT(is_unit(InvalidWrongMagType{}), IsFalse());
}

TEST(IsUnit, FunctionalFormFalseForQuantityMaker) { EXPECT_THAT(is_unit(meters), IsFalse()); }

TEST(FitsInUnitSlot, TrueForUnitAndQuantityMakerAndSingularNameFor) {
    EXPECT_THAT(fits_in_unit_slot(meters), IsTrue());
    EXPECT_THAT(fits_in_unit_slot(meter), IsTrue());
    EXPECT_THAT(fits_in_unit_slot(Meters{}), IsTrue());

    EXPECT_THAT(fits_in_unit_slot(1.2), IsFalse());
}

TEST(Product, IsUnitWithProductOfMagnitudesAndDimensions) {
    constexpr auto foot_yards = Feet{} * Yards{};
    EXPECT_THAT(is_unit(foot_yards), IsTrue());

    StaticAssertTypeEq<MagT<decltype(foot_yards)>, MagProductT<MagT<Feet>, MagT<Yards>>>();
    StaticAssertTypeEq<DimT<decltype(foot_yards)>, DimProductT<DimT<Feet>, DimT<Yards>>>();
}

TEST(Quotient, IsUnitWithQuotientOfMagnitudesAndDimensions) {
    constexpr auto in_per_min = Inches{} / Minutes{};
    EXPECT_THAT(is_unit(in_per_min), IsTrue());
    StaticAssertTypeEq<MagT<decltype(in_per_min)>, MagQuotientT<MagT<Inches>, MagT<Minutes>>>();
    StaticAssertTypeEq<DimT<decltype(in_per_min)>, DimQuotientT<DimT<Inches>, DimT<Minutes>>>();
}

TEST(Pow, FunctionGivesUnitWhichIsEquivalentToManuallyComputedPower) {
    constexpr auto in = Inches{};
    constexpr auto cubic_inches = pow<3>(in);

    EXPECT_THAT(is_unit(cubic_inches), IsTrue());
    EXPECT_THAT(are_units_quantity_equivalent(cubic_inches, in * in * in), IsTrue());
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
    EXPECT_THAT(IsUnit<FootInches>::value, IsTrue());
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

TEST(AssociatedUnitT, IsIdentityForTypeWithNoAssociatedUnit) {
    // We might have returned `void`, but this would require depending on `IsUnit`, which could slow
    // down `AssociatedUnitT` because it's used so widely.  It's simpler to think of it as a trait
    // which "redirects" a type only when there is a definite, positive reason to do so.
    StaticAssertTypeEq<AssociatedUnitT<double>, double>();
}

TEST(AssociatedUnitT, HandlesWrappersWhichHaveSpecializedAssociatedUnit) {
    StaticAssertTypeEq<AssociatedUnitT<SomeUnitWrapper<Feet>>, Feet>();
}

TEST(AssociatedUnitT, SupportsSingularNameFor) {
    StaticAssertTypeEq<AssociatedUnitT<SingularNameFor<Feet>>, Feet>();
}

TEST(UnitInverseT, CommutesWithProduct) {
    StaticAssertTypeEq<UnitInverseT<UnitProductT<Feet, Minutes>>,
                       UnitProductT<UnitInverseT<Feet>, UnitInverseT<Minutes>>>();
}

TEST(Root, FunctionalInterfaceHandlesInstancesCorrectly) {
    constexpr auto cubic_inches = pow<3>(Inches{});
    constexpr auto cbrt_cubic_inches = root<3>(cubic_inches);

    EXPECT_THAT(is_unit(cbrt_cubic_inches), IsTrue());
    EXPECT_THAT(are_units_quantity_equivalent(cbrt_cubic_inches, Inches{}), IsTrue());
}

TEST(Inverse, RaisesToPowerNegativeOne) {
    EXPECT_THAT(are_units_quantity_equivalent(inverse(Meters{}), pow<-1>(Meters{})), IsTrue());
}

TEST(Squared, RaisesToPowerTwo) {
    EXPECT_THAT(are_units_quantity_equivalent(squared(Meters{}), pow<2>(Meters{})), IsTrue());
}

TEST(Cubed, RaisesToPowerThree) {
    EXPECT_THAT(are_units_quantity_equivalent(cubed(Meters{}), pow<3>(Meters{})), IsTrue());
}

TEST(Sqrt, TakesSecondRoot) {
    EXPECT_THAT(are_units_quantity_equivalent(sqrt(Meters{}), root<2>(Meters{})), IsTrue());
}

TEST(Cbrt, TakesThirdRoot) {
    EXPECT_THAT(are_units_quantity_equivalent(cbrt(Meters{}), root<3>(Meters{})), IsTrue());
}

TEST(IsDimensionless, PicksOutDimensionlessUnit) {
    EXPECT_THAT((IsDimensionless<Feet>::value), IsFalse());

    EXPECT_THAT((IsDimensionless<UnitQuotientT<Inches, Yards>>::value), IsTrue());

    EXPECT_THAT(
        (IsDimensionless<UnitQuotientT<AdHocSpeedUnit, UnitQuotientT<Feet, Minutes>>>::value),
        IsTrue());
}

TEST(IsDimensionless, FunctionalInterfaceHandlesInstancesCorrectly) {
    EXPECT_THAT(is_dimensionless(Feet{}), IsFalse());
    EXPECT_THAT(is_dimensionless(Inches{} / Yards{}), IsTrue());
    EXPECT_THAT(is_dimensionless(AdHocSpeedUnit{} / Feet{} * Minutes{}), IsTrue());
}

TEST(IsDimensionless, FunctionalInterfaceHandlesQuantityMakersCorrectly) {
    EXPECT_THAT(is_dimensionless(feet), IsFalse());
    EXPECT_THAT(is_dimensionless(inches / yards), IsTrue());
}

TEST(IsUnitlessUnit, PicksOutUnitlessUnit) {
    EXPECT_THAT(is_unitless_unit(Inches{} / Yards{}), IsFalse());
    EXPECT_THAT(is_unitless_unit(Inches{} / Inches{}), IsTrue());
}

TEST(IsUnitlessUnit, FunctionalInterfaceHandlesQuantityMakersCorrectly) {
    EXPECT_THAT(is_unitless_unit(inches / yards), IsFalse());
    EXPECT_THAT(is_unitless_unit(inches / inches), IsTrue());
}

TEST(HasSameDimension, TrueForAnySingleDimension) {
    EXPECT_THAT(HasSameDimension<Feet>::value, IsTrue());
    EXPECT_THAT(HasSameDimension<Minutes>::value, IsTrue());
    EXPECT_THAT(HasSameDimension<AdHocSpeedUnit>::value, IsTrue());
}

TEST(HasSameDimension, CorrectlyIdentifiesEquivalenceClass) {
    EXPECT_THAT((HasSameDimension<Feet, Feet>::value), IsTrue());
    EXPECT_THAT((HasSameDimension<Feet, Yards>::value), IsTrue());
    EXPECT_THAT((HasSameDimension<Feet, Yards, Inches>::value), IsTrue());

    EXPECT_THAT((HasSameDimension<Minutes, Yards>::value), IsFalse());
}

TEST(HasSameDimension, FunctionalInterfaceHandlesInstancesCorrectly) {
    EXPECT_THAT(has_same_dimension(Feet{}, Feet{}), IsTrue());
    EXPECT_THAT(has_same_dimension(Feet{}, Yards{}), IsTrue());

    EXPECT_THAT(has_same_dimension(Feet{}, Yards{}, Inches{}, Yards{}), IsTrue());
    EXPECT_THAT(has_same_dimension(Feet{}, Yards{}, Inches{}, Minutes{}), IsFalse());
}

TEST(HasSameDimension, FunctionalInterfaceHandlesQuantityMakersCorrectly) {
    EXPECT_THAT(has_same_dimension(feet, feet), IsTrue());
    EXPECT_THAT(has_same_dimension(feet, yards), IsTrue());

    EXPECT_THAT(has_same_dimension(feet, yards, inches, yards), IsTrue());
    EXPECT_THAT(has_same_dimension(feet, yards, inches, minutes), IsFalse());
}

TEST(UnitRatio, ComputesRatioForSameDimensionedUnits) {
    StaticAssertTypeEq<UnitRatioT<Yards, Inches>, decltype(mag<36>())>();
    StaticAssertTypeEq<UnitRatioT<Inches, Inches>, decltype(mag<1>())>();
    StaticAssertTypeEq<UnitRatioT<Inches, Yards>, decltype(mag<1>() / mag<36>())>();
}

TEST(UnitRatio, FunctionalInterfaceHandlesInstancesCorrectly) {
    EXPECT_THAT(unit_ratio(Yards{}, Inches{}), Eq(mag<36>()));
    EXPECT_THAT(unit_ratio(Inches{}, Inches{}), Eq(mag<1>()));
    EXPECT_THAT(unit_ratio(Inches{}, Yards{}), Eq(mag<1>() / mag<36>()));
}

TEST(UnitRatio, FunctionalInterfaceHandlesQuantityMakersCorrectly) {
    EXPECT_THAT(unit_ratio(yards, inches), Eq(mag<36>()));
}

TEST(AreUnitsQuantityEquivalent, UnitIsEquivalentToItself) {
    EXPECT_THAT((AreUnitsQuantityEquivalent<Feet, Feet>::value), IsTrue());
    EXPECT_THAT((AreUnitsQuantityEquivalent<Minutes, Minutes>::value), IsTrue());
}

TEST(AreUnitsQuantityEquivalent, InequivalentUnitCanBeMadeEquivalentByAppropriateScaling) {
    EXPECT_THAT((AreUnitsQuantityEquivalent<Feet, Inches>::value), IsFalse());

    using Stretchedinches = decltype(Inches{} * unit_ratio(Feet{}, Inches{}));
    ASSERT_THAT((detail::SameTypeIgnoringCvref<Feet, Stretchedinches>::value), IsFalse());
    EXPECT_THAT((AreUnitsQuantityEquivalent<Feet, Stretchedinches>::value), IsTrue());
}

TEST(AreUnitsQuantityEquivalent, DifferentDimensionedUnitsAreNotEquivalent) {
    EXPECT_THAT((AreUnitsQuantityEquivalent<Feet, Minutes>::value), IsFalse());
}

TEST(AreUnitsQuantityEquivalent, FunctionalInterfaceHandlesInstancesCorrectly) {
    EXPECT_THAT(are_units_quantity_equivalent(Feet{}, Minutes{}), IsFalse());
}

TEST(AreUnitsQuantityEquivalent, FunctionalInterfaceHandlesQuantityMakersCorrectly) {
    EXPECT_THAT(are_units_quantity_equivalent(feet, minutes), IsFalse());
}

TEST(AreUnitsPointEquivalent, UnitIsEquivalentToItself) {
    EXPECT_THAT((AreUnitsPointEquivalent<Feet, Feet>::value), IsTrue());
    EXPECT_THAT((AreUnitsPointEquivalent<Celsius, Celsius>::value), IsTrue());
}

TEST(AreUnitsPointEquivalent, InequivalentUnitCanBeMadeEquivalentByAppropriateScaling) {
    EXPECT_THAT((AreUnitsPointEquivalent<Feet, Inches>::value), IsFalse());

    using Stretchedinches = decltype(Inches{} * unit_ratio(Feet{}, Inches{}));
    ASSERT_THAT((detail::SameTypeIgnoringCvref<Feet, Stretchedinches>::value), IsFalse());
    EXPECT_THAT((AreUnitsPointEquivalent<Feet, Stretchedinches>::value), IsTrue());
}

TEST(AreUnitsPointEquivalent, DifferentDimensionedUnitsAreNotEquivalent) {
    EXPECT_THAT((AreUnitsPointEquivalent<Feet, Minutes>::value), IsFalse());
}

TEST(AreUnitsPointEquivalent, UnitsWithDifferentOriginsAreNotPointEquivalent) {
    ASSERT_THAT(are_units_quantity_equivalent(Celsius{}, Kelvins{}), IsTrue());
    EXPECT_THAT(are_units_point_equivalent(Celsius{}, Kelvins{}), IsFalse());
}

TEST(AreUnitsPointEquivalent, FunctionalInterfaceHandlesQuantityMakersCorrectly) {
    ASSERT_THAT(are_units_quantity_equivalent(celsius, kelvins), IsTrue());
    EXPECT_THAT(are_units_point_equivalent(celsius, kelvins), IsFalse());
}

TEST(AreUnitsPointEquivalent, DifferentUnitsWithDifferentButEquivalentOriginsArePointEquivalent) {
    // The origins of these units represent the same conceptual Quantity, although they are
    // represented in quantity-inequivalent units.
    ASSERT_THAT(Celsius::origin(), Eq(AlternateCelsius::origin()));
    ASSERT_THAT(are_units_quantity_equivalent(decltype(Celsius::origin())::unit,
                                              decltype(AlternateCelsius::origin())::unit),
                IsFalse());

    // We check that this difference isn't enough to prevent them from being point-equivalent.  We
    // definitely want to be able to freely convert between `QuantityPoint` instances in either of
    // these point-equivalent units.
    EXPECT_THAT(are_units_point_equivalent(Celsius{}, AlternateCelsius{}), IsTrue());
}

struct OffsetCelsius : Celsius {
    // The origin is _specified_ in the common units of the Celsius origin (which we gave in mK),
    // and a difference of 10 in units of [(1/6) K].  This means that the type of the origin should
    // be in units of [(1/3000) K].  However, the difference should boil down to a constant of
    // exactly (5/3 K)
    static constexpr auto origin() {
        return detail::OriginOf<Celsius>::value() + (kelvins / mag<6>())(10);
    }
    static constexpr const char label[] = "offset_deg_C";
};
constexpr const char OffsetCelsius::label[];

TEST(DisplaceOrigin, DisplacesOrigin) {
    EXPECT_THAT(origin_displacement(Celsius{}, OffsetCelsius{}), Eq((kelvins / mag<3>())(5)));
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

TEST(CommonUnit, DedupesUnitsMadeIdenticalAfterUnscalingSameScaledUnit) {
    StaticAssertTypeEq<CommonUnitT<decltype(Feet{} * mag<3>()), decltype(Feet{} * mag<5>())>,
                       Feet>();
}

TEST(CommonUnit, HandlesAndNeglectsZero) {
    StaticAssertTypeEq<CommonUnitT<Yards, Zero, Feet>, Feet>();
    StaticAssertTypeEq<CommonUnitT<Zero, Feet, Zero>, Feet>();
}

TEST(CommonUnit, ZeroIfAllInputsAreZero) {
    StaticAssertTypeEq<CommonUnitT<>, Zero>();
    StaticAssertTypeEq<CommonUnitT<Zero>, Zero>();
    StaticAssertTypeEq<CommonUnitT<Zero, Zero>, Zero>();
    StaticAssertTypeEq<CommonUnitT<Zero, Zero, Zero>, Zero>();
}

TEST(CommonUnit, DownranksAnonymousScaledUnits) {
    StaticAssertTypeEq<CommonUnitT<Yards, decltype(Feet{} * mag<3>())>, Yards>();
}

TEST(CommonUnit, WhenCommonUnitLabelWouldBeIdenticalToSomeUnitJustUsesThatUnit) {
    StaticAssertTypeEq<CommonUnitT<decltype(Feet{} * mag<6>()), decltype(Feet{} * mag<10>())>,
                       decltype(Feet{} * mag<2>())>();
}

TEST(CommonUnit, AlwaysPositiveUnlessAllInputsAreNegative) {
    constexpr auto NEG = -mag<1>();

    // If any unit is positive, so is the common unit.
    EXPECT_THAT(common_unit(feet, inches), QuantityEquivalentToUnit(inches));
    EXPECT_THAT(common_unit(feet * NEG, inches), QuantityEquivalentToUnit(inches));
    EXPECT_THAT(common_unit(feet, inches * NEG), QuantityEquivalentToUnit(inches));

    // The common unit of all negative units is negative.
    EXPECT_THAT(common_unit(feet * NEG, inches * NEG), QuantityEquivalentToUnit(inches * NEG));
}

// Four coprime units of the same dimension.
struct W : decltype(Inches{} * mag<2>()) {};
struct X : decltype(Inches{} * mag<3>()) {};
struct Y : decltype(Inches{} * mag<5>()) {};
struct Z : decltype(Inches{} * mag<7>()) {};

TEST(CommonUnit, UnpacksTypesInNestedCommonUnit) {
    using C1 = CommonUnitT<W, X>;
    ASSERT_THAT((detail::IsPackOf<CommonUnit, C1>{}), IsTrue());

    using C2 = CommonUnitT<Y, Z>;
    ASSERT_THAT((detail::IsPackOf<CommonUnit, C2>{}), IsTrue());

    using Common = CommonUnitT<C1, C2>;
    ASSERT_THAT((detail::IsPackOf<CommonUnit, Common>{}), IsTrue());

    // Check that `c(c(w, x), c(y, z))` is the same as `c(w, x, y, z)`.
    StaticAssertTypeEq<Common, CommonUnitT<W, X, Y, Z>>();
}

TEST(CommonUnit, CanCombineUnitsThatWouldBothBeAnonymousScaledUnits) {
    EXPECT_THAT((feet / mag<3>())(1), Eq((inches * mag<4>())(1)));
}

TEST(CommonUnit, SupportsUnitSlots) {
    StaticAssertTypeEq<decltype(common_unit(feet, meters)), CommonUnitT<Feet, Meters>>();
}

TEST(CommonPointUnit, FindsCommonMagnitude) {
    EXPECT_THAT((CommonPointUnitT<Feet, Feet>{}), PointEquivalentToUnit(Feet{}));
    EXPECT_THAT((CommonPointUnitT<Feet, Inches>{}), PointEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonPointUnitT<Inches, Feet>{}), PointEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonPointUnitT<Inches, Inches>{}), PointEquivalentToUnit(Inches{}));
}

TEST(CommonPointUnit, HasMinimumOrigin) {
    EXPECT_THAT(origin_displacement(CommonPointUnitT<Kelvins, Celsius>{}, Kelvins{}), Eq(ZERO));
    EXPECT_THAT(origin_displacement(CommonPointUnitT<Fahrenheit, Celsius>{}, Fahrenheit{}),
                Eq(ZERO));
}

TEST(CommonPointUnit, TakesOriginMagnitudeIntoAccount) {
    using CommonByQuantity = CommonUnitT<Kelvins, Celsius>;
    using CommonByPoint = CommonPointUnitT<Kelvins, Celsius>;

    EXPECT_THAT(unit_ratio(CommonByQuantity{}, CommonByPoint{}), Eq(mag<20>()));

    constexpr auto expected = Kelvins{} / mag<20>();
    EXPECT_THAT(CommonByPoint{}, PointEquivalentToUnit(expected))
        << "Expected " << unit_label(expected) << ", got " << unit_label(CommonByPoint{});
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
    ASSERT_THAT((detail::IsPackOf<CommonPointUnit, C1>{}), IsTrue());

    using C2 = CommonPointUnitT<Y, Z>;
    ASSERT_THAT((detail::IsPackOf<CommonPointUnit, C2>{}), IsTrue());

    using Common = CommonPointUnitT<C1, C2>;
    ASSERT_THAT((detail::IsPackOf<CommonPointUnit, Common>{}), IsTrue());

    // Check that `c(c(w, x), c(y, z))` is the same as `c(w, x, y, z)`.
    StaticAssertTypeEq<Common, CommonPointUnitT<W, X, Y, Z>>();
}

TEST(CommonPointUnit, SupportsUnitSlots) {
    StaticAssertTypeEq<decltype(common_point_unit(kelvins_pt, celsius_pt)),
                       CommonPointUnitT<Kelvins, Celsius>>();
}

TEST(MakeCommon, PreservesCategory) {
    constexpr auto feeters = make_common(feet, meters);
    EXPECT_THAT(feet(1u) % feeters(1u), Eq(ZERO));
    EXPECT_THAT(meters(1u) % feeters(1u), Eq(ZERO));
    EXPECT_THAT(detail::gcd(feet(1u).in(feeters), meters(1u).in(feeters)), Eq(1u));

    using symbols::ft;
    using symbols::m;
    EXPECT_THAT(123 * make_common(m, ft), SameTypeAndValue(feeters(123)));
}

TEST(MakeCommonPoint, PreservesCategory) {
    constexpr auto celsenheit_pt = make_common_point(celsius_pt, fahrenheit_pt);

    // The origin of the common point unit is the lowest origin among all input units.
    EXPECT_THAT(celsenheit_pt(0), Eq(fahrenheit_pt(0)));
    EXPECT_THAT(celsenheit_pt(0), Lt(celsius_pt(0)))
        << "Difference: " << celsius_pt(0) - celsenheit_pt(0);

    // The common point unit should evenly divide both input units.
    //
    // (We can't necessarily say that it is the _largest_ such unit, as we could for the common
    // unit, because we also have to accomodate the unit for the _difference of the origins_.)
    constexpr auto one_f = fahrenheit_pt(1) - fahrenheit_pt(0);
    constexpr auto one_c = celsius_pt(1) - celsius_pt(0);
    constexpr auto one_ch = celsenheit_pt(1) - celsenheit_pt(0);
    EXPECT_THAT(one_f % one_ch, Eq(ZERO));
    EXPECT_THAT(one_c % one_ch, Eq(ZERO));
}

TEST(UnitLabel, DefaultsToUnlabeledUnit) {
    EXPECT_THAT(unit_label<UnlabeledUnit>(), StrEq("[UNLABELED UNIT]"));
    EXPECT_THAT(sizeof(unit_label<UnlabeledUnit>()), Eq(17));
}

TEST(UnitLabel, PicksUpLabelForLabeledUnit) {
    EXPECT_THAT(unit_label<Feet>(), StrEq("ft"));
    EXPECT_THAT(sizeof(unit_label<Feet>()), Eq(3));
}

TEST(UnitLabel, PrependsScaleFactorToLabelForScaledUnit) {
    EXPECT_THAT(unit_label<decltype(Feet{} * mag<3>())>(), StrEq("[3 ft]"));
    EXPECT_THAT(unit_label<decltype(Feet{} * (-mag<3>()))>(), StrEq("[-3 ft]"));
    EXPECT_THAT(unit_label<decltype(Feet{} / mag<12>())>(), StrEq("[(1 / 12) ft]"));
    EXPECT_THAT(unit_label<decltype(Feet{} / (-mag<12>()))>(), StrEq("[(-1 / 12) ft]"));
}

TEST(UnitLabel, ApplyingMultipleScaleFactorsComposesToOneSingleScaleFactor) {
    EXPECT_THAT(unit_label<decltype(Feet{} * mag<7>() / mag<12>())>(), StrEq("[(7 / 12) ft]"));
    EXPECT_THAT(unit_label<decltype(Feet{} * mag<2>() / mag<3>() * mag<5>() / mag<7>())>(),
                StrEq("[(10 / 21) ft]"));
}

TEST(UnitLabel, NegatedUnitOmitsNumerals) {
    EXPECT_THAT(unit_label<decltype(Feet{} * (-mag<1>()))>(), StrEq("[-ft]"));
}

TEST(UnitLabel, OmitsTrivialScaleFactor) {
    EXPECT_THAT(unit_label<decltype(Feet{} * mag<1>())>(), StrEq("ft"));
    EXPECT_THAT(unit_label<decltype((Feet{} * mag<3>()) / mag<3>())>(), StrEq("ft"));
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
    EXPECT_THAT(sizeof(unit_label<TraitLabeledUnit>()), Eq(4));
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
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 127) in], [(1 / 5000) m]}"),
                      StrEq("EQUIV{[(1 / 5000) m], [(1 / 127) in]}")));
}

TEST(UnitLabel, CommonUnitLabelWorksWithUnitProduct) {
    using U = CommonUnitT<UnitQuotientT<Meters, Minutes>, UnitQuotientT<Inches, Minutes>>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 127) in / min], [(1 / 5000) m / min]}"),
                      StrEq("EQUIV{[(1 / 5000) m / min], [(1 / 127) in / min]}")));
}

TEST(UnitLabel, RemovesDuplicatesFromCommonUnitLabel) {
    // A likely failure mode for this test would be `"EQUIV{[24 in], [2 ft], [2 ft]}"`.
    EXPECT_THAT(
        unit_label(common_unit(Feet{} * mag<6>(), Feet{} * mag<10>(), Inches{} * mag<48>())),
        AnyOf(StrEq("EQUIV{[2 ft], [24 in]}"), StrEq("EQUIV{[24 in], [2 ft]}")));
}

TEST(UnitLabel, ReducesToSingleUnitLabelIfAllUnitsAreTheSame) {
    // A likely failure mode for this test would be `"EQUIV{[2 ft], [2 ft]}"`.
    EXPECT_THAT(unit_label(common_unit(Feet{} * mag<6>(), Feet{} * mag<10>())), StrEq("[2 ft]"));
}

TEST(UnitLabel, LabelsCommonPointUnitCorrectly) {
    using U = CommonPointUnitT<Inches, Meters>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 127) in], [(1 / 5000) m]}"),
                      StrEq("EQUIV{[(1 / 5000) m], [(1 / 127) in]}")));
}

TEST(UnitLabel, CommonPointUnitLabelWorksWithUnitProduct) {
    using U = CommonPointUnitT<UnitQuotientT<Meters, Minutes>, UnitQuotientT<Inches, Minutes>>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 127) in / min], [(1 / 5000) m / min]}"),
                      StrEq("EQUIV{[(1 / 5000) m / min], [(1 / 127) in / min]}")));
}

TEST(UnitLabel, CommonPointUnitLabelTakesOriginOffsetIntoAccount) {
    using U = CommonPointUnitT<Celsius, OffsetCelsius>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 3) deg_C], [(1 / 5) (@(0 offset_deg_C) - @(0 deg_C))]}"),
                      StrEq("EQUIV{[(1 / 5) (@(0 offset_deg_C) - @(0 deg_C))], [(1 / 3) deg_C]}")));
}

TEST(UnitLabel, APICompatibleWithUnitSlots) { EXPECT_THAT(unit_label(feet), StrEq("ft")); }

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

TEST(UnitOfLowestOrigin, SelectsSingleUnit) {
    StaticAssertTypeEq<UnitOfLowestOrigin<Celsius>, Celsius>();
    StaticAssertTypeEq<UnitOfLowestOrigin<Kelvins>, Kelvins>();
}

TEST(UnitOfLowestOrigin, SelectsLowestOriginUnit) {
    StaticAssertTypeEq<UnitOfLowestOrigin<Celsius, Kelvins>, Kelvins>();
    StaticAssertTypeEq<UnitOfLowestOrigin<Kelvins, Celsius>, Kelvins>();
}

TEST(UnitOfLowestOrigin, ProducesConsistentResultsRegardlessOfOrdering) {
    StaticAssertTypeEq<UnitOfLowestOrigin<Celsius, Milli<Celsius>>,
                       UnitOfLowestOrigin<Milli<Celsius>, Celsius>>();
}

TEST(ComputeOriginDisplacementUnit, ZeroForSameOrigin) {
    StaticAssertTypeEq<ComputeOriginDisplacementUnit<Celsius, Milli<Celsius>>, Zero>();
}

TEST(ComputeOriginDisplacementUnit, HasExpectedMagnitudeAndSign) {
    constexpr auto disp_k0_to_c0 = make_constant(ComputeOriginDisplacementUnit<Kelvins, Celsius>{});
    EXPECT_THAT(disp_k0_to_c0, Eq(centi(kelvins)(273'15)));

    constexpr auto disp_c0_to_k0 = make_constant(ComputeOriginDisplacementUnit<Celsius, Kelvins>{});
    EXPECT_THAT(disp_c0_to_k0, Eq(centi(kelvins)(-273'15)));
}

TEST(ComputeOriginDisplacementUnit, HasExpectedLabel) {
    EXPECT_THAT(unit_label(origin_displacement_unit(kelvins_pt, celsius_pt)),
                StrEq("(@(0 deg_C) - @(0 K))"));
    EXPECT_THAT(unit_label(origin_displacement_unit(celsius_pt, kelvins_pt)),
                StrEq("(@(0 K) - @(0 deg_C))"));
}

TEST(EliminateRedundantUnits, IdentityForEmptySet) {
    StaticAssertTypeEq<EliminateRedundantUnits<SomePack<>>, SomePack<>>();
}

TEST(EliminateRedundantUnits, IdentityForSingleUnit) {
    StaticAssertTypeEq<EliminateRedundantUnits<SomePack<Feet>>, SomePack<Feet>>();
}

TEST(EliminateRedundantUnits, RemovesRepeatedUnit) {
    StaticAssertTypeEq<EliminateRedundantUnits<SomePack<Feet, Feet>>, SomePack<Feet>>();
    StaticAssertTypeEq<EliminateRedundantUnits<SomePack<Feet, Meters, Feet>>,
                       SomePack<Meters, Feet>>();
}

TEST(EliminateRedundantUnits, AlwaysRemovesSameUnitAmongQuantityEquivalentChoices) {
    using Twelvinch = decltype(Inches{} * mag<12>());
    StaticAssertTypeEq<EliminateRedundantUnits<SomePack<Feet, Twelvinch>>,
                       EliminateRedundantUnits<SomePack<Twelvinch, Feet>>>();
}

TEST(UnscaledUnit, IdentityForGeneralUnits) {
    StaticAssertTypeEq<UnscaledUnit<Feet>, Feet>();
    StaticAssertTypeEq<UnscaledUnit<Celsius>, Celsius>();
}

TEST(UnscaledUnit, RemovesScaleFactorFromScaledUnit) {
    StaticAssertTypeEq<UnscaledUnit<decltype(Feet{} * mag<3>())>, Feet>();
    StaticAssertTypeEq<UnscaledUnit<decltype(Celsius{} / mag<2>())>, Celsius>();
}

TEST(DistinctUnscaledUnits, UnitListOfOneElementForNonCommonUnit) {
    StaticAssertTypeEq<DistinctUnscaledUnits<Feet>, UnitList<Feet>>();
    StaticAssertTypeEq<DistinctUnscaledUnits<decltype(Feet{} / mag<12>())>, UnitList<Feet>>();
}

TEST(DistinctUnscaledUnits, RemovesDupesFromCommonUnit) {
    StaticAssertTypeEq<
        DistinctUnscaledUnits<decltype(common_unit(Feet{} * mag<3>(), Feet{} / mag<12>()))>,
        UnitList<Feet>>();
    StaticAssertTypeEq<DistinctUnscaledUnits<decltype(common_unit(
                           Feet{} * mag<3>(), Inches{} * mag<48>(), Feet{} * mag<5>()))>,
                       UnitList<Inches, Feet>>();
}

TEST(SimplifyIfOnlyOneUnscaledUnit, IdentityForNonCommonUnit) {
    StaticAssertTypeEq<SimplifyIfOnlyOneUnscaledUnit<Feet>, Feet>();
    StaticAssertTypeEq<SimplifyIfOnlyOneUnscaledUnit<decltype(Feet{} * mag<3>())>,
                       decltype(Feet{} * mag<3>())>();
}

}  // namespace detail
}  // namespace au
