// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/unit_of_measure.hh"

namespace au {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NOTE: The unit definitions in this file are more complicated than what end users would need.
// `au` aims to be a header-only library for simplicity of distribution.  Normally, this would force
// us to include definitions for our unit labels in a separate `.cc` file, whose existence would
// complicate distribution.  The way to get around that is to turn the structs that hold the labels
// into templates.
//
// Normal users can just declare the label member directly in the unit class, and make a `.cc` file
// for the definition.
//

////////////////////////////////////////////////////////////////////////////////////////////////////
// Dimensionless units.

//
// Unos: the "Unit 1": the only Unit which is equal to its own square.
//
template <typename T>
struct UnosLabel {
    static constexpr const char label[] = "U";
};
template <typename T>
constexpr const char UnosLabel<T>::label[];
struct Unos : UnitProductT<>, UnosLabel<void> {
    using UnosLabel<void>::label;
};
constexpr auto unos = QuantityMaker<Unos>{};

//
// Percent
//
template <typename T>
struct PercentLabel {
    static constexpr const char label[] = "%";
};
template <typename T>
constexpr const char PercentLabel<T>::label[];
struct Percent : decltype(Unos{} / mag<100>()), PercentLabel<void> {
    using PercentLabel<void>::label;
};
constexpr auto percent = QuantityMaker<Percent>{};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Units defined without reference to any other Unit.

//
// Meters
//
template <typename T>
struct MetersLabel {
    static constexpr const char label[] = "m";
};
template <typename T>
constexpr const char MetersLabel<T>::label[];
struct Meters : UnitImpl<Length>, MetersLabel<void> {
    using MetersLabel<void>::label;
};
constexpr auto meter = SingularNameFor<Meters>{};
constexpr auto meters = QuantityMaker<Meters>{};
constexpr auto meters_pt = QuantityPointMaker<Meters>{};

//
// Grams
//
template <typename T>
struct GramsLabel {
    static constexpr const char label[] = "g";
};
template <typename T>
constexpr const char GramsLabel<T>::label[];
struct Grams : UnitImpl<Mass>, GramsLabel<void> {
    using GramsLabel<void>::label;
};
constexpr auto gram = SingularNameFor<Grams>{};
constexpr auto grams = QuantityMaker<Grams>{};

//
// Seconds
//
template <typename T>
struct SecondsLabel {
    static constexpr const char label[] = "s";
};
template <typename T>
constexpr const char SecondsLabel<T>::label[];
struct Seconds : UnitImpl<Time>, SecondsLabel<void> {
    using SecondsLabel<void>::label;
};
constexpr auto second = SingularNameFor<Seconds>{};
constexpr auto seconds = QuantityMaker<Seconds>{};

//
// Amperes
//
template <typename T>
struct AmperesLabel {
    static constexpr const char label[] = "A";
};
template <typename T>
constexpr const char AmperesLabel<T>::label[];
struct Amperes : UnitImpl<Current>, AmperesLabel<void> {
    using AmperesLabel<void>::label;
};
constexpr auto ampere = SingularNameFor<Amperes>{};
constexpr auto amperes = QuantityMaker<Amperes>{};

//
// Kelvins
//
template <typename T>
struct KelvinsLabel {
    static constexpr const char label[] = "K";
};
template <typename T>
constexpr const char KelvinsLabel<T>::label[];
struct Kelvins : UnitImpl<Temperature>, KelvinsLabel<void> {
    using KelvinsLabel<void>::label;
};
constexpr auto kelvin = SingularNameFor<Kelvins>{};
constexpr auto kelvins = QuantityMaker<Kelvins>{};
constexpr auto kelvins_pt = QuantityPointMaker<Kelvins>{};

//
// Radians
//
template <typename T>
struct RadiansLabel {
    static constexpr const char label[] = "rad";
};
template <typename T>
constexpr const char RadiansLabel<T>::label[];
struct Radians : UnitImpl<Angle>, RadiansLabel<void> {
    using RadiansLabel<void>::label;
};
constexpr auto radian = SingularNameFor<Radians>{};
constexpr auto radians = QuantityMaker<Radians>{};

//
// Bits
//
template <typename T>
struct BitsLabel {
    static constexpr const char label[] = "b";
};
template <typename T>
constexpr const char BitsLabel<T>::label[];
struct Bits : UnitImpl<Information>, BitsLabel<void> {
    using BitsLabel<void>::label;
};
constexpr auto bit = SingularNameFor<Bits>{};
constexpr auto bits = QuantityMaker<Bits>{};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Units defined in terms of other Units.

//
// Hertz
//
template <typename T>
struct HertzLabel {
    static constexpr const char label[] = "Hz";
};
template <typename T>
constexpr const char HertzLabel<T>::label[];
struct Hertz : UnitInverseT<Seconds>, HertzLabel<void> {
    using HertzLabel<void>::label;
};
constexpr auto hertz = QuantityMaker<Hertz>{};

//
// Newtons
//
template <typename T>
struct NewtonsLabel {
    static constexpr const char label[] = "N";
};
template <typename T>
constexpr const char NewtonsLabel<T>::label[];
struct Newtons : decltype(Kilo<Grams>{} * Meters{} / squared(Seconds{})), NewtonsLabel<void> {
    using NewtonsLabel<void>::label;
};
constexpr auto newton = SingularNameFor<Newtons>{};
constexpr auto newtons = QuantityMaker<Newtons>{};

//
// Joules
//
template <typename T>
struct JoulesLabel {
    static constexpr const char label[] = "J";
};
template <typename T>
constexpr const char JoulesLabel<T>::label[];
struct Joules : decltype(Newtons{} * Meters{}), JoulesLabel<void> {
    using JoulesLabel<void>::label;
};
constexpr auto joule = SingularNameFor<Joules>{};
constexpr auto joules = QuantityMaker<Joules>{};

//
// Pascals
//
template <typename T>
struct PascalsLabel {
    static constexpr const char label[] = "Pa";
};
template <typename T>
constexpr const char PascalsLabel<T>::label[];
struct Pascals : decltype(Newtons{} / squared(Meters{})), PascalsLabel<void> {
    using PascalsLabel<void>::label;
};
constexpr auto pascal = SingularNameFor<Pascals>{};
constexpr auto pascals = QuantityMaker<Pascals>{};
constexpr QuantityPointMaker<Pascals> pascals_pt{};

//
// Bars
//
template <typename T>
struct BarsLabel {
    static constexpr const char label[] = "bar";
};
template <typename T>
constexpr const char BarsLabel<T>::label[];
struct Bars : decltype(Kilo<Pascals>{} * mag<100>()), BarsLabel<void> {
    using BarsLabel<void>::label;
};
constexpr auto bar = SingularNameFor<Bars>{};
constexpr auto bars = QuantityMaker<Bars>{};

//
// Watts
//
template <typename T>
struct WattsLabel {
    static constexpr const char label[] = "W";
};
template <typename T>
constexpr const char WattsLabel<T>::label[];
struct Watts : decltype(Joules{} / Seconds{}), WattsLabel<void> {
    using WattsLabel<void>::label;
};
constexpr auto watt = SingularNameFor<Watts>{};
constexpr auto watts = QuantityMaker<Watts>{};

//
// Volts
//
template <typename T>
struct VoltsLabel {
    static constexpr const char label[] = "V";
};
template <typename T>
constexpr const char VoltsLabel<T>::label[];
struct Volts : decltype(Watts{} / Amperes{}), VoltsLabel<void> {
    using VoltsLabel<void>::label;
};
constexpr auto volt = SingularNameFor<Volts>{};
constexpr auto volts = QuantityMaker<Volts>{};

//
// Ohms
//
template <typename T>
struct OhmsLabel {
    static constexpr const char label[] = "ohm";
};
template <typename T>
constexpr const char OhmsLabel<T>::label[];
struct Ohms : decltype(Volts{} / Amperes{}), OhmsLabel<void> {
    using OhmsLabel<void>::label;
};
constexpr auto ohm = SingularNameFor<Ohms>{};
constexpr auto ohms = QuantityMaker<Ohms>{};

//
// Coulombs
//
template <typename T>
struct CoulombsLabel {
    static constexpr const char label[] = "C";
};
template <typename T>
constexpr const char CoulombsLabel<T>::label[];
struct Coulombs : decltype(Amperes{} * Seconds{}), CoulombsLabel<void> {
    using CoulombsLabel<void>::label;
};
constexpr auto coulomb = SingularNameFor<Coulombs>{};
constexpr auto coulombs = QuantityMaker<Coulombs>{};

//
// Degrees
//
template <typename T>
struct DegreesLabel {
    static constexpr const char label[] = "deg";
};
template <typename T>
constexpr const char DegreesLabel<T>::label[];
struct Degrees : decltype(Radians{} * PI / mag<180>()), DegreesLabel<void> {
    using DegreesLabel<void>::label;
};
constexpr auto degree = SingularNameFor<Degrees>{};
constexpr auto degrees = QuantityMaker<Degrees>{};

//
// Revolutions
//
template <typename T>
struct RevolutionsLabel {
    static constexpr const char label[] = "rev";
};
template <typename T>
constexpr const char RevolutionsLabel<T>::label[];
struct Revolutions : decltype(Degrees{} * mag<360>()), RevolutionsLabel<void> {
    using RevolutionsLabel<void>::label;
};
constexpr auto revolution = SingularNameFor<Revolutions>{};
constexpr auto revolutions = QuantityMaker<Revolutions>{};

//
// Inches
//
template <typename T>
struct InchesLabel {
    static constexpr const char label[] = "in";
};
template <typename T>
constexpr const char InchesLabel<T>::label[];
struct Inches : decltype(Centi<Meters>{} * mag<254>() / mag<100>()), InchesLabel<void> {
    using InchesLabel<void>::label;
};
constexpr auto inch = SingularNameFor<Inches>{};
constexpr auto inches = QuantityMaker<Inches>{};

//
// Feet
//
template <typename T>
struct FeetLabel {
    static constexpr const char label[] = "ft";
};
template <typename T>
constexpr const char FeetLabel<T>::label[];
struct Feet : decltype(Inches{} * mag<12>()), FeetLabel<void> {
    using FeetLabel<void>::label;
};
constexpr auto foot = SingularNameFor<Feet>{};
constexpr auto feet = QuantityMaker<Feet>{};

//
// Yards
//
template <typename T>
struct YardsLabel {
    static constexpr const char label[] = "yd";
};
template <typename T>
constexpr const char YardsLabel<T>::label[];
struct Yards : decltype(Feet{} * mag<3>()), YardsLabel<void> {
    using YardsLabel<void>::label;
};
constexpr auto yard = SingularNameFor<Yards>{};
constexpr auto yards = QuantityMaker<Yards>{};

//
// Miles
//
template <typename T>
struct MilesLabel {
    static constexpr const char label[] = "mi";
};
template <typename T>
constexpr const char MilesLabel<T>::label[];
struct Miles : decltype(Feet{} * mag<5'280>()), MilesLabel<void> {
    using MilesLabel<void>::label;
};
constexpr auto mile = SingularNameFor<Miles>{};
constexpr auto miles = QuantityMaker<Miles>{};

//
// Liters
//
template <typename T>
struct LitersLabel {
    static constexpr const char label[] = "L";
};
template <typename T>
constexpr const char LitersLabel<T>::label[];
struct Liters : decltype(cubed(Deci<Meters>{})), LitersLabel<void> {
    using LitersLabel<void>::label;
};
constexpr auto liter = SingularNameFor<Liters>{};
constexpr auto liters = QuantityMaker<Liters>{};

//
// Minutes
//
template <typename T>
struct MinutesLabel {
    static constexpr const char label[] = "min";
};
template <typename T>
constexpr const char MinutesLabel<T>::label[];
struct Minutes : decltype(Seconds{} * mag<60>()), MinutesLabel<void> {
    using MinutesLabel<void>::label;
};
constexpr auto minute = SingularNameFor<Minutes>{};
constexpr auto minutes = QuantityMaker<Minutes>{};

//
// Hours
//
template <typename T>
struct HoursLabel {
    static constexpr const char label[] = "h";
};
template <typename T>
constexpr const char HoursLabel<T>::label[];
struct Hours : decltype(Minutes{} * mag<60>()), HoursLabel<void> {
    using HoursLabel<void>::label;
};
constexpr auto hour = SingularNameFor<Hours>{};
constexpr auto hours = QuantityMaker<Hours>{};

//
// Days
//
template <typename T>
struct DaysLabel {
    static constexpr const char label[] = "d";
};
template <typename T>
constexpr const char DaysLabel<T>::label[];
struct Days : decltype(Hours{} * mag<24>()), DaysLabel<void> {
    using DaysLabel<void>::label;
};
constexpr auto day = SingularNameFor<Days>{};
constexpr auto days = QuantityMaker<Days>{};

//
// Celsius
//
template <typename T>
struct CelsiusLabel {
    static constexpr const char label[] = "degC";
};
template <typename T>
constexpr const char CelsiusLabel<T>::label[];
struct Celsius : Kelvins, CelsiusLabel<void> {
    using CelsiusLabel<void>::label;
    static constexpr auto origin() { return centi(kelvins)(273'15); }
};
constexpr auto celsius = QuantityMaker<Celsius>{};
constexpr auto celsius_pt = QuantityPointMaker<Celsius>{};

//
// Fahrenheit
//
template <typename T>
struct FahrenheitLabel {
    static constexpr const char label[] = "F";
};
template <typename T>
constexpr const char FahrenheitLabel<T>::label[];
struct Rankines : decltype(Kelvins{} * mag<5>() / mag<9>()) {};
constexpr auto rankines = QuantityMaker<Rankines>{};
struct Fahrenheit : Rankines, FahrenheitLabel<void> {
    using FahrenheitLabel<void>::label;
    static constexpr auto origin() { return centi(rankines)(459'67); }
};
constexpr auto fahrenheit = QuantityMaker<Fahrenheit>{};
constexpr auto fahrenheit_pt = QuantityPointMaker<Fahrenheit>{};

//
// Bytes
//
template <typename T>
struct BytesLabel {
    static constexpr const char label[] = "B";
};
template <typename T>
constexpr const char BytesLabel<T>::label[];
struct Bytes : decltype(Bits{} * mag<8>()), BytesLabel<void> {
    using BytesLabel<void>::label;
};
constexpr auto byte = SingularNameFor<Bytes>{};
constexpr auto bytes = QuantityMaker<Bytes>{};

//
// StandardGravity
//
template <typename T>
struct StandardGravityLabel {
    static constexpr const char label[] = "g_0";
};
template <typename T>
constexpr const char StandardGravityLabel<T>::label[];
struct StandardGravity
    : decltype((Meters{} / squared(Seconds{})) * (mag<980'665>() / mag<100'000>())),
      StandardGravityLabel<void> {
    using StandardGravityLabel<void>::label;
};
constexpr auto standard_gravity = QuantityMaker<StandardGravity>{};

//
// PoundsMass
//
template <typename T>
struct PoundsMassLabel {
    static constexpr const char label[] = "lb";
};
template <typename T>
constexpr const char PoundsMassLabel<T>::label[];
struct PoundsMass : decltype(Micro<Grams>{} * mag<453'592'370>()), PoundsMassLabel<void> {
    using PoundsMassLabel<void>::label;
};
constexpr auto pound_mass = SingularNameFor<PoundsMass>{};
constexpr auto pounds_mass = QuantityMaker<PoundsMass>{};

//
// PoundsForce
//
template <typename T>
struct PoundsForceLabel {
    static constexpr const char label[] = "lbf";
};
template <typename T>
constexpr const char PoundsForceLabel<T>::label[];
struct PoundsForce : decltype(PoundsMass{} * StandardGravity{}), PoundsForceLabel<void> {
    using PoundsForceLabel<void>::label;
};
constexpr auto pound_force = SingularNameFor<PoundsForce>{};
constexpr auto pounds_force = QuantityMaker<PoundsForce>{};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Aliases for common compound units.
//
// We use simple aliases, rather than strong types, so that we can correctly reason about products
// and powers.  For example, we want to know that `MetersPerSecondSquared` times `Seconds` yields
// `MetersPerSecond`.

using AmpHours = UnitProductT<Amperes, Hours>;
using BitsPerSecond = UnitQuotientT<Bits, Seconds>;
using BytesPerSecond = UnitQuotientT<::au::Bytes, Seconds>;
using DegreesPerMeter = UnitQuotientT<Degrees, Meters>;
using DegreesPerSecond = UnitQuotientT<Degrees, Seconds>;
using InverseMeters = UnitPowerT<Meters, -1>;
using JoulesPerRadian = UnitQuotientT<Joules, Radians>;
using KilogramsPerMeterCubed = UnitQuotientT<Kilo<Grams>, UnitPowerT<Meters, 3>>;
using KilometersPerHour = UnitQuotientT<Kilo<Meters>, Hours>;
using MetersCubed = UnitPowerT<Meters, 3>;
using MetersPerSecond = UnitQuotientT<Meters, Seconds>;
using MetersPerSecondCubed = UnitQuotientT<Meters, UnitPowerT<Seconds, 3>>;
using MetersPerSecondSquared = UnitQuotientT<Meters, UnitPowerT<Seconds, 2>>;
using MetersPerSecondToTheFourth = UnitQuotientT<Meters, UnitPowerT<Seconds, 4>>;
using MetersSquared = UnitPowerT<Meters, 2>;
using MilesPerHour = UnitQuotientT<Miles, Hours>;
using NewtonSecondsPerMeter = UnitProductT<Newtons, UnitInverseT<MetersPerSecond>>;
using NewtonsPerMeter = UnitQuotientT<Newtons, Meters>;
using NewtonsPerMeterCubed = UnitQuotientT<Newtons, UnitPowerT<Meters, 3>>;
using NewtonsPerMeterCubedRadian = UnitProductT<NewtonsPerMeterCubed, UnitInverseT<Radians>>;
using NewtonsPerRadian = UnitProductT<Newtons, UnitInverseT<Radians>>;
using RadiansPerMeter = UnitQuotientT<Radians, Meters>;
using RadiansPerMeterCubed = UnitQuotientT<Radians, UnitPowerT<Meters, 3>>;
using RadiansPerMeterSecond = UnitProductT<RadiansPerMeter, UnitInverseT<Seconds>>;
using RadiansPerMeterSecondSquared = UnitProductT<RadiansPerMeterSecond, UnitInverseT<Seconds>>;
using RadiansPerMeterSquared = UnitQuotientT<Radians, MetersSquared>;
using RadiansPerSecond = UnitQuotientT<Radians, Seconds>;
using RadiansPerSecondSquared = UnitQuotientT<Radians, UnitPowerT<Seconds, 2>>;
using RevolutionsPerMinute = UnitQuotientT<Revolutions, Minutes>;
using RevolutionsPerSecond = UnitQuotientT<Revolutions, Seconds>;
using WattHours = UnitProductT<Watts, Hours>;

}  // namespace au
