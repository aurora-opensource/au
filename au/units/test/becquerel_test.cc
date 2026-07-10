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

#include "au/units/becquerel.hh"

#include "au/constant.hh"
#include "au/magnitude.hh"
#include "au/testing.hh"
#include "au/units/literals/becquerel.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

TEST(Becquerel, HasExpectedLabel) { expect_label<Becquerel>("Bq"); }

TEST(Becquerel, EquivalentToInverseSeconds) {
    EXPECT_THAT(becquerel(4.0), QuantityEquivalent(inverse(seconds)(4.0)));
}

TEST(Becquerel, HasExpectedSymbol) {
    using symbols::Bq;
    EXPECT_THAT(5 * Bq, SameTypeAndValue(becquerel(5)));
}

TEST(Becquerel, LiteralMakesEquivalentConstant) {
    using namespace ::au::au_literals;
    using literals::operator""_Bq;
    EXPECT_THAT(1.28e-4_Bq, SameTypeAndValue(make_constant(becquerel * 1.28e-4_mag)));
}

}  // namespace au
