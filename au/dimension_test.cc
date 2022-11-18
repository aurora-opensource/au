// Copyright 2022 Aurora Operations, Inc.

#include "au/dimension.hh"

#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {

using Speed = DimQuotientT<Length, Time>;
using Accel = DimQuotientT<Speed, Time>;

TEST(Dimension, AllProvidedBaseDimensionsAreCompatible) {
    // This tests the strict total ordering for all recognized base dimensions.  It makes sure they
    // are all distinguishable and orderable, and thus can be combined in a single dimension.
    (void)DimProductT<Length, Mass, Time, Current, Temperature, Angle, Information>{};
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
