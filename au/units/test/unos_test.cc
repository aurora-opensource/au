// Copyright 2022 Aurora Operations, Inc.
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

#include "au/units/unos.hh"

#include "au/testing.hh"
#include "au/units/percent.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;

TEST(Unos, HasExpectedLabel) { expect_label<Unos>("U"); }

TEST(Unos, OneHundredPercent) { EXPECT_THAT(unos(2), Eq(percent(200))); }

TEST(Unos, ImplicitlyConvertToRawNumbers) {
    constexpr double x = unos(1.23);
    EXPECT_THAT(x, SameTypeAndValue(1.23));
}

}  // namespace au
