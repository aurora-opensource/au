// Copyright 2026 Aurora Operations, Inc.
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

module;

#define AU_INLINE_VARIABLES

#include "au/au.hh"
#include "au/io.hh"
#include "au/view.hh"
#include "au/std_format.hh"

#include "au/constants/avogadro_constant.hh"
#include "au/constants/boltzmann_constant.hh"
#include "au/constants/cesium_hyperfine_transition_frequency.hh"
#include "au/constants/elementary_charge.hh"
#include "au/constants/luminous_efficacy_540_terahertz.hh"
#include "au/constants/planck_constant.hh"
#include "au/constants/reduced_planck_constant.hh"
#include "au/constants/speed_of_light.hh"
#include "au/constants/standard_gravity.hh"

#include "au/units/amperes.hh"
#include "au/units/arcminutes.hh"
#include "au/units/arcseconds.hh"
#include "au/units/astronomical_units.hh"
#include "au/units/bars.hh"
#include "au/units/becquerel.hh"
#include "au/units/bits.hh"
#include "au/units/bytes.hh"
#include "au/units/candelas.hh"
#include "au/units/celsius.hh"
#include "au/units/coulombs.hh"
#include "au/units/days.hh"
#include "au/units/degrees.hh"
#include "au/units/fahrenheit.hh"
#include "au/units/farads.hh"
#include "au/units/fathoms.hh"
#include "au/units/feet.hh"
#include "au/units/football_fields.hh"
#include "au/units/furlongs.hh"
#include "au/units/grams.hh"
#include "au/units/grays.hh"
#include "au/units/henries.hh"
#include "au/units/hertz.hh"
#include "au/units/hours.hh"
#include "au/units/inches.hh"
#include "au/units/joules.hh"
#include "au/units/katals.hh"
#include "au/units/kelvins.hh"
#include "au/units/knots.hh"
#include "au/units/liters.hh"
#include "au/units/lumens.hh"
#include "au/units/lux.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/minutes.hh"
#include "au/units/moles.hh"
#include "au/units/nautical_miles.hh"
#include "au/units/newtons.hh"
#include "au/units/ohms.hh"
#include "au/units/pascals.hh"
#include "au/units/percent.hh"
#include "au/units/pounds_force.hh"
#include "au/units/pounds_mass.hh"
#include "au/units/radians.hh"
#include "au/units/rankine.hh"
#include "au/units/revolutions.hh"
#include "au/units/seconds.hh"
#include "au/units/siemens.hh"
#include "au/units/slugs.hh"
#include "au/units/standard_gravity.hh"
#include "au/units/steradians.hh"
#include "au/units/tesla.hh"
#include "au/units/unos.hh"
#include "au/units/us_gallons.hh"
#include "au/units/us_pints.hh"
#include "au/units/us_quarts.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "au/units/webers.hh"
#include "au/units/yards.hh"

#include "au/units/literals/amperes.hh"
#include "au/units/literals/arcminutes.hh"
#include "au/units/literals/arcseconds.hh"
#include "au/units/literals/astronomical_units.hh"
#include "au/units/literals/bars.hh"
#include "au/units/literals/becquerel.hh"
#include "au/units/literals/bits.hh"
#include "au/units/literals/bytes.hh"
#include "au/units/literals/candelas.hh"
#include "au/units/literals/celsius.hh"
#include "au/units/literals/coulombs.hh"
#include "au/units/literals/days.hh"
#include "au/units/literals/degrees.hh"
#include "au/units/literals/fahrenheit.hh"
#include "au/units/literals/farads.hh"
#include "au/units/literals/fathoms.hh"
#include "au/units/literals/feet.hh"
#include "au/units/literals/football_fields.hh"
#include "au/units/literals/furlongs.hh"
#include "au/units/literals/grams.hh"
#include "au/units/literals/grays.hh"
#include "au/units/literals/henries.hh"
#include "au/units/literals/hertz.hh"
#include "au/units/literals/hours.hh"
#include "au/units/literals/inches.hh"
#include "au/units/literals/joules.hh"
#include "au/units/literals/katals.hh"
#include "au/units/literals/kelvins.hh"
#include "au/units/literals/knots.hh"
#include "au/units/literals/liters.hh"
#include "au/units/literals/lumens.hh"
#include "au/units/literals/lux.hh"
#include "au/units/literals/meters.hh"
#include "au/units/literals/miles.hh"
#include "au/units/literals/minutes.hh"
#include "au/units/literals/moles.hh"
#include "au/units/literals/nautical_miles.hh"
#include "au/units/literals/newtons.hh"
#include "au/units/literals/ohms.hh"
#include "au/units/literals/pascals.hh"
#include "au/units/literals/percent.hh"
#include "au/units/literals/pounds_force.hh"
#include "au/units/literals/pounds_mass.hh"
#include "au/units/literals/radians.hh"
#include "au/units/literals/rankine.hh"
#include "au/units/literals/revolutions.hh"
#include "au/units/literals/seconds.hh"
#include "au/units/literals/siemens.hh"
#include "au/units/literals/slugs.hh"
#include "au/units/literals/standard_gravity.hh"
#include "au/units/literals/steradians.hh"
#include "au/units/literals/tesla.hh"
#include "au/units/literals/us_gallons.hh"
#include "au/units/literals/us_pints.hh"
#include "au/units/literals/us_quarts.hh"
#include "au/units/literals/volts.hh"
#include "au/units/literals/watts.hh"
#include "au/units/literals/webers.hh"
#include "au/units/literals/yards.hh"

export module au;

export namespace au {
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Core library

    // `au/zero.hh`
    using au::ValueOfZero;
    using au::ZERO;
    using au::Zero;

    // `au/rep.hh`
    using au::IsProductValidRep;
    using au::IsQuotientValidRep;
    using au::IsValidRep;
    using au::SameRep;

    // `au/packs.hh`
    using au::AreAllPowersNonzero;
    using au::AreBasesInOrder;
    using au::AreElementsInOrder;
    using au::AsPack;
    using au::AsPackImpl;
    using au::AsPackT;
    using au::Base;
    using au::BaseImpl;
    using au::BaseT;
    using au::Dimension;
    using au::Exp;
    using au::ExpImpl;
    using au::ExpT;
    using au::FlatDedupedTypeList;
    using au::FlatDedupedTypeListImpl;
    using au::FlatDedupedTypeListT;
    using au::InOrderFor;
    using au::InsertUsingOrderingFor;
    using au::InsertUsingOrderingForImpl;
    using au::InStandardPackOrder;
    using au::IsValidPack;
    using au::LexicographicTotalOrdering;
    using au::Magnitude;
    using au::PackInverse;
    using au::PackPower;
    using au::PackPowerImpl;
    using au::PackProduct;
    using au::PackProductImpl;
    using au::PackQuotient;
    using au::Pow;
    using au::RatioPow;
    using au::SortAs;
    using au::SortAsImpl;
    using au::UnpackIfSolo;
    using au::UnpackIfSoloImpl;
    using au::UnpackIfSoloT;

    // `au/power_aliases.hh`
    using au::cbrt;
    using au::Cbrt;
    using au::Cubed;
    using au::cubed;
    using au::Inverse;
    using au::inverse;
    using au::pow;
    using au::root;
    using au::sqrt;
    using au::Sqrt;
    using au::Squared;
    using au::squared;

    // `au/dimension.hh`
    using au::AmountOfSubstance;
    using au::Angle;
    using au::CommonDimension;
    using au::CommonDimensionImpl;
    using au::CommonDimensionT;
    using au::Current;
    using au::DimInverse;
    using au::DimInverseT;
    using au::DimPower;
    using au::DimPowerT;
    using au::DimProduct;
    using au::DimProductT;
    using au::DimQuotient;
    using au::DimQuotientT;
    using au::Information;
    using au::Length;
    using au::LuminousIntensity;
    using au::Mass;
    using au::operator*;
    using au::operator/;
    using au::Temperature;
    using au::Time;

    namespace base_dim {
        using au::base_dim::AmountOfSubstance;
        using au::base_dim::Angle;
        using au::base_dim::BaseDimension;
        using au::base_dim::Current;
        using au::base_dim::Information;
        using au::base_dim::Length;
        using au::base_dim::LuminousIntensity;
        using au::base_dim::Mass;
        using au::base_dim::OrderByBaseDimIndex;
        using au::base_dim::Temperature;
        using au::base_dim::Time;
    }  // namespace base_dim

    // `au/magnitude.hh`
    using au::abs;
    using au::Abs;
    using au::AbsImpl;
    using au::common_magnitude;
    using au::CommonMagnitude;
    using au::CommonMagnitudeImpl;
    using au::CommonMagnitudeT;
    using au::denominator;
    using au::Denominator;
    using au::DenominatorT;
    using au::get_value;
    using au::integer_part;
    using au::IntegerPart;
    using au::IntegerPartImpl;
    using au::IntegerPartOfBasePower;
    using au::IntegerPartT;
    using au::is_integer;
    using au::is_positive;
    using au::is_rational;
    using au::IsInteger;
    using au::IsPositive;
    using au::IsRational;
    using au::mag;
    using au::mag_ceil;
    using au::mag_floor;
    using au::mag_round;
    using au::mag_trunc;
    using au::MagInverse;
    using au::MagInverseT;
    using au::MagnitudeLabel;
    using au::MagPower;
    using au::MagPowerT;
    using au::MagProduct;
    using au::MagProductT;
    using au::MagQuotient;
    using au::MagQuotientT;
    using au::MagSum;
    using au::MagSumImpl;
    using au::Negative;
    using au::Numerator;
    using au::numerator;
    using au::NumeratorImpl;
    using au::NumeratorT;
    using au::ONE;
    using au::operator!=;
    using au::operator%;
    using au::operator+;
    using au::operator-;
    using au::operator<;
    using au::operator<=;
    using au::operator==;
    using au::operator>;
    using au::operator>=;
    using au::Pi;
    using au::Prime;
    using au::representable_in;
    using au::ScalarOf;
    using au::ScalarOfTrait;
    using au::Sign;
    using au::sign;
    using au::SignImpl;

    // `au/conversion_policy.hh`
    using au::ALL_RISKS;
    using au::ConstructionPolicy;
    using au::implicit_rep_permitted_from_source_to_target;
    using au::ImplicitRepPermitted;
    using au::IsConversionRiskPolicy;
    using au::OVERFLOW_RISK;
    using au::TRUNCATION_RISK;

    // `au/unit_of_measure.hh`
    using au::AppropriateAssociatedUnit;
    using au::AppropriateAssociatedUnitImpl;
    using au::AppropriateCommonUnit;
    using au::AppropriateCommonUnitImpl;
    using au::are_units_point_equivalent;
    using au::are_units_quantity_equivalent;
    using au::AreUnitsPointEquivalent;
    using au::AreUnitsQuantityEquivalent;
    using au::associated_unit;
    using au::associated_unit_for_points;
    using au::AssociatedUnit;
    using au::AssociatedUnitForPoints;
    using au::AssociatedUnitForPointsImpl;
    using au::AssociatedUnitForPointsT;
    using au::AssociatedUnitImpl;
    using au::AssociatedUnitT;
    using au::common_point_unit;
    using au::common_unit;
    using au::CommonAmongUnitsAndOriginDisplacements;
    using au::CommonPointUnit;
    using au::CommonPointUnitPack;
    using au::CommonPointUnitT;
    using au::CommonUnit;
    using au::CommonUnitLabel;
    using au::CommonUnitPack;
    using au::CommonUnitT;
    using au::ComputeCommonPointUnit;
    using au::ComputeCommonPointUnitImpl;
    using au::ComputeCommonUnit;
    using au::ComputeCommonUnitImpl;
    using au::ComputeScaledUnit;
    using au::ComputeScaledUnitImpl;
    using au::DefaultUnitLabel;
    using au::fits_in_unit_slot;
    using au::has_same_dimension;
    using au::HasSameDimension;
    using au::is_dimensionless;
    using au::is_forward_declared_unit_valid;
    using au::is_unit;
    using au::is_unitless_unit;
    using au::IsDimensionless;
    using au::IsNonzero;
    using au::IsUnit;
    using au::IsUnitlessUnit;
    using au::make_common;
    using au::make_common_point;
    using au::ScaledUnit;
    using au::SingularNameFor;
    using au::unit_ratio;
    using au::unit_sign;
    using au::UnitImpl;
    using au::UnitInverse;
    using au::UnitInverseT;
    using au::UnitLabel;
    using au::UnitList;
    using au::UnitOrderTiebreaker;
    using au::UnitPower;
    using au::UnitPowerT;
    using au::UnitProduct;
    using au::UnitProductPack;
    using au::UnitProductT;
    using au::UnitQuotient;
    using au::UnitQuotientT;
    using au::UnitRatio;
    using au::UnitRatioImpl;
    using au::UnitRatioT;
    using au::UnitSign;
    using au::UnitSum;
    using au::UnitSumPack;

    // `au/unit_symbol.hh`
    using au::symbol_for;
    using au::SymbolFor;

    // `au/quantity.hh`
    using au::AlwaysDivisibleQuantity;
    using au::AreQuantityTypesEquivalent;
    using au::as_quantity;
    using au::as_raw_number;
    using au::CommonQuantity;
    using au::CorrespondingQuantity;
    using au::divide_using_common_unit;
    using au::is_conversion_lossy;
    using au::make_quantity;
    using au::Quantity;
    using au::QuantityFormatter;
    using au::QuantityMaker;
    using au::rep_cast;
    using au::unblock_int_div;
    using au::will_conversion_overflow;
    using au::will_conversion_truncate;

    // `au/quantity_point.hh`
    using au::AreQuantityPointTypesEquivalent;
    using au::make_quantity_point;
    using au::origin_displacement;
    using au::OriginDisplacement;
    using au::QuantityPoint;
    using au::QuantityPointFormatter;
    using au::QuantityPointMaker;

    // `au/constant.hh`
    using au::Constant;
    using au::make_constant;

    // `au/chrono_interop.hh`
    using au::as_chrono_duration;
    using au::SpecialCorrespondingQuantity;

    // `au/math.hh`
    using au::arccos;
    using au::arcsin;
    using au::arctan;
    using au::arctan2;
    using au::ceil_as;
    using au::ceil_in;
    using au::clamp;
    using au::copysign;
    using au::cos;
    using au::floor_as;
    using au::floor_in;
    using au::fmod;
    using au::hypot;
    using au::int_ceil_as;
    using au::int_ceil_in;
    using au::int_floor_as;
    using au::int_floor_in;
    using au::int_pow;
    using au::int_round_as;
    using au::int_round_in;
    using au::inverse_as;
    using au::inverse_in;
    using au::isinf;
    using au::isnan;
    using au::lerp;
    using au::max;
    using au::mean;
    using au::min;
    using au::remainder;
    using au::round_as;
    using au::round_in;
    using au::sin;
    using au::tan;

    // `au/prefix.hh`
    using au::Atto;
    using au::atto;
    using au::Centi;
    using au::centi;
    using au::deci;
    using au::Deci;
    using au::Deka;
    using au::deka;
    using au::Exa;
    using au::exa;
    using au::exbi;
    using au::Exbi;
    using au::Femto;
    using au::femto;
    using au::gibi;
    using au::Gibi;
    using au::Giga;
    using au::giga;
    using au::hecto;
    using au::Hecto;
    using au::Kibi;
    using au::kibi;
    using au::kilo;
    using au::Kilo;
    using au::Mebi;
    using au::mebi;
    using au::mega;
    using au::Mega;
    using au::Micro;
    using au::micro;
    using au::Milli;
    using au::milli;
    using au::nano;
    using au::Nano;
    using au::Pebi;
    using au::pebi;
    using au::peta;
    using au::Peta;
    using au::pico;
    using au::Pico;
    using au::PrefixApplier;
    using au::Quecto;
    using au::quecto;
    using au::Quetta;
    using au::quetta;
    using au::ronna;
    using au::Ronna;
    using au::Ronto;
    using au::ronto;
    using au::tebi;
    using au::Tebi;
    using au::Tera;
    using au::tera;
    using au::yobi;
    using au::Yobi;
    using au::Yocto;
    using au::yocto;
    using au::yotta;
    using au::Yotta;
    using au::zebi;
    using au::Zebi;
    using au::zepto;
    using au::Zepto;
    using au::Zetta;
    using au::zetta;

    // `au/io.hh`
    using au::operator<<;

    // `au/view.hh`
    using au::IsView;
    using au::make_view;
    using au::View;

    // `au/fwd.hh`
    using au::ForwardDeclareUnitPow;
    using au::ForwardDeclareUnitProduct;
    using au::QuantityD;
    using au::QuantityF;
    using au::QuantityI;
    using au::QuantityI16;
    using au::QuantityI32;
    using au::QuantityI64;
    using au::QuantityI8;
    using au::QuantityPointD;
    using au::QuantityPointF;
    using au::QuantityPointI;
    using au::QuantityPointI16;
    using au::QuantityPointI32;
    using au::QuantityPointI64;
    using au::QuantityPointI8;
    using au::QuantityPointU;
    using au::QuantityPointU16;
    using au::QuantityPointU32;
    using au::QuantityPointU64;
    using au::QuantityPointU8;
    using au::QuantityU;
    using au::QuantityU16;
    using au::QuantityU32;
    using au::QuantityU64;
    using au::QuantityU8;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Constants (`au/constants/*.hh`)

    using au::AVOGADRO_CONSTANT;
    using au::BOLTZMANN_CONSTANT;
    using au::CESIUM_HYPERFINE_TRANSITION_FREQUENCY;
    using au::ELEMENTARY_CHARGE;
    using au::LUMINOUS_EFFICACY_540_TERAHERTZ;
    using au::PLANCK_CONSTANT;
    using au::REDUCED_PLANCK_CONSTANT;
    using au::SPEED_OF_LIGHT;
    using au::STANDARD_GRAVITY;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Units (`au/units/*.hh`).

    // `au/units/amperes.hh`
    using au::ampere;
    using au::amperes;
    using au::Amperes;
    using au::AmperesLabel;

    // `au/units/arcminutes.hh`
    using au::arcminute;
    using au::arcminutes;
    using au::Arcminutes;
    using au::ArcminutesLabel;

    // `au/units/arcseconds.hh`
    using au::arcsecond;
    using au::arcseconds;
    using au::Arcseconds;
    using au::ArcsecondsLabel;

    // `au/units/astronomical_units.hh`
    using au::astronomical_unit;
    using au::astronomical_units;
    using au::AstronomicalUnits;
    using au::AstronomicalUnitsLabel;

    // `au/units/bars.hh`
    using au::bar;
    using au::bars;
    using au::Bars;
    using au::BarsLabel;

    // `au/units/becquerel.hh`
    using au::Becquerel;
    using au::becquerel;
    using au::BecquerelLabel;

    // `au/units/bits.hh`
    using au::bit;
    using au::bits;
    using au::Bits;
    using au::BitsLabel;

    // `au/units/bytes.hh`
    using au::byte;
    using au::bytes;
    using au::Bytes;
    using au::BytesLabel;

    // `au/units/candelas.hh`
    using au::candela;
    using au::candelas;
    using au::Candelas;
    using au::CandelasLabel;

    // `au/units/celsius.hh`
    using au::Celsius;
    using au::celsius_pt;
    using au::celsius_qty;
    using au::CelsiusLabel;

    // `au/units/coulombs.hh`
    using au::coulomb;
    using au::Coulombs;
    using au::coulombs;
    using au::CoulombsLabel;

    // `au/units/days.hh`
    using au::day;
    using au::Days;
    using au::days;
    using au::DaysLabel;

    // `au/units/degrees.hh`
    using au::degree;
    using au::Degrees;
    using au::degrees;
    using au::DegreesLabel;

    // `au/units/fahrenheit.hh`
    using au::Fahrenheit;
    using au::fahrenheit_pt;
    using au::fahrenheit_qty;
    using au::FahrenheitLabel;

    // `au/units/farads.hh`
    using au::farad;
    using au::Farads;
    using au::farads;
    using au::FaradsLabel;

    // `au/units/fathoms.hh`
    using au::fathom;
    using au::Fathoms;
    using au::fathoms;
    using au::FathomsLabel;

    // `au/units/feet.hh`
    using au::Feet;
    using au::feet;
    using au::FeetLabel;
    using au::foot;

    // `au/units/football_fields.hh`
    using au::football_field;
    using au::football_fields;
    using au::FootballFields;
    using au::FootballFieldsLabel;

    // `au/units/furlongs.hh`
    using au::furlong;
    using au::furlongs;
    using au::Furlongs;
    using au::FurlongsLabel;

    // `au/units/grams.hh`
    using au::gram;
    using au::Grams;
    using au::grams;
    using au::GramsLabel;

    // `au/units/grays.hh`
    using au::gray;
    using au::Grays;
    using au::grays;
    using au::GraysLabel;

    // `au/units/henries.hh`
    using au::Henries;
    using au::henries;
    using au::HenriesLabel;
    using au::henry;

    // `au/units/hertz.hh`
    using au::hertz;
    using au::Hertz;
    using au::HertzLabel;

    // `au/units/hours.hh`
    using au::hour;
    using au::hours;
    using au::Hours;
    using au::HoursLabel;

    // `au/units/inches.hh`
    using au::inch;
    using au::inches;
    using au::Inches;
    using au::InchesLabel;

    // `au/units/joules.hh`
    using au::joule;
    using au::Joules;
    using au::joules;
    using au::JoulesLabel;

    // `au/units/katals.hh`
    using au::katal;
    using au::katals;
    using au::Katals;
    using au::KatalsLabel;

    // `au/units/kelvins.hh`
    using au::kelvin;
    using au::kelvins;
    using au::Kelvins;
    using au::kelvins_pt;
    using au::KelvinsLabel;

    // `au/units/knots.hh`
    using au::knot;
    using au::Knots;
    using au::knots;
    using au::KnotsLabel;

    // `au/units/liters.hh`
    using au::liter;
    using au::liters;
    using au::Liters;
    using au::LitersLabel;

    // `au/units/lumens.hh`
    using au::lumen;
    using au::lumens;
    using au::Lumens;
    using au::LumensLabel;

    // `au/units/lux.hh`
    using au::lux;
    using au::Lux;
    using au::LuxLabel;

    // `au/units/meters.hh`
    using au::meter;
    using au::Meters;
    using au::meters;
    using au::meters_pt;
    using au::MetersLabel;

    // `au/units/miles.hh`
    using au::mile;
    using au::miles;
    using au::Miles;
    using au::MilesLabel;

    // `au/units/minutes.hh`
    using au::minute;
    using au::minutes;
    using au::Minutes;
    using au::MinutesLabel;

    // `au/units/moles.hh`
    using au::mole;
    using au::moles;
    using au::Moles;
    using au::MolesLabel;

    // `au/units/nautical_miles.hh`
    using au::nautical_mile;
    using au::nautical_miles;
    using au::NauticalMiles;
    using au::NauticalMilesLabel;

    // `au/units/newtons.hh`
    using au::newton;
    using au::newtons;
    using au::Newtons;
    using au::NewtonsLabel;

    // `au/units/ohms.hh`
    using au::ohm;
    using au::Ohms;
    using au::ohms;
    using au::OhmsLabel;

    // `au/units/pascals.hh`
    using au::pascals;
    using au::Pascals;
    using au::pascals_pt;
    using au::PascalsLabel;

    // `au/units/percent.hh`
    using au::percent;
    using au::Percent;
    using au::PercentLabel;

    // `au/units/pounds_force.hh`
    using au::pound_force;
    using au::pounds_force;
    using au::PoundsForce;
    using au::PoundsForceLabel;

    // `au/units/pounds_mass.hh`
    using au::pound_mass;
    using au::pounds_mass;
    using au::PoundsMass;
    using au::PoundsMassLabel;

    // `au/units/radians.hh`
    using au::radian;
    using au::radians;
    using au::Radians;
    using au::RadiansLabel;

    // `au/units/rankine.hh`
    using au::Rankine;
    using au::rankine;
    using au::rankine_pt;
    using au::RankineLabel;

    // `au/units/revolutions.hh`
    using au::revolution;
    using au::revolutions;
    using au::Revolutions;
    using au::RevolutionsLabel;

    // `au/units/seconds.hh`
    using au::second;
    using au::seconds;
    using au::Seconds;
    using au::SecondsLabel;

    // `au/units/siemens.hh`
    using au::siemen;
    using au::Siemens;
    using au::siemens;
    using au::SiemensLabel;

    // `au/units/slugs.hh`
    using au::slug;
    using au::slugs;
    using au::Slugs;
    using au::SlugsLabel;

    // `au/units/standard_gravity.hh`
    using au::standard_gravity;
    using au::StandardGravity;
    using au::StandardGravityLabel;

    // `au/units/steradians.hh`
    using au::steradian;
    using au::steradians;
    using au::Steradians;
    using au::SteradiansLabel;

    // `au/units/tesla.hh`
    using au::Tesla;
    using au::tesla;
    using au::TeslaLabel;

    // `au/units/unos.hh`
    using au::Unos;
    using au::unos;
    using au::UnosLabel;

    // `au/units/us_gallons.hh`
    using au::us_gallon;
    using au::us_gallons;
    using au::USGallons;
    using au::USGallonsLabel;

    // `au/units/us_pints.hh`
    using au::us_pint;
    using au::us_pints;
    using au::USPints;
    using au::USPintsLabel;

    // `au/units/us_quarts.hh`
    using au::us_quart;
    using au::us_quarts;
    using au::USQuarts;
    using au::USQuartsLabel;

    // `au/units/volts.hh`
    using au::volt;
    using au::volts;
    using au::Volts;
    using au::VoltsLabel;

    // `au/units/watts.hh`
    using au::watt;
    using au::Watts;
    using au::watts;
    using au::WattsLabel;

    // `au/units/webers.hh`
    using au::weber;
    using au::Webers;
    using au::webers;
    using au::WebersLabel;

    // `au/units/yards.hh`
    using au::yard;
    using au::Yards;
    using au::yards;
    using au::YardsLabel;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Unit symbols (`au/units/*.hh`)

    namespace symbols {
        using au::symbols::A;
        using au::symbols::am;
        using au::symbols::as;
        using au::symbols::AU;
        using au::symbols::b;
        using au::symbols::B;
        using au::symbols::bar;
        using au::symbols::Bq;
        using au::symbols::C;
        using au::symbols::cd;
        using au::symbols::d;
        using au::symbols::deg;
        using au::symbols::degC_qty;
        using au::symbols::degF_qty;
        using au::symbols::degR;
        using au::symbols::F;
        using au::symbols::ft;
        using au::symbols::ftbl_fld;
        using au::symbols::ftm;
        using au::symbols::fur;
        using au::symbols::g;
        using au::symbols::g_0;
        using au::symbols::Gy;
        using au::symbols::h;
        using au::symbols::H;
        using au::symbols::Hz;
        using au::symbols::in;
        using au::symbols::J;
        using au::symbols::K;
        using au::symbols::kat;
        using au::symbols::kn;
        using au::symbols::L;
        using au::symbols::lb;
        using au::symbols::lbf;
        using au::symbols::lm;
        using au::symbols::lx;
        using au::symbols::m;
        using au::symbols::mi;
        using au::symbols::min;
        using au::symbols::mol;
        using au::symbols::N;
        using au::symbols::nmi;
        using au::symbols::ohm;
        using au::symbols::Pa;
        using au::symbols::pct;
        using au::symbols::rad;
        using au::symbols::rev;
        using au::symbols::s;
        using au::symbols::S;
        using au::symbols::slug;
        using au::symbols::sr;
        using au::symbols::T;
        using au::symbols::US_gal;
        using au::symbols::US_pt;
        using au::symbols::US_qt;
        using au::symbols::V;
        using au::symbols::W;
        using au::symbols::Wb;
        using au::symbols::yd;
    }  // namespace symbols

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // Unit literals (`au/magnitude.hh` and `au/units/literals/*.hh`)

    namespace au_literals {
        using au::au_literals::operator""_A;
        using au::au_literals::operator""_am;
        using au::au_literals::operator""_as;
        using au::au_literals::operator""_AU;
        using au::au_literals::operator""_b;
        using au::au_literals::operator""_B;
        using au::au_literals::operator""_bar;
        using au::au_literals::operator""_Bq;
        using au::au_literals::operator""_C;
        using au::au_literals::operator""_cd;
        using au::au_literals::operator""_d;
        using au::au_literals::operator""_deg;
        using au::au_literals::operator""_degC_qty;
        using au::au_literals::operator""_degF_qty;
        using au::au_literals::operator""_degR;
        using au::au_literals::operator""_F;
        using au::au_literals::operator""_ft;
        using au::au_literals::operator""_ftbl_fld;
        using au::au_literals::operator""_ftm;
        using au::au_literals::operator""_fur;
        using au::au_literals::operator""_g;
        using au::au_literals::operator""_g_0;
        using au::au_literals::operator""_Gy;
        using au::au_literals::operator""_H;
        using au::au_literals::operator""_h;
        using au::au_literals::operator""_Hz;
        using au::au_literals::operator""_in;
        using au::au_literals::operator""_J;
        using au::au_literals::operator""_K;
        using au::au_literals::operator""_kat;
        using au::au_literals::operator""_kn;
        using au::au_literals::operator""_L;
        using au::au_literals::operator""_lb;
        using au::au_literals::operator""_lbf;
        using au::au_literals::operator""_lm;
        using au::au_literals::operator""_lx;
        using au::au_literals::operator""_m;
        using au::au_literals::operator""_mag;
        using au::au_literals::operator""_mi;
        using au::au_literals::operator""_min;
        using au::au_literals::operator""_mol;
        using au::au_literals::operator""_N;
        using au::au_literals::operator""_nmi;
        using au::au_literals::operator""_ohm;
        using au::au_literals::operator""_Pa;
        using au::au_literals::operator""_pct;
        using au::au_literals::operator""_rad;
        using au::au_literals::operator""_rev;
        using au::au_literals::operator""_S;
        using au::au_literals::operator""_s;
        using au::au_literals::operator""_slug;
        using au::au_literals::operator""_sr;
        using au::au_literals::operator""_T;
        using au::au_literals::operator""_US_gal;
        using au::au_literals::operator""_US_pt;
        using au::au_literals::operator""_US_qt;
        using au::au_literals::operator""_V;
        using au::au_literals::operator""_W;
        using au::au_literals::operator""_Wb;
        using au::au_literals::operator""_yd;
    }  // namespace au_literals
}  // namespace au
