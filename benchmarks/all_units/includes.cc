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

// Compile time benchmark: every unit Au ships.  The upper bound on what including units can cost
// you.
//
// The list has to name every header in `au/units/` exactly once, which is a thing a human editing
// it will eventually get wrong.  `all_units_listing_test` checks it against the directory, so a
// new unit that never makes it into this file turns that test red.

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
