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

// --8<-- [start:frontmatter]
#include "au/au.hh"

#include <iostream>

#include "au/io.hh"
#include "au/prefix.hh"
#include "au/units/volts.hh"

// This is a `.cc` file, so we import the names we use, one at a time.  See the "Namespaces and
// includes" discussion page for why we do this rather than `using namespace au;`.
using au::mag;
using au::Milli;
using au::milli;
using au::QuantityMaker;
using au::TRUNCATION_RISK;
using au::Volts;
using au::volts;
// --8<-- [end:frontmatter]

// clang-format off
// --8<-- [start:example]
// A 12-bit analog-to-digital converter (ADC) reports `counts` out of 4095,
// spanning a 3300 millivolt (mV) reference.  That ratio is exact, so name it a unit.
struct AdcCounts : decltype(Milli<Volts>{} * mag<3300>() / mag<4095>()) {
    static constexpr const char label[] = "counts";
};
constexpr const char AdcCounts::label[];  // C++14 needs this; C++17 does not.
constexpr auto adc_counts = QuantityMaker<AdcCounts>{};

int main() {
    const auto v = adc_counts(2048);
    // Exact rational conversion, applied once.  Without `ignore(...)`, this would not compile.
    std::cout << v.as(milli(volts), ignore(TRUNCATION_RISK)) << std::endl;
}
// --8<-- [end:example]
// clang-format on
