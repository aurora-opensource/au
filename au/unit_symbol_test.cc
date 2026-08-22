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

#include "au/unit_symbol.hh"

#include <complex>
#include <type_traits>
#include <vector>

#include "au/stdx/experimental/is_detected.hh"
#include "au/testing.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "gtest/gtest.h"

using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

namespace au {
namespace {
constexpr auto m = symbol_for(meters);
constexpr auto s = symbol_for(seconds);

// Detects which types a symbol will, and will not, make a quantity out of.
template <typename T>
using TimesMeterSymbol = decltype(std::declval<T>() * m);

// A stand-in for an Eigen vector; `//au/compatibility:eigen_test` covers the real thing.
template <typename T>
struct FakeEigenVector {
    using Scalar = T;
    T data[3];
};

struct Empty {};
}  // namespace

TEST(SymbolFor, TakesUnitSlot) {
    StaticAssertTypeEq<std::decay_t<decltype(m)>, SymbolFor<Meters>>();
}

TEST(SymbolFor, CreatesQuantityFromRawNumber) {
    EXPECT_THAT(3.5f * m, SameTypeAndValue(meters(3.5f)));
}

TEST(SymbolFor, ScalesUnitsOfExistingQuantity) {
    EXPECT_THAT(meters(25.4) / s, SameTypeAndValue((meters / second)(25.4)));
}

TEST(SymbolFor, CompatibleWithUnitSlot) { EXPECT_THAT(meters(35u).in(m), SameTypeAndValue(35u)); }

TEST(SymbolFor, CanScaleByMagnitude) {
    // Yes, the identifier name is a little awkward for these symbols for anonymous scaled units.
    // But it's still important to have this functionality.
    constexpr auto u100_m = mag<100>() * m;

    EXPECT_THAT(3.5f / u100_m, SameTypeAndValue(inverse(meters * mag<100>())(3.5f)));
}

TEST(SymbolFor, MakesQuantityFromAnyValidRep) {
    EXPECT_THAT((stdx::experimental::is_detected<TimesMeterSymbol, FakeEigenVector<double>>{}),
                IsTrue());
    EXPECT_THAT((stdx::experimental::is_detected<TimesMeterSymbol, std::complex<double>>{}),
                IsTrue());
}

TEST(SymbolFor, RefusesTypesThatCannotBeReps) {
    EXPECT_THAT((stdx::experimental::is_detected<TimesMeterSymbol, Empty>{}), IsFalse());

    // A container of *quantities* can never be a valid rep: using it as one would nest units.
    EXPECT_THAT(
        (stdx::experimental::is_detected<TimesMeterSymbol, std::vector<QuantityD<Meters>>>{}),
        IsFalse());
}

TEST(SymbolFor, QuantityStillScalesRatherThanBecomingARep) {
    // If a `Quantity` also looked like a valid rep, this would be ambiguous with `ScalesQuantity`.
    EXPECT_THAT(seconds(3.0) * m, SameTypeAndValue((seconds * meters)(3.0)));
}

TEST(SymbolFor, CanApplyNamedPowerFunctions) {
    StaticAssertTypeEq<decltype(squared(m)), decltype(m * m)>();
}

}  // namespace au
