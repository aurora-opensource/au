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

#include <string>

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
#include "au/units/yards.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::AllOf;
using ::testing::DoubleNear;
using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Ne;
using ::testing::StaticAssertTypeEq;

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

// Constants for testing Constant support in rounding functions.
constexpr auto SEVEN_THIRDS_METERS = make_constant(meters * mag<7>() / mag<3>());
constexpr auto FIVE_HALVES_METERS = make_constant(meters * mag<5>() / mag<2>());

// Matcher for label strings.
MATCHER_P(LabelIs, expected, "") {
    *result_listener << "which is \"" << arg << "\"";
    return arg == expected;
}

// Type trait to check if a type is a Constant.
template <typename T>
struct IsConstantType : std::false_type {};
template <typename U>
struct IsConstantType<Constant<U>> : std::true_type {};

MATCHER(IsConstant, "is a Constant") {
    return IsConstantType<stdx::remove_cvref_t<decltype(arg)>>::value;
}

// Checks that arg is a Constant, and applies the inner matcher to its unit label.
template <typename InnerMatcher>
auto IsConstantWithUnitMatching(InnerMatcher m) {
    return ::testing::AllOf(IsConstant(),
                            ::testing::ResultOf(
                                "unit label",
                                [](const auto &c) {
                                    using Unit = AssociatedUnit<stdx::remove_cvref_t<decltype(c)>>;
                                    return std::string(unit_label<Unit>());
                                },
                                m));
}

}  // namespace

TEST(Abs, AlwaysReturnsNonnegativeVersionOfInput) {
    EXPECT_THAT(abs(meters(-1)), Eq(meters(1)));
    EXPECT_THAT(abs(meters(0)), Eq(meters(0)));
    EXPECT_THAT(abs(meters(1)), Eq(meters(1)));

    EXPECT_THAT(abs(radians(-2.f)), Eq(radians(2.f)));
    EXPECT_THAT(abs(radians(0.f)), Eq(radians(0.f)));
    EXPECT_THAT(abs(radians(2.f)), Eq(radians(2.f)));
}

TEST(Abs, FollowsSamePolicyAsStdAbsForInf) {
    EXPECT_THAT(abs(degrees(INFINITY)), Eq(degrees(std::abs(INFINITY))));
    EXPECT_THAT(abs(degrees(-INFINITY)), Eq(degrees(std::abs(-INFINITY))));
}

TEST(Abs, SameAsStdAbsForNumericTypes) {
    EXPECT_THAT(abs(-1), Eq(1));
    EXPECT_THAT(abs(0), Eq(0));
    EXPECT_THAT(abs(1), Eq(1));
}

TEST(Cbrt, OutputRepDependsOnInputRep) {
    EXPECT_THAT(cbrt(cubed(meters)(8)), QuantityEquivalent(meters(2.)));
    EXPECT_THAT(cbrt(cubed(meters)(8.)), QuantityEquivalent(meters(2.)));
    EXPECT_THAT(cbrt(cubed(meters)(8.f)), QuantityEquivalent(meters(2.f)));
    EXPECT_THAT(cbrt(cubed(meters)(8.L)), QuantityEquivalent(meters(2.L)));
}

TEST(Cbrt, SameAsStdCbrtForNumericTypes) {
    EXPECT_THAT(cbrt(1), Eq(std::cbrt(1)));
    EXPECT_THAT(cbrt(1.), Eq(std::cbrt(1.)));
    EXPECT_THAT(cbrt(1.f), Eq(std::cbrt(1.f)));
    EXPECT_THAT(cbrt(1.L), Eq(std::cbrt(1.L)));
}

TEST(Cbrt, CanConvertIfConversionFactorRational) {
    const auto geo_mean_length = cbrt(inches(1) * meters(1) * yards(1));

    // Using Quantity-equivalent Unit just retrieves the value stored in `geo_mean_length`.
    const auto retrieved_value = geo_mean_length.in(cbrt(inch * meter * yards));
    EXPECT_THAT(retrieved_value, SameTypeAndValue(1.0));

    // This conversion factor is another "easy" case because it doesn't have any rational powers.
    const auto rationally_converted_value = geo_mean_length.in(cbrt(inch * milli(meter) * yards));
    EXPECT_THAT(rationally_converted_value, SameTypeAndValue(10.0));

    // This test case is "hard": we need to compute radical conversion factors at compile time.
    const auto radically_converted_value = geo_mean_length.in(inches);
    EXPECT_THAT(radically_converted_value, DoubleNear(11.232841, 0.000001));
}

TEST(Clamp, QuantityConsistentWithStdClampWhenTypesAreIdentical) {
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

TEST(Clamp, QuantityProducesResultsInCommonUnitOfInputs) {
    EXPECT_THAT(clamp(kilo(meters)(2), milli(meters)(999), meters(20)),
                SameTypeAndValue(milli(meters)(20'000)));

    EXPECT_THAT(clamp(kilo(meters)(2), meters(999), meters(2'999)),
                SameTypeAndValue(meters(2'000)));
}

TEST(Clamp, QuantityPointConsistentWithStdClampWhenTypesAreIdentical) {
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

TEST(Clamp, QuantityPointProducesResultsInCommonUnitOfInputs) {
    EXPECT_THAT(clamp(kilo(meters_pt)(2), milli(meters_pt)(999), meters_pt(20)),
                SameTypeAndValue(milli(meters_pt)(20'000)));

    EXPECT_THAT(clamp(kilo(meters_pt)(2), meters_pt(999), meters_pt(2'999)),
                SameTypeAndValue(meters_pt(2'000)));
}

TEST(Clamp, QuantityPointTakesOffsetIntoAccount) {
    // Recall that 0 degrees Celsius is 273.15 Kelvins.  We know that `clamp` must take the origin
    // into account for this mixed result.  This means whatever unit we return must be at most 1/20
    // Kelvins, and must evenly divide 1/20 Kelvins.
    constexpr auto celsius_origin = clamp(celsius_pt(0), kelvins_pt(200), kelvins_pt(300));
    ASSERT_THAT(is_integer(unit_ratio(Kelvins{} / mag<20>(), decltype(celsius_origin)::unit)),
                IsTrue());
    EXPECT_THAT(celsius_origin, Eq(centi(kelvins_pt)(273'15)));
}

TEST(Clamp, SupportsZeroForLowerBoundaryArgument) {
    EXPECT_THAT(clamp(feet(-1), ZERO, inches(18)), SameTypeAndValue(inches(0)));
    EXPECT_THAT(clamp(feet(+1), ZERO, inches(18)), SameTypeAndValue(inches(12)));
    EXPECT_THAT(clamp(feet(+2), ZERO, inches(18)), SameTypeAndValue(inches(18)));
}

TEST(Clamp, SupportsZeroForUpperBoundaryArgument) {
    EXPECT_THAT(clamp(feet(-2), inches(-18), ZERO), SameTypeAndValue(inches(-18)));
    EXPECT_THAT(clamp(feet(-1), inches(-18), ZERO), SameTypeAndValue(inches(-12)));
    EXPECT_THAT(clamp(feet(+1), inches(-18), ZERO), SameTypeAndValue(inches(0)));
}

TEST(Clamp, SupportsZeroForValueArgument) {
    EXPECT_THAT(clamp(ZERO, inches(-18), inches(18)), SameTypeAndValue(inches(0)));
    EXPECT_THAT(clamp(ZERO, inches(24), inches(60)), SameTypeAndValue(inches(24)));
    EXPECT_THAT(clamp(ZERO, feet(2), inches(60)), SameTypeAndValue(inches(24)));
}

TEST(Clamp, SupportsZeroForMultipleArguments) {
    EXPECT_THAT(clamp(ZERO, inches(-8), ZERO), SameTypeAndValue(inches(0)));
    EXPECT_THAT(clamp(ZERO, ZERO, feet(2)), SameTypeAndValue(feet(0)));
    EXPECT_THAT(clamp(feet(6), ZERO, ZERO), SameTypeAndValue(feet(0)));
}

TEST(Hypot, QuantityConsistentWithStdHypotWhenTypesAreIdentical) {
    auto expect_consistent_with_std_hypot = [](auto u, auto v) {
        const auto expected = ohms(std::hypot(u, v));
        const auto actual = hypot(ohms(u), ohms(v));
        EXPECT_THAT(actual, SameTypeAndValue(expected));
    };

    // Rep: `int`.
    expect_consistent_with_std_hypot(-1, 0);
    expect_consistent_with_std_hypot(0, 0);
    expect_consistent_with_std_hypot(1, 0);
    expect_consistent_with_std_hypot(2, 0);
    expect_consistent_with_std_hypot(4, 2);

    // Rep: `double`.
    expect_consistent_with_std_hypot(-1.0, 0.0);
    expect_consistent_with_std_hypot(0.0, 0.0);
    expect_consistent_with_std_hypot(1.0, 0.0);
    expect_consistent_with_std_hypot(2.0, 0.0);
    expect_consistent_with_std_hypot(4.0, 2.0);
}

TEST(Hypot, QuantityProducesResultsInCommonUnitOfInputs) {
    EXPECT_THAT(hypot(centi(meters)(30), milli(meters)(400)),
                SameTypeAndValue(milli(meters)(500.0)));

    EXPECT_THAT(hypot(inches(5.f), feet(1.f)), SameTypeAndValue(inches(13.f)));
}

TEST(Copysign, ReturnsSameTypesAsStdCopysignForSameUnitInputs) {
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

TEST(Cos, TypeDependsOnInputType) {
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

TEST(Cos, SameAsStdCosForNumericTypes) {
    EXPECT_THAT(cos(1), Eq(std::cos(1)));
    EXPECT_THAT(cos(1.), Eq(std::cos(1.)));
    EXPECT_THAT(cos(1.f), Eq(std::cos(1.f)));
    EXPECT_THAT(cos(1.L), Eq(std::cos(1.L)));
}

TEST(Cos, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(cos(radians(1.23)), Eq(std::cos(1.23)));
    EXPECT_THAT(cos(radians(4.56f)), Eq(std::cos(4.56f)));
}

TEST(Cos, GivesCorrectAnswersForInputsInDegrees) {
    constexpr auto TOL = 1e-15;
    EXPECT_THAT(cos(degrees(0)), DoubleNear(1.0, TOL));
    EXPECT_THAT(cos(degrees(45)), DoubleNear(std::sqrt(0.5), TOL));
    EXPECT_THAT(cos(degrees(60)), DoubleNear(0.5, TOL));
    EXPECT_THAT(cos(degrees(90)), DoubleNear(0.0, TOL));
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

TEST(Fmod, SameAsStdFmodForNumericTypes) {
    const auto a = 3.5;
    const auto b = 3;

    EXPECT_THAT(fmod(a, b), Eq(std::fmod(a, b)));
}

TEST(Fmod, ReturnsSameTypesAsStdModForSameUnitInputs) {
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

TEST(Fmod, MixedUnitsSupportedWithCasting) {
    constexpr auto a = meters(1);
    constexpr auto b = centi(meters)(11);
    constexpr auto expected_result = centi(meters)(1);

    EXPECT_THAT(fmod(a, b), IsNear(expected_result, make_quantity<Nano<Meters>>(1)));
}

TEST(Fmod, HandlesIrrationalCommonUnit) {
    EXPECT_THAT(fmod(radians(1), degrees(57)), IsNear(degrees(0.2958), degrees(0.0001)));
}

TEST(Remainder, SameAsStdRemainderForNumericTypes) {
    EXPECT_THAT(remainder(3.5, 3), Eq(std::remainder(3.5, 3)));
    EXPECT_THAT(remainder(2.5, 3), Eq(std::remainder(2.5, 3)));
}

TEST(Remainder, ReturnsSameTypesAsStdRemainderForSameUnitInputs) {
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

TEST(Remainder, MixedUnitsSupportedWithCasting) {
    constexpr auto a = meters(1);
    constexpr auto b = centi(meters)(11);
    constexpr auto expected_result = centi(meters)(1);

    EXPECT_THAT(remainder(a, b), IsNear(expected_result, make_quantity<Nano<Meters>>(1)));
}

TEST(Remainder, HandlesIrrationalCommonUnit) {
    EXPECT_THAT(remainder(radians(1), degrees(57)), IsNear(degrees(+0.2958), degrees(0.0001)));
    EXPECT_THAT(remainder(radians(1), degrees(58)), IsNear(degrees(-0.7042), degrees(0.0001)));
}

TEST(Remainder, CenteredAroundZero) {
    EXPECT_THAT(remainder(degrees(90), revolutions(1)), IsNear(degrees(90), degrees(1e-9)));
    EXPECT_THAT(remainder(degrees(270), revolutions(1)), IsNear(degrees(-90), degrees(1e-9)));
}

TEST(Max, ReturnsLarger) {
    constexpr auto result = max(centi(meters)(1), inches(1));
    EXPECT_THAT(result, Eq(inches(1)));
}

TEST(Max, HandlesDifferentOriginQuantityPoints) {
    constexpr auto result = max(fahrenheit_pt(30), celsius_pt(0));
    EXPECT_THAT(result, Eq(celsius_pt(0)));
}

TEST(Max, ReturnsByValueForSameExactQuantityType) {
    // If two Quantity types are EXACTLY the same, we risk ambiguity with `std::max`.
    const auto a = meters(1);
    const auto b = meters(2);
    const auto &max_a_b = max(a, b);

    EXPECT_THAT(max_a_b, Eq(b));
    EXPECT_THAT(&max_a_b, Ne(&b));
}

TEST(Max, SupportsConstexprForSameExactQuantityType) {
    constexpr auto result = max(meters(1), meters(2));
    EXPECT_THAT(result, Eq(meters(2)));
}

TEST(Max, ReturnsByValueForSameExactQuantityPointType) {
    // If two QuantityPoint types are EXACTLY the same, we risk ambiguity with `std::max`.
    const auto a = meters_pt(1);
    const auto b = meters_pt(2);
    const auto &max_a_b = max(a, b);

    EXPECT_THAT(max_a_b, Eq(b));
    EXPECT_THAT(&max_a_b, Ne(&b));
}

TEST(Max, SupportsConstexprForSameExactQuantityPointType) {
    constexpr auto result = max(meters_pt(1), meters_pt(2));
    EXPECT_THAT(result, Eq(meters_pt(2)));
}

TEST(Max, SameAsStdMaxForNumericTypes) {
    const auto a = 2;
    const auto b = 3;

    const auto &max_result = max(a, b);

    EXPECT_THAT(&b, Eq(&max_result));
}

TEST(Max, SupportsZeroForFirstArgument) {
    constexpr auto positive_result = max(ZERO, meters(8));
    EXPECT_THAT(positive_result, SameTypeAndValue(meters(8)));

    constexpr auto negative_result = max(ZERO, meters(-8));
    EXPECT_THAT(negative_result, SameTypeAndValue(meters(0)));
}

TEST(Max, SupportsZeroForSecondArgument) {
    constexpr auto positive_result = max(meters(8), ZERO);
    EXPECT_THAT(positive_result, SameTypeAndValue(meters(8)));

    constexpr auto negative_result = max(meters(-8), ZERO);
    EXPECT_THAT(negative_result, SameTypeAndValue(meters(0)));
}

TEST(Min, ReturnsSmaller) {
    constexpr auto result = min(centi(meters)(1), inches(1));
    EXPECT_THAT(result, Eq(centi(meters)(1)));
}

TEST(Min, HandlesDifferentOriginQuantityPoints) {
    constexpr auto result = min(fahrenheit_pt(30), celsius_pt(0));
    EXPECT_THAT(result, Eq(fahrenheit_pt(30)));
}

TEST(Min, ReturnsByValueForSameExactQuantityType) {
    // If two Quantity types are EXACTLY the same, we risk ambiguity with `std::min`.
    const auto a = meters(1);
    const auto b = meters(2);
    const auto &min_a_b = min(a, b);

    EXPECT_THAT(min_a_b, Eq(a));
    EXPECT_THAT(&min_a_b, Ne(&a));
}

TEST(Min, SupportsConstexprForSameExactQuantityType) {
    constexpr auto result = min(meters(1), meters(2));
    EXPECT_THAT(result, Eq(meters(1)));
}

TEST(Min, ReturnsByValueForSameExactQuantityPointType) {
    // If two QuantityPoint types are EXACTLY the same, we risk ambiguity with `std::min`.
    const auto a = meters_pt(1);
    const auto b = meters_pt(2);
    const auto &min_a_b = min(a, b);

    EXPECT_THAT(min_a_b, Eq(a));
    EXPECT_THAT(&min_a_b, Ne(&a));
}

TEST(Min, SupportsConstexprForSameExactQuantityPointType) {
    constexpr auto result = min(meters_pt(1), meters_pt(2));
    EXPECT_THAT(result, Eq(meters_pt(1)));
}

TEST(Min, SameAsStdMinForNumericTypes) {
    const auto a = 2;
    const auto b = 3;

    const auto &min_result = min(a, b);

    EXPECT_THAT(&a, Eq(&min_result));
}

TEST(Min, SupportsZeroForFirstArgument) {
    constexpr auto positive_result = min(ZERO, meters(8));
    EXPECT_THAT(positive_result, SameTypeAndValue(meters(0)));

    constexpr auto negative_result = min(ZERO, meters(-8));
    EXPECT_THAT(negative_result, SameTypeAndValue(meters(-8)));
}

TEST(Min, SupportsZeroForSecondArgument) {
    constexpr auto positive_result = min(meters(8), ZERO);
    EXPECT_THAT(positive_result, SameTypeAndValue(meters(0)));

    constexpr auto negative_result = min(meters(-8), ZERO);
    EXPECT_THAT(negative_result, SameTypeAndValue(meters(-8)));
}

TEST(IntPow, OutputRepMatchesInputRep) {
    EXPECT_THAT(int_pow<-1>(meters(2.)), QuantityEquivalent(pow<-1>(meters)(0.5)));
    EXPECT_THAT(int_pow<2>(meters(2.)), QuantityEquivalent(squared(meters)(4.)));
    EXPECT_THAT(int_pow<2>(meters(2)), QuantityEquivalent(squared(meters)(4)));
    EXPECT_THAT(int_pow<5>(meters(2.)), QuantityEquivalent(pow<5>(meters)(32.)));
    EXPECT_THAT(int_pow<2>(meters(2.f)), QuantityEquivalent(squared(meters)(4.f)));
    EXPECT_THAT(int_pow<2>(meters(2.L)), QuantityEquivalent(squared(meters)(4.L)));
}

TEST(IntPow, MixedUnitsSupportedWithCasting) {
    constexpr auto cubic_inch = int_pow<3>(inches(1.0));
    constexpr auto expected_cm3 = cubed(centi(meters))(2.54 * 2.54 * 2.54);

    EXPECT_THAT(cubic_inch, IsNear(expected_cm3, nano(cubed(meters))(1.)));
}

TEST(Sin, TypeDependsOnInputType) {
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

TEST(Sin, SameAsStdSinForNumericTypes) {
    EXPECT_THAT(sin(1), Eq(std::sin(1)));
    EXPECT_THAT(sin(1.), Eq(std::sin(1.)));
    EXPECT_THAT(sin(1.f), Eq(std::sin(1.f)));
    EXPECT_THAT(sin(1.L), Eq(std::sin(1.L)));
}

TEST(Sin, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(sin(radians(1.23)), Eq(std::sin(1.23)));
    EXPECT_THAT(sin(radians(4.56f)), Eq(std::sin(4.56f)));
}

TEST(Sin, GivesCorrectAnswersForInputsInDegrees) {
    constexpr auto TOL = 1e-15;
    EXPECT_THAT(sin(degrees(0)), DoubleNear(0.0, TOL));
    EXPECT_THAT(sin(degrees(30)), DoubleNear(0.5, TOL));
    EXPECT_THAT(sin(degrees(45)), DoubleNear(std::sqrt(0.5), TOL));
    EXPECT_THAT(sin(degrees(90)), DoubleNear(1.0, TOL));
}

TEST(Mean, QuantityMeanGivesCommonUnit) {
    EXPECT_THAT(mean(feet(1), inches(12), yards(2)), SameTypeAndValue(inches(32)));
}

TEST(Mean, QuantityPointMeanGivesCommonUnit) {
    EXPECT_THAT(mean(meters_pt(2), centi(meters_pt)(150), milli(meters_pt)(2500)),
                SameTypeAndValue(milli(meters_pt)(2000)));
}

TEST(Sqrt, OutputRepDependsOnInputRep) {
    EXPECT_THAT(sqrt(squared(meters)(4)), QuantityEquivalent(meters(2.)));
    EXPECT_THAT(sqrt(squared(meters)(4.)), QuantityEquivalent(meters(2.)));
    EXPECT_THAT(sqrt(squared(meters)(4.f)), QuantityEquivalent(meters(2.f)));
    EXPECT_THAT(sqrt(squared(meters)(4.L)), QuantityEquivalent(meters(2.L)));
}

TEST(Sqrt, MixedUnitsSupportedWithCasting) {
    constexpr auto x_in = inches(1);
    constexpr auto y_cm = centi(meters)(2.54);

    EXPECT_THAT(sqrt(x_in * y_cm.as(inches)), IsNear(x_in, nano(meters)(1)));
}

TEST(Sqrt, SameAsStdSqrtForNumericTypes) {
    EXPECT_THAT(sqrt(1), Eq(std::sqrt(1)));
    EXPECT_THAT(sqrt(1.), Eq(std::sqrt(1.)));
    EXPECT_THAT(sqrt(1.f), Eq(std::sqrt(1.f)));
    EXPECT_THAT(sqrt(1.L), Eq(std::sqrt(1.L)));
}

TEST(Sqrt, CanConvertIfConversionFactorRational) {
    const auto geo_mean_length = sqrt(inches(1) * meters(1));

    // Using Quantity-equivalent Unit just retrieves the value stored in `geo_mean_length`.
    const auto retrieved_value = geo_mean_length.in(sqrt(inch * meters));
    EXPECT_THAT(retrieved_value, SameTypeAndValue(1.0));

    // This conversion is "easy", because the conversion factor doesn't have any rational powers.
    const auto rationally_converted_value = geo_mean_length.in(sqrt(inch * centi(meters)));
    EXPECT_THAT(rationally_converted_value, SameTypeAndValue(10.0));

    // This test case is "hard": we need to compute radical conversion factors at compile time.
    const auto radically_converted_value = geo_mean_length.in(inches);
    EXPECT_THAT(radically_converted_value, DoubleNear(6.274558, 0.000001));
}

TEST(Tan, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/tan
    StaticAssertTypeEq<decltype(tan(radians(0))), double>();
    StaticAssertTypeEq<decltype(tan(radians(0.))), double>();
    StaticAssertTypeEq<decltype(tan(radians(0.f))), float>();
    StaticAssertTypeEq<decltype(tan(radians(0.L))), long double>();

    // Make sure we support integral Degrees (related to Radians by an irrational scale factor).
    StaticAssertTypeEq<decltype(tan(degrees(0))), double>();
}

TEST(Tan, SameAsStdTanForNumericTypes) {
    EXPECT_THAT(tan(1), Eq(std::tan(1)));
    EXPECT_THAT(tan(1.), Eq(std::tan(1.)));
    EXPECT_THAT(tan(1.f), Eq(std::tan(1.f)));
    EXPECT_THAT(tan(1.L), Eq(std::tan(1.L)));
}

TEST(Tan, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(tan(radians(1.23)), SameTypeAndValue(std::tan(1.23)));
    EXPECT_THAT(tan(radians(4.56f)), SameTypeAndValue(std::tan(4.56f)));
}

TEST(Arccos, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/acos
    StaticAssertTypeEq<decltype(arccos(0)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arccos(0.)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arccos(0.f)), Quantity<Radians, float>>();
    StaticAssertTypeEq<decltype(arccos(0.L)), Quantity<Radians, long double>>();
}

TEST(Arccos, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(arccos(0.123), SameTypeAndValue(radians(std::acos(0.123))));
    EXPECT_THAT(arccos(0.456f), SameTypeAndValue(radians(std::acos(0.456f))));
}

TEST(Arcsin, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/asin
    StaticAssertTypeEq<decltype(arcsin(0)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arcsin(0.)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arcsin(0.f)), Quantity<Radians, float>>();
    StaticAssertTypeEq<decltype(arcsin(0.L)), Quantity<Radians, long double>>();
}

TEST(Arcsin, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(arcsin(0.123), SameTypeAndValue(radians(std::asin(0.123))));
    EXPECT_THAT(arcsin(0.456f), SameTypeAndValue(radians(std::asin(0.456f))));
}

TEST(Arcsin, ExampleFromReferenceDocs) {
    constexpr auto TOL = degrees(1e-12);
    EXPECT_THAT(arcsin(0.5).as(degrees), IsNear(degrees(30.0), TOL));
}

TEST(Arctan, TypeDependsOnInputType) {
    // See: https://en.cppreference.com/w/cpp/numeric/math/atan
    StaticAssertTypeEq<decltype(arctan(1)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan(1.)), Quantity<Radians, double>>();
    StaticAssertTypeEq<decltype(arctan(1.f)), Quantity<Radians, float>>();
    StaticAssertTypeEq<decltype(arctan(1.L)), Quantity<Radians, long double>>();
}

TEST(Arctan, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(arctan(3), SameTypeAndValue(radians(std::atan(3))));
    EXPECT_THAT(arctan(-5.f), SameTypeAndValue(radians(std::atan(-5.f))));
}

TEST(Arctan2, TypeDependsOnInputType) {
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

TEST(Arctan2, GivesSameAnswersAsRawNumbersButInStrongTypes) {
    EXPECT_THAT(arctan2(3, -5), SameTypeAndValue(radians(std::atan2(3, -5))));
    EXPECT_THAT(arctan2(3.f, -5.f), SameTypeAndValue(radians(std::atan2(3.f, -5.f))));
}

TEST(Arctan2, QuantityOverloadTypeDependsOnInputType) {
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

TEST(Arctan2, SupportsUnitsTypes) {
    // 100" == 254 cm.
    const auto angle = arctan2(make_quantity<Inches>(100), make_quantity<Centi<Meters>>(254));

    EXPECT_THAT(angle, IsNear(degrees(45), pico(degrees)(1)));
}

TEST(Isinf, TransparentlyActsOnSameAsValue) {
    const std::vector<double> values{{
        0.,
        1.23,
        -4.5e6,
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
    }};

    for (const double x : values) {
        EXPECT_THAT(isinf(meters(x)), Eq(std::isinf(x)));
        EXPECT_THAT(isinf(meters_pt(x)), Eq(std::isinf(x)));
        EXPECT_THAT(isinf((radians / second)(x)), Eq(std::isinf(x)));
    }
}

TEST(Isinf, UnqualifiedCallsGiveStdVersions) {
    // This test exists to make sure we don't break code with unqualified isinf calls.
    const bool b = isinf(5.5);
    EXPECT_THAT(b, IsFalse());
}

TEST(Isnan, TransparentlyActsOnSameAsValue) {
    const std::vector<double> values{{
        0.,
        1.23,
        -4.5e6,
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::signaling_NaN(),
    }};

    for (const double x : values) {
        EXPECT_THAT(isnan(meters(x)), Eq(std::isnan(x)));
        EXPECT_THAT(isnan(meters_pt(x)), Eq(std::isnan(x)));
        EXPECT_THAT(isnan((radians / second)(x)), Eq(std::isnan(x)));
    }
}

TEST(Isnan, UnqualifiedCallsGiveStdVersions) {
    // This test exists to make sure we don't break code with unqualified isnan calls.
    const bool b = isnan(5.5);
    EXPECT_THAT(b, IsFalse());
}

template <typename U, typename R>
struct NumericLimits {
    using Q = Quantity<U, R>;

    // To validly extend std::numeric_limits<T>, we must define all members declared static
    // constexpr in the primary template, in such a way that they are usable as integral constant
    // expressions.
    //
    // Source for rule: https://en.cppreference.com/w/cpp/language/extending_std
    // List of members: https://en.cppreference.com/w/cpp/types/numeric_limits
    static void parity() {
        EXPECT_THAT(std::numeric_limits<Q>::is_specialized, IsTrue());

        EXPECT_THAT(std::numeric_limits<Q>::is_signed, Eq(std::numeric_limits<R>::is_signed));
        EXPECT_THAT(std::numeric_limits<Q>::is_integer, Eq(std::numeric_limits<R>::is_integer));
        EXPECT_THAT(std::numeric_limits<Q>::is_exact, Eq(std::numeric_limits<R>::is_exact));
        EXPECT_THAT(std::numeric_limits<Q>::has_infinity, Eq(std::numeric_limits<R>::has_infinity));
        EXPECT_THAT(std::numeric_limits<Q>::has_quiet_NaN,
                    Eq(std::numeric_limits<R>::has_quiet_NaN));
        EXPECT_THAT(std::numeric_limits<Q>::has_signaling_NaN,
                    Eq(std::numeric_limits<R>::has_signaling_NaN));
        EXPECT_THAT(std::numeric_limits<Q>::has_denorm, Eq(std::numeric_limits<R>::has_denorm));
        EXPECT_THAT(std::numeric_limits<Q>::has_denorm_loss,
                    Eq(std::numeric_limits<R>::has_denorm_loss));
        EXPECT_THAT(std::numeric_limits<Q>::round_style, Eq(std::numeric_limits<R>::round_style));
        EXPECT_THAT(std::numeric_limits<Q>::is_iec559, Eq(std::numeric_limits<R>::is_iec559));
        EXPECT_THAT(std::numeric_limits<Q>::is_bounded, Eq(std::numeric_limits<R>::is_bounded));
        EXPECT_THAT(std::numeric_limits<Q>::is_modulo, Eq(std::numeric_limits<R>::is_modulo));
        EXPECT_THAT(std::numeric_limits<Q>::digits, Eq(std::numeric_limits<R>::digits));
        EXPECT_THAT(std::numeric_limits<Q>::digits10, Eq(std::numeric_limits<R>::digits10));
        EXPECT_THAT(std::numeric_limits<Q>::max_digits10, Eq(std::numeric_limits<R>::max_digits10));
        EXPECT_THAT(std::numeric_limits<Q>::radix, Eq(std::numeric_limits<R>::radix));
        EXPECT_THAT(std::numeric_limits<Q>::min_exponent, Eq(std::numeric_limits<R>::min_exponent));
        EXPECT_THAT(std::numeric_limits<Q>::min_exponent10,
                    Eq(std::numeric_limits<R>::min_exponent10));
        EXPECT_THAT(std::numeric_limits<Q>::max_exponent, Eq(std::numeric_limits<R>::max_exponent));
        EXPECT_THAT(std::numeric_limits<Q>::max_exponent10,
                    Eq(std::numeric_limits<R>::max_exponent10));
        EXPECT_THAT(std::numeric_limits<Q>::traps, Eq(std::numeric_limits<R>::traps));
        EXPECT_THAT(std::numeric_limits<Q>::tinyness_before,
                    Eq(std::numeric_limits<R>::tinyness_before));
    }
};

TEST(NumericLimits, MemberVariablesSetCorrectlyForQuantitySpecializationInt) {
    NumericLimits<Meters, int>::parity();
}

TEST(NumericLimits, MemberVariablesSetCorrectlyForQuantitySpecializationUint32T) {
    NumericLimits<Radians, uint32_t>::parity();
}

TEST(NumericLimits, MemberVariablesSetCorrectlyForQuantitySpecializationFloat) {
    NumericLimits<Celsius, float>::parity();
}

TEST(NumericLimits, ProvidesLimitsForQuantity) {
    using nl1 = std::numeric_limits<Quantity<Meters, int>>;
    EXPECT_THAT(nl1::max(), Eq(meters(std::numeric_limits<int>::max())));
    EXPECT_THAT(nl1::lowest(), Eq(meters(std::numeric_limits<int>::lowest())));
    EXPECT_THAT(nl1::min(), Eq(meters(std::numeric_limits<int>::min())));
    EXPECT_THAT(nl1::epsilon(), Eq(meters(std::numeric_limits<int>::epsilon())));
    EXPECT_THAT(nl1::round_error(), Eq(meters(std::numeric_limits<int>::round_error())));
    EXPECT_THAT(nl1::infinity(), Eq(meters(std::numeric_limits<int>::infinity())));
    EXPECT_THAT(nl1::denorm_min(), Eq(meters(std::numeric_limits<int>::denorm_min())));

    using nl2 = std::numeric_limits<Quantity<Ohms, float>>;
    EXPECT_THAT(nl2::max(), Eq(ohms(std::numeric_limits<float>::max())));
    EXPECT_THAT(nl2::lowest(), Eq(ohms(std::numeric_limits<float>::lowest())));
    EXPECT_THAT(nl2::min(), Eq(ohms(std::numeric_limits<float>::min())));
    EXPECT_THAT(nl2::epsilon(), Eq(ohms(std::numeric_limits<float>::epsilon())));
    EXPECT_THAT(nl2::round_error(), Eq(ohms(std::numeric_limits<float>::round_error())));
    EXPECT_THAT(nl2::infinity(), Eq(ohms(std::numeric_limits<float>::infinity())));
    EXPECT_THAT(nl2::denorm_min(), Eq(ohms(std::numeric_limits<float>::denorm_min())));

    // We cannot currently test `quiet_NaN` or `signaling_NaN`.  Later, we could provide overloads
    // for `isnan()`, which people could find via ADL.
}

TEST(NumericLimits, InsensitiveToCvQualificationForQuantity) {
    using Q = Quantity<Degrees, float>;

    EXPECT_THAT(std::numeric_limits<Q>::max(), Eq(std::numeric_limits<const Q>::max()));
    EXPECT_THAT(std::numeric_limits<Q>::max(), Eq(std::numeric_limits<volatile Q>::max()));
    EXPECT_THAT(std::numeric_limits<Q>::max(), Eq(std::numeric_limits<const volatile Q>::max()));

    EXPECT_THAT(std::numeric_limits<Q>::lowest(), Eq(std::numeric_limits<const Q>::lowest()));
    EXPECT_THAT(std::numeric_limits<Q>::lowest(), Eq(std::numeric_limits<volatile Q>::lowest()));
    EXPECT_THAT(std::numeric_limits<Q>::lowest(),
                Eq(std::numeric_limits<const volatile Q>::lowest()));

    EXPECT_THAT(std::numeric_limits<Q>::min(), Eq(std::numeric_limits<const Q>::min()));
    EXPECT_THAT(std::numeric_limits<Q>::min(), Eq(std::numeric_limits<volatile Q>::min()));
    EXPECT_THAT(std::numeric_limits<Q>::min(), Eq(std::numeric_limits<const volatile Q>::min()));

    EXPECT_THAT(std::numeric_limits<Q>::epsilon(), Eq(std::numeric_limits<const Q>::epsilon()));
    EXPECT_THAT(std::numeric_limits<Q>::epsilon(), Eq(std::numeric_limits<volatile Q>::epsilon()));
    EXPECT_THAT(std::numeric_limits<Q>::epsilon(),
                Eq(std::numeric_limits<const volatile Q>::epsilon()));

    EXPECT_THAT(std::numeric_limits<Q>::round_error(),
                Eq(std::numeric_limits<const Q>::round_error()));
    EXPECT_THAT(std::numeric_limits<Q>::round_error(),
                Eq(std::numeric_limits<volatile Q>::round_error()));
    EXPECT_THAT(std::numeric_limits<Q>::round_error(),
                Eq(std::numeric_limits<const volatile Q>::round_error()));

    EXPECT_THAT(std::numeric_limits<Q>::infinity(), Eq(std::numeric_limits<const Q>::infinity()));
    EXPECT_THAT(std::numeric_limits<Q>::infinity(),
                Eq(std::numeric_limits<volatile Q>::infinity()));
    EXPECT_THAT(std::numeric_limits<Q>::infinity(),
                Eq(std::numeric_limits<const volatile Q>::infinity()));

    // It's hard to test `quiet_NaN` or `signaling_NaN`, because they have the property that x != x.

    EXPECT_THAT(std::numeric_limits<Q>::denorm_min(),
                Eq(std::numeric_limits<const Q>::denorm_min()));
    EXPECT_THAT(std::numeric_limits<Q>::denorm_min(),
                Eq(std::numeric_limits<volatile Q>::denorm_min()));
    EXPECT_THAT(std::numeric_limits<Q>::denorm_min(),
                Eq(std::numeric_limits<const volatile Q>::denorm_min()));
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

TEST(RoundAs, ConstantSupportsConstexpr) {
    constexpr auto result = round_as(meters, SEVEN_THIRDS_METERS);
    EXPECT_THAT(result, AllOf(Eq(meters(2)), IsConstantWithUnitMatching(LabelIs("[2 m]"))));
}

TEST(RoundAs, ConstantRoundsToZero) {
    EXPECT_THAT(round_as(kilo(meters), SEVEN_THIRDS_METERS), SameTypeAndValue(ZERO));
}

TEST(RoundAs, ConstantRoundsHalfAwayFromZero) {
    EXPECT_THAT(round_as(meters, FIVE_HALVES_METERS),
                AllOf(Eq(meters(3)), IsConstantWithUnitMatching(LabelIs("[3 m]"))));
    EXPECT_THAT(round_as(meters, -FIVE_HALVES_METERS),
                AllOf(Eq(meters(-3)), IsConstantWithUnitMatching(LabelIs("[-3 m]"))));
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

TEST(RoundIn, ConstantRequiresExplicitRep) {
    EXPECT_THAT(round_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(2));
    EXPECT_THAT(round_in<std::size_t>(meters, SEVEN_THIRDS_METERS),
                SameTypeAndValue(std::size_t{2}));
}

TEST(RoundIn, ConstantRoundsToNearestInteger) {
    EXPECT_THAT(round_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(2));
}

TEST(RoundIn, ConstantRoundsHalfAwayFromZero) {
    EXPECT_THAT(round_in<int>(meters, FIVE_HALVES_METERS), SameTypeAndValue(3));
    EXPECT_THAT(round_in<int>(meters, -FIVE_HALVES_METERS), SameTypeAndValue(-3));
}

TEST(RoundIn, ConstantSupportsConstexpr) {
    constexpr int result = round_in<int>(meters, SEVEN_THIRDS_METERS);
    EXPECT_THAT(result, SameTypeAndValue(2));
}

TEST(RoundIn, ConstantSupportsZeroOutcome) {
    constexpr int result = round_in<int>(kilo(meters), SEVEN_THIRDS_METERS);
    EXPECT_THAT(result, SameTypeAndValue(0));
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

TEST(FloorAs, ConstantSupportsConstexpr) {
    constexpr auto result = floor_as(meters, SEVEN_THIRDS_METERS);
    EXPECT_THAT(result, AllOf(Eq(meters(2)), IsConstantWithUnitMatching(LabelIs("[2 m]"))));
}

TEST(FloorAs, ConstantReturnsLargestIntegerNotGreaterThanInput) {
    EXPECT_THAT(floor_as(meters, SEVEN_THIRDS_METERS),
                AllOf(Eq(meters(2)), IsConstantWithUnitMatching(LabelIs("[2 m]"))));
    EXPECT_THAT(floor_as(meters, FIVE_HALVES_METERS),
                AllOf(Eq(meters(2)), IsConstantWithUnitMatching(LabelIs("[2 m]"))));
    EXPECT_THAT(floor_as(meters, -FIVE_HALVES_METERS),
                AllOf(Eq(meters(-3)), IsConstantWithUnitMatching(LabelIs("[-3 m]"))));
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

TEST(FloorIn, ConstantRequiresExplicitRep) {
    EXPECT_THAT(floor_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(2));
    EXPECT_THAT(floor_in<std::size_t>(meters, SEVEN_THIRDS_METERS),
                SameTypeAndValue(std::size_t{2}));
}

TEST(FloorIn, ConstantReturnsLargestIntegerNotGreaterThanInput) {
    EXPECT_THAT(floor_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(2));
    EXPECT_THAT(floor_in<int>(meters, FIVE_HALVES_METERS), SameTypeAndValue(2));
    EXPECT_THAT(floor_in<int>(meters, -FIVE_HALVES_METERS), SameTypeAndValue(-3));
}

TEST(FloorIn, ConstantSupportsConstexpr) {
    constexpr int result = floor_in<int>(meters, SEVEN_THIRDS_METERS);
    EXPECT_THAT(result, SameTypeAndValue(2));
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

TEST(CeilAs, ConstantSupportsConstexpr) {
    constexpr auto result = ceil_as(meters, SEVEN_THIRDS_METERS);
    EXPECT_THAT(result, AllOf(Eq(meters(3)), IsConstantWithUnitMatching(LabelIs("[3 m]"))));
}

TEST(CeilAs, ConstantReturnsSmallestIntegerNotLessThanInput) {
    EXPECT_THAT(ceil_as(meters, SEVEN_THIRDS_METERS),
                AllOf(Eq(meters(3)), IsConstantWithUnitMatching(LabelIs("[3 m]"))));
    EXPECT_THAT(ceil_as(meters, FIVE_HALVES_METERS),
                AllOf(Eq(meters(3)), IsConstantWithUnitMatching(LabelIs("[3 m]"))));
    EXPECT_THAT(ceil_as(meters, -FIVE_HALVES_METERS),
                AllOf(Eq(meters(-2)), IsConstantWithUnitMatching(LabelIs("[-2 m]"))));
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

TEST(CeilIn, ConstantRequiresExplicitRep) {
    EXPECT_THAT(ceil_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(3));
    EXPECT_THAT(ceil_in<std::size_t>(meters, SEVEN_THIRDS_METERS),
                SameTypeAndValue(std::size_t{3}));
}

TEST(CeilIn, ConstantReturnsSmallestIntegerNotLessThanInput) {
    EXPECT_THAT(ceil_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(3));
    EXPECT_THAT(ceil_in<int>(meters, FIVE_HALVES_METERS), SameTypeAndValue(3));
    EXPECT_THAT(ceil_in<int>(meters, -FIVE_HALVES_METERS), SameTypeAndValue(-2));
}

TEST(CeilIn, ConstantSupportsConstexpr) {
    constexpr int result = ceil_in<int>(meters, SEVEN_THIRDS_METERS);
    EXPECT_THAT(result, SameTypeAndValue(3));
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

TEST(IntRoundIn, SupportsConstexpr) {
    constexpr auto result = int_round_in(meters, milli(meters)(1'500));
    EXPECT_THAT(result, SameTypeAndValue(2));
}

TEST(IntRoundIn, RoundsHalfAwayFromZero) {
    EXPECT_THAT(int_round_in(meters, milli(meters)(-1'500)), SameTypeAndValue(-2));
    EXPECT_THAT(int_round_in(meters, milli(meters)(-500)), SameTypeAndValue(-1));
    EXPECT_THAT(int_round_in(meters, milli(meters)(500)), SameTypeAndValue(1));
    EXPECT_THAT(int_round_in(meters, milli(meters)(1'500)), SameTypeAndValue(2));
}

TEST(IntRoundIn, RoundsToNearestWhenUnambiguous) {
    EXPECT_THAT(int_round_in(meters, milli(meters)(-501)), SameTypeAndValue(-1));
    EXPECT_THAT(int_round_in(meters, milli(meters)(-499)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in(meters, milli(meters)(499)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in(meters, milli(meters)(501)), SameTypeAndValue(1));
}

TEST(IntRoundIn, ScalesUpByIntegerFactor) {
    EXPECT_THAT(int_round_in(milli(meters), meters(3)), SameTypeAndValue(3'000));
}

TEST(IntRoundIn, ScalesDownByIntegerFactor) {
    EXPECT_THAT(int_round_in(meters, milli(meters)(2'700)), SameTypeAndValue(3));
}

TEST(IntRoundIn, HandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_round_in(meters, feet(1)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in(meters, feet(2)), SameTypeAndValue(1));
    EXPECT_THAT(int_round_in(meters, feet(5)), SameTypeAndValue(2));
}

TEST(IntRoundIn, SupportsUnsignedTypes) {
    EXPECT_THAT(int_round_in(meters, milli(meters)(1'499u)), SameTypeAndValue(1u));
    EXPECT_THAT(int_round_in(meters, milli(meters)(1'500u)), SameTypeAndValue(2u));
}

TEST(IntRoundIn, SupportsInt64) {
    constexpr int64_t large_value = 9'000'000'000'000'000'500LL;
    EXPECT_THAT(int_round_in(kilo(meters), meters(large_value)),
                SameTypeAndValue(int64_t{9'000'000'000'000'001LL}));
}

TEST(IntRoundIn, QuantityPointBasicRounding) {
    EXPECT_THAT(int_round_in(meters_pt, milli(meters_pt)(1'499)), SameTypeAndValue(1));
    EXPECT_THAT(int_round_in(meters_pt, milli(meters_pt)(1'500)), SameTypeAndValue(2));
}

TEST(IntRoundIn, QuantityPointWithNontrivialOffset) {
    // 273'150 mK == 0 degrees Celsius
    EXPECT_THAT(int_round_in(celsius_pt, milli(kelvins_pt)(273'150)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in(celsius_pt, milli(kelvins_pt)(273'649)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in(celsius_pt, milli(kelvins_pt)(273'650)), SameTypeAndValue(1));
}

TEST(IntRoundIn, ExplicitRepSupportsConstexpr) {
    constexpr auto result = int_round_in<int>(meters, milli(meters)(1'500));
    EXPECT_THAT(result, SameTypeAndValue(2));
}

TEST(IntRoundIn, ExplicitRepRoundsHalfAwayFromZero) {
    EXPECT_THAT(int_round_in<int>(meters, milli(meters)(-1'500)), SameTypeAndValue(-2));
    EXPECT_THAT(int_round_in<int>(meters, milli(meters)(-500)), SameTypeAndValue(-1));
    EXPECT_THAT(int_round_in<int>(meters, milli(meters)(500)), SameTypeAndValue(1));
    EXPECT_THAT(int_round_in<int>(meters, milli(meters)(1'500)), SameTypeAndValue(2));
}

TEST(IntRoundIn, ExplicitRepRoundsToNearestWhenUnambiguous) {
    EXPECT_THAT(int_round_in<int>(meters, milli(meters)(-501)), SameTypeAndValue(-1));
    EXPECT_THAT(int_round_in<int>(meters, milli(meters)(-499)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in<int>(meters, milli(meters)(499)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in<int>(meters, milli(meters)(501)), SameTypeAndValue(1));
}

TEST(IntRoundIn, ExplicitRepAcceptsFloatingPointInput) {
    EXPECT_THAT(int_round_in<int>(meters, meters(-1.5)), SameTypeAndValue(-2));

    EXPECT_THAT(int_round_in<int>(meters, meters(1.4)), SameTypeAndValue(1));
    EXPECT_THAT(int_round_in<int>(meters, meters(1.5)), SameTypeAndValue(2));
    EXPECT_THAT(int_round_in<int>(meters, meters(1.6)), SameTypeAndValue(2));
}

TEST(IntRoundIn, ExplicitRepAcceptsFloatInput) {
    EXPECT_THAT(int_round_in<int>(meters, meters(1.5f)), SameTypeAndValue(2));
    EXPECT_THAT(int_round_in<int>(meters, meters(2.5f)), SameTypeAndValue(3));
}

TEST(IntRoundIn, ExplicitRepHandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_round_in<int>(meters, feet(1.0)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in<int>(meters, feet(1.7)), SameTypeAndValue(1));
}

TEST(IntRoundIn, ExplicitRepSupportsInt64Output) {
    EXPECT_THAT(int_round_in<int64_t>(meters, meters(1.5)), SameTypeAndValue(int64_t{2}));
}

TEST(IntRoundIn, ExplicitRepSupportsUnsignedOutput) {
    EXPECT_THAT(int_round_in<unsigned>(meters, meters(1.5)), SameTypeAndValue(2u));
}

TEST(IntRoundIn, ExplicitRepQuantityPointAcceptsFloatingPointInput) {
    EXPECT_THAT(int_round_in<int>(meters_pt, meters_pt(1.4)), SameTypeAndValue(1));
    EXPECT_THAT(int_round_in<int>(meters_pt, meters_pt(1.5)), SameTypeAndValue(2));
}

TEST(IntRoundIn, ExplicitRepQuantityPointWithNontrivialOffset) {
    // 273.15 K == 0 degrees Celsius
    EXPECT_THAT(int_round_in<int>(celsius_pt, kelvins_pt(273.15)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in<int>(celsius_pt, kelvins_pt(273.64)), SameTypeAndValue(0));
    EXPECT_THAT(int_round_in<int>(celsius_pt, kelvins_pt(273.65)), SameTypeAndValue(1));
}

TEST(IntRoundIn, ConstantRequiresExplicitRep) {
    EXPECT_THAT(int_round_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(2));
    EXPECT_THAT(int_round_in<std::size_t>(meters, SEVEN_THIRDS_METERS),
                SameTypeAndValue(std::size_t{2}));
}

TEST(IntRoundIn, ConstantRoundsHalfAwayFromZero) {
    EXPECT_THAT(int_round_in<int>(meters, FIVE_HALVES_METERS), SameTypeAndValue(3));
    EXPECT_THAT(int_round_in<int>(meters, -FIVE_HALVES_METERS), SameTypeAndValue(-3));
}

TEST(IntRoundAs, SupportsConstexpr) {
    constexpr auto result = int_round_as(meters, milli(meters)(1'500));
    EXPECT_THAT(result, SameTypeAndValue(meters(2)));
}

TEST(IntRoundAs, ReturnsQuantityWithTargetUnit) {
    const auto result = int_round_as(kilo(meters), meters(2'500));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters)(3)));
}

TEST(IntRoundAs, RoundsHalfAwayFromZero) {
    EXPECT_THAT(int_round_as(meters, milli(meters)(-500)), SameTypeAndValue(meters(-1)));
    EXPECT_THAT(int_round_as(meters, milli(meters)(500)), SameTypeAndValue(meters(1)));
}

TEST(IntRoundAs, RoundsToNearestWhenUnambiguous) {
    EXPECT_THAT(int_round_as(meters, milli(meters)(499)), SameTypeAndValue(meters(0)));
    EXPECT_THAT(int_round_as(meters, milli(meters)(501)), SameTypeAndValue(meters(1)));
}

TEST(IntRoundAs, HandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_round_as(meters, feet(1)), SameTypeAndValue(meters(0)));
    EXPECT_THAT(int_round_as(meters, feet(2)), SameTypeAndValue(meters(1)));
}

TEST(IntRoundAs, SupportsUnsignedTypes) {
    EXPECT_THAT(int_round_as(meters, milli(meters)(1'500u)), SameTypeAndValue(meters(2u)));
}

TEST(IntRoundAs, QuantityPointReturnsQuantityPointWithTargetUnit) {
    const auto result = int_round_as(kilo(meters_pt), meters_pt(2'500));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters_pt)(3)));
}

TEST(IntRoundAs, QuantityPointWithNontrivialOffset) {
    // 273'150 mK == 0 degrees Celsius
    EXPECT_THAT(int_round_as(celsius_pt, milli(kelvins_pt)(273'150)),
                SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(int_round_as(celsius_pt, milli(kelvins_pt)(273'650)),
                SameTypeAndValue(celsius_pt(1)));
}

TEST(IntRoundAs, ExplicitRepSupportsConstexpr) {
    constexpr auto result = int_round_as<int>(meters, milli(meters)(1'500));
    EXPECT_THAT(result, SameTypeAndValue(meters(2)));
}

TEST(IntRoundAs, ExplicitRepReturnsQuantityWithTargetUnitAndRep) {
    const auto result = int_round_as<int64_t>(kilo(meters), meters(2'500));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters)(int64_t{3})));
}

TEST(IntRoundAs, ExplicitRepAcceptsFloatingPointInput) {
    EXPECT_THAT(int_round_as<int>(meters, meters(1.4)), SameTypeAndValue(meters(1)));
    EXPECT_THAT(int_round_as<int>(meters, meters(1.5)), SameTypeAndValue(meters(2)));
    EXPECT_THAT(int_round_as<int>(meters, meters(-1.5)), SameTypeAndValue(meters(-2)));
}

TEST(IntRoundAs, ExplicitRepHandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_round_as<int>(meters, feet(1.7)), SameTypeAndValue(meters(1)));
}

TEST(IntRoundAs, ExplicitRepQuantityPointAcceptsFloatingPointInput) {
    EXPECT_THAT(int_round_as<int>(meters_pt, meters_pt(1.4)), SameTypeAndValue(meters_pt(1)));
    EXPECT_THAT(int_round_as<int>(meters_pt, meters_pt(1.5)), SameTypeAndValue(meters_pt(2)));
}

TEST(IntRoundAs, ExplicitRepQuantityPointWithNontrivialOffset) {
    // 273.15 K == 0 degrees Celsius
    EXPECT_THAT(int_round_as<int>(celsius_pt, kelvins_pt(273.15)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(int_round_as<int>(celsius_pt, kelvins_pt(273.65)), SameTypeAndValue(celsius_pt(1)));
}

TEST(IntRoundAs, ConstantBehavesIdenticallyToRoundAs) {
    EXPECT_THAT(int_round_as(meters, SEVEN_THIRDS_METERS),
                AllOf(Eq(meters(2)), IsConstantWithUnitMatching(LabelIs("[2 m]"))));
    EXPECT_THAT(int_round_as(meters, FIVE_HALVES_METERS),
                AllOf(Eq(meters(3)), IsConstantWithUnitMatching(LabelIs("[3 m]"))));
    EXPECT_THAT(int_round_as(meters, -FIVE_HALVES_METERS),
                AllOf(Eq(meters(-3)), IsConstantWithUnitMatching(LabelIs("[-3 m]"))));
}

TEST(IntFloorIn, SupportsConstexpr) {
    constexpr auto result = int_floor_in(meters, milli(meters)(1'999));
    EXPECT_THAT(result, SameTypeAndValue(1));
}

TEST(IntFloorIn, ReturnsLargestIntegerNotGreaterThanInput) {
    EXPECT_THAT(int_floor_in(meters, milli(meters)(-1'001)), SameTypeAndValue(-2));
    EXPECT_THAT(int_floor_in(meters, milli(meters)(-1'000)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in(meters, milli(meters)(-999)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in(meters, milli(meters)(-1)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in(meters, milli(meters)(1'000)), SameTypeAndValue(1));
    EXPECT_THAT(int_floor_in(meters, milli(meters)(1'001)), SameTypeAndValue(1));
    EXPECT_THAT(int_floor_in(meters, milli(meters)(1'999)), SameTypeAndValue(1));
}

TEST(IntFloorIn, ScalesUpByIntegerFactor) {
    EXPECT_THAT(int_floor_in(milli(meters), meters(3)), SameTypeAndValue(3'000));
}

TEST(IntFloorIn, ScalesDownByIntegerFactor) {
    EXPECT_THAT(int_floor_in(meters, milli(meters)(2'999)), SameTypeAndValue(2));
    EXPECT_THAT(int_floor_in(meters, milli(meters)(3'000)), SameTypeAndValue(3));
}

TEST(IntFloorIn, HandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_floor_in(meters, feet(-1)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in(meters, feet(3)), SameTypeAndValue(0));
    EXPECT_THAT(int_floor_in(meters, feet(4)), SameTypeAndValue(1));
}

TEST(IntFloorIn, SupportsUnsignedTypes) {
    EXPECT_THAT(int_floor_in(meters, milli(meters)(1'999u)), SameTypeAndValue(1u));
}

TEST(IntFloorIn, SupportsInt64) {
    constexpr int64_t large_value = 9'000'000'000'000'000'999LL;
    EXPECT_THAT(int_floor_in(kilo(meters), meters(large_value)),
                SameTypeAndValue(int64_t{9'000'000'000'000'000LL}));
}

TEST(IntFloorIn, QuantityPointBasicFloor) {
    EXPECT_THAT(int_floor_in(meters_pt, milli(meters_pt)(1'999)), SameTypeAndValue(1));
    EXPECT_THAT(int_floor_in(meters_pt, milli(meters_pt)(2'000)), SameTypeAndValue(2));
}

TEST(IntFloorIn, QuantityPointWithNontrivialOffset) {
    // 273'150 mK == 0 degrees Celsius
    EXPECT_THAT(int_floor_in(celsius_pt, milli(kelvins_pt)(273'149)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in(celsius_pt, milli(kelvins_pt)(273'150)), SameTypeAndValue(0));
    EXPECT_THAT(int_floor_in(celsius_pt, milli(kelvins_pt)(274'149)), SameTypeAndValue(0));
    EXPECT_THAT(int_floor_in(celsius_pt, milli(kelvins_pt)(274'150)), SameTypeAndValue(1));
}

TEST(IntFloorIn, ExplicitRepSupportsConstexpr) {
    constexpr auto result = int_floor_in<int>(meters, milli(meters)(1'999));
    EXPECT_THAT(result, SameTypeAndValue(1));
}

TEST(IntFloorIn, ExplicitRepReturnsLargestIntegerNotGreaterThanInput) {
    EXPECT_THAT(int_floor_in<int>(meters, milli(meters)(-1'001)), SameTypeAndValue(-2));
    EXPECT_THAT(int_floor_in<int>(meters, milli(meters)(-1'000)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in<int>(meters, milli(meters)(1'000)), SameTypeAndValue(1));
    EXPECT_THAT(int_floor_in<int>(meters, milli(meters)(1'999)), SameTypeAndValue(1));
}

TEST(IntFloorIn, ExplicitRepAcceptsFloatingPointInput) {
    EXPECT_THAT(int_floor_in<int>(meters, meters(-1.1)), SameTypeAndValue(-2));
    EXPECT_THAT(int_floor_in<int>(meters, meters(-1.0)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in<int>(meters, meters(1.0)), SameTypeAndValue(1));
    EXPECT_THAT(int_floor_in<int>(meters, meters(1.9)), SameTypeAndValue(1));
}

TEST(IntFloorIn, ExplicitRepAcceptsFloatInput) {
    EXPECT_THAT(int_floor_in<int>(meters, meters(1.9f)), SameTypeAndValue(1));
    EXPECT_THAT(int_floor_in<int>(meters, meters(2.0f)), SameTypeAndValue(2));
}

TEST(IntFloorIn, ExplicitRepHandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_floor_in<int>(meters, feet(-0.1)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in<int>(meters, feet(3.2)), SameTypeAndValue(0));
    EXPECT_THAT(int_floor_in<int>(meters, feet(3.3)), SameTypeAndValue(1));
}

TEST(IntFloorIn, ExplicitRepSupportsInt64Output) {
    EXPECT_THAT(int_floor_in<int64_t>(meters, meters(1.9)), SameTypeAndValue(int64_t{1}));
}

TEST(IntFloorIn, ExplicitRepSupportsUnsignedOutput) {
    EXPECT_THAT(int_floor_in<unsigned>(meters, meters(1.9)), SameTypeAndValue(1u));
}

TEST(IntFloorIn, ExplicitRepQuantityPointAcceptsFloatingPointInput) {
    EXPECT_THAT(int_floor_in<int>(meters_pt, meters_pt(1.9)), SameTypeAndValue(1));
    EXPECT_THAT(int_floor_in<int>(meters_pt, meters_pt(2.0)), SameTypeAndValue(2));
}

TEST(IntFloorIn, ExplicitRepQuantityPointWithNontrivialOffset) {
    // 273.15 K == 0 degrees Celsius
    EXPECT_THAT(int_floor_in<int>(celsius_pt, kelvins_pt(273.14)), SameTypeAndValue(-1));
    EXPECT_THAT(int_floor_in<int>(celsius_pt, kelvins_pt(273.15)), SameTypeAndValue(0));
    EXPECT_THAT(int_floor_in<int>(celsius_pt, kelvins_pt(274.14)), SameTypeAndValue(0));
    EXPECT_THAT(int_floor_in<int>(celsius_pt, kelvins_pt(274.15)), SameTypeAndValue(1));
}

TEST(IntFloorIn, ConstantRequiresExplicitRep) {
    EXPECT_THAT(int_floor_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(2));
    EXPECT_THAT(int_floor_in<std::size_t>(meters, SEVEN_THIRDS_METERS),
                SameTypeAndValue(std::size_t{2}));
}

TEST(IntFloorIn, ConstantReturnsLargestIntegerNotGreaterThanInput) {
    EXPECT_THAT(int_floor_in<int>(meters, FIVE_HALVES_METERS), SameTypeAndValue(2));
    EXPECT_THAT(int_floor_in<int>(meters, -FIVE_HALVES_METERS), SameTypeAndValue(-3));
}

TEST(IntFloorAs, SupportsConstexpr) {
    constexpr auto result = int_floor_as(meters, milli(meters)(1'999));
    EXPECT_THAT(result, SameTypeAndValue(meters(1)));
}

TEST(IntFloorAs, ReturnsQuantityWithTargetUnit) {
    const auto result = int_floor_as(kilo(meters), meters(2'999));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters)(2)));
}

TEST(IntFloorAs, ReturnsLargestIntegerNotGreaterThanInput) {
    EXPECT_THAT(int_floor_as(meters, milli(meters)(-1'001)), SameTypeAndValue(meters(-2)));
    EXPECT_THAT(int_floor_as(meters, milli(meters)(-1)), SameTypeAndValue(meters(-1)));
    EXPECT_THAT(int_floor_as(meters, milli(meters)(1'999)), SameTypeAndValue(meters(1)));
}

TEST(IntFloorAs, HandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_floor_as(meters, feet(3)), SameTypeAndValue(meters(0)));
    EXPECT_THAT(int_floor_as(meters, feet(4)), SameTypeAndValue(meters(1)));
}

TEST(IntFloorAs, SupportsUnsignedTypes) {
    EXPECT_THAT(int_floor_as(meters, milli(meters)(1'999u)), SameTypeAndValue(meters(1u)));
}

TEST(IntFloorAs, QuantityPointReturnsQuantityPointWithTargetUnit) {
    const auto result = int_floor_as(kilo(meters_pt), meters_pt(2'999));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters_pt)(2)));
}

TEST(IntFloorAs, QuantityPointWithNontrivialOffset) {
    // 273'150 mK == 0 degrees Celsius
    EXPECT_THAT(int_floor_as(celsius_pt, milli(kelvins_pt)(273'149)),
                SameTypeAndValue(celsius_pt(-1)));
    EXPECT_THAT(int_floor_as(celsius_pt, milli(kelvins_pt)(274'149)),
                SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(int_floor_as(celsius_pt, milli(kelvins_pt)(274'150)),
                SameTypeAndValue(celsius_pt(1)));
}

TEST(IntFloorAs, ExplicitRepSupportsConstexpr) {
    constexpr auto result = int_floor_as<int>(meters, milli(meters)(1'999));
    EXPECT_THAT(result, SameTypeAndValue(meters(1)));
}

TEST(IntFloorAs, ExplicitRepReturnsQuantityWithTargetUnitAndRep) {
    const auto result = int_floor_as<int64_t>(kilo(meters), meters(2'999));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters)(int64_t{2})));
}

TEST(IntFloorAs, ExplicitRepAcceptsFloatingPointInput) {
    EXPECT_THAT(int_floor_as<int>(meters, meters(-1.1)), SameTypeAndValue(meters(-2)));
    EXPECT_THAT(int_floor_as<int>(meters, meters(1.9)), SameTypeAndValue(meters(1)));
}

TEST(IntFloorAs, ExplicitRepHandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_floor_as<int>(meters, feet(3.2)), SameTypeAndValue(meters(0)));
    EXPECT_THAT(int_floor_as<int>(meters, feet(3.3)), SameTypeAndValue(meters(1)));
}

TEST(IntFloorAs, ExplicitRepQuantityPointAcceptsFloatingPointInput) {
    EXPECT_THAT(int_floor_as<int>(meters_pt, meters_pt(1.9)), SameTypeAndValue(meters_pt(1)));
    EXPECT_THAT(int_floor_as<int>(meters_pt, meters_pt(2.0)), SameTypeAndValue(meters_pt(2)));
}

TEST(IntFloorAs, ExplicitRepQuantityPointWithNontrivialOffset) {
    // 273.15 K == 0 degrees Celsius
    EXPECT_THAT(int_floor_as<int>(celsius_pt, kelvins_pt(273.14)),
                SameTypeAndValue(celsius_pt(-1)));
    EXPECT_THAT(int_floor_as<int>(celsius_pt, kelvins_pt(274.14)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(int_floor_as<int>(celsius_pt, kelvins_pt(274.15)), SameTypeAndValue(celsius_pt(1)));
}

TEST(IntFloorAs, ConstantBehavesIdenticallyToFloorAs) {
    EXPECT_THAT(int_floor_as(meters, SEVEN_THIRDS_METERS),
                AllOf(Eq(meters(2)), IsConstantWithUnitMatching(LabelIs("[2 m]"))));
    EXPECT_THAT(int_floor_as(meters, FIVE_HALVES_METERS),
                AllOf(Eq(meters(2)), IsConstantWithUnitMatching(LabelIs("[2 m]"))));
    EXPECT_THAT(int_floor_as(meters, -FIVE_HALVES_METERS),
                AllOf(Eq(meters(-3)), IsConstantWithUnitMatching(LabelIs("[-3 m]"))));
}

TEST(IntCeilIn, SupportsConstexpr) {
    constexpr auto result = int_ceil_in(meters, milli(meters)(1'001));
    EXPECT_THAT(result, SameTypeAndValue(2));
}

TEST(IntCeilIn, ReturnsSmallestIntegerNotLessThanInput) {
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(-1'999)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(-1'001)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(-1'000)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(-999)), SameTypeAndValue(0));
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(1'001)), SameTypeAndValue(2));
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(1'999)), SameTypeAndValue(2));
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(2'000)), SameTypeAndValue(2));
}

TEST(IntCeilIn, ScalesUpByIntegerFactor) {
    EXPECT_THAT(int_ceil_in(milli(meters), meters(3)), SameTypeAndValue(3'000));
}

TEST(IntCeilIn, ScalesDownByIntegerFactor) {
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(3'000)), SameTypeAndValue(3));
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(3'001)), SameTypeAndValue(4));
}

TEST(IntCeilIn, HandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_ceil_in(meters, feet(-4)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in(meters, feet(3)), SameTypeAndValue(1));
    EXPECT_THAT(int_ceil_in(meters, feet(4)), SameTypeAndValue(2));
}

TEST(IntCeilIn, SupportsUnsignedTypes) {
    EXPECT_THAT(int_ceil_in(meters, milli(meters)(1'001u)), SameTypeAndValue(2u));
}

TEST(IntCeilIn, SupportsInt64) {
    constexpr int64_t large_value = 9'000'000'000'000'000'001LL;
    EXPECT_THAT(int_ceil_in(kilo(meters), meters(large_value)),
                SameTypeAndValue(int64_t{9'000'000'000'000'001LL}));
}

TEST(IntCeilIn, QuantityPointBasicCeil) {
    EXPECT_THAT(int_ceil_in(meters_pt, milli(meters_pt)(1'000)), SameTypeAndValue(1));
    EXPECT_THAT(int_ceil_in(meters_pt, milli(meters_pt)(1'001)), SameTypeAndValue(2));
}

TEST(IntCeilIn, QuantityPointWithNontrivialOffset) {
    // 273'150 mK == 0 degrees Celsius
    EXPECT_THAT(int_ceil_in(celsius_pt, milli(kelvins_pt)(272'150)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in(celsius_pt, milli(kelvins_pt)(273'149)), SameTypeAndValue(0));
    EXPECT_THAT(int_ceil_in(celsius_pt, milli(kelvins_pt)(273'150)), SameTypeAndValue(0));
    EXPECT_THAT(int_ceil_in(celsius_pt, milli(kelvins_pt)(273'151)), SameTypeAndValue(1));
}

TEST(IntCeilIn, ExplicitRepSupportsConstexpr) {
    constexpr auto result = int_ceil_in<int>(meters, milli(meters)(1'001));
    EXPECT_THAT(result, SameTypeAndValue(2));
}

TEST(IntCeilIn, ExplicitRepReturnsSmallestIntegerNotLessThanInput) {
    EXPECT_THAT(int_ceil_in<int>(meters, milli(meters)(-1'001)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in<int>(meters, milli(meters)(-1'000)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in<int>(meters, milli(meters)(1'000)), SameTypeAndValue(1));
    EXPECT_THAT(int_ceil_in<int>(meters, milli(meters)(1'001)), SameTypeAndValue(2));
}

TEST(IntCeilIn, ExplicitRepAcceptsFloatingPointInput) {
    EXPECT_THAT(int_ceil_in<int>(meters, meters(-1.1)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in<int>(meters, meters(-1.0)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in<int>(meters, meters(1.0)), SameTypeAndValue(1));
    EXPECT_THAT(int_ceil_in<int>(meters, meters(1.1)), SameTypeAndValue(2));
}

TEST(IntCeilIn, ExplicitRepAcceptsFloatInput) {
    EXPECT_THAT(int_ceil_in<int>(meters, meters(1.0f)), SameTypeAndValue(1));
    EXPECT_THAT(int_ceil_in<int>(meters, meters(1.1f)), SameTypeAndValue(2));
}

TEST(IntCeilIn, ExplicitRepHandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_ceil_in<int>(meters, feet(-3.3)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in<int>(meters, feet(3.2)), SameTypeAndValue(1));
    EXPECT_THAT(int_ceil_in<int>(meters, feet(3.3)), SameTypeAndValue(2));
}

TEST(IntCeilIn, ExplicitRepSupportsInt64Output) {
    EXPECT_THAT(int_ceil_in<int64_t>(meters, meters(1.1)), SameTypeAndValue(int64_t{2}));
}

TEST(IntCeilIn, ExplicitRepSupportsUnsignedOutput) {
    EXPECT_THAT(int_ceil_in<unsigned>(meters, meters(1.1)), SameTypeAndValue(2u));
}

TEST(IntCeilIn, ExplicitRepQuantityPointAcceptsFloatingPointInput) {
    EXPECT_THAT(int_ceil_in<int>(meters_pt, meters_pt(1.0)), SameTypeAndValue(1));
    EXPECT_THAT(int_ceil_in<int>(meters_pt, meters_pt(1.1)), SameTypeAndValue(2));
}

TEST(IntCeilIn, ExplicitRepQuantityPointWithNontrivialOffset) {
    // 273.15 K == 0 degrees Celsius
    EXPECT_THAT(int_ceil_in<int>(celsius_pt, kelvins_pt(272.15)), SameTypeAndValue(-1));
    EXPECT_THAT(int_ceil_in<int>(celsius_pt, kelvins_pt(273.14)), SameTypeAndValue(0));
    EXPECT_THAT(int_ceil_in<int>(celsius_pt, kelvins_pt(273.15)), SameTypeAndValue(0));
    EXPECT_THAT(int_ceil_in<int>(celsius_pt, kelvins_pt(273.16)), SameTypeAndValue(1));
}

TEST(IntCeilIn, ConstantRequiresExplicitRep) {
    EXPECT_THAT(int_ceil_in<int>(meters, SEVEN_THIRDS_METERS), SameTypeAndValue(3));
    EXPECT_THAT(int_ceil_in<std::size_t>(meters, SEVEN_THIRDS_METERS),
                SameTypeAndValue(std::size_t{3}));
}

TEST(IntCeilIn, ConstantReturnsSmallestIntegerNotLessThanInput) {
    EXPECT_THAT(int_ceil_in<int>(meters, FIVE_HALVES_METERS), SameTypeAndValue(3));
    EXPECT_THAT(int_ceil_in<int>(meters, -FIVE_HALVES_METERS), SameTypeAndValue(-2));
}

TEST(IntCeilAs, SupportsConstexpr) {
    constexpr auto result = int_ceil_as(meters, milli(meters)(1'001));
    EXPECT_THAT(result, SameTypeAndValue(meters(2)));
}

TEST(IntCeilAs, ReturnsQuantityWithTargetUnit) {
    const auto result = int_ceil_as(kilo(meters), meters(3'001));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters)(4)));
}

TEST(IntCeilAs, ReturnsSmallestIntegerNotLessThanInput) {
    EXPECT_THAT(int_ceil_as(meters, milli(meters)(-1'999)), SameTypeAndValue(meters(-1)));
    EXPECT_THAT(int_ceil_as(meters, milli(meters)(-999)), SameTypeAndValue(meters(0)));
    EXPECT_THAT(int_ceil_as(meters, milli(meters)(1'001)), SameTypeAndValue(meters(2)));
}

TEST(IntCeilAs, HandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_ceil_as(meters, feet(3)), SameTypeAndValue(meters(1)));
    EXPECT_THAT(int_ceil_as(meters, feet(4)), SameTypeAndValue(meters(2)));
}

TEST(IntCeilAs, SupportsUnsignedTypes) {
    EXPECT_THAT(int_ceil_as(meters, milli(meters)(1'001u)), SameTypeAndValue(meters(2u)));
}

TEST(IntCeilAs, QuantityPointReturnsQuantityPointWithTargetUnit) {
    const auto result = int_ceil_as(kilo(meters_pt), meters_pt(3'001));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters_pt)(4)));
}

TEST(IntCeilAs, QuantityPointWithNontrivialOffset) {
    // 273'150 mK == 0 degrees Celsius
    EXPECT_THAT(int_ceil_as(celsius_pt, milli(kelvins_pt)(273'149)),
                SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(int_ceil_as(celsius_pt, milli(kelvins_pt)(273'150)),
                SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(int_ceil_as(celsius_pt, milli(kelvins_pt)(273'151)),
                SameTypeAndValue(celsius_pt(1)));
}

TEST(IntCeilAs, ExplicitRepSupportsConstexpr) {
    constexpr auto result = int_ceil_as<int>(meters, milli(meters)(1'001));
    EXPECT_THAT(result, SameTypeAndValue(meters(2)));
}

TEST(IntCeilAs, ExplicitRepReturnsQuantityWithTargetUnitAndRep) {
    const auto result = int_ceil_as<int64_t>(kilo(meters), meters(3'001));
    EXPECT_THAT(result, SameTypeAndValue(kilo(meters)(int64_t{4})));
}

TEST(IntCeilAs, ExplicitRepAcceptsFloatingPointInput) {
    EXPECT_THAT(int_ceil_as<int>(meters, meters(-1.1)), SameTypeAndValue(meters(-1)));
    EXPECT_THAT(int_ceil_as<int>(meters, meters(1.1)), SameTypeAndValue(meters(2)));
}

TEST(IntCeilAs, ExplicitRepHandlesRationalConversionFactor) {
    // 1 foot == (381 / 1250) meters
    EXPECT_THAT(int_ceil_as<int>(meters, feet(3.2)), SameTypeAndValue(meters(1)));
    EXPECT_THAT(int_ceil_as<int>(meters, feet(3.3)), SameTypeAndValue(meters(2)));
}

TEST(IntCeilAs, ExplicitRepQuantityPointAcceptsFloatingPointInput) {
    EXPECT_THAT(int_ceil_as<int>(meters_pt, meters_pt(1.0)), SameTypeAndValue(meters_pt(1)));
    EXPECT_THAT(int_ceil_as<int>(meters_pt, meters_pt(1.1)), SameTypeAndValue(meters_pt(2)));
}

TEST(IntCeilAs, ExplicitRepQuantityPointWithNontrivialOffset) {
    // 273.15 K == 0 degrees Celsius
    EXPECT_THAT(int_ceil_as<int>(celsius_pt, kelvins_pt(273.14)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(int_ceil_as<int>(celsius_pt, kelvins_pt(273.15)), SameTypeAndValue(celsius_pt(0)));
    EXPECT_THAT(int_ceil_as<int>(celsius_pt, kelvins_pt(273.16)), SameTypeAndValue(celsius_pt(1)));
}

TEST(IntCeilAs, ConstantBehavesIdenticallyToCeilAs) {
    EXPECT_THAT(int_ceil_as(meters, SEVEN_THIRDS_METERS),
                AllOf(Eq(meters(3)), IsConstantWithUnitMatching(LabelIs("[3 m]"))));
    EXPECT_THAT(int_ceil_as(meters, FIVE_HALVES_METERS),
                AllOf(Eq(meters(3)), IsConstantWithUnitMatching(LabelIs("[3 m]"))));
    EXPECT_THAT(int_ceil_as(meters, -FIVE_HALVES_METERS),
                AllOf(Eq(meters(-2)), IsConstantWithUnitMatching(LabelIs("[-2 m]"))));
}

}  // namespace au
