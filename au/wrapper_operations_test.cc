// Copyright 2023 Aurora Operations, Inc.
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

#include "au/wrapper_operations.hh"

#include "au/testing.hh"
#include "au/units/liters.hh"
#include "au/units/meters.hh"
#include "au/units/moles.hh"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {
namespace {

constexpr auto PI = Magnitude<Pi>{};

template <typename Unit>
struct UnitWrapper : MakesQuantityFromNumber<UnitWrapper, Unit>,
                     ScalesQuantity<UnitWrapper, Unit>,
                     ComposesWith<UnitWrapper, Unit, UnitWrapper, UnitWrapper>,
                     ComposesWith<UnitWrapper, Unit, QuantityMaker, QuantityMaker>,
                     SupportsRationalPowers<UnitWrapper, Unit>,
                     CanScaleByMagnitude<UnitWrapper, Unit> {};

TEST(MakesQuantityFromNumber, MakesQuantityWhenPostMultiplyingNumericValue) {
    constexpr auto mol = UnitWrapper<Moles>{};
    EXPECT_THAT(1.0f * mol, SameTypeAndValue(moles(1.0f)));
}

TEST(MakesQuantityFromNumber, MakesQuantityWhenPreMultiplyingNumericValue) {
    constexpr auto mol = UnitWrapper<Moles>{};
    EXPECT_THAT(mol * 35u, SameTypeAndValue(moles(35u)));
}

TEST(MakesQuantityFromNumber, MakesQuantityOfInverseUnitWhenDividingIntoNumericValue) {
    constexpr auto mol = UnitWrapper<Moles>{};
    EXPECT_THAT(10 / mol, SameTypeAndValue(inverse(moles)(10)));
}

TEST(MakesQuantityFromNumber, MakesQuantityWhenDividingNumericValue) {
    constexpr auto mol = UnitWrapper<Moles>{};
    EXPECT_THAT(mol / 4.0, SameTypeAndValue(moles(0.25)));

    // The following must not compile, because it would use integer division with an implicit
    // numerator of `1`, and would therefore almost always be zero.
    //
    // Uncomment to make sure the compilation fails.  (We set it up with incorrect values as a
    // failsafe, so that even if it does compile, the test will still fail.)
    //
    //    EXPECT_THAT((mol / 2), SameTypeAndValue(mol / 1));
}

TEST(ScalesQuantity, ChangesQuantityUnitsWhenPreMultiplying) {
    constexpr auto L = UnitWrapper<Liters>{};
    EXPECT_THAT(L * moles(5), SameTypeAndValue((liter * moles)(5)));
}

TEST(ScalesQuantity, ChangesQuantityUnitsWhenPostMultiplying) {
    constexpr auto L = UnitWrapper<Liters>{};
    EXPECT_THAT(moles(5) * L, SameTypeAndValue((mole * liters)(5)));
}

TEST(ScalesQuantity, ChangesQuantityUnitsWhenDividingInto) {
    constexpr auto L = UnitWrapper<Liters>{};
    EXPECT_THAT(moles(3u) / L, SameTypeAndValue((moles / liter)(3u)));
}

TEST(ScalesQuantity, ChangesUnitsAndInvertsQuantityWhenDividing) {
    constexpr auto mol = UnitWrapper<Moles>{};
    EXPECT_THAT(mol / liters(0.5), SameTypeAndValue((moles / liter)(2.0)));

    // The following must not compile, because it would use integer division with an implicit
    // numerator of `1`, and would therefore almost always be zero.
    //
    // Uncomment to make sure the compilation fails.  (We set it up with incorrect values as a
    // failsafe, so that even if it does compile, the test will still fail.)
    //
    //    EXPECT_THAT((mol / liters(2)), SameTypeAndValue(mol / liters(1)));
}

TEST(ComposesWith, ComposesWithSelf) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(mol * mol), UnitWrapper<UnitProduct<Moles, Moles>>>();
    StaticAssertTypeEq<decltype(mol / mol), UnitWrapper<UnitProduct<>>>();
}

TEST(ComposesWith, ComposesWithOtherSpecializationsOfSameWrapper) {
    constexpr auto mol = UnitWrapper<Moles>{};
    constexpr auto L = UnitWrapper<Liters>{};
    StaticAssertTypeEq<decltype(mol * L), UnitWrapper<UnitProduct<Moles, Liters>>>();
    StaticAssertTypeEq<decltype(mol / L), UnitWrapper<UnitQuotient<Moles, Liters>>>();
}

TEST(ComposesWith, MakesScaledQuantityMakerWhenPreMultiplyingQuantityMaker) {
    constexpr auto L = UnitWrapper<Liters>{};
    StaticAssertTypeEq<decltype(L * moles), QuantityMaker<UnitProduct<Moles, Liters>>>();
}

TEST(ComposesWith, MakesScaledQuantityMakerWhenPostMultiplyingQuantityMaker) {
    constexpr auto L = UnitWrapper<Liters>{};
    StaticAssertTypeEq<decltype(moles * L), QuantityMaker<UnitProduct<Moles, Liters>>>();
}

TEST(ComposesWith, MakesScaledQuantityMakerWhenDividingIntoQuantityMaker) {
    constexpr auto L = UnitWrapper<Liters>{};
    StaticAssertTypeEq<decltype(moles / L), QuantityMaker<UnitQuotient<Moles, Liters>>>();
}

TEST(ComposesWith, MakesScaledQuantityMakerWhenDividingQuantityMaker) {
    constexpr auto L = UnitWrapper<Liters>{};
    StaticAssertTypeEq<decltype(L / moles), QuantityMaker<UnitQuotient<Liters, Moles>>>();
}

TEST(CanScaleByMagnitude, MakesScaledWrapperWhenPreMultiplyingByMagnitude) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(mag<3>() * mol), UnitWrapper<decltype(Moles{} * mag<3>())>>();
}

TEST(CanScaleByMagnitude, MakesScaledWrapperWhenPostMultiplyingByMagnitude) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(mol * mag<3>()), UnitWrapper<decltype(Moles{} * mag<3>())>>();
}

TEST(CanScaleByMagnitude, CanPreMultiplyByZero) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(ZERO * mol), Zero>();
}

TEST(CanScaleByMagnitude, CanPostMultiplyByZero) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(mol * ZERO), Zero>();
}

TEST(CanScaleByMagnitude, MakesScaledWrapperOfInverseUnitWhenDividingIntoMagnitude) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(PI / mol), UnitWrapper<decltype(inverse(Moles{}) * PI)>>();
}

TEST(CanScaleByMagnitude, MakesScaledWrapperWhenDividingByMagnitude) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(mol / PI), UnitWrapper<decltype(Moles{} / PI)>>();
}

TEST(SupportsRationalPowers, RaisesUnitToGivenPower) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(pow<3>(mol)), UnitWrapper<decltype(pow<3>(Moles{}))>>();
}

TEST(SupportsRationalPowers, EnablesTakingRoots) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(root<8>(mol)), UnitWrapper<decltype(root<8>(Moles{}))>>();
}

TEST(SupportsRationalPowers, UnlocksNamedPowerHelpers) {
    constexpr auto mol = UnitWrapper<Moles>{};

    StaticAssertTypeEq<decltype(cubed(mol)), decltype(pow<3>(mol))>();
    StaticAssertTypeEq<decltype(squared(mol)), decltype(pow<2>(mol))>();
    StaticAssertTypeEq<decltype(sqrt(mol)), decltype(root<2>(mol))>();
}

TEST(CanScaleByMagnitude, SupportsNegation) {
    constexpr auto mol = UnitWrapper<Moles>{};
    StaticAssertTypeEq<decltype(-mol), UnitWrapper<decltype(Moles{} * (-mag<1>()))>>();
}

TEST(ForbidsComposingWith, FailsToCompileWhenMultiplyingOrDividingWithForbiddenWrapper) {
    // Uncomment each line below individually to verify.

    // UnitWrapper<Meters>{} * meters_pt;
    // UnitWrapper<Meters>{} / meters_pt;
    // meters_pt *UnitWrapper<Meters>{};
    // meters_pt / UnitWrapper<Meters>{};

    // UnitWrapper<Meters>{} * meters_pt(1);
    // UnitWrapper<Meters>{} / meters_pt(1);
    // meters_pt(1) * UnitWrapper<Meters>{};
    // meters_pt(1) / UnitWrapper<Meters>{};
}

}  // namespace
}  // namespace detail
}  // namespace au
