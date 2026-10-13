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

#pragma once

// Compile time benchmark: a middle-sized unit set, plus the compound unit aliases a project needs
// to describe motion --- positions, angles, and their derivatives with respect to time.
//
// Aliases like these instantiate the unit algebra, which including units alone never does: every
// unit Au ships bakes its dimension and magnitude in as a literal.  So this benchmark is sensitive
// to changes in how units combine, in a way the plain include benchmarks are not.

#include "au/prefix.hh"
#include "au/units/amperes.hh"
#include "au/units/bars.hh"
#include "au/units/bits.hh"
#include "au/units/bytes.hh"
#include "au/units/candelas.hh"
#include "au/units/celsius.hh"
#include "au/units/coulombs.hh"
#include "au/units/days.hh"
#include "au/units/degrees.hh"
#include "au/units/fahrenheit.hh"
#include "au/units/feet.hh"
#include "au/units/grams.hh"
#include "au/units/hertz.hh"
#include "au/units/hours.hh"
#include "au/units/inches.hh"
#include "au/units/joules.hh"
#include "au/units/kelvins.hh"
#include "au/units/liters.hh"
#include "au/units/lumens.hh"
#include "au/units/lux.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/minutes.hh"
#include "au/units/newtons.hh"
#include "au/units/ohms.hh"
#include "au/units/pascals.hh"
#include "au/units/percent.hh"
#include "au/units/pounds_force.hh"
#include "au/units/pounds_mass.hh"
#include "au/units/radians.hh"
#include "au/units/revolutions.hh"
#include "au/units/seconds.hh"
#include "au/units/standard_gravity.hh"
#include "au/units/steradians.hh"
#include "au/units/unos.hh"
#include "au/units/volts.hh"
#include "au/units/watts.hh"
#include "au/units/yards.hh"

namespace au {

constexpr auto centimeters = centi(meters);
constexpr auto centimeters_per_second = centi(meters / second);
constexpr auto degrees_per_second = degrees / second;
constexpr auto kilometers = kilo(meters);
constexpr auto kilometers_per_hour = kilo(meters) / hour;
constexpr auto meters_per_second = meters / second;
constexpr auto meters_per_second_cubed = meters / cubed(second);
constexpr auto meters_per_second_squared = meters / squared(second);
constexpr auto meters_per_second_to_the_fourth = meters / pow<4>(second);
constexpr auto micrometers = micro(meters);
constexpr auto microradians = micro(radians);
constexpr auto microseconds = micro(seconds);
constexpr auto miles_per_hour = miles / hour;
constexpr auto millimeters = milli(meters);
constexpr auto milliradians = milli(radians);
constexpr auto milliradians_per_second = milli(radians / second);
constexpr auto milliseconds = milli(seconds);
constexpr auto nanometers = nano(meters);
constexpr auto nanoseconds = nano(seconds);
constexpr auto per_inch = pow<-1>(inches);
constexpr auto per_meter = pow<-1>(meters);
constexpr auto radians_per_meter = radians / meter;
constexpr auto radians_per_meter_cubed = radians / cubed(meter);
constexpr auto radians_per_meter_second = radians / meter / second;
constexpr auto radians_per_meter_second_squared = radians / meter / squared(second);
constexpr auto radians_per_meter_squared = radians / squared(meter);
constexpr auto radians_per_second = radians / second;
constexpr auto radians_per_second_squared = radians / squared(second);
constexpr auto revolutions_per_minute = revolutions / minute;

}  // namespace au
