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

#include "au/quantity.hh"
#include "au/testing.hh"
#include "au/units/feet.hh"
#include "au/units/hours.hh"
#include "au/units/miles.hh"
#include "au/units/yards.hh"
#include "gtest/gtest.h"

namespace au {

// This file is for any tests which would fail if `-Wconversion` were enabled.
//
// In general, the policy for Au is that we will not _add_ `-Wconversion` errors.  We illustrate
// what this means with two examples.
//
// First, suppose a project does not use `-Wconversion`, and contains a computation with
// `-Wconversion` would not permit.  If that project "wraps" this computation with Au, then it would
// still fail with `-Wconversion`.  But Au has not "added" this failure.
//
// Second, suppose a project does use `-Wconversion`, and "wraps" one of their computations with Au.
// If this wrapping causes the project to fail the build, then Au _would_ have "added" this failure.
// This is what we want to avoid.
//
// Therefore, we confine all our tests which exercise `-Wconversion`-incompatible logic to this
// file, and we enable `-Wconversion` when running all other tests in CI.

TEST(Quantity, MultiplicationRespectUnderlyingTypes) {
    auto expect_multiplication_respects_types = [](auto t, auto u) {
        const auto t_quantity = (feet / hour)(t);
        const auto u_quantity = hours(u);

        const auto r = t * u;

        EXPECT_THAT(t_quantity * u_quantity, QuantityEquivalent(feet(r)));
        EXPECT_THAT(t_quantity * u, QuantityEquivalent((feet / hour)(r)));
        EXPECT_THAT(t * u_quantity, QuantityEquivalent(hours(r)));
    };

    expect_multiplication_respects_types(2., 3.);
    expect_multiplication_respects_types(2., 3.f);
    expect_multiplication_respects_types(2., 3);

    expect_multiplication_respects_types(2.f, 3.);
    expect_multiplication_respects_types(2.f, 3.f);
    expect_multiplication_respects_types(2.f, 3);

    expect_multiplication_respects_types(2, 3.);
    expect_multiplication_respects_types(2, 3.f);
    expect_multiplication_respects_types(2, 3);
}

TEST(Quantity, DivisionRespectsUnderlyingTypes) {
    auto expect_division_respects_types = [](auto t, auto u) {
        const auto t_quantity = miles(t);
        const auto u_quantity = hours(u);

        const auto q = t / u;

        EXPECT_THAT(t_quantity / u_quantity, QuantityEquivalent((miles / hour)(q)));
        EXPECT_THAT(t_quantity / u, QuantityEquivalent(miles(q)));
        EXPECT_THAT(t / u_quantity, QuantityEquivalent(pow<-1>(hours)(q)));
    };

    expect_division_respects_types(2., 3.);
    expect_division_respects_types(2., 3.f);
    expect_division_respects_types(2., 3);

    expect_division_respects_types(2.f, 3.);
    expect_division_respects_types(2.f, 3.f);
    expect_division_respects_types(2.f, 3);

    // We omit the integer division case, because we forbid it for Quantity.  When combined with
    // implicit conversions, it is too prone to truncate significantly and surprise users.
    expect_division_respects_types(2, 3.);
    expect_division_respects_types(2, 3.f);
    // expect_division_respects_types(2, 3);
}

TEST(QuantityShorthandMultiplicationAndDivisionAssignment, RespectUnderlyingTypes) {
    auto expect_shorthand_assignment_models_underlying_types = [](auto t, auto u) {
        auto t_quantity = yards(t);

        t_quantity *= u;
        t *= u;
        EXPECT_THAT(t_quantity.in(yards), SameTypeAndValue(t));

        t_quantity /= u;
        t /= u;
        EXPECT_THAT(t_quantity.in(yards), SameTypeAndValue(t));
    };

    expect_shorthand_assignment_models_underlying_types(2., 3.);
    expect_shorthand_assignment_models_underlying_types(2., 3.f);
    expect_shorthand_assignment_models_underlying_types(2., 3);

    expect_shorthand_assignment_models_underlying_types(2.f, 3.);
    expect_shorthand_assignment_models_underlying_types(2.f, 3.f);
    expect_shorthand_assignment_models_underlying_types(2.f, 3);

    // Although a raw integer apparently does support `operator*=(T)` for floating point `T`, we
    // don't want to allow that because it's error prone and loses precision.  Thus, we comment out
    // those test cases here.
    expect_shorthand_assignment_models_underlying_types(2, 3);
    // expect_shorthand_assignment_models_underlying_types(2, 3.f);
    // expect_shorthand_assignment_models_underlying_types(2, 3.);
}

}  // namespace au
