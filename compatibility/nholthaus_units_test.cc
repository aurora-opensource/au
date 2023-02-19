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

#include "au/au.hh"
#include "au/testing.hh"
#include "au/units/bars.hh"
#include "au/units/degrees.hh"
#include "au/units/feet.hh"
#include "au/units/hertz.hh"
#include "au/units/hours.hh"
#include "au/units/liters.hh"
#include "au/units/miles.hh"
#include "au/units/minutes.hh"
#include "au/units/newtons.hh"
#include "au/units/percent.hh"
#include "au/units/pounds_force.hh"
#include "au/units/revolutions.hh"
#include "au/units/unos.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "compatibility/nholthaus_units_example_usage.hh"
#include "gtest/gtest.h"

namespace {

using namespace au;

// Usage: `expect_equivalent<::units::dim::specific_unit_t>(specific_unit)`.
//
// Here, `specific_unit_t` is the _type_ from the nholthaus library which represents a quantity of
// that unit.  (The rep, which is universal in that library, is assumed to be `double`.)
// `specific_unit` is a `QuantityMaker` from the Aurora units library.
//
// The meaning of `expect_equivalent<specific_unit_t>(specific_unit)` is that:
//   - an `as_quantity` mapping exists for `specific_unit_t` to some equivalent `au` Quantity;
//   - an instance of `specific_unit_t` can be _implicitly_ converted to this mapped type;
//   - the result of this conversion is equivalent to calling the given QuantityMaker on the
//     underlying value;
//   - the mapped type can also be _implicitly_ converted back to `specific_unit_t`, and the round
//     trip is the identity.
template <typename NholthausType, typename AuUnit>
void expect_equivalent(QuantityMaker<AuUnit> expected_au_unit) {
    const auto original = NholthausType{1.2};
    const auto equivalent_to_expected_au_unit = QuantityEquivalent(expected_au_unit(1.2));

    // Check that an `as_quantity` mapping _exists_ for this nholthaus type, and that its result is
    // quantity-equivalent to the given QuantityMaker applied to the same underlying value.
    const auto converted_to_quantity = as_quantity(original);
    EXPECT_THAT(converted_to_quantity, equivalent_to_expected_au_unit);

    // Check that this nholthaus type is _implicitly_ convertible to its equivalent type, and that
    // again, the result is quantity-equivalent to the given QuantityMaker applied to the same
    // underlying value.
    const decltype(converted_to_quantity) implicitly_converted_to_quantity = original;
    EXPECT_THAT(implicitly_converted_to_quantity, equivalent_to_expected_au_unit);

    // Check that the equivalent Quantity type can be _implicitly_ converted back to the original
    // nholthaus type, and that this round trip is the identity.
    const NholthausType round_trip = implicitly_converted_to_quantity;
    EXPECT_EQ(round_trip, original);
}

TEST(NholthausTypes, MapsBaseUnitsOntoCorrectAuQuantityTypes) {
    expect_equivalent<::units::length::meter_t>(meters);
    expect_equivalent<::units::mass::kilogram_t>(kilo(grams));
    expect_equivalent<::units::time::second_t>(seconds);
    expect_equivalent<::units::angle::radian_t>(radians);
    expect_equivalent<::units::current::ampere_t>(amperes);
    expect_equivalent<::units::temperature::kelvin_t>(kelvins);
    expect_equivalent<::units::data::byte_t>(bytes);
}

TEST(NholthausTypes, MapsFactorsOfPiToCorrectMagnitude) {
    expect_equivalent<::units::angle::degree_t>(degrees);
}

TEST(NholthausTypes, MapsDerivedUnitsFoundInCodebaseCorrectly) {
    expect_equivalent<::units::acceleration::feet_per_second_squared_t>(feet / squared(second));
    expect_equivalent<::units::acceleration::meters_per_second_squared_t>(meters / squared(second));

    expect_equivalent<::units::angular_velocity::degrees_per_second_t>(degrees / second);
    expect_equivalent<::units::angular_velocity::radians_per_second_t>(radians / second);
    expect_equivalent<::units::angular_velocity::revolutions_per_minute_t>(revolutions / minute);

    expect_equivalent<::units::area::square_meter_t>(squared(meters));

    expect_equivalent<::units::concentration::percent_t>(percent);

    expect_equivalent<::units::data_transfer_rate::bytes_per_second_t>(bytes / second);
    expect_equivalent<::units::data_transfer_rate::megabytes_per_second_t>(mega(bytes) / second);

    expect_equivalent<::units::dimensionless::dimensionless_t>(unos);
    expect_equivalent<::units::dimensionless::scalar_t>(unos);

    expect_equivalent<::units::force::newton_t>(newtons);

    expect_equivalent<::units::frequency::hertz_t>(hertz);
    expect_equivalent<::units::frequency::megahertz_t>(mega(hertz));

    expect_equivalent<::units::length::centimeter_t>(centi(meters));
    expect_equivalent<::units::length::foot_t>(feet);
    expect_equivalent<::units::length::kilometer_t>(kilo(meters));
    expect_equivalent<::units::length::micrometer_t>(micro(meters));
    expect_equivalent<::units::length::mile_t>(miles);
    expect_equivalent<::units::length::millimeter_t>(milli(meters));

    expect_equivalent<::units::power::milliwatt_t>(milli(watts));
    expect_equivalent<::units::power::watt_t>(watts);

    expect_equivalent<::units::pressure::bar_t>(bars);
    expect_equivalent<::units::pressure::kilopascal_t>(kilo(pascals));
    expect_equivalent<::units::pressure::mbar_t>(milli(bars));

    expect_equivalent<::units::time::hour_t>(hours);
    expect_equivalent<::units::time::microsecond_t>(micro(seconds));
    expect_equivalent<::units::time::millisecond_t>(milli(seconds));
    expect_equivalent<::units::time::minute_t>(minutes);
    expect_equivalent<::units::time::nanosecond_t>(nano(seconds));

    expect_equivalent<::units::torque::newton_meter_t>(newton * meters);

    expect_equivalent<::units::velocity::kilometers_per_hour_t>(kilo(meters) / hour);
    expect_equivalent<::units::velocity::meters_per_second_t>(meters / second);
    expect_equivalent<::units::velocity::miles_per_hour_t>(miles / hour);

    expect_equivalent<::units::voltage::volt_t>(volts);

    expect_equivalent<::units::volume::liter_t>(liters);

    expect_equivalent<::units::luminous_intensity::candela_t>(candelas);

    expect_equivalent<::units::substance::mole_t>(moles);
}

TEST(FootPounds, EquivalentWithinAFewPartsPerBillion) {
    // Every nholthaus unit derived from the slug is wrong:
    // https://github.com/nholthaus/units/issues/289
    //
    // In our codebase, this affects foot-pounds of torque.
    const auto original = ::units::torque::foot_pound_t{1.2};
    constexpr auto foot_pounds = foot * pounds_force;

    const auto converted_to_au = as_quantity(original);
    EXPECT_THAT(converted_to_au, IsNear((foot_pounds)(1.2), nano(foot_pounds)(4)));

    const ::units::torque::foot_pound_t round_trip = converted_to_au;
    EXPECT_EQ(round_trip, original);
}

}  // namespace
