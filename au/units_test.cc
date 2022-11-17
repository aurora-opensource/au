// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#include "au/units.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

TEST(DefinedUnits, HaveExpectedLabels) {
    expect_label<Amperes>("A");
    expect_label<Bars>("bar");
    expect_label<Bits>("b");
    expect_label<Bytes>("B");
    expect_label<Celsius>("degC");
    expect_label<Coulombs>("C");
    expect_label<Degrees>("deg");
    expect_label<Fahrenheit>("F");
    expect_label<Feet>("ft");
    expect_label<Grams>("g");
    expect_label<Hertz>("Hz");
    expect_label<Hours>("h");
    expect_label<Inches>("in");
    expect_label<Joules>("J");
    expect_label<Kelvins>("K");
    expect_label<Liters>("L");
    expect_label<Meters>("m");
    expect_label<Miles>("mi");
    expect_label<Minutes>("min");
    expect_label<Newtons>("N");
    expect_label<Ohms>("ohm");
    expect_label<Pascals>("Pa");
    expect_label<Percent>("%");
    expect_label<PoundsForce>("lbf");
    expect_label<PoundsMass>("lb");
    expect_label<Radians>("rad");
    expect_label<Revolutions>("rev");
    expect_label<Seconds>("s");
    expect_label<StandardGravity>("g_0");
    expect_label<Volts>("V");
    expect_label<Watts>("W");
    expect_label<Yards>("yd");
}

TEST(DerivedSIUnits, HaveExpectedRelationships) {
    // Newton's second law.
    EXPECT_TRUE(
        are_units_quantity_equivalent(Newtons{}, Kilo<Grams>{} * Meters{} / squared(Seconds{})));

    // Ohm's law.
    EXPECT_TRUE(are_units_quantity_equivalent(Volts{}, Amperes{} * Ohms{}));

    EXPECT_TRUE(are_units_quantity_equivalent(Revolutions{}, Radians{} * mag<2>() * PI));

    EXPECT_TRUE(are_units_quantity_equivalent(Hours{}, Seconds{} * mag<3'600>()));

    EXPECT_TRUE(is_dimensionless(Hertz{} * Seconds{}));
}

TEST(Pounds, VariousUnitsOfThisNameHaveExpectedRelationshipsAndValues) {
    EXPECT_EQ(pounds_mass(100'000'000L), (kilo(grams)(45'359'237L)));
    EXPECT_EQ(pounds_force(1L), (pound_mass * standard_gravity)(1L));
}

TEST(Slugs, ExactDefinitionIsCorrect) {
    // We don't care very much about slugs per se---in fact, we don't even define it as part of the
    // library.  What we do care about is that if users _do_ create it according to its
    // authoritative definition, that it is exactly correct.  (This has implications for other units
    // downstream, such as foot-pounds of torque.)
    constexpr auto slugs = pound_force * squared(seconds) / foot;

    // The automatic conversions to the common unit here will cause overflow.  However, _unsigned_
    // integer overflow is well defined.  And if these values both overflow to the same number, it
    // adds confidence that the definition is correct.
    EXPECT_EQ(slugs(609'600'000'000ULL), kilo(grams)(8'896'443'230'521ULL));

    // These test cases check for _approximate_ correctness of the definition, within some
    // tolerance.  They complement the overflowing-integer test case just above.
    EXPECT_THAT(slugs(1.0), IsNear(kilo(grams)(14.59390293720636), nano(grams)(1)));
    EXPECT_THAT(slugs(1.0), IsNear(pounds_mass(32.174), milli(pounds_mass)(1)));
}

TEST(QuantityMakers, MakeQuantitiesConsistentWithEachOther) {
    EXPECT_EQ(meters(2), centi(meters)(200));
    EXPECT_EQ(seconds(1.5), milli(seconds)(1'500.0));
    EXPECT_EQ(centi(meters)(254), inches(100));
}

TEST(QuantityPointMakers, OffsetUnitsHaveCorrectOrigins) {
    EXPECT_EQ(milli(kelvins_pt)(273'150.0).as(celsius_pt), celsius_pt(0.0));
    EXPECT_EQ(fahrenheit_pt(32.0).as(celsius_pt), celsius_pt(0));
}

}  // namespace au
