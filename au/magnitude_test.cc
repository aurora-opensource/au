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

using ::testing::DoubleEq;
using ::testing::Eq;
using ::testing::FloatEq;
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

TEST(IntegerPart, IdentityForIntegers) {
    EXPECT_EQ(integer_part(mag<1>()), mag<1>());
    EXPECT_EQ(integer_part(mag<2>()), mag<2>());
    EXPECT_EQ(integer_part(mag<2380>()), mag<2380>());
}

TEST(IntegerPart, PicksOutIntegersFromNumerator) {
    // sqrt(32) = 4 * sqrt(2)
    EXPECT_EQ(integer_part(PI * sqrt(mag<32>()) / mag<15>()), mag<4>());
}

TEST(Numerator, IsIdentityForInteger) {
    EXPECT_EQ(numerator(mag<2>()), mag<2>());
    EXPECT_EQ(numerator(mag<31415>()), mag<31415>());
}

TEST(Numerator, PutsFractionInLowestTerms) {
    EXPECT_EQ(numerator(mag<24>() / mag<16>()), mag<3>());
}

TEST(Numerator, IncludesNonIntegersWithPositiveExponent) {
    EXPECT_EQ(numerator(PI * sqrt(mag<24>() / mag<16>())), PI * sqrt(mag<3>()));
}

TEST(Denominator, PutsFractionInLowestTerms) {
    EXPECT_EQ(denominator(mag<24>() / mag<16>()), mag<2>());
}

TEST(Denominator, IncludesNonIntegersWithNegativeExponent) {
    EXPECT_EQ(denominator(sqrt(mag<24>() / mag<16>()) / PI), PI * sqrt(mag<2>()));
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

TEST(RepresentableIn, DocumentationExamplesAreCorrect) {
    EXPECT_TRUE(representable_in<int>(mag<1>()));

    // (1 / 2) is not an integer.
    EXPECT_FALSE(representable_in<int>(mag<1>() / mag<2>()));

    EXPECT_TRUE(representable_in<float>(mag<1>() / mag<2>()));

    EXPECT_TRUE(representable_in<uint32_t>(mag<4'000'000'000>()));

    // 4 billion is larger than the max value representable in `int32_t`.
    EXPECT_FALSE(representable_in<int32_t>(mag<4'000'000'000>()));
}

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

TEST(GetValue, HandlesRoots) {
    constexpr auto sqrt_2 = get_value<double>(root<2>(mag<2>()));
    EXPECT_DOUBLE_EQ(sqrt_2 * sqrt_2, 2.0);
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

MATCHER(CannotFit, "") {
    return (arg.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT) && (arg.value == 0);
}

MATCHER(NonIntegerInIntegerType, "") {
    return (arg.outcome == MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE) &&
           (arg.value == 0);
}

MATCHER(InvalidRoot, "") {
    return (arg.outcome == MagRepresentationOutcome::ERR_INVALID_ROOT) && (arg.value == 0);
}

template <typename T, typename ValueMatcher>
auto FitsAndMatchesValue(ValueMatcher &&matcher) {
    return ::testing::AllOf(
        ::testing::Field(&MagRepresentationOrError<T>::outcome,
                         ::testing::Eq(MagRepresentationOutcome::OK)),
        ::testing::Field(&MagRepresentationOrError<T>::value, std::forward<ValueMatcher>(matcher)));
}

template <typename T>
auto FitsAndProducesValue(T val) {
    return FitsAndMatchesValue<T>(SameTypeAndValue(val));
}

TEST(CheckedIntPow, FindsAppropriateLimits) {
    EXPECT_THAT(checked_int_pow(int16_t{2}, 14), FitsAndProducesValue(int16_t{16384}));
    EXPECT_THAT(checked_int_pow(int16_t{2}, 15), CannotFit());

    EXPECT_THAT(checked_int_pow(uint16_t{2}, 15), FitsAndProducesValue(uint16_t{32768}));
    EXPECT_THAT(checked_int_pow(uint16_t{2}, 16), CannotFit());

    EXPECT_THAT(checked_int_pow(uint64_t{2}, 63),
                FitsAndProducesValue(uint64_t{9'223'372'036'854'775'808u}));
    EXPECT_THAT(checked_int_pow(uint64_t{2}, 64), CannotFit());

    EXPECT_THAT(checked_int_pow(10.0, 308), FitsAndMatchesValue<double>(DoubleEq(1e308)));
    EXPECT_THAT(checked_int_pow(10.0, 309), CannotFit());
}

TEST(Root, ReturnsErrorForIntegralType) {
    EXPECT_THAT(root(4, 2), NonIntegerInIntegerType());
    EXPECT_THAT(root(uint8_t{125}, 3), NonIntegerInIntegerType());
}

TEST(Root, ReturnsErrorForZerothRoot) {
    EXPECT_THAT(root(4.0, 0), InvalidRoot());
    EXPECT_THAT(root(125.0, 0), InvalidRoot());
}

TEST(Root, NegativeRootsWorkForOddPowersOnly) {
    EXPECT_THAT(root(-4.0, 2), InvalidRoot());
    EXPECT_THAT(root(-125.0, 3), FitsAndProducesValue(-5.0));
    EXPECT_THAT(root(-10000.0, 4), InvalidRoot());
}

TEST(Root, AnyRootOfOneIsOne) {
    for (const std::uintmax_t r : {1, 2, 3, 4, 5, 6, 7, 8, 9}) {
        EXPECT_THAT(root(1.0, r), FitsAndProducesValue(1.0));
    }
}

TEST(Root, AnyRootOfZeroIsZero) {
    for (const std::uintmax_t r : {1, 2, 3, 4, 5, 6, 7, 8, 9}) {
        EXPECT_THAT(root(0.0, r), FitsAndProducesValue(0.0));
    }
}

TEST(Root, OddRootOfNegativeOneIsItself) {
    EXPECT_THAT(root(-1.0, 1), FitsAndProducesValue(-1.0));
    EXPECT_THAT(root(-1.0, 2), InvalidRoot());
    EXPECT_THAT(root(-1.0, 3), FitsAndProducesValue(-1.0));
    EXPECT_THAT(root(-1.0, 4), InvalidRoot());
    EXPECT_THAT(root(-1.0, 5), FitsAndProducesValue(-1.0));
}

TEST(Root, RecoversExactValueWherePossible) {
    {
        const auto sqrt_4f = root(4.0f, 2);
        EXPECT_THAT(sqrt_4f.outcome, Eq(MagRepresentationOutcome::OK));
        EXPECT_THAT(sqrt_4f.value, SameTypeAndValue(2.0f));
    }

    {
        const auto cbrt_125l = root(125.0l, 3);
        EXPECT_THAT(cbrt_125l.outcome, Eq(MagRepresentationOutcome::OK));
        EXPECT_THAT(cbrt_125l.value, SameTypeAndValue(5.0l));
    }
}

TEST(Root, HandlesArgumentsBetweenOneAndZero) {
    EXPECT_THAT(root(0.25, 2), FitsAndProducesValue(0.5));
    EXPECT_THAT(root(0.0001, 4), FitsAndMatchesValue<double>(DoubleEq(0.1)));
}

TEST(Root, ResultIsVeryCloseToStdPowForPureRoots) {
    for (const double x : {55.5, 123.456, 789.012, 3456.789, 12345.6789, 5.67e25}) {
        for (const int r : {2, 3, 4, 5, 6, 7, 8, 9}) {
            const auto double_result = root(x, r);
            EXPECT_THAT(double_result.outcome, Eq(MagRepresentationOutcome::OK));
            EXPECT_THAT(double_result.value, DoubleEq(std::pow(x, 1.0l / r)));

            const auto float_result = root(static_cast<float>(x), r);
            EXPECT_THAT(float_result.outcome, Eq(MagRepresentationOutcome::OK));
            EXPECT_THAT(float_result.value, FloatEq(std::pow(x, 1.0l / r)));
        }
    }
}

TEST(Root, ResultAtLeastAsGoodAsStdPowForRationalPowers) {
    struct RationalPower {
        std::uintmax_t num;
        std::uintmax_t den;
    };

    auto result_via_root = [](double x, RationalPower power) {
        return static_cast<double>(
            root(checked_int_pow(static_cast<long double>(x), power.num).value, power.den).value);
    };

    auto result_via_std_pow = [](double x, RationalPower power) {
        return static_cast<double>(
            std::pow(static_cast<long double>(x),
                     static_cast<long double>(power.num) / static_cast<long double>(power.den)));
    };

    auto round_trip_error = [](double x, RationalPower power, auto func) {
        const auto round_trip_result = func(func(x, power), {power.den, power.num});
        return std::abs(round_trip_result - x);
    };

    for (const auto base : {2.0, 3.1415, 98.6, 1.2e-10, 5.5e15}) {
        for (const auto power : std::vector<RationalPower>{{5, 2}, {2, 3}, {7, 4}}) {
            const auto error_from_root = round_trip_error(base, power, result_via_root);
            const auto error_from_std_pow = round_trip_error(base, power, result_via_std_pow);
            EXPECT_LE(error_from_root, error_from_std_pow);
        }
    }
}

TEST(GetValueResult, HandlesNumbersTooBigForUintmax) {
    EXPECT_THAT(get_value_result<std::uintmax_t>(pow<64>(mag<2>())), CannotFit());
}

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
