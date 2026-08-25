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
// There is deliberately no `eval()` in this file, even though it computes with Eigen expression
// templates throughout.  Every place a result is *stored* names a concrete type -- the return type
// `Position`, and the declaration of `x_new` -- and constructing those is what evaluates the
// expression.  That is exactly how the raw tab avoids the same trap.  `eval()` earns its keep when
// you would otherwise write `auto`; adding it here would suggest it is always required, which is
// a different and wrong lesson.  See `docs/discussion/concepts/eigen_safety.md`.

// --8<-- [start:frontmatter]
#include "au/au.hh"

#include <iostream>

#include "Eigen/Core"
#include "au/compatibility/eigen.hh"
#include "au/io.hh"
#include "au/units/hours.hh"
#include "au/constants/standard_gravity.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"

// This is a `.cc` file, so we import the names we use, one at a time.  See the "Namespaces and
// includes" discussion page for why we do this rather than `using namespace au;`.
using au::kilo;
using au::Meters;
using au::milli;
using au::STANDARD_GRAVITY;
using au::norm;
using au::Quantity;
using au::QuantityD;
using au::Seconds;
using au::transpose;
using au::UnitPower;
using au::UnitQuotient;
using au::symbols::h;
using au::symbols::m;
using au::symbols::s;

// Symbols for the prefixed units we use.  A prefix applier turns an existing symbol into one for
// the prefixed unit, which is the most readable of the three ways to spell this.
constexpr auto km = kilo(m);
constexpr auto ms = milli(s);

// Aliases for the vector quantity types, so the signature below reads well.  A type alias
// introduces one name we chose, so it is fine at namespace scope even in a header.
using Position = Quantity<Meters, Eigen::Vector3d>;
using Velocity = Quantity<UnitQuotient<Meters, Seconds>, Eigen::Vector3d>;
using Acceleration = Quantity<UnitQuotient<Meters, UnitPower<Seconds, 2>>, Eigen::Vector3d>;
// --8<-- [end:frontmatter]

// clang-format off
// --8<-- [start:example]
// The types state the units.  Nothing to remember; nothing to convert.
Position advanced_position(const Position &x,
                           const Velocity &v,
                           const Acceleration &a,
                           QuantityD<Seconds> dt) {
    return x + v * dt + 0.5 * a * dt * dt;
}

int main() {
    const auto x = Eigen::Vector3d{0.0, 0.0, 120.0} * m;

    // Any units of the right dimension will do: the conversion is generated at compile time.
    const auto v = Eigen::Vector3d{72.0, 0.0, 0.0} * km / h;

    const auto a = Eigen::Vector3d{0.0, 0.0, -1.0} * STANDARD_GRAVITY;


    const Position x_new          = advanced_position(x, v, a, 250.0 * ms);

    std::cout << transpose(x_new) << '\n';
    std::cout << norm(x_new) << '\n';
}
// --8<-- [end:example]
// clang-format on
