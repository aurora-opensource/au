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

#include "au/dimension.hh"

#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {

using Speed = DimQuotientT<Length, Time>;
using Accel = DimQuotientT<Speed, Time>;

TEST(Dimension, AllProvidedBaseDimensionsAreCompatible) {
    // This tests the strict total ordering for all recognized base dimensions.  It makes sure they
    // are all distinguishable and orderable, and thus can be combined in a single dimension.
    (void)DimProductT<Length,
                      Mass,
                      Time,
                      Current,
                      Temperature,
                      AmountOfSubstance,
                      LuminousIntensity,
                      Angle,
                      Information>{};
}

TEST(Dimension, ProductAndQuotientBehaveAsExpected) {
    StaticAssertTypeEq<DimProductT<Speed, Time>, Length>();

    StaticAssertTypeEq<DimQuotientT<DimProductT<Length, Time>, Length>, Time>();
}

TEST(Dimension, PowersBehaveAsExpected) {
    StaticAssertTypeEq<DimQuotientT<DimPowerT<Speed, 2>, Length>, Accel>();

    StaticAssertTypeEq<DimProductT<Accel, DimPowerT<Time, 2>>, Length>();
}

}  // namespace au
