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
#include "au/quantity.hh"
#include "au/quantity_point.hh"
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
using ::testing::Lt;
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
    using Dim = DimQuotient<Length, Time>;
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
struct AssociatedUnitImpl<SomeUnitWrapper<UnitT>> : stdx::type_identity<UnitT> {};

// Useful for testing parameter pack logic.
template <typename... Units>
struct SomePack {};
template <typename A, typename B>
struct InOrderFor<SomePack, A, B> : InOrderFor<CommonUnitPack, A, B> {};

struct UnlabeledUnit : UnitImpl<Length> {};

MATCHER_P(QuantityEquivalentToUnit, target, "") {
    return are_units_quantity_equivalent(arg, target);
}

MATCHER_P(PointEquivalentToUnit, target, "") { return are_units_point_equivalent(arg, target); }

TEST(Unit, ProductWithMagnitudeGivesSameDimensionAndMultipliesMagnitude) {
    StaticAssertTypeEq<UnitRatio<Yards, Feet>, decltype(mag<3>())>();
    StaticAssertTypeEq<UnitRatio<Yards, Inches>, decltype(mag<36>())>();
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

    StaticAssertTypeEq<MagT<decltype(foot_yards)>, MagProduct<MagT<Feet>, MagT<Yards>>>();
    StaticAssertTypeEq<DimT<decltype(foot_yards)>, DimProduct<DimT<Feet>, DimT<Yards>>>();
}

TEST(Quotient, IsUnitWithQuotientOfMagnitudesAndDimensions) {
    constexpr auto in_per_min = Inches{} / Minutes{};
    EXPECT_THAT(is_unit(in_per_min), IsTrue());
    StaticAssertTypeEq<MagT<decltype(in_per_min)>, MagQuotient<MagT<Inches>, MagT<Minutes>>>();
    StaticAssertTypeEq<DimT<decltype(in_per_min)>, DimQuotient<DimT<Inches>, DimT<Minutes>>>();
}

TEST(Pow, FunctionGivesUnitWhichIsEquivalentToManuallyComputedPower) {
    constexpr auto in = Inches{};
    constexpr auto cubic_inches = pow<3>(in);

    EXPECT_THAT(is_unit(cubic_inches), IsTrue());
    EXPECT_THAT(are_units_quantity_equivalent(cubic_inches, in * in * in), IsTrue());
}

TEST(UnitProductPack, IsUnitlessUnitForNoInputs) {
    StaticAssertTypeEq<DimT<UnitProductPack<>>, Dimension<>>();
    StaticAssertTypeEq<MagT<UnitProductPack<>>, Magnitude<>>();
}

TEST(UnitProductPack, ExactlyCancellingInstancesYieldsNullPack) {
    StaticAssertTypeEq<decltype(Feet{} * Inches{} / Minutes{} / Inches{} * Minutes{} / Feet{}),
                       UnitProductPack<>>();
}

TEST(UnitSumPack, HasDimensionThatMatchesAllInputs) {
    StaticAssertTypeEq<DimT<UnitSumPack<Feet, Inches>>, Length>();
    StaticAssertTypeEq<DimT<UnitSumPack<Meters, Feet, Inches>>, Length>();
}

TEST(UnitSumPack, HasMagnitudeThatIsSumOfInputMagnitudes) {
    StaticAssertTypeEq<MagT<UnitSumPack<Feet, Inches>>,
                       MagProduct<MagT<Inches>, Magnitude<Prime<13>>>>();
}

TEST(UnitSumPack, IsAUnit) {
    EXPECT_THAT((IsUnit<UnitSumPack<Feet, Inches>>::value), IsTrue());
    EXPECT_THAT((IsUnit<UnitSumPack<Meters, Feet, Inches>>::value), IsTrue());
}

TEST(UnitSumPack, PositiveUnitsJoinedWithPlus) {
    EXPECT_THAT(unit_label(UnitSumPack<Feet, Inches>{}), StrEq("(ft + in)"));
}

TEST(UnitSumPack, ThreePositiveUnitsJoinedWithPlus) {
    EXPECT_THAT(unit_label(UnitSumPack<Meters, Feet, Inches>{}), StrEq("(m + ft + in)"));
}

TEST(UnitSumPack, NegativeUnitAfterPositiveUsesMinusSign) {
    using NegInches = ScaledUnit<Inches, Magnitude<Negative>>;
    EXPECT_THAT(unit_label(UnitSumPack<Feet, NegInches>{}), StrEq("(ft - in)"));
}

TEST(UnitSumPack, NegativeUnitAtStartGetsMinusPrefix) {
    using NegFeet = ScaledUnit<Feet, Magnitude<Negative>>;
    EXPECT_THAT(unit_label(UnitSumPack<NegFeet, Inches>{}), StrEq("(-ft + in)"));
}

TEST(UnitSumPack, MultipleNegativeUnitsShowMinusSigns) {
    using NegFeet = ScaledUnit<Feet, Magnitude<Negative>>;
    using NegInches = ScaledUnit<Inches, Magnitude<Negative>>;
    EXPECT_THAT(unit_label(UnitSumPack<NegFeet, NegInches>{}), StrEq("(-ft - in)"));
}

TEST(UnitSumPack, MixedSignsShowAppropriateOperators) {
    using NegFeet = ScaledUnit<Feet, Magnitude<Negative>>;
    EXPECT_THAT(unit_label(UnitSumPack<Meters, NegFeet, Inches>{}), StrEq("(m - ft + in)"));
}

TEST(UnitSumPack, PositiveScaledUnitShowsScaleInLabel) {
    using TwoFeet = decltype(Feet{} * mag<2>());
    EXPECT_THAT(unit_label(UnitSumPack<TwoFeet, Inches>{}), StrEq("(2 ft + in)"));
}

TEST(UnitSumPack, NegativeScaledUnitShowsScaleWithoutSign) {
    using NegTwoFeet = decltype(Feet{} * (-mag<2>()));
    EXPECT_THAT(unit_label(UnitSumPack<NegTwoFeet, Inches>{}), StrEq("(-2 ft + in)"));
}

TEST(UnitSumPack, FractionalScaleShowsInLabel) {
    using ThreeQuartersInches = decltype(Inches{} * mag<3>() / mag<4>());
    EXPECT_THAT(unit_label(UnitSumPack<Feet, ThreeQuartersInches>{}), StrEq("(ft + (3 / 4) in)"));
}

TEST(UnitSumPack, NegativeFractionalScaleShowsCorrectly) {
    using NegThreeQuartersInches = decltype(Inches{} * (-mag<3>()) / mag<4>());
    EXPECT_THAT(unit_label(UnitSumPack<Feet, NegThreeQuartersInches>{}),
                StrEq("(ft - (3 / 4) in)"));
}

TEST(UnitSumPack, ComplexMixedScalesAndSigns) {
    using NegTwoFeet = decltype(Feet{} * (-mag<2>()));
    using ThreeQuartersInches = decltype(Inches{} * mag<3>() / mag<4>());
    EXPECT_THAT(unit_label(UnitSumPack<NegTwoFeet, ThreeQuartersInches, Meters>{}),
                StrEq("(-2 ft + (3 / 4) in + m)"));
}

TEST(UnitSum, EmptyUnitSumIsZero) { StaticAssertTypeEq<UnitSum<>, Zero>(); }

TEST(UnitSum, ZeroWhenSingleUnitCancelsItself) {
    using NegFeet = decltype(Feet{} * (-mag<1>()));
    StaticAssertTypeEq<UnitSum<Feet, NegFeet>, Zero>();
}

TEST(UnitSum, ZeroWhenMultipleUnitsAllCancel) {
    using NegFeet = decltype(Feet{} * (-mag<1>()));
    using NegInches = decltype(Inches{} * (-mag<1>()));
    StaticAssertTypeEq<UnitSum<Feet, Inches, NegFeet, NegInches>, Zero>();
}

TEST(UnitSum, SingleUnitReturnedDirectly) { StaticAssertTypeEq<UnitSum<Feet>, Feet>(); }

TEST(UnitSum, SingleUnitWithCoefficientReturnedAsScaledUnit) {
    using TwoFeet = decltype(Feet{} * mag<2>());
    StaticAssertTypeEq<UnitSum<TwoFeet>, TwoFeet>();
}

TEST(UnitSum, SameUnscaledUnitsCollectIntoSingleScaledUnit) {
    using TwoFeet = decltype(Feet{} * mag<2>());
    StaticAssertTypeEq<UnitSum<Feet, Feet>, TwoFeet>();
}

TEST(UnitSum, DifferentScalesOfSameUnitCombine) {
    using TwoFeet = decltype(Feet{} * mag<2>());
    using ThreeFeet = decltype(Feet{} * mag<3>());
    using FiveFeet = decltype(Feet{} * mag<5>());
    StaticAssertTypeEq<UnitSum<TwoFeet, ThreeFeet>, FiveFeet>();
}

TEST(UnitSum, MultipleUnscaledUnitsCancelToLeaveSingleUnit) {
    using NegInches = decltype(Inches{} * (-mag<1>()));
    StaticAssertTypeEq<UnitSum<Inches, Feet, NegInches>, Feet>();
}

TEST(UnitSum, CollectsLikeTerms) {
    using TwoFeet = decltype(Feet{} * mag<2>());
    using ThreeFeet = decltype(Feet{} * mag<3>());
    using ThreeInches = decltype(Inches{} * mag<3>());

    ASSERT_THAT((InOrderFor<UnitSumPack, Inches, Feet>::value), IsTrue());
    StaticAssertTypeEq<UnitSum<TwoFeet, ThreeInches, Feet>, UnitSumPack<ThreeInches, ThreeFeet>>();
}

TEST(UnitSum, PositiveCoefficientsBeforeNegative) {
    using NegInches = decltype(Inches{} * (-mag<1>()));

    ASSERT_THAT((InOrderFor<UnitSumPack, Inches, Feet>::value), IsTrue());
    StaticAssertTypeEq<UnitSum<NegInches, Feet>, UnitSumPack<Feet, NegInches>>();
}

TEST(UnitSum, PositiveCoefficientsBeforeNegativeWithMultipleUnits) {
    using NegFeet = decltype(Feet{} * (-mag<1>()));
    using NegMeters = decltype(Meters{} * (-mag<1>()));

    ASSERT_THAT((InOrderFor<UnitSumPack, Inches, Yards>::value), IsTrue());
    ASSERT_THAT((InOrderFor<UnitSumPack, Meters, Feet>::value), IsTrue());
    StaticAssertTypeEq<UnitSum<NegFeet, Inches, NegMeters, Yards>,
                       UnitSumPack<Inches, Yards, NegMeters, NegFeet>>();
}

TEST(UnitSum, OutputIsIndependentOfInputOrder) {
    StaticAssertTypeEq<UnitSum<Inches, Feet, Meters>, UnitSum<Meters, Inches, Feet>>();
    StaticAssertTypeEq<UnitSum<Feet, Meters, Inches>, UnitSum<Inches, Meters, Feet>>();
}

TEST(UnitSum, OutputIsIndependentOfInputOrderWithNegatives) {
    using NegFeet = decltype(Feet{} * (-mag<1>()));
    using NegInches = decltype(Inches{} * (-mag<1>()));
    using NegMeters = decltype(Meters{} * (-mag<1>()));

    StaticAssertTypeEq<UnitSum<NegInches, NegFeet, NegMeters>,
                       UnitSum<NegMeters, NegInches, NegFeet>>();
}

TEST(UnitSum, MixedSignsWithProperOrdering) {
    using NegFeet = decltype(Feet{} * (-mag<1>()));
    using NegMeters = decltype(Meters{} * (-mag<1>()));

    ASSERT_THAT((InOrderFor<UnitSumPack, Inches, Yards>::value), IsTrue());
    ASSERT_THAT((InOrderFor<UnitSumPack, Meters, Feet>::value), IsTrue());
    StaticAssertTypeEq<UnitSum<NegFeet, Inches, NegMeters, Yards>,
                       UnitSumPack<Inches, Yards, NegMeters, NegFeet>>();
}

TEST(UnitSum, FractionalCoefficientsWork) {
    using HalfFeet = decltype(Feet{} / mag<2>());
    using QuarterFeet = decltype(Feet{} / mag<4>());
    using ThreeQuartersFeet = decltype(Feet{} * mag<3>() / mag<4>());
    StaticAssertTypeEq<UnitSum<HalfFeet, QuarterFeet>, ThreeQuartersFeet>();
}

TEST(UnitSum, PartialCancellationLeavesRemainder) {
    using ThreeFeet = decltype(Feet{} * mag<3>());
    using NegTwoFeet = decltype(Feet{} * (-mag<2>()));
    StaticAssertTypeEq<UnitSum<ThreeFeet, NegTwoFeet>, Feet>();
}

TEST(UnitSum, UnwrapsUnitSumPackArguments) {
    using Sum1 = UnitSum<Feet, Inches>;
    using Sum2 = UnitSum<Meters, Yards>;
    StaticAssertTypeEq<UnitSum<Sum1, Sum2>, UnitSum<Feet, Inches, Meters, Yards>>();
}

TEST(UnitSum, UnwrapsAndCollectsLikeTerms) {
    using TwoFeet = decltype(Feet{} * mag<2>());
    using ThreeFeet = decltype(Feet{} * mag<3>());
    using Sum1 = UnitSum<TwoFeet, Inches>;
    StaticAssertTypeEq<UnitSum<Sum1, Feet>, UnitSum<ThreeFeet, Inches>>();
}

TEST(UnitSum, UnwrapsAndCancels) {
    using NegFeet = decltype(Feet{} * (-mag<1>()));
    using Sum1 = UnitSum<Feet, Inches>;
    StaticAssertTypeEq<UnitSum<Sum1, NegFeet>, Inches>();
}

TEST(UnitProduct, IdentityForSingleUnit) {
    StaticAssertTypeEq<UnitProduct<Feet>, Feet>();
    StaticAssertTypeEq<UnitProduct<Minutes>, Minutes>();
}

TEST(UnitProduct, ProductOfTwoUnitsIsUnitWithProductsOfDimAndMag) {
    using FootInches = UnitProduct<Feet, Inches>;
    EXPECT_THAT(IsUnit<FootInches>::value, IsTrue());
    StaticAssertTypeEq<DimT<FootInches>, DimProduct<DimT<Feet>, DimT<Inches>>>();
    StaticAssertTypeEq<MagT<FootInches>, MagProduct<MagT<Feet>, MagT<Inches>>>();
}

TEST(UnitProduct, AchievesExactCancellations) {
    using FootInchesPerFoot = UnitProduct<Feet, Inches, UnitInverse<Feet>>;
    StaticAssertTypeEq<FootInchesPerFoot, Inches>();
}

TEST(UnitProduct, CreatesPowForIntegerPowers) {
    using FeetSquared = UnitProduct<Feet, Feet>;
    StaticAssertTypeEq<FeetSquared, Pow<Feet, 2>>();

    StaticAssertTypeEq<UnitProduct<FeetSquared, UnitInverse<Feet>>, Feet>();
}

TEST(UnitProduct, CanonicalizesOrdering) {
    StaticAssertTypeEq<UnitProduct<Feet, Inches>, UnitProduct<Inches, Feet>>();
    StaticAssertTypeEq<UnitProduct<Feet, Minutes>, UnitProduct<Minutes, Feet>>();
}

TEST(UnitPower, ProducesSimplifiedPowersOfAllExponents) {
    using Input = UnitProduct<Feet, Pow<Minutes, 3>, Pow<Inches, -6>, RatioPow<Yards, 3, 2>>;

    using ExpectedCbrtInput =
        UnitProduct<RatioPow<Feet, 1, 3>, Minutes, Pow<Inches, -2>, RatioPow<Yards, 1, 2>>;

    StaticAssertTypeEq<UnitPower<Input, 1, 3>, ExpectedCbrtInput>();
}

TEST(UnitQuotient, InteractsAndCancelsAsExpected) {
    StaticAssertTypeEq<UnitProduct<UnitQuotient<Feet, Minutes>, Minutes>, Feet>();
}

TEST(UnitInverse, CreatesAppropriateUnitPower) {
    StaticAssertTypeEq<UnitInverse<Feet>, Pow<Feet, -1>>();
    StaticAssertTypeEq<UnitInverse<UnitInverse<Minutes>>, Minutes>();
}

TEST(AssociatedUnit, IsIdentityForUnits) { StaticAssertTypeEq<AssociatedUnit<Feet>, Feet>(); }

TEST(AssociatedUnit, FunctionalInterfaceHandlesInstancesCorrectly) {
    StaticAssertTypeEq<decltype(associated_unit(Feet{} / Minutes{})),
                       decltype(Feet{} / Minutes{})>();
}

TEST(AssociatedUnit, IsIdentityForTypeWithNoAssociatedUnit) {
    // We might have returned `void`, but this would require depending on `IsUnit`, which could slow
    // down `AssociatedUnit` because it's used so widely.  It's simpler to think of it as a trait
    // which "redirects" a type only when there is a definite, positive reason to do so.
    StaticAssertTypeEq<AssociatedUnit<double>, double>();
}

TEST(AssociatedUnit, HandlesWrappersWhichHaveSpecializedAssociatedUnit) {
    StaticAssertTypeEq<AssociatedUnit<SomeUnitWrapper<Feet>>, Feet>();
}

TEST(AssociatedUnit, SupportsSingularNameFor) {
    StaticAssertTypeEq<AssociatedUnit<SingularNameFor<Feet>>, Feet>();
}

TEST(AppropriateAssociatedUnit, GivesAssociatedUnitForQuantity) {
    StaticAssertTypeEq<AppropriateAssociatedUnit<Quantity, Feet>, Feet>();
    StaticAssertTypeEq<AppropriateAssociatedUnit<Quantity, QuantityMaker<Feet>>, Feet>();
}

TEST(AppropriateAssociatedUnit, GivesAssociatedUnitForPointsForQuantityPoint) {
    StaticAssertTypeEq<AppropriateAssociatedUnit<QuantityPoint, Kelvins>, Kelvins>();
    StaticAssertTypeEq<AppropriateAssociatedUnit<QuantityPoint, QuantityPointMaker<Kelvins>>,
                       Kelvins>();
}

TEST(UnitInverse, CommutesWithProduct) {
    StaticAssertTypeEq<UnitInverse<UnitProduct<Feet, Minutes>>,
                       UnitProduct<UnitInverse<Feet>, UnitInverse<Minutes>>>();
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

    EXPECT_THAT((IsDimensionless<UnitQuotient<Inches, Yards>>::value), IsTrue());

    EXPECT_THAT((IsDimensionless<UnitQuotient<AdHocSpeedUnit, UnitQuotient<Feet, Minutes>>>::value),
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
    StaticAssertTypeEq<UnitRatio<Yards, Inches>, decltype(mag<36>())>();
    StaticAssertTypeEq<UnitRatio<Inches, Inches>, decltype(mag<1>())>();
    StaticAssertTypeEq<UnitRatio<Inches, Yards>, decltype(mag<1>() / mag<36>())>();
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
    EXPECT_THAT((CommonUnit<Feet, Feet>{}), QuantityEquivalentToUnit(Feet{}));
    EXPECT_THAT((CommonUnit<Feet, Inches>{}), QuantityEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonUnit<Inches, Feet>{}), QuantityEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonUnit<Inches, Inches>{}), QuantityEquivalentToUnit(Inches{}));
}

TEST(CommonUnit, IndependentOfOrderingAndRepetitions) {
    using U = CommonUnit<Feet, Yards, Inches>;
    StaticAssertTypeEq<U, CommonUnit<Yards, Feet, Inches>>();
    StaticAssertTypeEq<U, CommonUnit<Inches, Yards, Feet>>();
    StaticAssertTypeEq<U, CommonUnit<Feet, Yards, Feet, Feet, Inches, Yards>>();
}

TEST(CommonUnit, PrefersUnitFromListIfAnyIdentical) {
    StaticAssertTypeEq<CommonUnit<Feet, Feet>, Feet>();
    StaticAssertTypeEq<CommonUnit<Feet, Inches, Yards>, Inches>();
}

TEST(CommonUnit, DedupesUnitsMadeIdenticalAfterUnscalingSameScaledUnit) {
    StaticAssertTypeEq<CommonUnit<decltype(Feet{} * mag<3>()), decltype(Feet{} * mag<5>())>,
                       Feet>();
}

TEST(CommonUnit, HandlesAndNeglectsZero) {
    StaticAssertTypeEq<CommonUnit<Yards, Zero, Feet>, Feet>();
    StaticAssertTypeEq<CommonUnit<Zero, Feet, Zero>, Feet>();
}

TEST(CommonUnit, ZeroIfAllInputsAreZero) {
    StaticAssertTypeEq<CommonUnit<>, Zero>();
    StaticAssertTypeEq<CommonUnit<Zero>, Zero>();
    StaticAssertTypeEq<CommonUnit<Zero, Zero>, Zero>();
    StaticAssertTypeEq<CommonUnit<Zero, Zero, Zero>, Zero>();
}

TEST(CommonUnit, DownranksAnonymousScaledUnits) {
    StaticAssertTypeEq<CommonUnit<Yards, decltype(Feet{} * mag<3>())>, Yards>();
}

TEST(CommonUnit, WhenCommonUnitLabelWouldBeIdenticalToSomeUnitJustUsesThatUnit) {
    StaticAssertTypeEq<CommonUnit<decltype(Feet{} * mag<6>()), decltype(Feet{} * mag<10>())>,
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

TEST(CommonUnitPack, UnpacksTypesInNestedCommonUnit) {
    using C1 = CommonUnit<W, X>;
    ASSERT_THAT((detail::IsPackOf<CommonUnitPack, C1>{}), IsTrue());

    using C2 = CommonUnit<Y, Z>;
    ASSERT_THAT((detail::IsPackOf<CommonUnitPack, C2>{}), IsTrue());

    using Common = CommonUnit<C1, C2>;
    ASSERT_THAT((detail::IsPackOf<CommonUnitPack, Common>{}), IsTrue());

    // Check that `c(c(w, x), c(y, z))` is the same as `c(w, x, y, z)`.
    StaticAssertTypeEq<Common, CommonUnit<W, X, Y, Z>>();
}

TEST(CommonUnit, CanCombineUnitsThatWouldBothBeAnonymousScaledUnits) {
    EXPECT_THAT((feet / mag<3>())(1), Eq((inches * mag<4>())(1)));
}

TEST(CommonUnit, SupportsUnitSlots) {
    StaticAssertTypeEq<decltype(common_unit(feet, meters)), CommonUnit<Feet, Meters>>();
}

TEST(CommonPointUnit, FindsCommonMagnitude) {
    EXPECT_THAT((CommonPointUnit<Feet, Feet>{}), PointEquivalentToUnit(Feet{}));
    EXPECT_THAT((CommonPointUnit<Feet, Inches>{}), PointEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonPointUnit<Inches, Feet>{}), PointEquivalentToUnit(Inches{}));
    EXPECT_THAT((CommonPointUnit<Inches, Inches>{}), PointEquivalentToUnit(Inches{}));
}

TEST(CommonPointUnit, HasMinimumOrigin) {
    EXPECT_THAT(origin_displacement(CommonPointUnit<Kelvins, Celsius>{}, Kelvins{}), Eq(ZERO));
    EXPECT_THAT(origin_displacement(CommonPointUnit<Fahrenheit, Celsius>{}, Fahrenheit{}),
                Eq(ZERO));
}

TEST(CommonPointUnit, TakesOriginMagnitudeIntoAccount) {
    using CommonByQuantity = CommonUnit<Kelvins, Celsius>;
    using CommonByPoint = CommonPointUnit<Kelvins, Celsius>;

    EXPECT_THAT(unit_ratio(CommonByQuantity{}, CommonByPoint{}), Eq(mag<20>()));

    constexpr auto expected = Kelvins{} / mag<20>();
    EXPECT_THAT(CommonByPoint{}, PointEquivalentToUnit(expected))
        << "Expected " << unit_label(expected) << ", got " << unit_label(CommonByPoint{});
}

TEST(CommonPointUnit, IndependentOfOrderingAndRepetitions) {
    using U = CommonPointUnit<Celsius, Kelvins, Fahrenheit>;
    StaticAssertTypeEq<U, CommonPointUnit<Kelvins, Celsius, Fahrenheit>>();
    StaticAssertTypeEq<U, CommonPointUnit<Fahrenheit, Kelvins, Celsius>>();
    StaticAssertTypeEq<U, CommonPointUnit<Kelvins, Celsius, Celsius, Fahrenheit, Kelvins>>();
}

TEST(CommonPointUnit, PrefersUnitFromListIfAnyIdentical) {
    StaticAssertTypeEq<CommonPointUnit<Celsius, Celsius>, Celsius>();
    StaticAssertTypeEq<CommonPointUnit<Celsius, Milli<Kelvins>, Milli<Celsius>>, Milli<Kelvins>>();
}

TEST(CommonPointUnitPack, UnpacksTypesInNestedCommonUnit) {
    using C1 = CommonPointUnit<W, X>;
    ASSERT_THAT((detail::IsPackOf<CommonPointUnitPack, C1>{}), IsTrue());

    using C2 = CommonPointUnit<Y, Z>;
    ASSERT_THAT((detail::IsPackOf<CommonPointUnitPack, C2>{}), IsTrue());

    using Common = CommonPointUnit<C1, C2>;
    ASSERT_THAT((detail::IsPackOf<CommonPointUnitPack, Common>{}), IsTrue());

    // Check that `c(c(w, x), c(y, z))` is the same as `c(w, x, y, z)`.
    StaticAssertTypeEq<Common, CommonPointUnit<W, X, Y, Z>>();
}

TEST(CommonPointUnit, SupportsUnitSlots) {
    StaticAssertTypeEq<decltype(common_point_unit(kelvins_pt, celsius_pt)),
                       CommonPointUnit<Kelvins, Celsius>>();
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

TEST(UnitSignTrait, OneForPositiveUnits) {
    StaticAssertTypeEq<UnitSign<Feet>, Magnitude<>>();
    StaticAssertTypeEq<UnitSign<Inches>, Magnitude<>>();
}

TEST(UnitSignTrait, MinusOneForNegativeUnits) {
    using NegFeet = decltype(Feet{} * (-mag<1>()));
    using NegInches = decltype(Inches{} * (-mag<1>()));
    StaticAssertTypeEq<UnitSign<NegFeet>, Magnitude<Negative>>();
    StaticAssertTypeEq<UnitSign<NegInches>, Magnitude<Negative>>();
}

TEST(UnitSign, OneForPositiveUnits) {
    EXPECT_THAT(unit_sign(feet), Eq(mag<1>()));
    EXPECT_THAT(unit_sign(inches), Eq(mag<1>()));
}

TEST(UnitSign, MinusOneForNegativeUnits) {
    EXPECT_THAT(unit_sign(feet * (-mag<5280>())), Eq(-mag<1>()));
    EXPECT_THAT(unit_sign(inches * (-mag<36>())), Eq(-mag<1>()));
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

TEST(UnitLabel, EmptyForNullProduct) {
    EXPECT_THAT(unit_label<UnitProductPack<>>(), StrEq(""));
    EXPECT_THAT(sizeof(unit_label<UnitProductPack<>>()), Eq(1u));
}

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
    EXPECT_THAT(unit_label(UnitInverse<decltype(Feet{} * Minutes{})>{}),
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
    using U = CommonUnit<Inches, Meters>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 127) in], [(1 / 5000) m]}"),
                      StrEq("EQUIV{[(1 / 5000) m], [(1 / 127) in]}")));
}

TEST(UnitLabel, CommonUnitLabelWorksWithUnitProductPack) {
    using U = CommonUnit<UnitQuotient<Meters, Minutes>, UnitQuotient<Inches, Minutes>>;
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
    using U = CommonPointUnit<Inches, Meters>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 127) in], [(1 / 5000) m]}"),
                      StrEq("EQUIV{[(1 / 5000) m], [(1 / 127) in]}")));
}

TEST(UnitLabel, CommonPointUnitLabelWorksWithUnitProductPack) {
    using U = CommonPointUnit<UnitQuotient<Meters, Minutes>, UnitQuotient<Inches, Minutes>>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 127) in / min], [(1 / 5000) m / min]}"),
                      StrEq("EQUIV{[(1 / 5000) m / min], [(1 / 127) in / min]}")));
}

TEST(UnitLabel, CommonPointUnitLabelTakesOriginOffsetIntoAccount) {
    using U = CommonPointUnit<Celsius, OffsetCelsius>;
    EXPECT_THAT(unit_label(U{}),
                AnyOf(StrEq("EQUIV{[(1 / 3) deg_C], [(1 / 5) (@(0 offset_deg_C) - @(0 deg_C))]}"),
                      StrEq("EQUIV{[(1 / 5) (@(0 offset_deg_C) - @(0 deg_C))], [(1 / 3) deg_C]}")));
}

TEST(UnitLabel, CommonUnitOfCommonPointUnitsPerformsFlattening) {
    using U = CommonUnit<CommonPointUnit<Celsius, Kelvins>, CommonPointUnit<Celsius, Fahrenheit>>;

    // When enumerating the possibilities, we know that anonymous ad hoc units (such as the unit for
    // the origin displacement from Kelvins to Celsius) will go last.  But we want to remain
    // agnostic about orderings between named units.  This also means we'll be agnostic as to which
    // of the (quantity-)equivalent units, Kelvins and Celsius, gets retained in the label, since
    // one of them will be eliminated for being redundant with the other.
    //
    // Finally, we know not to expect the origin displacement from Fahrenheit to Celsius to show up
    // in the label in any case, because it's already redundant with Fahrenheit (being [32 degF]).

    // If Kelvins gets retained and Celsius gets treated as redundant:
    const auto e1 =
        StrEq("EQUIV{[(1 / 180) K], [(1 / 100) degF], [(1 / 49167) (@(0 deg_C) - @(0 K))]}");
    const auto e2 =
        StrEq("EQUIV{[(1 / 100) degF], [(1 / 180) K], [(1 / 49167) (@(0 deg_C) - @(0 K))]}");

    // If Celsius gets retained and Kelvins gets treated as redundant:
    const auto e3 =
        StrEq("EQUIV{[(1 / 180) deg_C], [(1 / 100) degF], [(1 / 49167) (@(0 deg_C) - @(0 K))]}");
    const auto e4 =
        StrEq("EQUIV{[(1 / 100) degF], [(1 / 180) deg_C], [(1 / 49167) (@(0 deg_C) - @(0 K))]}");

    EXPECT_THAT(unit_label(U{}), AnyOf(e1, e2, e3, e4));
}

TEST(UnitLabel, APICompatibleWithUnitSlots) { EXPECT_THAT(unit_label(feet), StrEq("ft")); }

struct Trinches : decltype(Inches{} * mag<3>()) {};
struct Quarterfeet : decltype(Feet{} / mag<4>()) {};

template <>
struct UnitOrderTiebreaker<Trinches> : std::integral_constant<int, 1> {};

TEST(UnitOrderTiebreaker, CanBreakTiesForDistinctButOtherwiseUnorderableUnits) {
    // The point of this test is that this line would fail to compile if not for the
    // `UnitOrderTiebreaker<Trinches>` specialization just above, whose value must not be `0`.
    StaticAssertTypeEq<UnitProduct<Trinches, Quarterfeet>, UnitProduct<Quarterfeet, Trinches>>();
}

namespace detail {

struct Threet : decltype(Feet{} * mag<3>()) {};

template <>
struct UnitAvoidance<Threet> : std::integral_constant<int, 1234> {};

TEST(UnitAvoidance, CanTemporarilyBreakTiesForDistinctButOtherwiseUnorderableUnits) {
    // The point of this test is that this line would fail to compile if not for the
    // `UnitAvoidance<Threet>` specialization just above.
    //
    // This method of making distinct units orderable is deprecated, because it relies on end users
    // naming a type in our `detail::` namespace.
    StaticAssertTypeEq<UnitProduct<Yards, Threet>, UnitProduct<Threet, Yards>>();
}

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
