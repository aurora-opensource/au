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

#include "au/math.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/celsius.hh"
#include "au/units/degrees.hh"
#include "au/units/fahrenheit.hh"
#include "au/units/feet.hh"
#include "au/units/hertz.hh"
#include "au/units/inches.hh"
#include "au/units/kelvins.hh"
#include "au/units/meters.hh"
#include "au/units/ohms.hh"
#include "au/units/revolutions.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {
namespace {
constexpr auto INTEGER_TOO_BIG_FOR_DOUBLE = 9'007'199'254'740'993LL;

// Backport of std::clamp() from C++17 for testing purposes.
//
// See for implementation: https://en.cppreference.com/w/cpp/algorithm/clamp
template <class T, class Compare>
constexpr const T &std_clamp(const T &v, const T &lo, const T &hi, Compare comp) {
    return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}
template <class T>
constexpr const T &std_clamp(const T &v, const T &lo, const T &hi) {
    return std_clamp(v, lo, hi, std::less<void>{});
}
}  // namespace

TEST(abs, AlwaysReturnsNonnegativeVersionOfInput) {
    EXPECT_EQ(abs(meters(-1)), meters(1));
    EXPECT_EQ(abs(meters(0)), meters(0));
    EXPECT_EQ(abs(meters(1)), meters(1));

    EXPECT_EQ(abs(radians(-2.f)), radians(2.f));
    EXPECT_EQ(abs(radians(0.f)), radians(0.f));
    EXPECT_EQ(abs(radians(2.f)), radians(2.f));
}

TEST(abs, FollowsSamePolicyAsStdAbsForInf) {
    EXPECT_EQ(abs(degrees(INFINITY)), degrees(std::abs(INFINITY)));
    EXPECT_EQ(abs(degrees(-INFINITY)), degrees(std::abs(-INFINITY)));
}

TEST(abs, SameAsStdAbsForNumericTypes) {
    EXPECT_EQ(abs(-1), 1);
    EXPECT_EQ(abs(0), 0);
    EXPECT_EQ(abs(1), 1);
}

TEST(clamp, QuantityConsistentWithStdClampWhenTypesAreIdentical) {
    auto expect_consistent_with_std_clamp = [](auto v, auto lo, auto hi) {
        const auto expected = ohms(std_clamp(v, lo, hi));
        const auto actual = clamp(ohms(v), ohms(lo), ohms(hi));
        EXPECT_THAT(actual, SameTypeAndValue(expected));
    };

    // Rep: `int`.
    expect_consistent_with_std_clamp(-1, 0, 1);
    expect_consistent_with_std_clamp(0, 0, 1);
    expect_consistent_with_std_clamp(1, 0, 1);
    expect_consistent_with_std_clamp(2, 0, 1);

    // Rep: `double`.
    expect_consistent_with_std_clamp(-1.0, 0.0, 1.0);
    expect_consistent_with_std_clamp(0.0, 0.0, 1.0);
    expect_consistent_with_std_clamp(1.0, 0.0, 1.0);
    expect_consistent_with_std_clamp(2.0, 0.0, 1.0);
}

TEST(clamp, QuantityProducesResultsInCommonUnitOfInputs) {
    EXPECT_THAT(clamp(kilo(meters)(2), milli(meters)(999), meters(20)),
                SameTypeAndValue(milli(meters)(20'000)));

    EXPECT_THAT(clamp(kilo(meters)(2), meters(999), meters(2'999)),
                SameTypeAndValue(meters(2'000)));
}

TEST(clamp, QuantityPointConsistentWithStdClampWhenTypesAreIdentical) {
    auto expect_consistent_with_std_clamp = [](auto v, auto lo, auto hi) {
        const auto expected = meters_pt(std_clamp(v, lo, hi));
        const auto actual = clamp(meters_pt(v), meters_pt(lo), meters_pt(hi));
        EXPECT_THAT(actual, SameTypeAndValue(expected));
    };

    // Rep: `int`.
    expect_consistent_with_std_clamp(-1, 0, 1);
    expect_consistent_with_std_clamp(0, 0, 1);
    expect_consistent_with_std_clamp(1, 0, 1);
    expect_consistent_with_std_clamp(2, 0, 1);

    // Rep: `double`.
    expect_consistent_with_std_clamp(-1.0, 0.0, 1.0);
    expect_consistent_with_std_clamp(0.0, 0.0, 1.0);
    expect_consistent_with_std_clamp(1.0, 0.0, 1.0);
    expect_consistent_with_std_clamp(2.0, 0.0, 1.0);
}

TEST(clamp, QuantityPointProducesResultsInCommonUnitOfInputs) {
    EXPECT_THAT(clamp(kilo(meters_pt)(2), milli(meters_pt)(999), meters_pt(20)),
                SameTypeAndValue(milli(meters_pt)(20'000)));

    EXPECT_THAT(clamp(kilo(meters_pt)(2), meters_pt(999), meters_pt(2'999)),
                SameTypeAndValue(meters_pt(2'000)));
}

TEST(clamp, QuantityPointTakesOffsetIntoAccount) {
    // Recall that 0 degrees Celsius is 273.15 Kelvins.  We know that `clamp` must take the origin
    // into account for this mixed result.  This means whatever unit we return must be at most 1/20
    // Kelvins, and must evenly divide 1/20 Kelvins.
    constexpr auto celsius_origin = clamp(celsius_pt(0), kelvins_pt(200), kelvins_pt(300));
    ASSERT_TRUE(is_integer(unit_ratio(Kelvins{} / mag<20>(), decltype(celsius_origin)::unit)));
    EXPECT_EQ(celsius_origin, centi(kelvins_pt)(273'15));
}

TEST(clamp, SupportsZeroForLowerBoundaryArgument) {
    EXPECT_THAT(clamp(feet(-1), ZERO, inches(18)), SameTypeAndValue(inches(0)));
    EXPECT_THAT(clamp(feet(+1), ZERO, inches(18)), SameTypeAndValue(inches(12)));
    EXPECT_THAT(clamp(feet(+2), ZERO, inches(18)), SameTypeAndValue(inches(18)));
}

TEST(clamp, SupportsZeroForUpperBoundaryArgument) {
    EXPECT_THAT(clamp(feet(-2), inches(-18), ZERO), SameTypeAndValue(inches(-18)));
    EXPECT_THAT(clamp(feet(-1), inches(-18), ZERO), SameTypeAndValue(inches(-12)));
    EXPECT_THAT(clamp(feet(+1), inches(-18), ZERO), SameTypeAndValue(inches(0)));
}

TEST(copysign, ReturnsSameTypesAsStdCopysignForSameUnitInputs) {
    auto expect_consistent_with_std_copysign = [](auto mag, auto raw_sgn) {
        for (const auto test_sgn : {-1, 0, +1}) {
            const auto sgn = static_cast<decltype(raw_sgn)>(test_sgn) * raw_sgn;

            EXPECT_THAT(copysign(mag, sgn), SameTypeAndValue(std::copysign(mag, sgn)));

            EXPECT_THAT(copysign(meters(mag), sgn),
                        SameTypeAndValue(meters(std::copysign(mag, sgn))));

            EXPECT_THAT(copysign(mag, seconds(sgn)), SameTypeAndValue(std::copysign(mag, sgn)));

            EXPECT_THAT(copysign(meters(mag), seconds(sgn)),
                        SameTypeAndValue(meters(std::copysign(mag, sgn))));
        }
    };

    expect_consistent_with_std_copysign(4, 3);
    expect_consistent_with_std_copysign(4.f, 3.f);
    expect_consistent_with_std_copysign(4., 3.);
    expect_consistent_with_std_copysign(4.l, 3.l);
    expect_consistent_with_std_copysign(4, 3.f);
    expect_consistent_with_std_copysign(4, 3.l);
    expect_consistent_with_std_copysign(4., 3.l);
}

TEST(cos, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/cos
    StaticAssertTypeEq<decltype(cos(radians(0))), double>();
    StaticAssertTypeEq<decltype(cos(radians(0.))), double>();
    StaticAssertTypeEq<decltype(cos(radians(0.f))), float>();
    StaticAssertTypeEq<decltype(cos(radians(0.L))), long double>();

    // Make sure we support integral Degrees (related to Radians by an irrational scale factor).
    StaticAssertTypeEq<decltype(cos(degrees(0))), double>();

    // Make sure floating point Degrees retains the Rep.
    StaticAssertTypeEq<decltype(cos(degrees(0.f))), float>();
    StaticAssertTypeEq<decltype(cos(degrees(0.))), double>();
    StaticAssertTypeEq<decltype(cos(degrees(0.L))), long double>();
}

TEST(cos, SameAsStdCosForNumericTypes) {
    EXPECT_EQ(cos(1), std::cos(1));
    EXPECT_EQ(cos(1.), std::cos(1.));
    EXPECT_EQ(cos(1.f), std::cos(1.f));
    EXPECT_EQ(cos(1.L), std::cos(1.L));
}

TEST(cos, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_EQ(cos(radians(1.23)), std::cos(1.23));
    EXPECT_EQ(cos(radians(4.56f)), std::cos(4.56f));
}

TEST(cos, GivesCorrectAnswersForInputsInDegrees) {
    constexpr auto TOL = 1e-15;
    EXPECT_NEAR(cos(degrees(0)), 1.0, TOL);
    EXPECT_NEAR(cos(degrees(45)), std::sqrt(0.5), TOL);
    EXPECT_NEAR(cos(degrees(60)), 0.5, TOL);
    EXPECT_NEAR(cos(degrees(90)), 0.0, TOL);
}

// Our `fmod` and `remainder` overloads mix conversions and computations.
//
// If their inputs have the same unit, then there is no conversion, only computation.  In that case,
// we want to make sure we're doing exactly what their `std` counterparts do w.r.t. input and output
// types (e.g., for `fmod`, see: https://en.cppreference.com/w/cpp/numeric/math/fmod).
template <typename AuFunc, typename StdFunc>
struct ExpectConsistentWith {
    template <typename U, typename R1, typename R2>
    void operator()(Quantity<U, R1> q1, Quantity<U, R2> q2) const {
        EXPECT_THAT(au_func(q1, q2),
                    QuantityEquivalent(make_quantity<U>(std_func(q1.in(U{}), q2.in(U{})))));
    }

    AuFunc au_func;
    StdFunc std_func;
};
template <typename AuFunc, typename StdFunc>
auto expect_consistent_with(AuFunc au_func, StdFunc std_func) {
    return ExpectConsistentWith<AuFunc, StdFunc>{au_func, std_func};
}

TEST(fmod, SameAsStdFmodForNumericTypes) {
    const auto a = 3.5;
    const auto b = 3;

    EXPECT_EQ(fmod(a, b), std::fmod(a, b));
}

TEST(fmod, ReturnsSameTypesAsStdModForSameUnitInputs) {
    const auto expect_consistent_with_std_fmod = expect_consistent_with(
        [](auto x, auto y) { return fmod(x, y); }, [](auto x, auto y) { return std::fmod(x, y); });

    expect_consistent_with_std_fmod(meters(4), meters(3));
    expect_consistent_with_std_fmod(meters(4.f), meters(3.f));
    expect_consistent_with_std_fmod(meters(4.), meters(3.));
    expect_consistent_with_std_fmod(meters(4.l), meters(3.l));
    expect_consistent_with_std_fmod(meters(4), meters(3.f));
    expect_consistent_with_std_fmod(meters(4), meters(3.l));
    expect_consistent_with_std_fmod(meters(4.), meters(3.l));
}

TEST(fmod, MixedUnitsSupportedWithCasting) {
    constexpr auto a = meters(1);
    constexpr auto b = centi(meters)(11);
    constexpr auto expected_result = centi(meters)(1);

    EXPECT_THAT(fmod(a, b), IsNear(expected_result, make_quantity<Nano<Meters>>(1)));
}

TEST(fmod, HandlesIrrationalCommonUnit) {
    EXPECT_THAT(fmod(radians(1), degrees(57)), IsNear(degrees(0.2958), degrees(0.0001)));
}

TEST(remainder, SameAsStdRemainderForNumericTypes) {
    EXPECT_EQ(remainder(3.5, 3), std::remainder(3.5, 3));
    EXPECT_EQ(remainder(2.5, 3), std::remainder(2.5, 3));
}

TEST(remainder, ReturnsSameTypesAsStdRemainderForSameUnitInputs) {
    const auto expect_consistent_with_std_remainder =
        expect_consistent_with([](auto x, auto y) { return remainder(x, y); },
                               [](auto x, auto y) { return std::remainder(x, y); });

    expect_consistent_with_std_remainder(meters(4), meters(3));
    expect_consistent_with_std_remainder(meters(4.f), meters(3.f));
    expect_consistent_with_std_remainder(meters(4.), meters(3.));
    expect_consistent_with_std_remainder(meters(4.l), meters(3.l));
    expect_consistent_with_std_remainder(meters(4), meters(3.f));
    expect_consistent_with_std_remainder(meters(4), meters(3.l));
    expect_consistent_with_std_remainder(meters(4.), meters(3.l));
}

TEST(remainder, MixedUnitsSupportedWithCasting) {
    constexpr auto a = meters(1);
    constexpr auto b = centi(meters)(11);
    constexpr auto expected_result = centi(meters)(1);

    EXPECT_THAT(remainder(a, b), IsNear(expected_result, make_quantity<Nano<Meters>>(1)));
}

TEST(remainder, HandlesIrrationalCommonUnit) {
    EXPECT_THAT(remainder(radians(1), degrees(57)), IsNear(degrees(+0.2958), degrees(0.0001)));
    EXPECT_THAT(remainder(radians(1), degrees(58)), IsNear(degrees(-0.7042), degrees(0.0001)));
}

TEST(remainder, CenteredAroundZero) {
    EXPECT_THAT(remainder(degrees(90), revolutions(1)), IsNear(degrees(90), degrees(1e-9)));
    EXPECT_THAT(remainder(degrees(270), revolutions(1)), IsNear(degrees(-90), degrees(1e-9)));
}

TEST(max, ReturnsLarger) {
    constexpr auto result = max(centi(meters)(1), inches(1));
    EXPECT_EQ(result, inches(1));
}

TEST(max, HandlesDifferentOriginQuantityPoints) {
    constexpr auto result = max(fahrenheit_pt(30), celsius_pt(0));
    EXPECT_EQ(result, celsius_pt(0));
}

TEST(max, ReturnsByValueForSameExactQuantityType) {
    // If two Quantity types are EXACTLY the same, we risk ambiguity with `std::max`.
    const auto a = meters(1);
    const auto b = meters(2);
    const auto &max_a_b = max(a, b);

    EXPECT_EQ(max_a_b, b);
    EXPECT_NE(&max_a_b, &b);
}

TEST(max, SupportsConstexprForSameExactQuantityType) {
    constexpr auto result = max(meters(1), meters(2));
    EXPECT_EQ(result, meters(2));
}

TEST(max, ReturnsByValueForSameExactQuantityPointType) {
    // If two QuantityPoint types are EXACTLY the same, we risk ambiguity with `std::max`.
    const auto a = meters_pt(1);
    const auto b = meters_pt(2);
    const auto &max_a_b = max(a, b);

    EXPECT_EQ(max_a_b, b);
    EXPECT_NE(&max_a_b, &b);
}

TEST(max, SupportsConstexprForSameExactQuantityPointType) {
    constexpr auto result = max(meters_pt(1), meters_pt(2));
    EXPECT_EQ(result, meters_pt(2));
}

TEST(max, SameAsStdMaxForNumericTypes) {
    const auto a = 2;
    const auto b = 3;

    const auto &max_result = max(a, b);

    EXPECT_EQ(&b, &max_result);
}

TEST(max, SupportsZeroForFirstArgument) {
    constexpr auto positive_result = max(ZERO, meters(8));
    EXPECT_THAT(positive_result, SameTypeAndValue(meters(8)));

    constexpr auto negative_result = max(ZERO, meters(-8));
    EXPECT_THAT(negative_result, SameTypeAndValue(meters(0)));
}

TEST(max, SupportsZeroForSecondArgument) {
    constexpr auto positive_result = max(meters(8), ZERO);
    EXPECT_THAT(positive_result, SameTypeAndValue(meters(8)));

    constexpr auto negative_result = max(meters(-8), ZERO);
    EXPECT_THAT(negative_result, SameTypeAndValue(meters(0)));
}

TEST(min, ReturnsSmaller) {
    constexpr auto result = min(centi(meters)(1), inches(1));
    EXPECT_EQ(result, centi(meters)(1));
}

TEST(min, HandlesDifferentOriginQuantityPoints) {
    constexpr auto result = min(fahrenheit_pt(30), celsius_pt(0));
    EXPECT_EQ(result, fahrenheit_pt(30));
}

TEST(min, ReturnsByValueForSameExactQuantityType) {
    // If two Quantity types are EXACTLY the same, we risk ambiguity with `std::min`.
    const auto a = meters(1);
    const auto b = meters(2);
    const auto &min_a_b = min(a, b);

    EXPECT_EQ(min_a_b, a);
    EXPECT_NE(&min_a_b, &a);
}

TEST(min, SupportsConstexprForSameExactQuantityType) {
    constexpr auto result = min(meters(1), meters(2));
    EXPECT_EQ(result, meters(1));
}

TEST(min, ReturnsByValueForSameExactQuantityPointType) {
    // If two QuantityPoint types are EXACTLY the same, we risk ambiguity with `std::min`.
    const auto a = meters_pt(1);
    const auto b = meters_pt(2);
    const auto &min_a_b = min(a, b);

    EXPECT_EQ(min_a_b, a);
    EXPECT_NE(&min_a_b, &a);
}

TEST(min, SupportsConstexprForSameExactQuantityPointType) {
    constexpr auto result = min(meters_pt(1), meters_pt(2));
    EXPECT_EQ(result, meters_pt(1));
}

TEST(min, SameAsStdMinForNumericTypes) {
    const auto a = 2;
    const auto b = 3;

    const auto &min_result = min(a, b);

    EXPECT_EQ(&a, &min_result);
}

TEST(min, SupportsZeroForFirstArgument) {
    constexpr auto positive_result = min(ZERO, meters(8));
    EXPECT_THAT(positive_result, SameTypeAndValue(meters(0)));

    constexpr auto negative_result = min(ZERO, meters(-8));
    EXPECT_THAT(negative_result, SameTypeAndValue(meters(-8)));
}

TEST(min, SupportsZeroForSecondArgument) {
    constexpr auto positive_result = min(meters(8), ZERO);
    EXPECT_THAT(positive_result, SameTypeAndValue(meters(0)));

    constexpr auto negative_result = min(meters(-8), ZERO);
    EXPECT_THAT(negative_result, SameTypeAndValue(meters(-8)));
}

TEST(int_pow, OutputRepMatchesInputRep) {
    EXPECT_THAT(int_pow<-1>(meters(2.)), QuantityEquivalent(pow<-1>(meters)(0.5)));
    EXPECT_THAT(int_pow<2>(meters(2.)), QuantityEquivalent(squared(meters)(4.)));
    EXPECT_THAT(int_pow<2>(meters(2)), QuantityEquivalent(squared(meters)(4)));
    EXPECT_THAT(int_pow<5>(meters(2.)), QuantityEquivalent(pow<5>(meters)(32.)));
    EXPECT_THAT(int_pow<2>(meters(2.f)), QuantityEquivalent(squared(meters)(4.f)));
    EXPECT_THAT(int_pow<2>(meters(2.L)), QuantityEquivalent(squared(meters)(4.L)));
}

TEST(int_pow, MixedUnitsSupportedWithCasting) {
    constexpr auto cubic_inch = int_pow<3>(inches(1.0));
    constexpr auto expected_cm3 = cubed(centi(meters))(2.54 * 2.54 * 2.54);

    EXPECT_THAT(cubic_inch, IsNear(expected_cm3, nano(cubed(meters))(1.)));
}

TEST(sin, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/sin
    StaticAssertTypeEq<decltype(sin(radians(0))), double>();
    StaticAssertTypeEq<decltype(sin(radians(0.))), double>();
    StaticAssertTypeEq<decltype(sin(radians(0.f))), float>();
    StaticAssertTypeEq<decltype(sin(radians(0.L))), long double>();

    // Make sure we support integral Degrees (related to Radians by an irrational scale factor).
    StaticAssertTypeEq<decltype(sin(degrees(0))), double>();

    // Make sure floating point Degrees retains the Rep.
    StaticAssertTypeEq<decltype(sin(degrees(0.f))), float>();
    StaticAssertTypeEq<decltype(sin(degrees(0.))), double>();
    StaticAssertTypeEq<decltype(sin(degrees(0.L))), long double>();
}

TEST(sin, SameAsStdSinForNumericTypes) {
    EXPECT_EQ(sin(1), std::sin(1));
    EXPECT_EQ(sin(1.), std::sin(1.));
    EXPECT_EQ(sin(1.f), std::sin(1.f));
    EXPECT_EQ(sin(1.L), std::sin(1.L));
}

TEST(sin, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_EQ(sin(radians(1.23)), std::sin(1.23));
    EXPECT_EQ(sin(radians(4.56f)), std::sin(4.56f));
}

TEST(sin, GivesCorrectAnswersForInputsInDegrees) {
    constexpr auto TOL = 1e-15;
    EXPECT_NEAR(sin(degrees(0)), 0.0, TOL);
    EXPECT_NEAR(sin(degrees(30)), 0.5, TOL);
    EXPECT_NEAR(sin(degrees(45)), std::sqrt(0.5), TOL);
    EXPECT_NEAR(sin(degrees(90)), 1.0, TOL);
}

TEST(sqrt, OutputRepDependsOnInputRep) {
    EXPECT_THAT(sqrt(squared(meters)(4)), QuantityEquivalent(meters(2.)));
    EXPECT_THAT(sqrt(squared(meters)(4.)), QuantityEquivalent(meters(2.)));
    EXPECT_THAT(sqrt(squared(meters)(4.f)), QuantityEquivalent(meters(2.f)));
    EXPECT_THAT(sqrt(squared(meters)(4.L)), QuantityEquivalent(meters(2.L)));
}

TEST(sqrt, MixedUnitsSupportedWithCasting) {
    constexpr auto x_in = inches(1);
    constexpr auto y_cm = centi(meters)(2.54);

    EXPECT_THAT(sqrt(x_in * y_cm.as(inches)), IsNear(x_in, nano(meters)(1)));
}

TEST(sqrt, SameAsStdSqrtForNumericTypes) {
    EXPECT_EQ(sqrt(1), std::sqrt(1));
    EXPECT_EQ(sqrt(1.), std::sqrt(1.));
    EXPECT_EQ(sqrt(1.f), std::sqrt(1.f));
    EXPECT_EQ(sqrt(1.L), std::sqrt(1.L));
}

TEST(sqrt, CanConvertIfConversionFactorRational) {
    const auto geo_mean_length = sqrt(inches(1) * meters(1));

    // Using Quantity-equivalent Unit just retrieves the stored value.
    // This _always_ works.
    const auto retrieved_value = geo_mean_length.in(sqrt(inch * meters));
    EXPECT_THAT(retrieved_value, SameTypeAndValue(1.0));

    // This conversion is OK, because the conversion factor doesn't have any rational powers.
    const auto rationally_converted_value = geo_mean_length.in(sqrt(inch * centi(meters)));
    EXPECT_THAT(rationally_converted_value, SameTypeAndValue(10.0));

    // This test case won't work until we can compute radical conversion factors at compile time.
    // const auto radically_converted_value = geo_mean_length.in(inches);
    // EXPECT_NEAR(radically_converted_value, 6.274558, 0.000001);
}

TEST(tan, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/tan
    StaticAssertTypeEq<decltype(tan(radians(0))), double>();
    StaticAssertTypeEq<decltype(tan(radians(0.))), double>();
    StaticAssertTypeEq<decltype(tan(radians(0.f))), float>();
    StaticAssertTypeEq<decltype(tan(radians(0.L))), long double>();

    // Make sure we support integral Degrees (related to Radians by an irrational scale factor).
    StaticAssertTypeEq<decltype(tan(degrees(0))), double>();
}

TEST(tan, SameAsStdTanForNumericTypes) {
    EXPECT_EQ(tan(1), std::tan(1));
    EXPECT_EQ(tan(1.), std::tan(1.));
    EXPECT_EQ(tan(1.f), std::tan(1.f));
    EXPECT_EQ(tan(1.L), std::tan(1.L));
}

TEST(tan, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(tan(radians(1.23)), SameTypeAndValue(std::tan(1.23)));
    EXPECT_THAT(tan(radians(4.56f)), SameTypeAndValue(std::tan(4.56f)));
}

TEST(arccos, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/acos
    StaticAssertTypeEq<decltype(arccos(0)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arccos(0.)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arccos(0.f)), Quantity<Radians, float>>();
    StaticAssertTypeEq<decltype(arccos(0.L)), Quantity<Radians, long double>>();
}

TEST(arccos, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(arccos(0.123), SameTypeAndValue(radians(std::acos(0.123))));
    EXPECT_THAT(arccos(0.456f), SameTypeAndValue(radians(std::acos(0.456f))));
}

TEST(arcsin, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/asin
    StaticAssertTypeEq<decltype(arcsin(0)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arcsin(0.)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arcsin(0.f)), Quantity<Radians, float>>();
    StaticAssertTypeEq<decltype(arcsin(0.L)), Quantity<Radians, long double>>();
}

TEST(arcsin, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(arcsin(0.123), SameTypeAndValue(radians(std::asin(0.123))));
    EXPECT_THAT(arcsin(0.456f), SameTypeAndValue(radians(std::asin(0.456f))));
}

TEST(arcsin, ExampleFromReferenceDocs) {
    constexpr auto TOL = degrees(1e-12);
    EXPECT_THAT(arcsin(0.5).as(degrees), IsNear(degrees(30.0), TOL));
}

TEST(arctan, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/atan
    StaticAssertTypeEq<decltype(arctan(1)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan(1.)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan(1.f)), Quantity<Radians, float>>();
    StaticAssertTypeEq<decltype(arctan(1.L)), Quantity<Radians, long double>>();
}

TEST(arctan, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(arctan(3), SameTypeAndValue(radians(std::atan(3))));
    EXPECT_THAT(arctan(-5.f), SameTypeAndValue(radians(std::atan(-5.f))));
}

TEST(arctan2, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/atan2
    StaticAssertTypeEq<decltype(arctan2(1, 0)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan2(1., 0.)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan2(1.f, 0.f)), Quantity<Radians, float>>();
    StaticAssertTypeEq<decltype(arctan2(1.L, 0.L)), Quantity<Radians, long double>>();

    StaticAssertTypeEq<decltype(arctan2(1.f, 0)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan2(1, 0.f)), Quantity<Radians, double>>();

    StaticAssertTypeEq<decltype(arctan2(1.L, 0)), Quantity<Radians, long double>>();
    StaticAssertTypeEq<decltype(arctan2(1, 0.L)), Quantity<Radians, long double>>();
}

TEST(arctan2, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(arctan2(3, -5), SameTypeAndValue(radians(std::atan2(3, -5))));
    EXPECT_THAT(arctan2(3.f, -5.f), SameTypeAndValue(radians(std::atan2(3.f, -5.f))));
}

TEST(arctan2, QuantityOverloadTypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/atan2
    StaticAssertTypeEq<decltype(arctan2(meters(1), meters(0))), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan2(meters(1.), meters(0.))), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan2(meters(1.f), meters(0.f))), Quantity<Radians, float>>();
    StaticAssertTypeEq<decltype(arctan2(meters(1.L), meters(0.L))),
                       Quantity<Radians, long double>>();

    StaticAssertTypeEq<decltype(arctan2(meters(1.f), meters(0))), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan2(meters(1), meters(0.f))), Quantity<Radians, double>>();

    StaticAssertTypeEq<decltype(arctan2(meters(1.L), meters(0))), Quantity<Radians, long double>>();
    StaticAssertTypeEq<decltype(arctan2(meters(1), meters(0.L))), Quantity<Radians, long double>>();
}

TEST(arctan2, SupportsUnitsTypes) {
    // 100" == 254 cm.
    const auto angle = arctan2(make_quantity<Inches>(100), make_quantity<Centi<Meters>>(254));

    EXPECT_THAT(angle, IsNear(degrees(45), pico(degrees)(1)));
}

TEST(isnan, TransparentlyActsOnSameAsValue) {
    const std::vector<double> values{{
        0.,
        1.23,
        -4.5e6,
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::signaling_NaN(),
    }};

    for (const double x : values) {
        EXPECT_EQ(isnan(meters(x)), std::isnan(x));
        EXPECT_EQ(isnan(meters_pt(x)), std::isnan(x));
        EXPECT_EQ(isnan((radians / second)(x)), std::isnan(x));
    }
}

TEST(isnan, UnqualifiedCallsGiveStdVersions) {
    // This test exists to make sure we don't break code with unqualified isnan calls.
    const bool b = isnan(5.5);
    EXPECT_FALSE(b);
}

TEST(numeric_limits, MemberVariablesSetCorrectlyForQuantitySpecialization) {
    // To validly extend std::numeric_limits<T>, we must define all members declared static
    // constexpr in the primary template, in such a way that they are usable as integral constant
    // expressions.
    //
    // Source for rule: https://en.cppreference.com/w/cpp/language/extending_std
    // List of members: https://en.cppreference.com/w/cpp/types/numeric_limits
    using meters_limits_int = std::numeric_limits<Quantity<Meters, int>>;
    EXPECT_TRUE(meters_limits_int::is_specialized);
    EXPECT_TRUE(meters_limits_int::is_signed);
    EXPECT_TRUE(meters_limits_int::is_integer);
    EXPECT_TRUE(meters_limits_int::is_exact);
    EXPECT_FALSE(meters_limits_int::has_infinity);
    EXPECT_FALSE(meters_limits_int::has_quiet_NaN);
    EXPECT_FALSE(meters_limits_int::has_signaling_NaN);
    EXPECT_EQ(meters_limits_int::has_denorm, std::denorm_absent);
    EXPECT_FALSE(meters_limits_int::has_denorm_loss);
    EXPECT_EQ(meters_limits_int::round_style, std::round_toward_zero);
    EXPECT_FALSE(meters_limits_int::is_iec559);
    EXPECT_TRUE(meters_limits_int::is_bounded);
    EXPECT_EQ(meters_limits_int::is_modulo, std::numeric_limits<int>::is_modulo);
    EXPECT_EQ(meters_limits_int::digits, std::numeric_limits<int>::digits);
    EXPECT_EQ(meters_limits_int::digits10, std::numeric_limits<int>::digits10);
    EXPECT_EQ(meters_limits_int::max_digits10, 0);
    EXPECT_EQ(meters_limits_int::radix, 2);
    EXPECT_EQ(meters_limits_int::min_exponent, 0);
    EXPECT_EQ(meters_limits_int::min_exponent10, 0);
    EXPECT_EQ(meters_limits_int::max_exponent, 0);
    EXPECT_EQ(meters_limits_int::max_exponent10, 0);
    EXPECT_TRUE(meters_limits_int::traps);
    EXPECT_FALSE(meters_limits_int::tinyness_before);

    using radians_limits_uint32_t = std::numeric_limits<Quantity<Radians, uint32_t>>;
    EXPECT_TRUE(radians_limits_uint32_t::is_specialized);
    EXPECT_FALSE(radians_limits_uint32_t::is_signed);
    EXPECT_TRUE(radians_limits_uint32_t::is_integer);
    EXPECT_TRUE(radians_limits_uint32_t::is_exact);
    EXPECT_FALSE(radians_limits_uint32_t::has_infinity);
    EXPECT_FALSE(radians_limits_uint32_t::has_quiet_NaN);
    EXPECT_FALSE(radians_limits_uint32_t::has_signaling_NaN);
    EXPECT_EQ(radians_limits_uint32_t::has_denorm, std::denorm_absent);
    EXPECT_FALSE(radians_limits_uint32_t::has_denorm_loss);
    EXPECT_EQ(radians_limits_uint32_t::round_style, std::round_toward_zero);
    EXPECT_FALSE(radians_limits_uint32_t::is_iec559);
    EXPECT_TRUE(radians_limits_uint32_t::is_bounded);
    EXPECT_TRUE(radians_limits_uint32_t::is_modulo);
    EXPECT_EQ(radians_limits_uint32_t::digits, std::numeric_limits<uint32_t>::digits);
    EXPECT_EQ(radians_limits_uint32_t::digits10, std::numeric_limits<uint32_t>::digits10);
    EXPECT_EQ(radians_limits_uint32_t::max_digits10, 0);
    EXPECT_EQ(radians_limits_uint32_t::radix, 2);
    EXPECT_EQ(radians_limits_uint32_t::min_exponent, 0);
    EXPECT_EQ(radians_limits_uint32_t::min_exponent10, 0);
    EXPECT_EQ(radians_limits_uint32_t::max_exponent, 0);
    EXPECT_EQ(radians_limits_uint32_t::max_exponent10, 0);
    EXPECT_TRUE(radians_limits_uint32_t::traps);
    EXPECT_FALSE(radians_limits_uint32_t::tinyness_before);

    using celsius_limits_float = std::numeric_limits<Quantity<Celsius, float>>;
    EXPECT_TRUE(celsius_limits_float::is_specialized);
    EXPECT_TRUE(celsius_limits_float::is_signed);
    EXPECT_FALSE(celsius_limits_float::is_integer);
    EXPECT_FALSE(celsius_limits_float::is_exact);
    EXPECT_TRUE(celsius_limits_float::has_infinity);
    EXPECT_TRUE(celsius_limits_float::has_quiet_NaN);
    EXPECT_TRUE(celsius_limits_float::has_signaling_NaN);
    EXPECT_EQ(celsius_limits_float::has_denorm, std::denorm_present);
    EXPECT_EQ(celsius_limits_float::has_denorm_loss, std::numeric_limits<float>::has_denorm_loss);
    EXPECT_EQ(celsius_limits_float::round_style, std::round_to_nearest);
    EXPECT_TRUE(celsius_limits_float::is_iec559);
    EXPECT_TRUE(celsius_limits_float::is_bounded);
    EXPECT_FALSE(celsius_limits_float::is_modulo);
    EXPECT_EQ(celsius_limits_float::digits, FLT_MANT_DIG);
    EXPECT_EQ(celsius_limits_float::digits10, FLT_DIG);
    EXPECT_EQ(celsius_limits_float::max_digits10, std::numeric_limits<float>::max_digits10);
    EXPECT_EQ(celsius_limits_float::radix, FLT_RADIX);
    EXPECT_EQ(celsius_limits_float::min_exponent, FLT_MIN_EXP);
    EXPECT_EQ(celsius_limits_float::min_exponent10, FLT_MIN_10_EXP);
    EXPECT_EQ(celsius_limits_float::max_exponent, FLT_MAX_EXP);
    EXPECT_EQ(celsius_limits_float::max_exponent10, FLT_MAX_10_EXP);
    EXPECT_FALSE(celsius_limits_float::traps);
    EXPECT_EQ(celsius_limits_float::tinyness_before, std::numeric_limits<float>::tinyness_before);
}

TEST(numeric_limits, ProvidesLimitsForQuantity) {
    using nl1 = std::numeric_limits<Quantity<Meters, int>>;
    EXPECT_EQ(nl1::max(), meters(std::numeric_limits<int>::max()));
    EXPECT_EQ(nl1::lowest(), meters(std::numeric_limits<int>::lowest()));
    EXPECT_EQ(nl1::min(), meters(std::numeric_limits<int>::min()));
    EXPECT_EQ(nl1::epsilon(), meters(std::numeric_limits<int>::epsilon()));
    EXPECT_EQ(nl1::round_error(), meters(std::numeric_limits<int>::round_error()));
    EXPECT_EQ(nl1::infinity(), meters(std::numeric_limits<int>::infinity()));
    EXPECT_EQ(nl1::denorm_min(), meters(std::numeric_limits<int>::denorm_min()));

    using nl2 = std::numeric_limits<Quantity<Ohms, float>>;
    EXPECT_EQ(nl2::max(), ohms(std::numeric_limits<float>::max()));
    EXPECT_EQ(nl2::lowest(), ohms(std::numeric_limits<float>::lowest()));
    EXPECT_EQ(nl2::min(), ohms(std::numeric_limits<float>::min()));
    EXPECT_EQ(nl2::epsilon(), ohms(std::numeric_limits<float>::epsilon()));
    EXPECT_EQ(nl2::round_error(), ohms(std::numeric_limits<float>::round_error()));
    EXPECT_EQ(nl2::infinity(), ohms(std::numeric_limits<float>::infinity()));
    EXPECT_EQ(nl2::denorm_min(), ohms(std::numeric_limits<float>::denorm_min()));

    // We cannot currently test `quiet_NaN` or `signaling_NaN`.  Later, we could provide overloads
    // for `isnan()`, which people could find via ADL.
}

TEST(numeric_limits, InsensitiveToCvQualificationForQuantity) {
    using Q = Quantity<Degrees, float>;

    EXPECT_EQ(std::numeric_limits<Q>::max(), std::numeric_limits<const Q>::max());
    EXPECT_EQ(std::numeric_limits<Q>::max(), std::numeric_limits<volatile Q>::max());
    EXPECT_EQ(std::numeric_limits<Q>::max(), std::numeric_limits<const volatile Q>::max());

    EXPECT_EQ(std::numeric_limits<Q>::lowest(), std::numeric_limits<const Q>::lowest());
    EXPECT_EQ(std::numeric_limits<Q>::lowest(), std::numeric_limits<volatile Q>::lowest());
    EXPECT_EQ(std::numeric_limits<Q>::lowest(), std::numeric_limits<const volatile Q>::lowest());

    EXPECT_EQ(std::numeric_limits<Q>::min(), std::numeric_limits<const Q>::min());
    EXPECT_EQ(std::numeric_limits<Q>::min(), std::numeric_limits<volatile Q>::min());
    EXPECT_EQ(std::numeric_limits<Q>::min(), std::numeric_limits<const volatile Q>::min());

    EXPECT_EQ(std::numeric_limits<Q>::epsilon(), std::numeric_limits<const Q>::epsilon());
    EXPECT_EQ(std::numeric_limits<Q>::epsilon(), std::numeric_limits<volatile Q>::epsilon());
    EXPECT_EQ(std::numeric_limits<Q>::epsilon(), std::numeric_limits<const volatile Q>::epsilon());

    EXPECT_EQ(std::numeric_limits<Q>::round_error(), std::numeric_limits<const Q>::round_error());
    EXPECT_EQ(std::numeric_limits<Q>::round_error(),
              std::numeric_limits<volatile Q>::round_error());
    EXPECT_EQ(std::numeric_limits<Q>::round_error(),
              std::numeric_limits<const volatile Q>::round_error());

    EXPECT_EQ(std::numeric_limits<Q>::infinity(), std::numeric_limits<const Q>::infinity());
    EXPECT_EQ(std::numeric_limits<Q>::infinity(), std::numeric_limits<volatile Q>::infinity());
    EXPECT_EQ(std::numeric_limits<Q>::infinity(),
              std::numeric_limits<const volatile Q>::infinity());

    // It's hard to test `quiet_NaN` or `signaling_NaN`, because they have the property that x != x.

    EXPECT_EQ(std::numeric_limits<Q>::denorm_min(), std::numeric_limits<const Q>::denorm_min());
    EXPECT_EQ(std::numeric_limits<Q>::denorm_min(), std::numeric_limits<volatile Q>::denorm_min());
    EXPECT_EQ(std::numeric_limits<Q>::denorm_min(),
              std::numeric_limits<const volatile Q>::denorm_min());
}

TEST(RoundAs, SameAsStdRoundForSameUnits) {
    EXPECT_THAT(round_as(meters, meters(3)), SameTypeAndValue(meters(std::round(3))));
    EXPECT_THAT(round_as(meters, meters(3.14)), SameTypeAndValue(meters(std::round(3.14))));
    EXPECT_THAT(round_as(meters, meters(3.14f)), SameTypeAndValue(meters(std::round(3.14f))));
    EXPECT_THAT(round_as(meters, meters(3.14L)), SameTypeAndValue(meters(std::round(3.14L))));

    EXPECT_THAT(round_as(meters_pt, meters_pt(3)), SameTypeAndValue(meters_pt(std::round(3))));
    EXPECT_THAT(round_as(meters_pt, meters_pt(3.14)),
                SameTypeAndValue(meters_pt(std::round(3.14))));
    EXPECT_THAT(round_as(meters_pt, meters_pt(3.14f)),
                SameTypeAndValue(meters_pt(std::round(3.14f))));
    EXPECT_THAT(round_as(meters_pt, meters_pt(3.14L)),
                SameTypeAndValue(meters_pt(std::round(3.14L))));

    EXPECT_THAT(round_as(meters, meters(INTEGER_TOO_BIG_FOR_DOUBLE)),
                SameTypeAndValue(meters(std::round(INTEGER_TOO_BIG_FOR_DOUBLE))));

    EXPECT_THAT(round_as(meters_pt, meters_pt(INTEGER_TOO_BIG_FOR_DOUBLE)),
                SameTypeAndValue(meters_pt(std::round(INTEGER_TOO_BIG_FOR_DOUBLE))));
}

TEST(RoundAs, RoundsAsExpectedForDifferentUnits) {
    EXPECT_THAT(round_as(kilo(meters), meters(999)), SameTypeAndValue(kilo(meters)(1.0)));
    EXPECT_THAT(round_as(kilo(meters), meters(999.9)), SameTypeAndValue(kilo(meters)(1.0)));
    EXPECT_THAT(round_as(kilo(meters), meters(999.9f)), SameTypeAndValue(kilo(meters)(1.0f)));
    EXPECT_THAT(round_as(kilo(meters), meters(999.9L)), SameTypeAndValue(kilo(meters)(1.0L)));

    EXPECT_THAT(round_as(kilo(meters_pt), meters_pt(999)), SameTypeAndValue(kilo(meters_pt)(1.0)));
    EXPECT_THAT(round_as(kilo(meters_pt), meters_pt(999.9)),
                SameTypeAndValue(kilo(meters_pt)(1.0)));
    EXPECT_THAT(round_as(kilo(meters_pt), meters_pt(999.9f)),
                SameTypeAndValue(kilo(meters_pt)(1.0f)));
    EXPECT_THAT(round_as(kilo(meters_pt), meters_pt(999.9L)),
                SameTypeAndValue(kilo(meters_pt)(1.0L)));
}

TEST(RoundAs, SupportsDifferentOutputTypes) {
    EXPECT_THAT(round_as<int>(meters, meters(3)),
                SameTypeAndValue(meters(static_cast<int>(std::round(3)))));
    EXPECT_THAT(round_as<int>(meters, meters(3.9)),
                SameTypeAndValue(meters(static_cast<int>(std::round(3.9)))));

    EXPECT_THAT(round_as<double>(kilo(meters), meters(999.9f)),
                SameTypeAndValue(kilo(meters)(1.0)));

    EXPECT_THAT(round_as<int>(meters_pt, meters_pt(3)),
                SameTypeAndValue(meters_pt(static_cast<int>(std::round(3)))));
    EXPECT_THAT(round_as<int>(meters_pt, meters_pt(3.9)),
                SameTypeAndValue(meters_pt(static_cast<int>(std::round(3.9)))));

    EXPECT_THAT(round_as<double>(kilo(meters_pt), meters_pt(999.9f)),
                SameTypeAndValue(kilo(meters_pt)(1.0)));
}

TEST(RoundAs, SupportsQuantityPointWithNontrivialOffset) {
    EXPECT_THAT(round_as(kelvins_pt, celsius_pt(20.0f)), SameTypeAndValue(kelvins_pt(293.0f)));
    EXPECT_THAT(round_as(kelvins_pt, celsius_pt(20.5f)), SameTypeAndValue(kelvins_pt(294.0f)));

    // Each degree Fahrenheit is 5/9 of a degree Celsius.  Thus, moving away from an exact
    // correspondence by one degree Fahrenheit will be enough to move to the next integer Celsius
    // when we round, but moving by half a degree will not.
    EXPECT_THAT(round_as<int>(celsius_pt, fahrenheit_pt(31.0)), SameTypeAndValue(celsius_pt(-1)));
    EXPECT_THAT(round_as<int>(celsius_pt, fahrenheit_pt(31.5)), SameTypeAndValue(celsius_pt(0)));

    EXPECT_THAT(round_as<int>(celsius_pt, fahrenheit_pt(32.0)), SameTypeAndValue(celsius_pt(0)));

    EXPECT_THAT(round_as<int>(celsius_pt, fahrenheit_pt(32.5)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(round_as<int>(celsius_pt, fahrenheit_pt(33.0)), SameTypeAndValue(celsius_pt(1)));
}

TEST(RoundIn, SameAsRoundAs) {
    EXPECT_THAT(round_in(kilo(meters), meters(754)), SameTypeAndValue(1.0));
    EXPECT_THAT(round_in(kilo(meters), meters(754.28)), SameTypeAndValue(1.0));
    EXPECT_THAT(round_in(kilo(meters), meters(754.28f)), SameTypeAndValue(1.0f));
    EXPECT_THAT(round_in(kilo(meters), meters(754.28L)), SameTypeAndValue(1.0L));

    EXPECT_THAT(round_in(kilo(meters_pt), meters_pt(754)), SameTypeAndValue(1.0));
    EXPECT_THAT(round_in(kilo(meters_pt), meters_pt(754.28)), SameTypeAndValue(1.0));
    EXPECT_THAT(round_in(kilo(meters_pt), meters_pt(754.28f)), SameTypeAndValue(1.0f));
    EXPECT_THAT(round_in(kilo(meters_pt), meters_pt(754.28L)), SameTypeAndValue(1.0L));
}

TEST(RoundIn, SupportsDifferentOutputTypes) {
    EXPECT_THAT(round_in<long double>(kilo(meters), meters(754.28f)), SameTypeAndValue(1.0L));
    EXPECT_THAT(round_in<long double>(kilo(meters_pt), meters_pt(754.28f)), SameTypeAndValue(1.0L));
}

TEST(FloorAs, SameAsStdFloorForSameUnits) {
    EXPECT_THAT(floor_as(meters, meters(3)), SameTypeAndValue(meters(std::floor(3))));
    EXPECT_THAT(floor_as(meters, meters(3.14)), SameTypeAndValue(meters(std::floor(3.14))));
    EXPECT_THAT(floor_as(meters, meters(3.14f)), SameTypeAndValue(meters(std::floor(3.14f))));
    EXPECT_THAT(floor_as(meters, meters(3.14L)), SameTypeAndValue(meters(std::floor(3.14L))));

    EXPECT_THAT(floor_as(meters_pt, meters_pt(3)), SameTypeAndValue(meters_pt(std::floor(3))));
    EXPECT_THAT(floor_as(meters_pt, meters_pt(3.14)),
                SameTypeAndValue(meters_pt(std::floor(3.14))));
    EXPECT_THAT(floor_as(meters_pt, meters_pt(3.14f)),
                SameTypeAndValue(meters_pt(std::floor(3.14f))));
    EXPECT_THAT(floor_as(meters_pt, meters_pt(3.14L)),
                SameTypeAndValue(meters_pt(std::floor(3.14L))));

    EXPECT_THAT(floor_as(meters, meters(INTEGER_TOO_BIG_FOR_DOUBLE)),
                SameTypeAndValue(meters(std::floor(INTEGER_TOO_BIG_FOR_DOUBLE))));

    EXPECT_THAT(floor_as(meters_pt, meters_pt(INTEGER_TOO_BIG_FOR_DOUBLE)),
                SameTypeAndValue(meters_pt(std::floor(INTEGER_TOO_BIG_FOR_DOUBLE))));
}

TEST(FloorAs, RoundsDownAsExpectedForDifferentUnits) {
    EXPECT_THAT(floor_as(kilo(meters), meters(999)), SameTypeAndValue(kilo(meters)(0.0)));
    EXPECT_THAT(floor_as(kilo(meters), meters(999.9)), SameTypeAndValue(kilo(meters)(0.0)));
    EXPECT_THAT(floor_as(kilo(meters), meters(999.9f)), SameTypeAndValue(kilo(meters)(0.0f)));
    EXPECT_THAT(floor_as(kilo(meters), meters(999.9L)), SameTypeAndValue(kilo(meters)(0.0L)));

    EXPECT_THAT(floor_as(kilo(meters_pt), meters_pt(999)), SameTypeAndValue(kilo(meters_pt)(0.0)));
    EXPECT_THAT(floor_as(kilo(meters_pt), meters_pt(999.9)),
                SameTypeAndValue(kilo(meters_pt)(0.0)));
    EXPECT_THAT(floor_as(kilo(meters_pt), meters_pt(999.9f)),
                SameTypeAndValue(kilo(meters_pt)(0.0f)));
    EXPECT_THAT(floor_as(kilo(meters_pt), meters_pt(999.9L)),
                SameTypeAndValue(kilo(meters_pt)(0.0L)));

    EXPECT_THAT(floor_as(kilo(meters), meters(1001)), SameTypeAndValue(kilo(meters)(1.0)));
    EXPECT_THAT(floor_as(kilo(meters), meters(1000.1)), SameTypeAndValue(kilo(meters)(1.0)));
    EXPECT_THAT(floor_as(kilo(meters), meters(1000.1f)), SameTypeAndValue(kilo(meters)(1.0f)));
    EXPECT_THAT(floor_as(kilo(meters), meters(1000.1L)), SameTypeAndValue(kilo(meters)(1.0L)));

    EXPECT_THAT(floor_as(kilo(meters_pt), meters_pt(1001)), SameTypeAndValue(kilo(meters_pt)(1.0)));
    EXPECT_THAT(floor_as(kilo(meters_pt), meters_pt(1000.1)),
                SameTypeAndValue(kilo(meters_pt)(1.0)));
    EXPECT_THAT(floor_as(kilo(meters_pt), meters_pt(1000.1f)),
                SameTypeAndValue(kilo(meters_pt)(1.0f)));
    EXPECT_THAT(floor_as(kilo(meters_pt), meters_pt(1000.1L)),
                SameTypeAndValue(kilo(meters_pt)(1.0L)));
}

TEST(FloorAs, SupportsDifferentOutputTypes) {
    EXPECT_THAT(floor_as<int>(meters, meters(3)),
                SameTypeAndValue(meters(static_cast<int>(std::floor(3)))));
    EXPECT_THAT(floor_as<int>(meters, meters(3.9)),
                SameTypeAndValue(meters(static_cast<int>(std::floor(3.9)))));

    EXPECT_THAT(floor_as<int>(meters_pt, meters_pt(3)),
                SameTypeAndValue(meters_pt(static_cast<int>(std::floor(3)))));
    EXPECT_THAT(floor_as<int>(meters_pt, meters_pt(3.9)),
                SameTypeAndValue(meters_pt(static_cast<int>(std::floor(3.9)))));

    EXPECT_THAT(floor_as<double>(kilo(meters), meters(1000.1f)),
                SameTypeAndValue(kilo(meters)(1.0)));

    EXPECT_THAT(floor_as<double>(kilo(meters_pt), meters_pt(1000.1f)),
                SameTypeAndValue(kilo(meters_pt)(1.0)));
}

TEST(FloorAs, SupportsQuantityPointWithNontrivialOffset) {
    EXPECT_THAT(floor_as(kelvins_pt, celsius_pt(20.0f)), SameTypeAndValue(kelvins_pt(293.0f)));
    EXPECT_THAT(floor_as(kelvins_pt, celsius_pt(20.8f)), SameTypeAndValue(kelvins_pt(293.0f)));
    EXPECT_THAT(floor_as(kelvins_pt, celsius_pt(20.9f)), SameTypeAndValue(kelvins_pt(294.0f)));

    EXPECT_THAT(floor_as<int>(celsius_pt, fahrenheit_pt(31.0)), SameTypeAndValue(celsius_pt(-1)));
    EXPECT_THAT(floor_as<int>(celsius_pt, fahrenheit_pt(31.5)), SameTypeAndValue(celsius_pt(-1)));

    EXPECT_THAT(floor_as<int>(celsius_pt, fahrenheit_pt(32.0)), SameTypeAndValue(celsius_pt(0)));

    EXPECT_THAT(floor_as<int>(celsius_pt, fahrenheit_pt(32.5)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(floor_as<int>(celsius_pt, fahrenheit_pt(33.0)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(floor_as<int>(celsius_pt, fahrenheit_pt(33.5)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(floor_as<int>(celsius_pt, fahrenheit_pt(34.0)), SameTypeAndValue(celsius_pt(1)));
}

TEST(FloorIn, SameAsFloorAs) {
    EXPECT_THAT(floor_in(kilo(meters), meters(1154)), SameTypeAndValue(1.0));
    EXPECT_THAT(floor_in(kilo(meters), meters(1154.28)), SameTypeAndValue(1.0));
    EXPECT_THAT(floor_in(kilo(meters), meters(1154.28f)), SameTypeAndValue(1.0f));
    EXPECT_THAT(floor_in(kilo(meters), meters(1154.28L)), SameTypeAndValue(1.0L));

    EXPECT_THAT(floor_in(kilo(meters_pt), meters_pt(1154)), SameTypeAndValue(1.0));
    EXPECT_THAT(floor_in(kilo(meters_pt), meters_pt(1154.28)), SameTypeAndValue(1.0));
    EXPECT_THAT(floor_in(kilo(meters_pt), meters_pt(1154.28f)), SameTypeAndValue(1.0f));
    EXPECT_THAT(floor_in(kilo(meters_pt), meters_pt(1154.28L)), SameTypeAndValue(1.0L));
}

TEST(FloorIn, SupportsDifferentOutputTypes) {
    EXPECT_THAT(floor_in<long double>(kilo(meters), meters(1154.28f)), SameTypeAndValue(1.0L));
    EXPECT_THAT(floor_in<long double>(kilo(meters_pt), meters_pt(1154.28f)),
                SameTypeAndValue(1.0L));
}

TEST(CeilAs, SameAsStdCeilForSameUnits) {
    EXPECT_THAT(ceil_as(meters, meters(3)), SameTypeAndValue(meters(std::ceil(3))));
    EXPECT_THAT(ceil_as(meters, meters(3.14)), SameTypeAndValue(meters(std::ceil(3.14))));
    EXPECT_THAT(ceil_as(meters, meters(3.14f)), SameTypeAndValue(meters(std::ceil(3.14f))));
    EXPECT_THAT(ceil_as(meters, meters(3.14L)), SameTypeAndValue(meters(std::ceil(3.14L))));

    EXPECT_THAT(ceil_as(meters_pt, meters_pt(3)), SameTypeAndValue(meters_pt(std::ceil(3))));
    EXPECT_THAT(ceil_as(meters_pt, meters_pt(3.14)), SameTypeAndValue(meters_pt(std::ceil(3.14))));
    EXPECT_THAT(ceil_as(meters_pt, meters_pt(3.14f)),
                SameTypeAndValue(meters_pt(std::ceil(3.14f))));
    EXPECT_THAT(ceil_as(meters_pt, meters_pt(3.14L)),
                SameTypeAndValue(meters_pt(std::ceil(3.14L))));

    EXPECT_THAT(ceil_as(meters, meters(INTEGER_TOO_BIG_FOR_DOUBLE)),
                SameTypeAndValue(meters(std::ceil(INTEGER_TOO_BIG_FOR_DOUBLE))));

    EXPECT_THAT(ceil_as(meters_pt, meters_pt(INTEGER_TOO_BIG_FOR_DOUBLE)),
                SameTypeAndValue(meters_pt(std::ceil(INTEGER_TOO_BIG_FOR_DOUBLE))));
}

TEST(CeilAs, RoundsUpAsExpectedForDifferentUnits) {
    EXPECT_THAT(ceil_as(kilo(meters), meters(999)), SameTypeAndValue(kilo(meters)(1.0)));
    EXPECT_THAT(ceil_as(kilo(meters), meters(999.9)), SameTypeAndValue(kilo(meters)(1.0)));
    EXPECT_THAT(ceil_as(kilo(meters), meters(999.9f)), SameTypeAndValue(kilo(meters)(1.0f)));
    EXPECT_THAT(ceil_as(kilo(meters), meters(999.9L)), SameTypeAndValue(kilo(meters)(1.0L)));

    EXPECT_THAT(ceil_as(kilo(meters_pt), meters_pt(999)), SameTypeAndValue(kilo(meters_pt)(1.0)));
    EXPECT_THAT(ceil_as(kilo(meters_pt), meters_pt(999.9)), SameTypeAndValue(kilo(meters_pt)(1.0)));
    EXPECT_THAT(ceil_as(kilo(meters_pt), meters_pt(999.9f)),
                SameTypeAndValue(kilo(meters_pt)(1.0f)));
    EXPECT_THAT(ceil_as(kilo(meters_pt), meters_pt(999.9L)),
                SameTypeAndValue(kilo(meters_pt)(1.0L)));

    EXPECT_THAT(ceil_as(kilo(meters), meters(1001)), SameTypeAndValue(kilo(meters)(2.0)));
    EXPECT_THAT(ceil_as(kilo(meters), meters(1000.1)), SameTypeAndValue(kilo(meters)(2.0)));
    EXPECT_THAT(ceil_as(kilo(meters), meters(1000.1f)), SameTypeAndValue(kilo(meters)(2.0f)));
    EXPECT_THAT(ceil_as(kilo(meters), meters(1000.1L)), SameTypeAndValue(kilo(meters)(2.0L)));

    EXPECT_THAT(ceil_as(kilo(meters_pt), meters_pt(1001)), SameTypeAndValue(kilo(meters_pt)(2.0)));
    EXPECT_THAT(ceil_as(kilo(meters_pt), meters_pt(1000.1)),
                SameTypeAndValue(kilo(meters_pt)(2.0)));
    EXPECT_THAT(ceil_as(kilo(meters_pt), meters_pt(1000.1f)),
                SameTypeAndValue(kilo(meters_pt)(2.0f)));
    EXPECT_THAT(ceil_as(kilo(meters_pt), meters_pt(1000.1L)),
                SameTypeAndValue(kilo(meters_pt)(2.0L)));
}

TEST(CeilAs, SupportsDifferentOutputTypes) {
    EXPECT_THAT(ceil_as<int>(meters, meters(3)),
                SameTypeAndValue(meters(static_cast<int>(std::ceil(3)))));
    EXPECT_THAT(ceil_as<int>(meters, meters(3.9)),
                SameTypeAndValue(meters(static_cast<int>(std::ceil(3.9)))));

    EXPECT_THAT(ceil_as<int>(meters_pt, meters_pt(3)),
                SameTypeAndValue(meters_pt(static_cast<int>(std::ceil(3)))));
    EXPECT_THAT(ceil_as<int>(meters_pt, meters_pt(3.9)),
                SameTypeAndValue(meters_pt(static_cast<int>(std::ceil(3.9)))));

    EXPECT_THAT(ceil_as<double>(kilo(meters), meters(1000.1f)),
                SameTypeAndValue(kilo(meters)(2.0)));

    EXPECT_THAT(ceil_as<double>(kilo(meters_pt), meters_pt(1000.1f)),
                SameTypeAndValue(kilo(meters_pt)(2.0)));
}

TEST(CeilAs, SupportsQuantityPointWithNontrivialOffset) {
    EXPECT_THAT(ceil_as(kelvins_pt, celsius_pt(20.0f)), SameTypeAndValue(kelvins_pt(294.0f)));
    EXPECT_THAT(ceil_as(kelvins_pt, celsius_pt(20.8f)), SameTypeAndValue(kelvins_pt(294.0f)));
    EXPECT_THAT(ceil_as(kelvins_pt, celsius_pt(20.9f)), SameTypeAndValue(kelvins_pt(295.0f)));

    EXPECT_THAT(ceil_as<int>(celsius_pt, fahrenheit_pt(30.0)), SameTypeAndValue(celsius_pt(-1)));
    EXPECT_THAT(ceil_as<int>(celsius_pt, fahrenheit_pt(30.5)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(ceil_as<int>(celsius_pt, fahrenheit_pt(31.0)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(ceil_as<int>(celsius_pt, fahrenheit_pt(31.5)), SameTypeAndValue(celsius_pt(0)));

    EXPECT_THAT(ceil_as<int>(celsius_pt, fahrenheit_pt(32.0)), SameTypeAndValue(celsius_pt(0)));

    EXPECT_THAT(ceil_as<int>(celsius_pt, fahrenheit_pt(32.5)), SameTypeAndValue(celsius_pt(1)));
    EXPECT_THAT(ceil_as<int>(celsius_pt, fahrenheit_pt(33.0)), SameTypeAndValue(celsius_pt(1)));
}

TEST(CeilIn, SameAsCeilAs) {
    EXPECT_THAT(ceil_in(kilo(meters), meters(354)), SameTypeAndValue(1.0));
    EXPECT_THAT(ceil_in(kilo(meters), meters(354.28)), SameTypeAndValue(1.0));
    EXPECT_THAT(ceil_in(kilo(meters), meters(354.28f)), SameTypeAndValue(1.0f));
    EXPECT_THAT(ceil_in(kilo(meters), meters(354.28L)), SameTypeAndValue(1.0L));

    EXPECT_THAT(ceil_in(kilo(meters_pt), meters_pt(354)), SameTypeAndValue(1.0));
    EXPECT_THAT(ceil_in(kilo(meters_pt), meters_pt(354.28)), SameTypeAndValue(1.0));
    EXPECT_THAT(ceil_in(kilo(meters_pt), meters_pt(354.28f)), SameTypeAndValue(1.0f));
    EXPECT_THAT(ceil_in(kilo(meters_pt), meters_pt(354.28L)), SameTypeAndValue(1.0L));
}

TEST(CeilIn, SupportsDifferentOutputTypes) {
    EXPECT_THAT(ceil_in<long double>(kilo(meters), meters(354.28f)), SameTypeAndValue(1.0L));
    EXPECT_THAT(ceil_in<long double>(kilo(meters_pt), meters_pt(354.28f)), SameTypeAndValue(1.0L));
}

TEST(InverseAs, HandlesIntegerRepCorrectly) {
    constexpr auto period = inverse_as(micro(seconds), hertz(40));
    EXPECT_THAT(period, SameTypeAndValue(micro(seconds)(25'000)));
}

TEST(InverseAs, SupportsDividendLessThanOneThousandForFloatingPointRepOnly) {
    // Does not compile (integer rep):
    // inverse_as(seconds, hertz(4));

    // Compiles, but produces inaccurate truncation because forced by explicit-Rep:
    EXPECT_THAT(inverse_as<int>(seconds, hertz(4)), SameTypeAndValue(seconds(0)));

    // Compiles, and produces accurate result due to explicit floating point Rep:
    EXPECT_THAT(inverse_as<double>(seconds, hertz(4)), SameTypeAndValue(seconds(0.25)));

    // Compiles and works; no explicit Rep needed because the input is already floating point:
    EXPECT_THAT(inverse_as(seconds, hertz(4.0)), SameTypeAndValue(seconds(0.25)));
}

TEST(InverseIn, HasSameValueAsInverseAs) {
    EXPECT_THAT(inverse_in(micro(seconds), hertz(3)),
                SameTypeAndValue(inverse_as(micro(seconds), hertz(3)).in(micro(seconds))));

    EXPECT_THAT((inverse_in<double>(seconds, hertz(3))),
                SameTypeAndValue(inverse_as<double>(seconds, hertz(3)).in(seconds)));
}

TEST(InverseAs, ProducesCorrectRep) {
    EXPECT_THAT(inverse_as<int64_t>(nano(seconds), hertz(50.0)),
                SameTypeAndValue(rep_cast<int64_t>(nano(seconds)(20'000'000))));
}

TEST(InverseAs, HandlesConversionsBetweenOverflowSafetySurfaceAndRepresentableLimits) {
    EXPECT_THAT(inverse_as(nano(seconds), hertz(10)), SameTypeAndValue(nano(seconds)(100'000'000)));

    // Must not compile.  (Error should likely mention "Cannot represent constant in this unit/rep"
    // and/or "Value outside range of destination type".)  Uncomment to check:
    // inverse_as(pico(seconds), hertz(10))
}
}  // namespace au
