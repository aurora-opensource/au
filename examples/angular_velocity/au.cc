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

// NOTE TO EDITORS: this file is line-aligned with `raw.cc`.  See the note in that file.
//
// The unparenthesized `v * rad / r` below is deliberate.  Writing `v * (rad / r)` asks for the
// ratio as a value, and since `rad` carries no number, that means materializing the reciprocal
// `1 / r`: a division *and* a multiplication, where `v * rad / r` costs only the division.  The doc
// page claims two floating point instructions against the raw version's three.  Parenthesizing
// makes that claim false while leaving the output, the line count, and every test unchanged --- so
// nothing but this note will catch it.

// --8<-- [start:frontmatter]
#include "au/au.hh"

#include <iostream>

#include "au/io.hh"
#include "au/units/meters.hh"
#include "au/units/minutes.hh"
#include "au/units/radians.hh"
#include "au/units/revolutions.hh"
#include "au/units/seconds.hh"

// This is a `.cc` file, so we import the names we use, one at a time.  See the "Namespaces and
// includes" discussion page for why we do this rather than `using namespace au;`.
using au::Meters;
using au::meters;
using au::milli;
using au::minute;
using au::Minutes;
using au::QuantityF;
using au::Revolutions;
using au::revolutions;
using au::second;
using au::Seconds;
using au::UnitQuotient;
using au::symbols::rad;

// Aliases for the compound units, so the signature below reads well.  A type alias introduces one
// name we chose, so it is fine at namespace scope even in a header.
using RevolutionsPerMinute = UnitQuotient<Revolutions, Minutes>;
using MetersPerSecond = UnitQuotient<Meters, Seconds>;
// --8<-- [end:frontmatter]

// clang-format off
// --8<-- [start:example]
// --8<-- [start:headline]
// The types state the units.  Nothing to remember; nothing to convert.
QuantityF<RevolutionsPerMinute> wheel_rpm(QuantityF<MetersPerSecond> v, QuantityF<Meters> r) {
    return v * rad / r;
}
// --8<-- [end:headline]

int main() {

    const auto omega = wheel_rpm((meters / second)(15.0f), milli(meters)(350.0f));
    std::cout << omega.as(revolutions / minute) << std::endl;
}
// --8<-- [end:example]
// clang-format on
