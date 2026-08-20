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

// --8<-- [start:frontmatter]
#include <iostream>

#include "au/io.hh"
#include "au/units/coulombs.hh"
#include "au/units/joules.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "examples/atomic_units/atomic_units.hh"

// This is a `.cc` file, so we import the names we use, one at a time.  Note that the header this
// includes does the opposite: it qualifies Au names with `au::` rather than importing any, because
// a namespace-scope `using` in a header would leak into every file that includes it.  See the
// "Namespaces and includes" discussion page.
using au::coulombs;
using au::joules;
using au::meters;
using au::seconds;

using atomic_units::ATOMIC_TIME;
using atomic_units::atomic_time_units;
using atomic_units::BOHR_RADIUS;
using atomic_units::e;
using atomic_units::HARTREE;
using atomic_units::hartrees;
using atomic_units::hbar;
// --8<-- [end:frontmatter]

// --8<-- [start:usage]
int main() {
    // Crossing out to SI.  The accuracy here is set by the two measured inputs, and nothing else:
    // Au composes the entire definition chain into one exact rational factor before applying it.
    //
    // Note that no unit label is written out below.  Every name in this output --- `a_0`, `m`, and
    // the rest --- is the label Au derives from the unit itself.
    std::cout << BOHR_RADIUS << " = " << BOHR_RADIUS.as<double>(meters) << '\n';
    std::cout << HARTREE << " = " << HARTREE.as<double>(joules) << '\n';
    std::cout << ATOMIC_TIME << " = " << ATOMIC_TIME.as<double>(seconds) << '\n';

    // The atomic unit of charge is the one that crosses to SI *exactly*: the elementary charge is
    // an SI-defining constant, so no measured quantity enters this conversion at all.
    std::cout << e << " = " << e.as<double>(coulombs) << '\n';

    // Staying inside the system.  This prints `1`, not `0.9999999997`, because the atomic time unit
    // is *defined* as hbar / E_h rather than pasted in as a decimal.  (This claim also has stronger
    // evidence than just the printed output: the `static_assert` in the header file.)
    std::cout << hbar << " = " << hbar.as<double>(hartrees * atomic_time_units) << '\n';
}
// --8<-- [end:usage]
