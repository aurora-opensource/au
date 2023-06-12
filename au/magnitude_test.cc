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

#include "au/magnitude.hh"

#include <cmath>

#include "au/testing.hh"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {
namespace {
template <typename T, typename = std::enable_if_t<std::is_arithmetic<T>::value>>
constexpr T cubed(T x) {
    return x * x * x;
}
}  // namespace

TEST(Magnitude, SupportsEqualityComparison) {
    constexpr auto mag_1 = mag<1>();
    EXPECT_EQ(mag_1, mag_1);

    constexpr auto mag_2 = mag<2>();
    EXPECT_EQ(mag_2, mag_2);

    EXPECT_NE(mag_1, mag_2);
}

TEST(Magnitude, ProductBehavesCorrectly) {
    EXPECT_EQ(mag<4>() * mag<6>(), mag<24>());
    EXPECT_EQ(mag<142857>() * mag<7>(), mag<999999>());
}

TEST(Magnitude, QuotientBehavesCorrectly) {
    EXPECT_EQ(mag<999999>() / mag<142857>(), mag<7>());
    EXPECT_EQ(mag<10>() / mag<6>(), mag<5>() / mag<3>());
}

TEST(Magnitude, PowersBehaveCorrectly) {
    EXPECT_EQ(pow<3>(mag<2>()), mag<8>());
    EXPECT_EQ(pow<-2>(mag<5>()), mag<1>() / mag<25>());
}

TEST(Magnitude, RootsBehaveCorrectly) { EXPECT_EQ(root<3>(mag<8>()), mag<2>()); }

TEST(Pi, HasCorrectValue) {
    // This pattern makes sure the test will fail if we _run_ on an architecture without `M_PIl`.
    // It does, however, permit us to _build_ on such an architecture with no problem.

#ifdef M_PIl
    EXPECT_EQ(Pi::value(), M_PIl);
#else
    EXPECT_TRUE(false) << "M_PIl not available on this architecture";
#endif
}

TEST(Inverse, RaisesToPowerNegativeOne) { EXPECT_EQ(inverse(mag<8>()), mag<1>() / mag<8>()); }

TEST(Squared, RaisesToPowerTwo) { EXPECT_EQ(squared(mag<7>()), mag<49>()); }

TEST(Cubed, RaisesToPowerThree) { EXPECT_EQ(cubed(mag<5>()), mag<125>()); }

TEST(Sqrt, TakesSecondRoot) { EXPECT_EQ(sqrt(mag<81>()), mag<9>()); }

TEST(Cbrt, TakesThirdRoot) { EXPECT_EQ(cbrt(mag<27>()), mag<3>()); }

TEST(Numerator, IsIdentityForInteger) {
    EXPECT_EQ(numerator(mag<2>()), mag<2>());
    EXPECT_EQ(numerator(mag<31415>()), mag<31415>());
}

TEST(Numerator, PutsFractionInLowestTerms) {
    EXPECT_EQ(numerator(mag<24>() / mag<16>()), mag<3>());
}

TEST(Denominator, PutsFractionInLowestTerms) {
    EXPECT_EQ(denominator(mag<24>() / mag<16>()), mag<2>());
}

TEST(IsRational, TrueForRatios) {
    EXPECT_TRUE(is_rational(mag<1>()));
    EXPECT_TRUE(is_rational(mag<9>()));
    EXPECT_TRUE(is_rational(mag<1>() / mag<10>()));
    EXPECT_TRUE(is_rational(mag<9>() / mag<10>()));
}

TEST(IsRational, FalseForInexactRoots) {
    EXPECT_TRUE(is_rational(root<2>(mag<9>())));
    EXPECT_FALSE(is_rational(root<3>(mag<9>())));
}

TEST(IsInteger, TrueForIntegers) {
    EXPECT_TRUE(is_integer(mag<1>()));
    EXPECT_TRUE(is_integer(mag<1234>()));
    EXPECT_TRUE(is_integer(mag<142857>()));
}

TEST(IsInteger, FalseForInexactFractions) {
    EXPECT_TRUE(is_integer(mag<6>() / mag<3>()));
    EXPECT_FALSE(is_integer(mag<7>() / mag<3>()));
    EXPECT_FALSE(is_integer(mag<8>() / mag<3>()));
    EXPECT_TRUE(is_integer(mag<9>() / mag<3>()));
}

TEST(IsInteger, FalseForIrrationalBase) { EXPECT_FALSE(is_integer(PI)); }

TEST(GetValue, SupportsIntegerOutputForIntegerMagnitude) {
    constexpr auto m = mag<412>();
    EXPECT_THAT(get_value<int>(m), SameTypeAndValue(412));
    EXPECT_THAT(get_value<std::size_t>(m), SameTypeAndValue(std::size_t{412}));
    EXPECT_THAT(get_value<float>(m), SameTypeAndValue(412.f));
    EXPECT_THAT(get_value<double>(m), SameTypeAndValue(412.));
}

TEST(GetValue, SupportsNegativePowersOfIntegerBase) {
    constexpr auto m = pow<-3>(mag<2>());
    EXPECT_THAT(get_value<float>(m), SameTypeAndValue(0.125f));
    EXPECT_THAT(get_value<double>(m), SameTypeAndValue(0.125));
}

TEST(GetValue, PiToThePower1HasCorrectValues) {
    EXPECT_THAT(get_value<float>(PI), SameTypeAndValue(static_cast<float>(M_PI)));
    EXPECT_THAT(get_value<double>(PI), SameTypeAndValue(M_PI));

#ifdef M_PIl
    EXPECT_THAT(get_value<long double>(PI), SameTypeAndValue(M_PIl));
#else
    EXPECT_TRUE(false) << "M_PIl not available on this architecture";
#endif
}

TEST(GetValue, PiToArbitraryPowerPerformsComputationsInMostAccurateTypeAtCompileTime) {
    constexpr auto pi_cubed = pow<3>(PI);

    constexpr auto result_via_float = cubed(get_value<float>(PI));
    constexpr auto result_via_long_double = static_cast<float>(cubed(get_value<long double>(PI)));

    constexpr auto pi_cubed_value = get_value<float>(pi_cubed);
    ASSERT_NE(pi_cubed_value, result_via_float);
    EXPECT_EQ(pi_cubed_value, result_via_long_double);
}

TEST(GetValue, ImpossibleRequestsArePreventedAtCompileTime) {
    // Naturally, we cannot actually write a test to verify a compiler error.  But any of these can
    // be uncommented if desired to verify that it breaks the build.

    // get_value<int8_t>(mag<412>());

    get_value<int64_t>(pow<62>(mag<2>()));  // Compiles, correctly.
    // get_value<int64_t>(pow<63>(mag<2>()));

    // TODO(chogg): Add checks for unsigned int64_t

    get_value<double>(pow<308>(mag<10>()));  // Compiles, correctly.
    // get_value<double>(pow<309>(mag<10>()));
    // get_value<double>(pow<3099>(mag<10>()));
    // get_value<double>(pow<3099999>(mag<10>()));

    constexpr auto sqrt_2 = root<2>(mag<2>());
    ASSERT_FALSE(is_integer(sqrt_2));
    // get_value<int>(sqrt_2);
}

TEST(GetValue, WorksForEmptyPack) {
    constexpr auto one = Magnitude<>{};
    EXPECT_THAT(get_value<int>(one), SameTypeAndValue(1));
    EXPECT_THAT(get_value<float>(one), SameTypeAndValue(1.f));
}

TEST(CommonMagnitude, ReturnsCommonMagnitudeWhenBothAreIdentical) {
    EXPECT_EQ(common_magnitude(mag<1>(), mag<1>()), mag<1>());
    EXPECT_EQ(common_magnitude(PI, PI), PI);

    constexpr auto x = pow<3>(PI) / root<2>(mag<2>()) * mag<412>();
    EXPECT_EQ(common_magnitude(x, x), x);
}

TEST(CommonMagnitude, ReturnsSmallerMagnitudeWhenItEvenlyDividesLarger) {
    EXPECT_EQ(common_magnitude(mag<1>(), mag<8>()), mag<1>());
    EXPECT_EQ(common_magnitude(mag<8>(), mag<1>()), mag<1>());

    constexpr auto one_eighth = mag<1>() / mag<8>();
    EXPECT_EQ(common_magnitude(mag<1>(), one_eighth), one_eighth);
    EXPECT_EQ(common_magnitude(one_eighth, mag<1>()), one_eighth);

    constexpr auto a = pow<3>(mag<2>()) * pow<-1>(mag<3>()) * pow<5>(mag<5>()) * pow<7>(mag<7>());
    constexpr auto b = /*              */ pow<-2>(mag<3>()) * /*                     */ mag<7>();
    EXPECT_EQ(common_magnitude(a, b), b);
    EXPECT_EQ(common_magnitude(b, a), b);
}

TEST(CommonMagnitude, DividesBothMagnitudes) {
    constexpr auto a = pow<10>(mag<2>()) * pow<-4>(mag<3>()) * pow<40>(mag<11>());
    constexpr auto b = pow<-1>(mag<2>()) * pow<12>(mag<3>()) * pow<-8>(mag<13>());

    ASSERT_FALSE(is_integer(a / b));
    ASSERT_FALSE(is_integer(b / a));

    EXPECT_EQ(common_magnitude(a, b), common_magnitude(b, a));
    EXPECT_TRUE(is_integer(a / common_magnitude(a, b)));
    EXPECT_TRUE(is_integer(b / common_magnitude(a, b)));
}

TEST(CommonMagnitude, HandlesMultiplePositivePowers) {
    EXPECT_EQ(common_magnitude(ONE, mag<1000>()), ONE);
}

TEST(CommonMagnitude, ZeroGetsIgnored) {
    EXPECT_EQ(common_magnitude(ZERO, mag<1000>()), mag<1000>());
    EXPECT_EQ(common_magnitude(PI, ZERO), PI);
}

TEST(CommonMagnitude, ZeroResultIndicatesAllInputsAreZero) {
    EXPECT_EQ(common_magnitude(ZERO), ZERO);
    EXPECT_EQ(common_magnitude(ZERO, ZERO), ZERO);
    EXPECT_EQ(common_magnitude(ZERO, ZERO, ZERO), ZERO);
    EXPECT_EQ(common_magnitude(ZERO, ZERO, ZERO, ZERO, ZERO), ZERO);
}

namespace detail {

TEST(PrimeFactorizationT, NullMagnitudeFor1) {
    StaticAssertTypeEq<PrimeFactorizationT<1u>, Magnitude<>>();
}

TEST(PrimeFactorizationT, FactorsInputs) {
    StaticAssertTypeEq<PrimeFactorizationT<2u>, Magnitude<Prime<2u>>>();
    StaticAssertTypeEq<PrimeFactorizationT<3u>, Magnitude<Prime<3u>>>();
    StaticAssertTypeEq<PrimeFactorizationT<4u>, Magnitude<Pow<Prime<2u>, 2u>>>();
    StaticAssertTypeEq<PrimeFactorizationT<5u>, Magnitude<Prime<5u>>>();
    StaticAssertTypeEq<PrimeFactorizationT<6u>, Magnitude<Prime<2u>, Prime<3u>>>();

    StaticAssertTypeEq<PrimeFactorizationT<12u>, Magnitude<Pow<Prime<2u>, 2u>, Prime<3u>>>();
}

}  // namespace detail
}  // namespace au
