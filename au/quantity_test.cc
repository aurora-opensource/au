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

#include "au/quantity.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/utility/type_traits.hh"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {

struct Feet : UnitImpl<Length> {};
constexpr auto feet = QuantityMaker<Feet>{};

struct Miles : decltype(Feet{} * mag<5'280>()) {};
constexpr auto mile = SingularNameFor<Miles>{};
constexpr auto miles = QuantityMaker<Miles>{};

struct Inches : decltype(Feet{} / mag<12>()) {};
constexpr auto inches = QuantityMaker<Inches>{};

struct Yards : decltype(Feet{} * mag<3>()) {};
constexpr auto yards = QuantityMaker<Yards>{};

struct Meters : decltype(Inches{} * mag<100>() / mag<254>() * mag<100>()) {};
static constexpr QuantityMaker<Meters> meters{};
static_assert(are_units_quantity_equivalent(Centi<Meters>{} * mag<254>(), Inches{} * mag<100>()),
              "Double-check this ad hoc definition of meters");

struct Hours : UnitImpl<Time> {};
constexpr auto hour = SingularNameFor<Hours>{};
constexpr auto hours = QuantityMaker<Hours>{};

struct Minutes : decltype(Hours{} / mag<60>()) {};
constexpr auto minute = SingularNameFor<Minutes>{};
constexpr auto minutes = QuantityMaker<Minutes>{};

struct Days : decltype(Hours{} * mag<24>()) {};
constexpr auto days = QuantityMaker<Days>{};

struct PerDay : decltype(UnitInverseT<Days>{}) {};
constexpr auto per_day = QuantityMaker<PerDay>{};

template <typename... Us>
constexpr auto num_units_in_product(UnitProduct<Us...>) {
    return sizeof...(Us);
}

TEST(Quantity, IsItaniumAbiRegisterCompatible) {
    // See: https://refspecs.linuxfoundation.org/cxxabi-1.86.html#normal-call
    //
    // This blog post also contains some nice background on the ability (or not) of types to be
    // passed in registers: https://quuxplusone.github.io/blog/2018/05/02/trivial-abi-101/
    EXPECT_TRUE(std::is_trivially_destructible<QuantityD<Feet>>::value);
    EXPECT_TRUE(std::is_trivially_copy_constructible<QuantityD<Feet>>::value);
    EXPECT_TRUE(std::is_trivially_move_constructible<QuantityD<Feet>>::value);
}

TEST(Quantity, HasCorrectRepNamedAliases) {
    StaticAssertTypeEq<QuantityD<Feet>, Quantity<Feet, double>>();
    StaticAssertTypeEq<QuantityF<Feet>, Quantity<Feet, float>>();
    StaticAssertTypeEq<QuantityI<Feet>, Quantity<Feet, int>>();
    StaticAssertTypeEq<QuantityU<Feet>, Quantity<Feet, unsigned int>>();
    StaticAssertTypeEq<QuantityI32<Feet>, Quantity<Feet, int32_t>>();
    StaticAssertTypeEq<QuantityU32<Feet>, Quantity<Feet, uint32_t>>();
    StaticAssertTypeEq<QuantityI64<Feet>, Quantity<Feet, int64_t>>();
    StaticAssertTypeEq<QuantityU64<Feet>, Quantity<Feet, uint64_t>>();
}

TEST(Quantity, CanCreateAndReadValuesByNamingUnits) {
    constexpr auto x = feet(3.14);
    constexpr double output_value = x.in(feet);
    EXPECT_EQ(output_value, 3.14);
}

TEST(Quantity, CanRequestOutputRepWhenCallingIn) { EXPECT_EQ(feet(3.14).in<int>(feet), 3); }

TEST(MakeQuantity, MakesQuantityInGivenUnit) {
    EXPECT_EQ(make_quantity<Feet>(1.234), feet(1.234));
    EXPECT_EQ(make_quantity<Feet>(99), feet(99));
}

TEST(Quantity, RationalConversionRecoversExactIntegerValues) {
    // This test would fail if our implementation multiplied by the float
    // representation of (1/13), instead of dividing by 13, under the hood.
    for (int i = 1; i < 100; ++i) {
        EXPECT_EQ(feet(static_cast<float>(i * 13)).in(feet * mag<13>()), i);
    }
}

TEST(QuantityMaker, CreatesAppropriateQuantityIfCalled) { EXPECT_EQ(yards(3.14).in(yards), 3.14); }

TEST(QuantityMaker, CanBeMultipliedBySingularUnitToGetMakerOfProductUnit) {
    StaticAssertTypeEq<decltype(hour * feet), QuantityMaker<UnitProductT<Feet, Hours>>>();
}

TEST(QuantityMaker, CanMultiplyByOtherMakerToGetMakerOfProductUnit) {
    StaticAssertTypeEq<decltype(hours * feet), QuantityMaker<UnitProductT<Feet, Hours>>>();
}

TEST(QuantityMaker, CanDivideBySingularUnitToGetMakerOfQuotientUnit) {
    StaticAssertTypeEq<decltype(feet / hour), QuantityMaker<UnitQuotientT<Feet, Hours>>>();
}

TEST(QuantityMaker, CanDivideByOtherMakerToGetMakerOfQuotientUnit) {
    StaticAssertTypeEq<decltype(feet / hours), QuantityMaker<UnitQuotientT<Feet, Hours>>>();
}

TEST(QuantityMaker, CanTakePowerToGetMakerOfPowerUnit) {
    StaticAssertTypeEq<decltype(pow<2>(feet)), QuantityMaker<Pow<Feet, 2>>>();
}

TEST(QuantityMaker, CanMultiplyByMagnitudeToGetMakerOfScaledUnit) {
    EXPECT_THAT((feet * mag<3>())(1.234), QuantityEquivalent(yards(1.234)));
}

TEST(QuantityMaker, CanDivideByMagnitudeToGetMakerOfDescaledUnit) {
    EXPECT_THAT((feet / mag<12>())(1.234), QuantityEquivalent(inches(1.234)));
}

TEST(QuantityMaker, CanMultiplyByMultipleSingularUnits) {
    StaticAssertTypeEq<decltype(mile * minute * days),
                       QuantityMaker<UnitProductT<Miles, Minutes, Days>>>();
}

TEST(Quantity, CanRetrieveInDifferentUnitsWithSameDimension) {
    EXPECT_EQ(feet(4).in(inches), 48);
    EXPECT_EQ(yards(4).in(inches), 144);
}

TEST(Quantity, SupportsDirectAccessWithSameUnit) {
    auto x = inches(3);
    ++(x.data_in(Inches{}));
    EXPECT_EQ(x, inches(4));
}

TEST(Quantity, SupportsDirectConstAccessWithSameUnit) {
    const auto x = meters(3.5);
    EXPECT_EQ(static_cast<const void *>(&x.data_in(Meters{})), static_cast<const void *>(&x));
}

TEST(Quantity, SupportsDirectAccessWithEquivalentUnit) {
    auto x = (kilo(feet) / hour)(3);
    ++(x.data_in(Feet{} / Milli<Hours>{}));
    EXPECT_EQ(x, (kilo(feet) / hour)(4));

    // Uncomment to test compile time failure:
    // ++(x.data_in(Feet{} / Kilo<Hours>{}));
}

TEST(Quantity, SupportsDirectConstAccessWithEquivalentUnit) {
    const auto x = (milli(meters) / minute)(3.5);
    EXPECT_EQ(static_cast<const void *>(&x.data_in(Meters{} / Kilo<Minutes>{})),
              static_cast<const void *>(&x));

    // Uncomment to test compile time failure:
    // EXPECT_EQ(static_cast<const void *>(&x.data_in(Meters{} / Mega<Minutes>{})),
    //           static_cast<const void *>(&x));
}

TEST(Quantity, SupportsDirectAccessWithQuantityMakerOfSameUnit) {
    auto x = inches(3);
    ++(x.data_in(inches));
    EXPECT_EQ(x, inches(4));
}

TEST(Quantity, SupportsDirectConstAccessWithQuantityMakerOfSameUnit) {
    const auto x = meters(3.5);
    EXPECT_EQ(static_cast<const void *>(&x.data_in(meters)), static_cast<const void *>(&x));
}

TEST(Quantity, SupportsDirectAccessWithQuantityMakerOfEquivalentUnit) {
    auto x = (kilo(feet) / hour)(3);
    ++(x.data_in(feet / milli(hour)));
    EXPECT_EQ(x, (kilo(feet) / hour)(4));

    // Uncomment to test compile time failure:
    // ++(x.data_in(feet / micro(hour)));
}

TEST(Quantity, SupportsDirectConstAccessWithQuantityMakerOfEquivalentUnit) {
    const auto x = (milli(meters) / minute)(3.5);
    EXPECT_EQ(static_cast<const void *>(&x.data_in(meters / kilo(minute))),
              static_cast<const void *>(&x));

    // Uncomment to test compile time failure:
    // EXPECT_EQ(static_cast<const void *>(&x.data_in(meters / mega(minute))),
    //           static_cast<const void *>(&x));
}

TEST(Quantity, CoerceAsWillForceLossyConversion) {
    // Truncation.
    EXPECT_THAT(inches(30).coerce_as(feet), SameTypeAndValue(feet(2)));

    // Unsigned overflow.
    ASSERT_EQ(static_cast<uint8_t>(30 * 12), 104);
    EXPECT_THAT(feet(uint8_t{30}).coerce_as(inches), SameTypeAndValue(inches(uint8_t{104})));
}

TEST(Quantity, CoerceAsExplicitRepSetsOutputType) {
    // Coerced truncation.
    EXPECT_THAT(inches(30).coerce_as<std::size_t>(feet), SameTypeAndValue(feet(std::size_t{2})));

    // Exact answer for floating point destination type.
    EXPECT_THAT(inches(30).coerce_as<float>(feet), SameTypeAndValue(feet(2.5f)));

    // Coerced unsigned overflow.
    ASSERT_EQ(static_cast<uint8_t>(30 * 12), 104);
    EXPECT_THAT(feet(30).coerce_as<uint8_t>(inches), SameTypeAndValue(inches(uint8_t{104})));
}

TEST(Quantity, CoerceInWillForceLossyConversion) {
    // Truncation.
    EXPECT_THAT(inches(30).coerce_in(feet), SameTypeAndValue(2));

    // Unsigned overflow.
    ASSERT_EQ(static_cast<uint8_t>(30 * 12), 104);
    EXPECT_THAT(feet(uint8_t{30}).coerce_in(inches), SameTypeAndValue(uint8_t{104}));
}

TEST(Quantity, CoerceInExplicitRepSetsOutputType) {
    // Coerced truncation.
    EXPECT_THAT(inches(30).coerce_in<std::size_t>(feet), SameTypeAndValue(std::size_t{2}));

    // Exact answer for floating point destination type.
    EXPECT_THAT(inches(30).coerce_in<float>(feet), SameTypeAndValue(2.5f));

    // Coerced unsigned overflow.
    ASSERT_EQ(static_cast<uint8_t>(30 * 12), 104);
    EXPECT_THAT(feet(30).coerce_in<uint8_t>(inches), SameTypeAndValue(uint8_t{104}));
}

TEST(Quantity, CanImplicitlyConvertToDifferentUnitOfSameDimension) {
    constexpr QuantityI32<Inches> x = yards(2);
    EXPECT_EQ(x.in(inches), 72);
}

TEST(Quantity, HandlesBaseDimensionsWithFractionalExponents) {
    using KiloRootFeet = decltype(root<2>(Mega<Feet>{}));
    constexpr auto x = make_quantity<KiloRootFeet>(5);
    EXPECT_EQ(x.in(root<2>(Feet{})), 5'000);
    EXPECT_EQ(x * x, mega(feet)(25));
}

TEST(Quantity, HandlesMagnitudesWithFractionalExponents) {
    using RootKiloFeet = decltype(root<2>(Kilo<Feet>{}));
    constexpr auto x = make_quantity<RootKiloFeet>(3);

    // We can retrieve the value in the same unit (regardless of the scale's fractional powers).
    EXPECT_EQ(x.in(RootKiloFeet{}), 3);

    // We can retrieve the value in a *different* unit, which *also* has fractional powers, as long
    // as their *ratio* has no fractional powers.
    EXPECT_EQ(x.in(root<2>(Milli<Feet>{})), 3'000);

    // Squaring the fractional base power gives us an exact non-fractional dimension and scale.
    EXPECT_EQ(x * x, kilo(feet)(9));
}

// A custom "Quantity-equivalent" type, whose interop with Quantity we'll provide below.
struct MyHours {
    int value;
};

// Define the equivalence of a MyHours with a Quantity<Hours, int>.
template <>
struct CorrespondingQuantity<MyHours> {
    using Unit = Hours;
    using Rep = int;

    // Support Quantity construction from Hours.
    static constexpr Rep extract_value(MyHours x) { return x.value; }

    // Support Quantity conversion to Hours.
    static constexpr MyHours construct_from_value(Rep x) { return {.value = x}; }
};

TEST(Quantity, ImplicitConstructionFromCorrespondingQuantity) {
    constexpr Quantity<Hours, int> x = MyHours{.value = 3};
    EXPECT_EQ(x, hours(3));
}

TEST(Quantity, ImplicitConstructionFromTwoHopCorrespondingQuantity) {
    constexpr Quantity<Minutes, int> x = MyHours{.value = 3};
    EXPECT_THAT(x, SameTypeAndValue(minutes(180)));
}

TEST(Quantity, ImplicitConstructionFromLvalueCorrespondingQuantity) {
    MyHours original{.value = 10};
    const Quantity<Hours, int> converted = original;
    EXPECT_EQ(converted, hours(10));
}

TEST(Quantity, ImplicitConversionToCorrespondingQuantity) {
    constexpr MyHours x = hours(46);
    EXPECT_THAT(x.value, SameTypeAndValue(46));
}

TEST(Quantity, ImplicitConstructionToTwoHopCorrespondingQuantity) {
    constexpr MyHours x = days(2);
    EXPECT_THAT(x.value, SameTypeAndValue(48));
}

TEST(Quantity, ImplicitConversionToLvalueCorrespondingQuantity) {
    auto original = hours(60);
    const MyHours converted = original;
    EXPECT_THAT(converted.value, SameTypeAndValue(60));
}

TEST(AsQuantity, DeducesCorrespondingQuantity) {
    constexpr auto q = as_quantity(MyHours{.value = 8});
    EXPECT_THAT(q, QuantityEquivalent(hours(8)));
}

TEST(Quantity, EqualityComparisonWorks) {
    constexpr auto a = feet(-4.8);
    constexpr auto b = feet(-4.8);
    EXPECT_EQ(a, b);
}

TEST(Quantity, InequalityComparisonWorks) {
    constexpr auto a = hours(3.9);
    constexpr auto b = hours(5.7);
    EXPECT_NE(a, b);
}

TEST(Quantity, RelativeComparisonsWork) {
    constexpr auto one_a = feet(1);
    constexpr auto one_b = feet(1);
    constexpr auto two = feet(2);

    EXPECT_FALSE(one_a < one_b);
    EXPECT_FALSE(one_a > one_b);
    EXPECT_TRUE(one_a <= one_b);
    EXPECT_TRUE(one_a >= one_b);

    EXPECT_TRUE(one_a < two);
    EXPECT_FALSE(one_a > two);
    EXPECT_TRUE(one_a <= two);
    EXPECT_FALSE(one_a >= two);
}

TEST(Quantity, CopyingWorksAndIsDeepCopy) {
    auto original = feet(1.5);
    const auto copy{original};
    EXPECT_EQ(original, copy);

    // To test that we're deep copying, modify the original.
    original += feet(2.5);
    EXPECT_NE(original, copy);
}

TEST(Quantity, CanAddLikeQuantities) {
    constexpr auto a = inches(1);
    constexpr auto b = inches(2);
    constexpr auto c = inches(3);
    EXPECT_EQ(a + b, c);
}

TEST(Quantity, CanSubtractLikeQuantities) {
    constexpr auto a = feet(1);
    constexpr auto b = feet(2);
    constexpr auto c = feet(3);
    EXPECT_EQ(c - b, a);
}

TEST(Quantity, AdditionAndSubtractionCommuteWithUnitTagging) {
    // The integer promotion rules of C++, retained for backwards compatibility with C, are what
    // make this test case non-trivial.  Most users neither know nor need to know about them.
    // However, it's important for our library to handle these operations in the most "C++-like"
    // way, for better or for worse.
    using NonClosedType = int8_t;
    static_assert(!std::is_same<decltype(NonClosedType{} + NonClosedType{}), NonClosedType>::value,
                  "Expected integer promotion to change type");
    static_assert(!std::is_same<decltype(NonClosedType{} - NonClosedType{}), NonClosedType>::value,
                  "Expected integer promotion to change type");

    constexpr NonClosedType a = 10;
    constexpr NonClosedType b = 5;

    EXPECT_THAT(feet(a) + feet(b), SameTypeAndValue(feet(a + b)));
    EXPECT_THAT(feet(a) - feet(b), SameTypeAndValue(feet(a - b)));
}

TEST(Quantity, CanMultiplyArbitraryQuantities) {
    constexpr auto v = (feet / hour)(2);
    constexpr auto t = hours(3);

    constexpr auto d = feet(6);

    v *t;
    EXPECT_EQ(d, v * t);
}

TEST(Quantity, ProductOfReciprocalTypesIsImplicitlyConvertibleToRawNumber) {
    constexpr int count = hours(2) * pow<-1>(hours)(3);
    EXPECT_EQ(count, 6);
}

TEST(Quantity, ScalarMultiplicationWorks) {
    constexpr auto d = feet(3);
    EXPECT_EQ(feet(6), 2 * d);
    EXPECT_EQ(feet(9), d * 3);
}

TEST(Quantity, CanDivideArbitraryQuantities) {
    constexpr auto d = feet(6.);
    constexpr auto t = hours(3.);

    constexpr auto v = (feet / hour)(2.);

    EXPECT_EQ(v, d / t);
}

TEST(Quantity, RatioOfSameTypeIsScalar) {
    constexpr auto x = yards(8.2);

    EXPECT_THAT(x / x, SameTypeAndValue(1.0));
}

TEST(Quantity, RatioOfEquivalentTypesIsScalar) {
    constexpr auto x = feet(10.0);
    constexpr auto y = (feet * mag<1>())(5.0);

    EXPECT_THAT(x / y, SameTypeAndValue(2.0));
}

TEST(Quantity, ProductOfInvertingUnitsIsScalar) {
    // We pass `UnitProductT` to this function template, which ensures that we get a `UnitProduct`
    // (note: NOT `UnitProductT`!) with the expected number of arguments.  Recall that
    // `UnitProductT` is the user-facing "unit computation" interface, and `UnitProduct` is the
    // named template which gets passed around the system.
    //
    // The point is to make sure that the product-unit of `Days` and `PerDay` does **not** reduce to
    // something trivial, like `UnitProduct<>`.  Rather, it should be its own non-trivial
    // unit---although, naturally, it must be **quantity-equivalent** to `UnitProduct<>`.
    ASSERT_EQ(num_units_in_product(UnitProductT<Days, PerDay>{}), 2);

    EXPECT_THAT(days(3) * per_day(8), SameTypeAndValue(24));
}

TEST(Quantity, ScalarDivisionWorks) {
    constexpr auto x = feet(10);
    EXPECT_EQ(x / 2, feet(5));
    EXPECT_EQ(20. / x, inverse(feet)(2.));
}

TEST(Quantity, ScalarDivisionIsConstexprCompatible) {
    constexpr auto quotient = feet(10.) / 2;
    EXPECT_EQ(quotient, feet(5.));
}

TEST(Quantity, ShortHandAdditionAssignmentWorks) {
    auto d = feet(1.25);
    d += feet(2.75);
    EXPECT_EQ(d, feet(4.));
}

TEST(Quantity, ShortHandAdditionHasReferenceCharacter) {
    auto d = feet(1);
    d += feet(1234) = feet(3);
    EXPECT_EQ(d, (feet(4)));
}

TEST(Quantity, ShortHandSubtractionAssignmentWorks) {
    auto d = feet(4.75);
    d -= feet(2.75);
    EXPECT_EQ(d, (feet(2.)));
}

TEST(Quantity, ShortHandSubtractionHasReferenceCharacter) {
    auto d = feet(4);
    d -= feet(1234) = feet(3);
    EXPECT_EQ(d, (feet(1)));
}

TEST(Quantity, ShortHandMultiplicationAssignmentWorks) {
    auto d = feet(1.25);
    d *= 2;
    EXPECT_EQ(d, (feet(2.5)));
}

TEST(Quantity, ShortHandMultiplicationHasReferenceCharacter) {
    auto d = feet(1);
    (d *= 3) = feet(19);
    EXPECT_EQ(d, (feet(19)));
}

TEST(Quantity, ShortHandDivisionAssignmentWorks) {
    auto d = feet(2.5);
    d /= 2;
    EXPECT_EQ(d, (feet(1.25)));
}

TEST(Quantity, ShortHandDivisionHasReferenceCharacter) {
    auto d = feet(19);
    (d /= 3) = feet(1);
    EXPECT_EQ(d, (feet(1)));
}

TEST(Quantity, UnaryPlusWorks) {
    // This may appear completely useless to the reader.  However, the reason this exists is because
    // some unit tests want to keep different test cases aligned, with user-defined literals, e.g.:
    //      test_my_function(+5_mpss);  // <-- needs unary plus!
    //      test_my_function(-5_mpss);
    constexpr auto d = hours(22);
    EXPECT_EQ(d, +d);
}

TEST(Quantity, UnaryMinusWorks) {
    constexpr auto d = hours(25);
    EXPECT_EQ((hours(-25)), -d);
}

TEST(Quantity, RepCastSupportsConstexprAndConst) {
    constexpr auto one_foot_double = feet(1.);
    constexpr auto one_foot_int = rep_cast<int>(one_foot_double);
    EXPECT_THAT(one_foot_int, SameTypeAndValue(feet(1)));
}

TEST(Quantity, CanCastToDifferentRep) {
    EXPECT_THAT(rep_cast<double>(hours(25)), SameTypeAndValue(hours(25.)));
    EXPECT_THAT(rep_cast<int>(inches(3.14)), SameTypeAndValue(inches(3)));
}

TEST(Quantity, UnitCastSupportsConstexprAndConst) {
    constexpr auto one_foot = feet(1);
    constexpr auto twelve_inches = one_foot.as(inches);
    EXPECT_THAT(twelve_inches, SameTypeAndValue(inches(12)));
}

TEST(Quantity, UnitCastRequiresExplicitTypeForDangerousReps) {
    // Some reps have so little range that we want to force users to be extra explicit if they cast
    // from them.  The cases in this test will try to cast a variety of types, using `.as(...)`.
    // The "safe" ones can omit the template parameter (which gives the new Rep).  The "unsafe" ones
    // will fail to compile if we try to omit it.

    // Safe instances: any floating point, or any integral type at least as big as `int`.
    EXPECT_THAT(feet(1.0).as(centi(feet)), SameTypeAndValue(centi(feet)(100.0)));
    EXPECT_THAT(feet(1.0f).as(centi(feet)), SameTypeAndValue(centi(feet)(100.0f)));
    EXPECT_THAT(feet(1).as(centi(feet)), SameTypeAndValue(centi(feet)(100)));

    // Unsafe instances: small integral types.
    //
    // To "test" these, try replacing `.coerce_as(...)` with `.as(...)`.  Make sure it fails with a
    // readable `static_assert`.
    EXPECT_THAT(feet(uint16_t{1}).coerce_as(centi(feet)),
                SameTypeAndValue(centi(feet)(uint16_t{100})));
}

TEST(Quantity, CanCastToDifferentUnit) {
    EXPECT_THAT(inches(6).coerce_as(feet), SameTypeAndValue(feet(0)));
    EXPECT_THAT(inches(6.).as(feet), SameTypeAndValue(feet(0.5)));
}

TEST(Quantity, QuantityCastSupportsConstexprAndConst) {
    constexpr auto eighteen_inches_double = inches(18.);
    constexpr auto one_foot_int = eighteen_inches_double.coerce_as<int>(feet);
    EXPECT_THAT(one_foot_int, SameTypeAndValue(feet(1)));
}

TEST(Quantity, QuantityCastAccurateForChangingUnitsAndGoingFromIntegralToFloatingPoint) {
    EXPECT_THAT(inches(3).as<double>(feet), SameTypeAndValue(feet(0.25)));
}

TEST(Quantity, QuantityCastAvoidsPreventableOverflowWhenGoingToLargerType) {
    constexpr auto lots_of_inches = inches(uint32_t{4'000'000'000});
    ASSERT_EQ(lots_of_inches.in(inches), 4'000'000'000);

    EXPECT_THAT(lots_of_inches.as<uint64_t>(nano(inches)),
                SameTypeAndValue(nano(inches)(uint64_t{4'000'000'000ULL * 1'000'000'000ULL})));
}

TEST(Quantity, QuantityCastAvoidsPreventableOverflowWhenGoingToSmallerType) {
    constexpr uint64_t would_overflow_uint32 = 9'000'000'000;
    ASSERT_GT(would_overflow_uint32, std::numeric_limits<uint32_t>::max());

    constexpr auto lots_of_nanoinches = nano(inches)(would_overflow_uint32);

    // Make sure we don't overflow in uint64_t.
    ASSERT_EQ(lots_of_nanoinches.in(nano(inches)), would_overflow_uint32);

    EXPECT_THAT(lots_of_nanoinches.coerce_as<uint32_t>(inches),
                SameTypeAndValue(inches(uint32_t{9})));
}

TEST(Quantity, CommonTypeMagnitudeEvenlyDividesBoth) {
    using A = Yards;
    using B = decltype(A{} * mag<2>() / mag<3>());
    ASSERT_FALSE(is_integer(unit_ratio(A{}, B{})));
    ASSERT_FALSE(is_integer(unit_ratio(B{}, A{})));

    constexpr auto c = std::common_type_t<QuantityD<A>, QuantityI32<B>>::unit;
    EXPECT_TRUE(is_integer(unit_ratio(A{}, c)));
    EXPECT_TRUE(is_integer(unit_ratio(B{}, c)));
}

TEST(Quantity, StdCommonTypeHasNoTypeMemberForDifferentDimensions) {
    EXPECT_FALSE((stdx::experimental::
                      is_detected<std::common_type_t, QuantityD<Hours>, QuantityD<Feet>>::value));
}

TEST(Quantity, PicksCommonTypeForRep) {
    using common_q_inches_double_float =
        std::common_type_t<Quantity<Inches, double>, Quantity<Inches, float>>;
    EXPECT_TRUE((
        AreQuantityTypesEquivalent<common_q_inches_double_float, Quantity<Inches, double>>::value));
}

TEST(Quantity, MixedUnitAdditionUsesCommonDenominator) {
    EXPECT_THAT(yards(2) + feet(3), QuantityEquivalent(feet(9)));
}

TEST(Quantity, MixedUnitSubtractionUsesCommonDenominator) {
    EXPECT_THAT(feet(1) - inches(2), QuantityEquivalent(inches(10)));
}

TEST(Quantity, MixedTypeAdditionUsesCommonRepType) {
    EXPECT_THAT(yards(1) + yards(2.), QuantityEquivalent(yards(3.)));
}

TEST(Quantity, MixedTypeSubtractionUsesCommonRepType) {
    EXPECT_THAT(feet(2.f) - feet(1.5), QuantityEquivalent(feet(0.5)));
}

TEST(Quantity, CommonTypeRespectsImplicitRepSafetyChecks) {
    // The following test should fail to compile.  Uncomment both lines to check.
    // constexpr auto feeters = QuantityMaker<CommonUnitT<Meters, Feet>>{};
    // EXPECT_NE(meters(uint16_t{53}), feeters(uint16_t{714}));

    // Why the above values?  The common unit of Meters and Feet (herein called the "Feeter") is
    // (Meters / 1250); a.k.a., (Feet / 381).  (For reference, think of it as "a little less than a
    // millimeter".)  (53 Meters) * (1250 Feeters / Meter) = 66250 Feeters.  However, the uint16_t
    // type overflows at 2^16 = 65536, so expressing this value in Feeters in a uint16_t would
    // actually yield 714.
    //
    // Thus, if our conversion to the common type did _not_ opt into our usual safety checks, then
    // the above code would convert (53 Meters) to (66250 Feeters), overflow to (714 Feeters), and
    // then compare equal to the explicitly constructed value of (714 Feeters).  Since the
    // uncommented test expects (quite reasonably!) that 53 Meters is _not_ 714 Feeters, this test
    // would fail, rather than failing to compile.
}

TEST(QuantityMaker, ProvidesAssociatedUnit) {
    StaticAssertTypeEq<AssociatedUnitT<QuantityMaker<Hours>>, Hours>();
}

TEST(WillConversionOverflow, SensitiveToTypeBoundariesForPureIntegerMultiply) {
    {
        auto will_m_to_mm_overflow_i32 = [](int32_t x) {
            return will_conversion_overflow(meters(x), milli(meters));
        };

        EXPECT_TRUE(will_m_to_mm_overflow_i32(2'147'484));
        EXPECT_FALSE(will_m_to_mm_overflow_i32(2'147'483));

        EXPECT_FALSE(will_m_to_mm_overflow_i32(-2'147'483));
        EXPECT_TRUE(will_m_to_mm_overflow_i32(-2'147'484));
    }

    {
        auto will_m_to_mm_overflow_u8 = [](uint8_t x) {
            return will_conversion_overflow(meters(x), milli(meters));
        };

        EXPECT_TRUE(will_m_to_mm_overflow_u8(255));

        EXPECT_TRUE(will_m_to_mm_overflow_u8(1));
        EXPECT_FALSE(will_m_to_mm_overflow_u8(0));
    }

    {
        auto will_m_to_mm_overflow_f = [](float x) {
            return will_conversion_overflow(meters(x), milli(meters));
        };

        EXPECT_TRUE(will_m_to_mm_overflow_f(3.41e+35f));
        EXPECT_FALSE(will_m_to_mm_overflow_f(3.40e+35f));

        EXPECT_FALSE(will_m_to_mm_overflow_f(-3.40e+35f));
        EXPECT_TRUE(will_m_to_mm_overflow_f(-3.41e+35f));
    }
}

TEST(WillConversionTruncate, UsesModForIntegerTypes) {
    auto will_in_to_ft_truncate_i32 = [](int32_t x) {
        return will_conversion_truncate(inches(x), feet);
    };

    EXPECT_TRUE(will_in_to_ft_truncate_i32(121));
    EXPECT_FALSE(will_in_to_ft_truncate_i32(120));
    EXPECT_TRUE(will_in_to_ft_truncate_i32(119));

    EXPECT_TRUE(will_in_to_ft_truncate_i32(13));
    EXPECT_FALSE(will_in_to_ft_truncate_i32(12));
    EXPECT_TRUE(will_in_to_ft_truncate_i32(11));

    EXPECT_TRUE(will_in_to_ft_truncate_i32(1));
    EXPECT_FALSE(will_in_to_ft_truncate_i32(0));
    EXPECT_TRUE(will_in_to_ft_truncate_i32(-1));

    EXPECT_TRUE(will_in_to_ft_truncate_i32(-11));
    EXPECT_FALSE(will_in_to_ft_truncate_i32(-12));
    EXPECT_TRUE(will_in_to_ft_truncate_i32(-13));

    EXPECT_TRUE(will_in_to_ft_truncate_i32(-119));
    EXPECT_FALSE(will_in_to_ft_truncate_i32(-120));
    EXPECT_TRUE(will_in_to_ft_truncate_i32(-121));
}

TEST(IsConversionLossy, CorrectlyDiscriminatesBetweenLossyAndLosslessConversions) {
    // We will check literally every representable value in the type, and make sure that the result
    // of `is_conversion_lossy()` matches perfectly with the inability to recover the initial value.
    auto test_round_trip_for_every_uint8_value = [](auto source_units, auto target_units) {
        for (int i = 0; i <= 65535; ++i) {
            const auto original = source_units(static_cast<uint16_t>(i));
            const auto converted = original.coerce_as(target_units);
            const auto round_trip = converted.coerce_as(source_units);

            const bool did_value_change = (original != round_trip);

            // Function under test:
            const bool is_lossy = is_conversion_lossy(original, target_units);

            // In order for the test to be valid, we assume the second "leg" of the round-trip
            // conversion never introduces any **new** lossiness.  (It's OK for it to be lossy, but
            // only if the first leg was also lossy.)
            //
            // Remember: it's the lossiness of the **first** conversion that we care about --- and,
            // fundamentally, "lossiness" is all about destroying the information you need to
            // recover the original value.
            if (!is_lossy) {
                const bool is_inverse_lossy = is_conversion_lossy(converted, source_units);
                ASSERT_FALSE(is_inverse_lossy);
            }

            EXPECT_EQ(is_lossy, did_value_change);
        }
    };

    // Inches-to-feet tests truncation.
    test_round_trip_for_every_uint8_value(inches, feet);

    // Feet-to-inches tests overflow.
    test_round_trip_for_every_uint8_value(feet, inches);
}

TEST(AreQuantityTypesEquivalent, RequiresSameRepAndEquivalentUnits) {
    using IntQFeet = decltype(feet(1));
    using IntQFeetTimesOne = decltype((feet * ONE)(1));

    ASSERT_FALSE((std::is_same<IntQFeet, IntQFeetTimesOne>::value));
    EXPECT_TRUE((AreQuantityTypesEquivalent<IntQFeet, IntQFeetTimesOne>::value));
}

TEST(integer_quotient, EnablesIntegerDivision) {
    constexpr auto dt = integer_quotient(meters(60), (miles / hour)(65));
    EXPECT_THAT(dt, QuantityEquivalent((hour * meters / mile)(0)));

    constexpr auto x = integer_quotient(meters(60), 31);
    EXPECT_THAT(x, SameTypeAndValue(meters(1)));

    constexpr auto freq = integer_quotient(1000, minutes(300));
    EXPECT_THAT(freq, SameTypeAndValue(inverse(minutes)(3)));
}

TEST(mod, ComputesRemainderForSameUnits) {
    constexpr auto remainder = inches(50) % inches(12);
    EXPECT_THAT(remainder, QuantityEquivalent(inches(2)));
}

TEST(mod, ReturnsCommonUnitForDifferentInputUnits) {
    EXPECT_THAT(inches(50) % feet(1), QuantityEquivalent(inches(2)));
    EXPECT_THAT(feet(4) % inches(10), QuantityEquivalent(inches(8)));
}

TEST(Zero, ComparableToArbitraryQuantities) {
    EXPECT_EQ(ZERO, meters(0));
    EXPECT_LT(ZERO, meters(1));
    EXPECT_GT(ZERO, meters(-1));

    EXPECT_EQ(ZERO, hours(0));
    EXPECT_LT(ZERO, hours(1));
    EXPECT_GT(ZERO, hours(-1));
}

TEST(Zero, AssignableToArbitraryQuantities) {
    constexpr Quantity<Inches, double> zero_inches = ZERO;
    EXPECT_THAT(zero_inches, QuantityEquivalent(inches(0.)));

    constexpr Quantity<Hours, int> zero_hours = ZERO;
    EXPECT_THAT(zero_hours, QuantityEquivalent(hours(0)));
}

}  // namespace au
