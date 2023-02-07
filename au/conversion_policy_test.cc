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

#include "au/unit_of_measure.hh"
#include "gtest/gtest.h"

namespace au {

struct Grams : UnitImpl<Mass> {};
struct Kilograms : decltype(Grams{} * pow<3>(mag<10>())) {};
struct Gigagrams : decltype(Grams{} * pow<9>(mag<10>())) {};
struct Milligrams : decltype(Grams{} / pow<3>(mag<10>())) {};
struct Nanograms : decltype(Grams{} / pow<9>(mag<10>())) {};

struct Degrees : UnitImpl<Angle> {};
struct EquivalentToDegrees : Degrees {};

TEST(CanScaleWithoutOverflow, DetectsOverflowLimits) {
    EXPECT_TRUE(can_scale_without_overflow<double>(mag<1000>(), 1e100));
    EXPECT_FALSE(
        can_scale_without_overflow<double>(mag<1000>(), 0.5 * std::numeric_limits<double>::max()));
}

TEST(ImplicitRepPermitted, TrueForIdentityMagnitude) {
    EXPECT_TRUE((ImplicitRepPermitted<long double, Magnitude<>>::value));
    EXPECT_TRUE((ImplicitRepPermitted<double, Magnitude<>>::value));
    EXPECT_TRUE((ImplicitRepPermitted<float, Magnitude<>>::value));
    EXPECT_TRUE((ImplicitRepPermitted<int, Magnitude<>>::value));
    EXPECT_TRUE((ImplicitRepPermitted<uint8_t, Magnitude<>>::value));
}

TEST(ImplicitRepPermitted, TrueForFloatingPointTypesForVeryWideRanges) {
    EXPECT_TRUE((ImplicitRepPermitted<float, decltype(pow<10>(mag<10>()))>::value));
    EXPECT_TRUE((ImplicitRepPermitted<float, decltype(pow<-10>(mag<10>()))>::value));

    EXPECT_TRUE((ImplicitRepPermitted<double, decltype(pow<10>(mag<10>()))>::value));
    EXPECT_TRUE((ImplicitRepPermitted<double, decltype(pow<-10>(mag<10>()))>::value));

    EXPECT_TRUE((ImplicitRepPermitted<long double, decltype(pow<10>(mag<10>()))>::value));
    EXPECT_TRUE((ImplicitRepPermitted<long double, decltype(pow<-10>(mag<10>()))>::value));
}

TEST(ImplicitRepPermitted, TrueForIntegralTypesIfPurelyIntegerAndThresholdWouldNotOverflow) {
    // int16_t max value: roughly 32k.
    EXPECT_TRUE((ImplicitRepPermitted<int16_t, decltype(mag<7>())>::value));
    EXPECT_FALSE((ImplicitRepPermitted<int16_t, decltype(mag<8>())>::value));

    // uint16_t max value: roughly 65k.
    EXPECT_TRUE((ImplicitRepPermitted<uint16_t, decltype(mag<15>())>::value));
    EXPECT_FALSE((ImplicitRepPermitted<uint16_t, decltype(mag<16>())>::value));

    // int32_t max value: roughly 2.147e3 million.
    EXPECT_TRUE((ImplicitRepPermitted<int32_t, decltype(mag<500'000>())>::value));
    EXPECT_FALSE((ImplicitRepPermitted<int32_t, decltype(mag<600'000>())>::value));

    // uint32_t max value: roughly 4.294e3 million.
    EXPECT_TRUE((ImplicitRepPermitted<uint32_t, decltype(mag<1'000'000>())>::value));
    EXPECT_FALSE((ImplicitRepPermitted<uint32_t, decltype(mag<1'100'000>())>::value));
}

TEST(ImplicitRepPermitted, FalseForIntegralTypesUnlessRelativeScaleIsIntegral) {
    EXPECT_FALSE((ImplicitRepPermitted<int, decltype(mag<2>() / mag<3>())>::value));
    EXPECT_FALSE((ImplicitRepPermitted<int, decltype(mag<3>() / mag<2>())>::value));

    // Two rational numbers whose ratio is an integer.
    {
        constexpr auto a = (mag<15>() / mag<2>());
        constexpr auto b = (mag<5>() / mag<4>());
        EXPECT_TRUE((ImplicitRepPermitted<int, decltype(a / b)>::value));
    }

    // Two irrational numbers whose ratio is an integer.
    {
        constexpr auto a = mag<15>() * PI;
        constexpr auto b = mag<5>() * PI;
        EXPECT_TRUE((ImplicitRepPermitted<int, decltype(a / b)>::value));
    }

    // Two irrational numbers whose ratio is NOT an integer.
    {
        constexpr auto a = mag<15>() * pow<2>(PI);
        constexpr auto b = mag<5>() * PI;
        EXPECT_FALSE((ImplicitRepPermitted<int, decltype(a / b)>::value));
    }
}

TEST(ImplicitRepPermitted, FunctionalInterfaceWorksAsExpected) {
    EXPECT_TRUE(implicit_rep_permitted_from_source_to_target<int>(Kilograms{}, Grams{}));
    EXPECT_FALSE(implicit_rep_permitted_from_source_to_target<int>(Grams{}, Kilograms{}));
    EXPECT_TRUE(implicit_rep_permitted_from_source_to_target<float>(Grams{}, Kilograms{}));
}

TEST(ConstructionPolicy, PermitImplicitFromWideVarietyOfTypesForFloatingPointTargets) {
    using gigagrams_float_policy = ConstructionPolicy<Gigagrams, float>;
    EXPECT_TRUE((gigagrams_float_policy::PermitImplicitFrom<Grams, int>::value));
    EXPECT_TRUE((gigagrams_float_policy::PermitImplicitFrom<Nanograms, double>::value));

    using grams_double_policy = ConstructionPolicy<Grams, double>;
    EXPECT_TRUE((grams_double_policy::PermitImplicitFrom<Gigagrams, uint64_t>::value));
    EXPECT_TRUE((grams_double_policy::PermitImplicitFrom<Nanograms, int>::value));

    using long_double_policy = ConstructionPolicy<Nanograms, long double>;
    EXPECT_TRUE((long_double_policy::PermitImplicitFrom<Grams, uint64_t>::value));
    EXPECT_TRUE((long_double_policy::PermitImplicitFrom<Gigagrams, int>::value));
}

TEST(ConstructionPolicy, PermitsImplicitFromIntegralTypesIffTargetScaleDividesSourceScaleEvenly) {
    using grams_int_policy = ConstructionPolicy<Grams, int>;
    EXPECT_TRUE((grams_int_policy::PermitImplicitFrom<Kilograms, int>::value));
    EXPECT_FALSE((grams_int_policy::PermitImplicitFrom<Milligrams, int>::value));
}

TEST(ConstructionPolicy, ForbidsImplicitConstructionOfIntegralTypeFromFloatingPtType) {
    using grams_int_policy = ConstructionPolicy<Grams, int>;
    EXPECT_FALSE((grams_int_policy::PermitImplicitFrom<Grams, double>::value));
}

TEST(ConstructionPolicy, AlwaysOkForSameRepAndEquivalentUnit) {
    EXPECT_TRUE((ConstructionPolicy<EquivalentToDegrees,
                                    int8_t>::PermitImplicitFrom<Degrees, int8_t>::value));
}

TEST(ConstructionPolicy, OkForIntegralRepAndEquivalentUnit) {
    EXPECT_TRUE(
        (ConstructionPolicy<EquivalentToDegrees, int8_t>::PermitImplicitFrom<Degrees, int>::value));
}

}  // namespace au
