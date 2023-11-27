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

#include "au/testing.hh"
#include "au/units/joules.hh"
#include "au/units/meters.hh"
#include "au/units/newtons.hh"
#include "au/units/radians.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::AnyOf;
using ::testing::StaticAssertTypeEq;
using ::testing::StrEq;

namespace au {
namespace {

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
}  // namespace

TEST(MakeConstant, MakesConstantFromUnit) {
    StaticAssertTypeEq<decltype(make_constant(SpeedOfLight{})), Constant<SpeedOfLight>>();
}

TEST(MakeConstant, MakesConstantFromQuantityMaker) {
    StaticAssertTypeEq<decltype(make_constant(speed_of_light)), Constant<SpeedOfLight>>();
}

TEST(MakeConstant, MakesAdHocConstantFromQuantityMaker) {
    constexpr auto ad_hoc_c = make_constant(meters / second * mag<299'792'458>());
    EXPECT_THAT((1.0 * ad_hoc_c).in(meters / second), SameTypeAndValue(299'792'458.0));

    auto foo = [](Quantity<UnitQuotientT<Meters, Seconds>, int> q) { std::cout << q << std::endl; };
    foo(c);
}

TEST(MakeConstant, MakesConstantFromSymbol) {
    constexpr auto m = symbol_for(meters);
    constexpr auto s = symbol_for(seconds);
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

TEST(Constant, MakesQuantityWhenPostMultiplyingNumericValue) {
    EXPECT_THAT(3.f * c, SameTypeAndValue(speed_of_light(3.f)));
}

TEST(Constant, MakesQuantityWhenPreMultiplyingNumericValue) {
    EXPECT_THAT((c * 2).coerce_as(meters / second),
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
    StaticAssertTypeEq<decltype(PI / c), Constant<decltype(UnitInverseT<SpeedOfLight>{} * PI)>>();
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

TEST(Constant, SupportsUnitSlotAPIs) {
    constexpr auto three_c_mps = (3.f * c).as(meters / second);
    EXPECT_THAT(three_c_mps.in(c), SameTypeAndValue(3.f));
}

TEST(CanStoreValueIn, ChecksRangeOfTypeForIntegers) {
    EXPECT_TRUE(decltype(c)::can_store_value_in<int32_t>(meters / second));
    EXPECT_FALSE(decltype(c)::can_store_value_in<int16_t>(meters / second));
}

}  // namespace au
