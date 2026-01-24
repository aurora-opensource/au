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

#include "au/constant.hh"

#include <sstream>

#include "au/chrono_interop.hh"
#include "au/testing.hh"
#include "au/units/degrees.hh"
#include "au/units/feet.hh"
#include "au/units/inches.hh"
#include "au/units/joules.hh"
#include "au/units/meters.hh"
#include "au/units/newtons.hh"
#include "au/units/radians.hh"
#include "au/units/revolutions.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Le;
using ::testing::Lt;
using ::testing::Ne;
using ::testing::Not;
using ::testing::StaticAssertTypeEq;
using ::testing::StrEq;

namespace {

template <typename U>
std::string constant_label_for(Constant<U>) {
    return unit_label(U{});
}

// Matcher for checking the unit label of a Constant.
// Usage: EXPECT_THAT(my_constant, ConstantLabelIs(StrEq("[21 in]")));
MATCHER_P(ConstantLabelIs,
          inner_matcher,
          "is a Constant whose unit label " +
              ::testing::DescribeMatcher<std::string>(inner_matcher)) {
    const std::string label(constant_label_for(arg));
    *result_listener << "whose unit label is \"" << label << "\"";
    return ::testing::SafeMatcherCast<std::string>(inner_matcher).Matches(label);
}

constexpr auto PI = Magnitude<Pi>{};
constexpr auto m = symbol_for(meters);
constexpr auto s = symbol_for(seconds);

template <typename U, typename R>
std::string stream_to_string(Quantity<U, R> q) {
    std::ostringstream oss;

    // Set the precision to the full precision of R.
    oss.precision(std::numeric_limits<R>::digits10 + 1);

    oss << q;
    return oss.str();
}

// Ad hoc Constant for the speed of light, along with associated variables.
constexpr auto C_MPS = mag<299'792'458>();
struct SpeedOfLight : decltype(Meters{} / Seconds{} * C_MPS) {
    static constexpr const char label[] = "c";
};
constexpr const char SpeedOfLight::label[];
constexpr auto speed_of_light = QuantityMaker<SpeedOfLight>{};
constexpr auto c = make_constant(speed_of_light);

// Ad hoc Constant for Planck's constant.
constexpr auto H_JS = mag<6'626'070'15>() / mag<1'000'000'00>() * pow<-34>(mag<10>());
struct PlancksConstant : decltype(Joules{} * Seconds{} * H_JS) {
    static constexpr const char label[] = "h";
};
constexpr auto plancks_constant = QuantityMaker<PlancksConstant>{};
constexpr auto h = make_constant(plancks_constant);

TEST(MakeConstant, MakesConstantFromUnit) {
    StaticAssertTypeEq<decltype(make_constant(SpeedOfLight{})), Constant<SpeedOfLight>>();
}

TEST(MakeConstant, MakesConstantFromQuantityMaker) {
    StaticAssertTypeEq<decltype(make_constant(speed_of_light)), Constant<SpeedOfLight>>();
}

TEST(MakeConstant, MakesAdHocConstantFromQuantityMaker) {
    constexpr auto ad_hoc_c = make_constant(meters / second * mag<299'792'458>());
    EXPECT_THAT((1.0 * ad_hoc_c).in(meters / second), SameTypeAndValue(299'792'458.0));

    auto foo = [](Quantity<UnitQuotient<Meters, Seconds>, int> q) { std::cout << q << std::endl; };
    foo(c);
}

TEST(MakeConstant, MakesConstantFromSymbol) {
    constexpr auto ad_hoc_c = mag<299'792'458>() * make_constant(m / s);
    EXPECT_THAT(123 * ad_hoc_c, QuantityEquivalent(123 * c));
}

TEST(Constant, CanGetQuantityBySpecifyingRep) {
    EXPECT_THAT(c.as<float>(), SameTypeAndValue(c * 1.0f));
    EXPECT_THAT(c.as<int>(), SameTypeAndValue(c * 1));
}

TEST(Constant, CanGetQuantityInSpecifiedUnitAndRep) {
    EXPECT_THAT(c.in<float>(meters / second), SameTypeAndValue(get_value<float>(C_MPS)));
    EXPECT_THAT(c.as<float>(meters / second),
                SameTypeAndValue((meters / second)(get_value<float>(C_MPS))));
}

TEST(Constant, UsesExactSafetyChecksInsteadOfHeuristics) {
    EXPECT_THAT(c.in<int>(meters / second), SameTypeAndValue(get_value<int>(C_MPS)));
    EXPECT_THAT(c.as<int>(meters / second),
                SameTypeAndValue((meters / second)(get_value<int>(C_MPS))));

    // The following code must not compile, since the speed of light in m/s can't fit in `int16_t`.
    //
    // Uncomment the following to test:
    // c.as<int16_t>(meters / second);
}

TEST(Constant, CanCoerce) {
    EXPECT_THAT(c.coerce_in<int>(kilo(meters) / second), SameTypeAndValue(299'792));
    EXPECT_THAT(c.coerce_as<int>(kilo(meters) / second),
                SameTypeAndValue((kilo(meters) / second)(299'792)));
}

TEST(Constant, CanProvidePolicy) {
    EXPECT_THAT(c.in<int>(kilo(meters) / second, ignore(TRUNCATION_RISK)),
                SameTypeAndValue(299'792));
    EXPECT_THAT(c.as<int>(kilo(meters) / second, ignore(TRUNCATION_RISK)),
                SameTypeAndValue((kilo(meters) / second)(299'792)));
}

TEST(Constant, CanNegate) {
    constexpr auto neg_c = -c;
    EXPECT_THAT(neg_c, Eq(-299'792'458 * m / s));
    EXPECT_THAT(stream_to_string(0.75 * neg_c), StrEq("0.75 [-c]"));

    constexpr auto double_neg_c = -neg_c;
    EXPECT_THAT(double_neg_c, Eq(299'792'458 * m / s));
    EXPECT_THAT(stream_to_string(0.75 * double_neg_c), StrEq("0.75 c"));
}

TEST(Constant, MakesQuantityWhenPostMultiplyingNumericValue) {
    EXPECT_THAT(3.f * c, SameTypeAndValue(speed_of_light(3.f)));
}

TEST(Constant, MakesQuantityWhenPreMultiplyingNumericValue) {
    EXPECT_THAT((c * 2).as(meters / second, ignore(OVERFLOW_RISK)),
                SameTypeAndValue((meters / second)(get_value<int>(mag<2>() * C_MPS))));
}

TEST(Constant, MakesQuantityWhenDividingIntoNumericValue) {
    EXPECT_THAT(20u / c, SameTypeAndValue(inverse(speed_of_light)(20u)));
}

TEST(Constant, MakesQuantityWhenDividedByNumericValue) {
    EXPECT_THAT((c / 2.0).as(meters / second),
                SameTypeAndValue((meters / second)(get_value<double>(C_MPS / mag<2>()))));

    // The following must not compile, because it would use integer division with an implicit
    // numerator of `1`, and would therefore always be zero.
    //
    // Uncomment to make sure the compilation fails:
    // c / 2;
}

TEST(Constant, AppliesConstantSymbolToUnitLabel) {
    constexpr auto lambda = nano(meters)(512.0);
    EXPECT_THAT(stream_to_string(h * c / lambda),
                AnyOf(StrEq("0.001953125 (h * c) / nm"), StrEq("0.001953125 (c * h) / nm")));
}

TEST(Constant, MakesScaledConstantWhenPostMultipliedByMagnitude) {
    StaticAssertTypeEq<decltype(c * mag<3>()), Constant<decltype(SpeedOfLight{} * mag<3>())>>();
}

TEST(Constant, MakesScaledConstantWhenDividedByMagnitude) {
    StaticAssertTypeEq<decltype(c / mag<3>()), Constant<decltype(SpeedOfLight{} / mag<3>())>>();
}

TEST(Constant, MakesScaledConstantWhenPreMultipliedByMagnitude) {
    StaticAssertTypeEq<decltype(PI * c), Constant<decltype(SpeedOfLight{} * PI)>>();
}

TEST(Constant, MakesScaledInverseConstantWhenDividedIntoMagnitude) {
    StaticAssertTypeEq<decltype(PI / c), Constant<decltype(UnitInverse<SpeedOfLight>{} * PI)>>();
}

TEST(Constant, ChangesUnitsForQuantityWhenPostMultiplying) {
    EXPECT_THAT(newtons(5.f) * c, SameTypeAndValue((newton * speed_of_light)(5.f)));
}

TEST(Constant, ChangesUnitsForQuantityWhenDividingIt) {
    EXPECT_THAT(joules(8) / c, SameTypeAndValue((joules / speed_of_light)(8)));
}

TEST(Constant, ChangesUnitsForQuantityWhenPreMultiplying) {
    EXPECT_THAT(c * seconds(5.f), SameTypeAndValue((speed_of_light * seconds)(5.f)));
}

TEST(Constant, ChangesUnitsForQuantityWhenDividedIntoIt) {
    EXPECT_THAT(c / meters(4.0), SameTypeAndValue((speed_of_light / meter)(0.25)));

    // The following must not compile, because it would use integer division with an implicit
    // numerator of `1`, and would therefore always be zero.
    //
    // Uncomment to make sure the compilation fails:
    // EXPECT_THAT(c / meters(4), SameTypeAndValue((speed_of_light / meter)(0.25)));
}

TEST(Constant, ChangesUnitsForQuantityMakerWhenPostMultiplying) {
    StaticAssertTypeEq<decltype(newtons * c),
                       QuantityMaker<decltype(Newtons{} * SpeedOfLight{})>>();
}

TEST(Constant, ChangesUnitsForQuantityMakerWhenDividingIt) {
    StaticAssertTypeEq<decltype(joules / c), QuantityMaker<decltype(Joules{} / SpeedOfLight{})>>();
}

TEST(Constant, ChangesUnitsForQuantityMakerWhenPreMultiplying) {
    StaticAssertTypeEq<decltype(c * seconds),
                       QuantityMaker<decltype(SpeedOfLight{} * Seconds{})>>();
}

TEST(Constant, ChangesUnitsForQuantityMakerWhenDividedIntoIt) {
    StaticAssertTypeEq<decltype(c / meters), QuantityMaker<decltype(SpeedOfLight{} / Meters{})>>();
}

TEST(Constant, ChangesUnitsForSingularNameWhenPostMultiplying) {
    StaticAssertTypeEq<decltype(newton * c),
                       SingularNameFor<decltype(Newtons{} * SpeedOfLight{})>>();
}

TEST(Constant, ChangesUnitsForSingularNameWhenDividingIt) {
    StaticAssertTypeEq<decltype(joule / c), SingularNameFor<decltype(Joules{} / SpeedOfLight{})>>();
}

TEST(Constant, ChangesUnitsForSingularNameWhenPreMultiplying) {
    StaticAssertTypeEq<decltype(c * second),
                       SingularNameFor<decltype(SpeedOfLight{} * Seconds{})>>();
}

TEST(Constant, ChangesUnitsForSingularNameWhenDividedIntoIt) {
    StaticAssertTypeEq<decltype(c / meter), SingularNameFor<decltype(SpeedOfLight{} / Meters{})>>();
}

TEST(Constant, ComposesViaMultiplication) {
    StaticAssertTypeEq<decltype(h * c), Constant<decltype(SpeedOfLight{} * PlancksConstant{})>>();
}

TEST(Constant, SupportsMultiplyingConstantByItself) {
    StaticAssertTypeEq<decltype(c * c), Constant<decltype(squared(SpeedOfLight{}))>>();
}

TEST(Constant, CanTakePowers) { StaticAssertTypeEq<decltype(squared(c)), decltype(c * c)>(); }

TEST(Constant, ComposesViaDivision) {
    StaticAssertTypeEq<decltype(c / h), Constant<decltype(SpeedOfLight{} / PlancksConstant{})>>();
}

TEST(Constant, FailsToCompileWhenMultiplyingOrDividingWithQuantityPoint) {
    // Uncomment each line below individually to verify.

    // make_constant(meters) * meters_pt(1);
    // make_constant(meters) / meters_pt(1);
    // meters_pt(1) * make_constant(meters);
    // meters_pt(1) / make_constant(meters);

    // make_constant(meters) * meters_pt;
    // make_constant(meters) / meters_pt;
    // meters_pt * make_constant(meters);
    // meters_pt / make_constant(meters);
}

TEST(Constant, ImplicitlyConvertsToAppropriateQuantityTypes) {
    constexpr QuantityI32<decltype(Meters{} / Seconds{})> v = c;
    EXPECT_THAT(v, SameTypeAndValue((meters / second)(get_value<int32_t>(C_MPS))));

    // The following must not compile.  Uncomment inside the scope to check.
    {
        // constexpr Quantity<decltype(Meters{} / Seconds{}), int16_t> v = c;
        // (void)c;
    }
}

TEST(Constant, ImplicitlyConvertsToNonAuTypesWithAppropriateCorrespondingQuantity) {
    constexpr auto dt_as_constant = make_constant(seconds * mag<1'000'000'000>());
    constexpr auto dt_as_quantity_u32 = dt_as_constant.as<uint32_t>();

    // If we had represented this constant as a quantity, it wouldn't be able to convert to a
    // quantity of seconds backed by `uint32_t`, because of the overflow safety surface.  However,
    // the constant value itself can be safely converted, because we know its exact value.
    ASSERT_THAT(
        (std::is_convertible<decltype(dt_as_quantity_u32), std::chrono::duration<uint32_t>>::value),
        IsFalse());
    EXPECT_THAT(
        (std::is_convertible<decltype(dt_as_constant), std::chrono::duration<uint32_t>>::value),
        IsTrue());

    // Here, the constant value itself can't be safely converted, because it's too big for a
    // `uint16_t`.
    EXPECT_THAT(
        (std::is_convertible<decltype(dt_as_constant), std::chrono::duration<uint16_t>>::value),
        IsFalse());
}

TEST(Constant, SupportsUnitSlotAPIs) {
    constexpr auto three_c_mps = (3.f * c).as(meters / second);
    EXPECT_THAT(three_c_mps.in(c), SameTypeAndValue(3.f));
}

TEST(Constant, SupportsMinWithQuantity) {
    EXPECT_THAT(min(c, (meters / second)(100)), SameTypeAndValue((meters / second)(100)));
    EXPECT_THAT(min((meters / second)(1'000'000'000), c),
                SameTypeAndValue((meters / second)(299'792'458)));
}

TEST(Constant, SupportsMaxWithQuantity) {
    EXPECT_THAT(max(c, (meters / second)(100)), SameTypeAndValue((meters / second)(299'792'458)));
    EXPECT_THAT(max((meters / second)(1'000'000'000), c),
                SameTypeAndValue((meters / second)(1'000'000'000)));
}

TEST(Constant, SupportsClampWithQuantity) {
    EXPECT_THAT(clamp((meters / second)(100), c / mag<2>(), c),
                SameTypeAndValue((meters / second)(149'896'229)));
}

TEST(Constant, SupportsModWithQuantity) {
    constexpr auto half_rev = make_constant(revolutions / mag<2>());
    EXPECT_THAT(half_rev % degrees(100), SameTypeAndValue(degrees(80)));
    EXPECT_THAT(degrees(300) % half_rev, SameTypeAndValue(degrees(120)));
}

TEST(Constant, ModResultIsIntegerMultipleOfCommonUnitWhenRatioIsExactInteger) {
    // 5 feet = 60 inches; 60 % 7 = 4
    constexpr auto five_feet = make_constant(feet * mag<5>());
    constexpr auto seven_inches = make_constant(inches * mag<7>());
    constexpr auto result = five_feet % seven_inches;

    EXPECT_THAT(result, AllOf(Eq(inches(4)), ConstantLabelIs(StrEq("[4 in]"))));
}

TEST(Constant, ModResultIsIntegerMultipleOfCommonUnitWhenRatioIsExactInverseInteger) {
    // 57 inches % 3 feet = 57 inches % 36 inches = 21 inches
    constexpr auto fifty_seven_inches = make_constant(inches * mag<57>());
    constexpr auto three_feet = make_constant(feet * mag<3>());
    constexpr auto result = fifty_seven_inches % three_feet;

    EXPECT_THAT(result, AllOf(Eq(inches(21)), ConstantLabelIs(StrEq("[21 in]"))));
}

TEST(Constant, ModResultIsIntegerMultipleOfCommonUnitWhenRatioIsRational) {
    // The common unit of feet and meters is (1/1250) meters = (1/381) feet.
    //
    // 3 meters = 3750 common units; 5 feet = 1905 common units.
    //
    // 3750 % 1905 = 1845 common units
    constexpr auto three_meters = make_constant(meters * mag<3>());
    constexpr auto five_feet = make_constant(feet * mag<5>());
    constexpr auto result = three_meters % five_feet;

    EXPECT_THAT(result,
                AllOf(

                    // Use well-tested `Quantity` results, with exact integer math, to be confident
                    // that `Constant` produces the correct quantity.
                    Eq(three_meters.as<int>() % five_feet.as<int>()),

                    // We don't want to depend on which order the EQUIV label shows the units.
                    ConstantLabelIs(AnyOf(StrEq("[1845 EQUIV{[(1 / 1250) m], [(1 / 381) ft]}]"),
                                          StrEq("[1845 EQUIV{[(1 / 381) ft], [(1 / 1250) m]}]")))));
}

TEST(Constant, ModWithFractionalScaledUnits) {
    // Denominators 7 and 5 are coprime with each other, and with all factors of 12, so the common
    // unit must be (1/35) inches.
    //
    // Input values are:
    // (12/7) * (12 * 35 units) = 720 units
    //  (8/5) * ( 1 * 35 units) = 56 units
    //
    // Overall, 720 % 56 = 48, and "units" is most economically expressed as (1/35) inches.
    constexpr auto twelve_sevenths_feet = make_constant(feet * mag<12>() / mag<7>());
    constexpr auto eight_fifths_inches = make_constant(inches * mag<8>() / mag<5>());
    constexpr auto result = twelve_sevenths_feet % eight_fifths_inches;

    EXPECT_THAT(result,
                AllOf(Eq((inches / mag<35>())(48)), ConstantLabelIs(StrEq("[(48 / 35) in]"))));
}

TEST(Constant, ModReturnsZeroWhenEvenlyDivisible) {
    StaticAssertTypeEq<decltype(make_constant(feet * mag<21>()) % make_constant(inches * mag<7>())),
                       Zero>();
}

TEST(Constant, ModWithZeroDividendReturnsZero) {
    constexpr auto result = make_constant(ZERO) % make_constant(feet);
    EXPECT_THAT(result, Eq(ZERO));
}

TEST(MakeConstant, IdentityForZero) { EXPECT_THAT(make_constant(ZERO), SameTypeAndValue(ZERO)); }

TEST(CanStoreValueIn, ChecksRangeOfTypeForIntegers) {
    EXPECT_THAT(decltype(c)::can_store_value_in<int32_t>(meters / second), IsTrue());
    EXPECT_THAT(decltype(c)::can_store_value_in<int16_t>(meters / second), IsFalse());
}

TEST(Constant, SupportsEqualityComparison) {
    constexpr auto one_meter = make_constant(meters);
    constexpr auto another_meter = make_constant(meters);

    EXPECT_THAT(one_meter, Eq(another_meter));
    EXPECT_THAT(one_meter, Not(Ne(another_meter)));
}

TEST(Constant, EqualityComparisonWorksWithDifferentButEquivalentUnits) {
    constexpr auto one_revolution = make_constant(revolutions);
    constexpr auto three_sixty_degrees = make_constant(degrees * mag<360>());

    EXPECT_THAT(one_revolution, Eq(three_sixty_degrees));
    EXPECT_THAT(one_revolution, Not(Ne(three_sixty_degrees)));
}

TEST(Constant, InequalityComparisonDetectsDifferentValues) {
    constexpr auto one_degree = make_constant(degrees);
    constexpr auto one_revolution = make_constant(revolutions);

    EXPECT_THAT(one_degree, Not(Eq(one_revolution)));
    EXPECT_THAT(one_degree, Ne(one_revolution));
}

TEST(Constant, SupportsOrderingComparison) {
    constexpr auto one_meter = make_constant(meters);
    constexpr auto one_kilometer = make_constant(kilo(meters));

    EXPECT_THAT(one_meter, Lt(one_kilometer));
    EXPECT_THAT(one_meter, Le(one_kilometer));
    EXPECT_THAT(one_meter, Not(Gt(one_kilometer)));
    EXPECT_THAT(one_meter, Not(Ge(one_kilometer)));

    EXPECT_THAT(one_kilometer, Gt(one_meter));
    EXPECT_THAT(one_kilometer, Ge(one_meter));
    EXPECT_THAT(one_kilometer, Not(Lt(one_meter)));
    EXPECT_THAT(one_kilometer, Not(Le(one_meter)));
}

TEST(Constant, OrderingWithEqualConstants) {
    constexpr auto c1 = make_constant(meters);
    constexpr auto c2 = make_constant(meters);

    EXPECT_THAT(c1, Le(c2));
    EXPECT_THAT(c1, Ge(c2));
    EXPECT_THAT(c1, Not(Lt(c2)));
    EXPECT_THAT(c1, Not(Gt(c2)));
}

TEST(Constant, OrderingWorksWithCompatibleUnits) {
    constexpr auto one_degree = make_constant(degrees);
    constexpr auto one_revolution = make_constant(revolutions);

    EXPECT_THAT(one_degree, Lt(one_revolution));
    EXPECT_THAT(one_degree, Le(one_revolution));
    EXPECT_THAT(one_degree, Not(Gt(one_revolution)));
    EXPECT_THAT(one_degree, Not(Ge(one_revolution)));
}

TEST(Constant, OrderingWorksWithNegativeConstants) {
    constexpr auto neg_c = -c;

    EXPECT_THAT(neg_c, Lt(c));
    EXPECT_THAT(neg_c, Le(c));
    EXPECT_THAT(neg_c, Not(Gt(c)));
    EXPECT_THAT(neg_c, Not(Ge(c)));

    EXPECT_THAT(c, Gt(neg_c));
    EXPECT_THAT(c, Ge(neg_c));
    EXPECT_THAT(c, Not(Lt(neg_c)));
    EXPECT_THAT(c, Not(Le(neg_c)));
}

TEST(Constant, OrderingWorksWithScaledConstants) {
    constexpr auto half_c = c / mag<2>();
    constexpr auto double_c = c * mag<2>();

    EXPECT_THAT(half_c, Lt(c));
    EXPECT_THAT(c, Lt(double_c));
    EXPECT_THAT(half_c, Lt(double_c));

    EXPECT_THAT(double_c, Gt(c));
    EXPECT_THAT(c, Gt(half_c));
    EXPECT_THAT(double_c, Gt(half_c));
}

}  // namespace
}  // namespace au
