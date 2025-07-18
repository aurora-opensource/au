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

#include "au/conversion_policy.hh"

#include <complex>

#include "au/unit_of_measure.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::IsFalse;
using ::testing::IsTrue;

constexpr auto PI = Magnitude<Pi>{};

struct Grams : UnitImpl<Mass> {};
struct Kilograms : decltype(Grams{} * pow<3>(mag<10>())) {};
struct Gigagrams : decltype(Grams{} * pow<9>(mag<10>())) {};
struct Milligrams : decltype(Grams{} / pow<3>(mag<10>())) {};
struct Nanograms : decltype(Grams{} / pow<9>(mag<10>())) {};

struct Degrees : UnitImpl<Angle> {};
struct EquivalentToDegrees : Degrees {};
struct NegativeDegrees : decltype(Degrees{} * (-mag<1>())) {};

TEST(ImplicitRepPermitted, TrueForIdentityMagnitude) {
    EXPECT_THAT((ImplicitRepPermitted<long double, Magnitude<>>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<double, Magnitude<>>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<float, Magnitude<>>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<int, Magnitude<>>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<uint8_t, Magnitude<>>::value), IsTrue());
}

TEST(ImplicitRepPermitted, TrueForNegativeOneUnlessUnsigned) {
    EXPECT_THAT((ImplicitRepPermitted<int64_t, Magnitude<Negative>>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<int8_t, Magnitude<Negative>>::value), IsTrue());

    EXPECT_THAT((ImplicitRepPermitted<uint64_t, Magnitude<Negative>>::value), IsFalse());
    EXPECT_THAT((ImplicitRepPermitted<uint8_t, Magnitude<Negative>>::value), IsFalse());
}

TEST(ImplicitRepPermitted, TrueForFloatingPointTypesForVeryWideRanges) {
    EXPECT_THAT((ImplicitRepPermitted<float, decltype(pow<10>(mag<10>()))>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<float, decltype(pow<-10>(mag<10>()))>::value), IsTrue());

    EXPECT_THAT((ImplicitRepPermitted<double, decltype(pow<10>(mag<10>()))>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<double, decltype(pow<-10>(mag<10>()))>::value), IsTrue());

    EXPECT_THAT((ImplicitRepPermitted<long double, decltype(pow<10>(mag<10>()))>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<long double, decltype(pow<-10>(mag<10>()))>::value),
                IsTrue());
}

TEST(ImplicitRepPermitted, TrueForIntegralTypesIfPurelyIntegerAndThresholdWouldNotOverflow) {
    // int16_t max value: roughly 32k.
    EXPECT_THAT((ImplicitRepPermitted<int16_t, decltype(mag<15>())>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<int16_t, decltype(mag<16>())>::value), IsFalse());

    // uint16_t max value: roughly 65k.
    EXPECT_THAT((ImplicitRepPermitted<uint16_t, decltype(mag<30>())>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<uint16_t, decltype(mag<31>())>::value), IsFalse());

    // int32_t max value: roughly 2.147e3 million.
    EXPECT_THAT((ImplicitRepPermitted<int32_t, decltype(mag<1'000'000>())>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<int32_t, decltype(mag<1'100'000>())>::value), IsFalse());

    // uint32_t max value: roughly 4.294e3 million.
    EXPECT_THAT((ImplicitRepPermitted<uint32_t, decltype(mag<2'000'000>())>::value), IsTrue());
    EXPECT_THAT((ImplicitRepPermitted<uint32_t, decltype(mag<2'100'000>())>::value), IsFalse());
}

TEST(ImplicitRepPermitted, FalseForIntegralTypesUnlessRelativeScaleIsIntegral) {
    EXPECT_THAT((ImplicitRepPermitted<int, decltype(mag<2>() / mag<3>())>::value), IsFalse());
    EXPECT_THAT((ImplicitRepPermitted<int, decltype(mag<3>() / mag<2>())>::value), IsFalse());

    // Two rational numbers whose ratio is an integer.
    {
        constexpr auto a = (mag<15>() / mag<2>());
        constexpr auto b = (mag<5>() / mag<4>());
        EXPECT_THAT((ImplicitRepPermitted<int, decltype(a / b)>::value), IsTrue());
    }

    // Two irrational numbers whose ratio is an integer.
    {
        constexpr auto a = mag<15>() * PI;
        constexpr auto b = mag<5>() * PI;
        EXPECT_THAT((ImplicitRepPermitted<int, decltype(a / b)>::value), IsTrue());
    }

    // Two irrational numbers whose ratio is NOT an integer.
    {
        constexpr auto a = mag<15>() * pow<2>(PI);
        constexpr auto b = mag<5>() * PI;
        EXPECT_THAT((ImplicitRepPermitted<int, decltype(a / b)>::value), IsFalse());
    }
}

TEST(ImplicitRepPermitted, FunctionalInterfaceWorksAsExpected) {
    EXPECT_THAT(implicit_rep_permitted_from_source_to_target<int>(Kilograms{}, Grams{}), IsTrue());
    EXPECT_THAT(implicit_rep_permitted_from_source_to_target<int>(Grams{}, Kilograms{}), IsFalse());
    EXPECT_THAT(implicit_rep_permitted_from_source_to_target<float>(Grams{}, Kilograms{}),
                IsTrue());
}

TEST(ImplicitRepPermitted, HandlesComplexRep) {
    // These test cases are the same as the ones in `FunctionalInterfaceWorksAsExpected`, except
    // that we replace the target type `T` with `std::complex<T>`.
    EXPECT_THAT(
        implicit_rep_permitted_from_source_to_target<std::complex<int>>(Kilograms{}, Grams{}),
        IsTrue());
    EXPECT_THAT(
        implicit_rep_permitted_from_source_to_target<std::complex<int>>(Grams{}, Kilograms{}),
        IsFalse());
    EXPECT_THAT(
        implicit_rep_permitted_from_source_to_target<std::complex<float>>(Grams{}, Kilograms{}),
        IsTrue());
}

TEST(ConstructionPolicy, PermitImplicitFromWideVarietyOfTypesForFloatingPointTargets) {
    using gigagrams_float_policy = ConstructionPolicy<Gigagrams, float>;
    EXPECT_THAT((gigagrams_float_policy::PermitImplicitFrom<Grams, int>::value), IsTrue());
    EXPECT_THAT((gigagrams_float_policy::PermitImplicitFrom<Nanograms, double>::value), IsTrue());

    using grams_double_policy = ConstructionPolicy<Grams, double>;
    EXPECT_THAT((grams_double_policy::PermitImplicitFrom<Gigagrams, uint64_t>::value), IsTrue());
    EXPECT_THAT((grams_double_policy::PermitImplicitFrom<Nanograms, int>::value), IsTrue());

    using long_double_policy = ConstructionPolicy<Nanograms, long double>;
    EXPECT_THAT((long_double_policy::PermitImplicitFrom<Grams, uint64_t>::value), IsTrue());
    EXPECT_THAT((long_double_policy::PermitImplicitFrom<Gigagrams, int>::value), IsTrue());
}

TEST(ConstructionPolicy, PermitsImplicitFromIntegralTypesIffTargetScaleDividesSourceScaleEvenly) {
    using grams_int_policy = ConstructionPolicy<Grams, int>;
    EXPECT_THAT((grams_int_policy::PermitImplicitFrom<Kilograms, int>::value), IsTrue());
    EXPECT_THAT((grams_int_policy::PermitImplicitFrom<Milligrams, int>::value), IsFalse());
}

TEST(ConstructionPolicy, ComplexToRealPreventsImplicitConversion) {
    using gigagrams_float_policy = ConstructionPolicy<Gigagrams, float>;
    EXPECT_THAT((gigagrams_float_policy::PermitImplicitFrom<Grams, std::complex<float>>::value),
                IsFalse());
}

TEST(ConstructionPolicy, ForbidsImplicitConstructionOfIntegralTypeFromFloatingPtType) {
    using grams_int_policy = ConstructionPolicy<Grams, int>;
    EXPECT_THAT((grams_int_policy::PermitImplicitFrom<Grams, double>::value), IsFalse());
}

TEST(ConstructionPolicy, AlwaysOkForSameRepAndEquivalentUnit) {
    EXPECT_THAT((ConstructionPolicy<EquivalentToDegrees,
                                    int8_t>::PermitImplicitFrom<Degrees, int8_t>::value),
                IsTrue());
    EXPECT_THAT(
        (ConstructionPolicy<NegativeDegrees, int8_t>::PermitImplicitFrom<Degrees, int8_t>::value),
        IsTrue());
}

TEST(ConstructionPolicy, NotOkForNegativeRatioAndUnsignedDestination) {
    EXPECT_THAT((ConstructionPolicy<NegativeDegrees, uint16_t>::PermitImplicitFrom<Degrees,
                                                                                   int16_t>::value),
                IsFalse());
    EXPECT_THAT((ConstructionPolicy<NegativeDegrees,
                                    uint16_t>::PermitImplicitFrom<Degrees, uint16_t>::value),
                IsFalse());

    // Make sure it's explicitly the unsigned _destination_ that we were forbidding.
    ASSERT_THAT((ConstructionPolicy<NegativeDegrees, int16_t>::PermitImplicitFrom<Degrees,
                                                                                  uint16_t>::value),
                IsTrue());
}

TEST(ConstructionPolicy, OkForIntegralRepAndEquivalentUnit) {
    EXPECT_THAT(
        (ConstructionPolicy<EquivalentToDegrees, int8_t>::PermitImplicitFrom<Degrees, int>::value),
        IsTrue());
}

}  // namespace
}  // namespace au
