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

#include "au.hh"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// NOTE:  Any changes below this line should be applied to au/error_examples.cc also!
//
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace au {

////////////////////////////////////////////////////////////////////////////////////////////////////
// SECTION: Private constructor

void set_timeout(QuantityD<Seconds> dt);

/*
void example_private_constructor() {
    // A (BROKEN): passing raw number where duration expected.
    set_timeout(0.5);

    // B (BROKEN): calling Quantity constructor directly.
    constexpr QuantityD<Meters> length{5.5};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SECTION: Dangerous conversion

void example_dangerous_conversion() {
    // A (BROKEN): inexact conversion.
    inches(24).as(feet);

    // B (BROKEN): overflow risk.
    giga(hertz)(1).as(hertz);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SECTION: No type named 'type' in 'std::common_type'

void example_no_type_named_type_in_std_common_type() {
    // (BROKEN): different dimensions.
    meters(1) + seconds(1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SECTION: Integer division forbidden

void example_integer_division_forbidden() {
    // (BROKEN): gives (60 / 65) == 0 before conversion!
    QuantityD<Seconds> t = meters(60) / (miles / hour)(65);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SECTION: Dangerous inversion

void example_dangerous_inversion() {
    // (BROKEN): excessive truncation risk.
    inverse_as(seconds, hertz(5));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SECTION: Deduced conflicting types

*/
void example_deduced_conflicting_types() {
    // (BROKEN): Initializer list confused by Hz and s^(-1).
    for (const auto &frequency : {
             hertz(1.0),
             (1 / seconds(2.0)),
         }) {
        // ...
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// SECTION:

/*
struct Quarterfeet : decltype(Feet{} / mag<4>()) {};
constexpr auto quarterfeet = QuantityMaker<Quarterfeet>{};

struct Trinches : decltype(Inches{} * mag<3>()) {};
constexpr auto trinches = QuantityMaker<Trinches>{};

void example_() {
    // (BROKEN): Can't tell how to order Quarterfeet and Trinches when forming common type
    if (quarterfeet(10) == trinches(10)) {
        // ...
    }
}
*/

}  // namespace au
