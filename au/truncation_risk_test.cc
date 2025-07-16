// Copyright 2025 Aurora Operations, Inc.
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

#include "au/truncation_risk.hh"

#include <complex>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {
namespace {

template <typename T, typename M>
using ValueTimesIntIsNotInteger = ValueTimesRatioIsNotInteger<T, M>;

template <typename T, typename M>
using ValueDivIntIsNotInteger = ValueTimesRatioIsNotInteger<T, MagInverseT<M>>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// `WouldValueTruncate` section:

TEST(WouldValueTruncate, AlwaysFalseForNoTruncationRisk) {
    EXPECT_THAT(NoTruncationRisk<float>::would_value_truncate(3.1415f), IsFalse());
    EXPECT_THAT(NoTruncationRisk<int8_t>::would_value_truncate(int8_t{-128}), IsFalse());
}

TEST(WouldValueTruncate, OnlyFalseForZeroForValueIsNotZeroFloat) {
    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(-1.23456e7f), IsTrue());
    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(-9.87e-12), IsTrue());

    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(0.0f), IsFalse());

    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(9.87e-12), IsTrue());
    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(1.23456e7f), IsTrue());
}

TEST(WouldValueTruncate, OnlyFalseForZeroForValueIsNotZeroInt) {
    EXPECT_THAT(ValueIsNotZero<int8_t>::would_value_truncate(int8_t{-128}), IsTrue());
    EXPECT_THAT(ValueIsNotZero<int8_t>::would_value_truncate(int8_t{-1}), IsTrue());

    EXPECT_THAT(ValueIsNotZero<int8_t>::would_value_truncate(int8_t{0}), IsFalse());

    EXPECT_THAT(ValueIsNotZero<int8_t>::would_value_truncate(int8_t{1}), IsTrue());
    EXPECT_THAT(ValueIsNotZero<int8_t>::would_value_truncate(int8_t{127}), IsTrue());
}

TEST(WouldValueTruncate, ValueTimesRatioIsNotIntegerUsesModOfDenominatorForIntegerTypes) {
    using IntDiv3IsNotInteger = ValueTimesRatioIsNotInteger<int, decltype(mag<2>() / mag<3>())>;

    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(-1000000), IsTrue());
    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(-999999), IsFalse());
    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(-999998), IsTrue());

    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(-1), IsTrue());
    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(0), IsFalse());
    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(1), IsTrue());

    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(2), IsTrue());
    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(3), IsFalse());
    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(4), IsTrue());

    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(299), IsTrue());
    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(300), IsFalse());
    EXPECT_THAT(IntDiv3IsNotInteger::would_value_truncate(301), IsTrue());
}

TEST(WouldValueTruncate, ValueTimesRatioIsNotIntegerDividesByDenominatorForFloatTypes) {
    using FloatTimesOneSeventhIsNotInteger =
        ValueTimesRatioIsNotInteger<float, decltype(mag<1>() / mag<7>())>;
    for (int i = 0; i < 1000; ++i) {
        const auto f = static_cast<float>(i) * 7.f;
        EXPECT_THAT(FloatTimesOneSeventhIsNotInteger::would_value_truncate(f), IsFalse())
            << "i = " << i << ", f = " << f;

        EXPECT_THAT(FloatTimesOneSeventhIsNotInteger::would_value_truncate(f - 1.f), IsTrue());
        EXPECT_THAT(FloatTimesOneSeventhIsNotInteger::would_value_truncate(f + 1.f), IsTrue());
    }
}

TEST(WouldValueTruncate, AssumedAlwaysTrueIfCannotAssessTruncationRisk) {
    using CannotAssessRisk = CannotAssessTruncationRiskFor<int>;

    EXPECT_THAT(CannotAssessRisk::would_value_truncate(0), IsTrue());
    EXPECT_THAT(CannotAssessRisk::would_value_truncate(1), IsTrue());
    EXPECT_THAT(CannotAssessRisk::would_value_truncate(-1), IsTrue());
}

}  // namespace
}  // namespace detail
}  // namespace au
