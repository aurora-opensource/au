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

constexpr auto PI = Magnitude<Pi>{};

template <typename T, typename M>
using ValueTimesIntIsNotInteger = ValueTimesRatioIsNotInteger<T, M>;

template <typename T, typename M>
using ValueDivIntIsNotInteger = ValueTimesRatioIsNotInteger<T, MagInverseT<M>>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// `TruncationRiskFor` section:

//
// `StaticCast` section:
//

TEST(TruncationRiskFor, StaticCastArithToArithFloatAssumedToNeverTruncate) {
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<int16_t, float>>, NoTruncationRisk<int16_t>>();

    StaticAssertTypeEq<TruncationRiskFor<StaticCast<uint16_t, double>>,
                       NoTruncationRisk<uint16_t>>();

    StaticAssertTypeEq<TruncationRiskFor<StaticCast<float, double>>, NoTruncationRisk<float>>();

    StaticAssertTypeEq<TruncationRiskFor<StaticCast<long double, float>>,
                       NoTruncationRisk<long double>>();
}

TEST(TruncationRiskFor, StaticCastArithIntToArithAssumedToNeverTruncate) {
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<int, int16_t>>, NoTruncationRisk<int>>();
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<int16_t, int>>, NoTruncationRisk<int16_t>>();
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<int, int8_t>>, NoTruncationRisk<int>>();
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<uint8_t, int>>, NoTruncationRisk<uint8_t>>();
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<uint64_t, float>>,
                       NoTruncationRisk<uint64_t>>();
}

TEST(TruncationRiskFor, StaticCastArithFloatToArithIntRisksNonIntegerValues) {
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<float, int>>, ValueIsNotInteger<float>>();

    StaticAssertTypeEq<TruncationRiskFor<StaticCast<double, uint16_t>>,
                       ValueIsNotInteger<double>>();
}

//
// `MultiplyTypeBy` section:
//

TEST(TruncationRiskFor, MultiplyAnythingByIntNeverTruncates) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<int16_t, decltype(mag<2>())>>,
                       NoTruncationRisk<int16_t>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<uint32_t, decltype(-mag<1>())>>,
                       NoTruncationRisk<uint32_t>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<float, decltype(mag<3000>())>>,
                       NoTruncationRisk<float>>();
}

TEST(TruncationRiskFor, MultiplyFloatByInverseIntNeverTruncates) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<float, decltype(mag<1>() / mag<2>())>>,
                       NoTruncationRisk<float>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<double, decltype(mag<1>() / mag<3456>())>>,
                       NoTruncationRisk<double>>();
}

TEST(TruncationRiskFor, MultiplyIntByIrrationalTruncatesForValueIsNotZero) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<uint8_t, decltype(PI / mag<180>())>>,
                       ValueIsNotZero<uint8_t>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<int32_t, decltype(sqrt(mag<2>()))>>,
                       ValueIsNotZero<int32_t>>();
}

TEST(TruncationRiskFor, MultiplyFloatByIrrationalNeverTruncates) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<float, decltype(PI / mag<180>())>>,
                       NoTruncationRisk<float>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<double, decltype(sqrt(mag<2>()))>>,
                       NoTruncationRisk<double>>();
}

//
// `DivideTypeByInteger` section:
//

TEST(TruncationRiskFor, DividingFloatByIntNeverTruncates) {
    StaticAssertTypeEq<TruncationRiskFor<DivideTypeByInteger<float, decltype(mag<2>())>>,
                       NoTruncationRisk<float>>();

    StaticAssertTypeEq<TruncationRiskFor<DivideTypeByInteger<double, decltype(mag<3456>())>>,
                       NoTruncationRisk<double>>();
}

TEST(TruncationRiskFor, DivideIntByIntTruncatesNumbersNotDivisibleByIt) {
    StaticAssertTypeEq<TruncationRiskFor<DivideTypeByInteger<int16_t, decltype(mag<3>())>>,
                       ValueDivIntIsNotInteger<int16_t, decltype(mag<3>())>>();

    StaticAssertTypeEq<TruncationRiskFor<DivideTypeByInteger<uint32_t, decltype(mag<432>())>>,
                       ValueDivIntIsNotInteger<uint32_t, decltype(mag<432>())>>();
}

TEST(TruncationRiskFor, DivideIntByTooBigIntGivesValuesIsNotZero) {
    StaticAssertTypeEq<TruncationRiskFor<DivideTypeByInteger<uint8_t, decltype(mag<256>())>>,
                       ValueIsNotZero<uint8_t>>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `WouldValueTruncate` section:

TEST(WouldValueTruncate, AlwaysFalseForNoTruncationRisk) {
    EXPECT_THAT(NoTruncationRisk<float>::would_value_truncate(3.1415f), IsFalse());
    EXPECT_THAT(NoTruncationRisk<int8_t>::would_value_truncate(int8_t{-128}), IsFalse());
}

TEST(WouldValueTruncate, OnlyFalseForZeroForValueIsNotZeroFloat) {
    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(-1.23456e7f), IsTrue());
    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(-9.87e-12f), IsTrue());

    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(0.0f), IsFalse());

    EXPECT_THAT(ValueIsNotZero<float>::would_value_truncate(9.87e-12f), IsTrue());
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UpdateRisk` section:

TEST(UpdateRisk, StaticCastFloatToFloatPreservesRiskButChangesInputType) {
    StaticAssertTypeEq<UpdateRisk<StaticCast<float, double>, NoTruncationRisk<double>>,
                       NoTruncationRisk<float>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<double, float>, ValueIsNotInteger<float>>,
                       ValueIsNotInteger<double>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<long double, float>, ValueIsNotZero<float>>,
                       ValueIsNotZero<long double>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<float, long double>,
                                  ValueDivIntIsNotInteger<long double, decltype(mag<3>())>>,
                       ValueDivIntIsNotInteger<float, decltype(mag<3>())>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<long double, double>,
                                  ValueTimesIntIsNotInteger<double, decltype(mag<4>())>>,
                       ValueTimesIntIsNotInteger<long double, decltype(mag<4>())>>();

    StaticAssertTypeEq<
        UpdateRisk<StaticCast<float, double>,
                   ValueTimesRatioIsNotInteger<double, decltype(mag<3>() / mag<4>())>>,
        ValueTimesRatioIsNotInteger<float, decltype(mag<3>() / mag<4>())>>();
}

TEST(UpdateRisk, AnyOpBeforeCannotAssessTruncationRiskUpdatesInputType) {
    StaticAssertTypeEq<UpdateRisk<StaticCast<float, int>, CannotAssessTruncationRiskFor<int>>,
                       CannotAssessTruncationRiskFor<float>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<int, decltype(mag<2>())>, CannotAssessTruncationRiskFor<int>>,
        CannotAssessTruncationRiskFor<int>>();

    StaticAssertTypeEq<UpdateRisk<DivideTypeByInteger<float, decltype(mag<3>())>,
                                  CannotAssessTruncationRiskFor<float>>,
                       CannotAssessTruncationRiskFor<float>>();
}

TEST(UpdateRisk, AnyOpBeforeValueIsNotZeroIsValueIsNotZero) {
    StaticAssertTypeEq<UpdateRisk<StaticCast<float, int>, ValueIsNotZero<int>>,
                       ValueIsNotZero<float>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<int16_t, double>, ValueIsNotZero<double>>,
                       ValueIsNotZero<int16_t>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<uint16_t, int32_t>, ValueIsNotZero<int32_t>>,
                       ValueIsNotZero<uint16_t>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<double, float>, ValueIsNotZero<float>>,
                       ValueIsNotZero<double>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<int, decltype(mag<2>())>, ValueIsNotZero<int>>,
                       ValueIsNotZero<int>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<float, decltype(mag<2>())>, ValueIsNotZero<float>>,
                       ValueIsNotZero<float>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<int, decltype(mag<1>() / mag<4>())>, ValueIsNotZero<int>>,
        ValueIsNotZero<int>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<float, decltype(mag<1>() / mag<4>())>, ValueIsNotZero<float>>,
        ValueIsNotZero<float>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<int, decltype(PI / mag<180>())>, ValueIsNotZero<int>>,
        ValueIsNotZero<int>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<float, decltype(PI / mag<180>())>, ValueIsNotZero<float>>,
        ValueIsNotZero<float>>();

    StaticAssertTypeEq<
        UpdateRisk<DivideTypeByInteger<int, decltype(mag<2>())>, ValueIsNotZero<int>>,
        ValueIsNotZero<int>>();

    StaticAssertTypeEq<
        UpdateRisk<DivideTypeByInteger<float, decltype(mag<2>())>, ValueIsNotZero<float>>,
        ValueIsNotZero<float>>();
}

TEST(UpdateRisk, AnyOpBeforeNoTruncationRiskIsNoTruncationRisk) {
    StaticAssertTypeEq<UpdateRisk<StaticCast<float, int>, NoTruncationRisk<int>>,
                       NoTruncationRisk<float>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<int16_t, double>, NoTruncationRisk<double>>,
                       NoTruncationRisk<int16_t>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<uint16_t, int32_t>, NoTruncationRisk<int32_t>>,
                       NoTruncationRisk<uint16_t>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<double, float>, NoTruncationRisk<float>>,
                       NoTruncationRisk<double>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<int, decltype(mag<2>())>, NoTruncationRisk<int>>,
                       NoTruncationRisk<int>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<float, decltype(mag<2>())>, NoTruncationRisk<float>>,
        NoTruncationRisk<float>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<int, decltype(mag<1>() / mag<4>())>, NoTruncationRisk<int>>,
        NoTruncationRisk<int>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<float, decltype(mag<1>() / mag<4>())>, NoTruncationRisk<float>>,
        NoTruncationRisk<float>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<int, decltype(PI / mag<180>())>, NoTruncationRisk<int>>,
        NoTruncationRisk<int>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<float, decltype(PI / mag<180>())>, NoTruncationRisk<float>>,
        NoTruncationRisk<float>>();

    StaticAssertTypeEq<
        UpdateRisk<DivideTypeByInteger<int, decltype(mag<2>())>, NoTruncationRisk<int>>,
        NoTruncationRisk<int>>();

    StaticAssertTypeEq<
        UpdateRisk<DivideTypeByInteger<float, decltype(mag<2>())>, NoTruncationRisk<float>>,
        NoTruncationRisk<float>>();
}

TEST(UpdateRisk, StaticCastIntToFloatBeforeValueIsNotIntegerIsNoTruncationRisk) {
    StaticAssertTypeEq<UpdateRisk<StaticCast<int16_t, float>, ValueIsNotInteger<float>>,
                       NoTruncationRisk<int16_t>>();
}

TEST(UpdateRisk, MultiplyFloatByIntBeforeValueIsNotIntegerIsValuesNotSomeIntegerDividedBy) {
    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<float, decltype(mag<6>())>, ValueIsNotInteger<float>>,
        ValueTimesIntIsNotInteger<float, decltype(mag<6>())>>();
}

TEST(UpdateRisk, DivideFloatByIntBeforeValueIsNotIntegerIsValuesNotSomeIntegerTimes) {
    StaticAssertTypeEq<
        UpdateRisk<DivideTypeByInteger<float, decltype(mag<6>())>, ValueIsNotInteger<float>>,
        ValueDivIntIsNotInteger<float, decltype(mag<6>())>>();
}

TEST(UpdateRisk, MultiplyFloatByIrrationalBeforeValueTimesRatioIsNotIntegerIsValueIsNotZero) {
    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<float, decltype(PI / mag<180>())>, ValueIsNotInteger<float>>,
        ValueIsNotZero<float>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<double, decltype(sqrt(mag<2>()))>,
                                  ValueTimesIntIsNotInteger<double, decltype(mag<8>())>>,
                       ValueIsNotZero<double>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<long double, decltype(PI / mag<180>())>,
                                  ValueDivIntIsNotInteger<long double, decltype(mag<123>())>>,
                       ValueIsNotZero<long double>>();

    StaticAssertTypeEq<
        UpdateRisk<MultiplyTypeBy<double, decltype(sqrt(mag<2>()))>,
                   ValueTimesRatioIsNotInteger<double, decltype(mag<3>() / mag<5>())>>,
        ValueIsNotZero<double>>();
}

TEST(UpdateRisk, StaticCastIntToFloatBeforeValueTimesIntIsNotIntegerIsNoTruncationRisk) {
    StaticAssertTypeEq<UpdateRisk<StaticCast<int16_t, float>,
                                  ValueTimesIntIsNotInteger<float, decltype(mag<23>())>>,
                       NoTruncationRisk<int16_t>>();
}

TEST(UpdateRisk, MultiplyFloatByIntBeforeValueTimesIntIsNotIntegerMultipliesFactor) {
    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<float, decltype(mag<6>())>,
                                  ValueTimesIntIsNotInteger<float, decltype(mag<9>())>>,
                       ValueTimesIntIsNotInteger<float, decltype(mag<54>())>>();
}

TEST(UpdateRisk, DivideFloatByIntBeforeValueTimesIntIsNotIntegerMakesFraction) {
    StaticAssertTypeEq<UpdateRisk<DivideTypeByInteger<float, decltype(mag<6>())>,
                                  ValueTimesIntIsNotInteger<float, decltype(mag<7>())>>,
                       ValueTimesRatioIsNotInteger<float, decltype(mag<7>() / mag<6>())>>();
}

TEST(UpdateRisk, StaticCastBeforeValueDivIntIsNotIntegerGivesSame) {
    StaticAssertTypeEq<
        UpdateRisk<StaticCast<int16_t, float>, ValueDivIntIsNotInteger<float, decltype(mag<23>())>>,
        ValueDivIntIsNotInteger<int16_t, decltype(mag<23>())>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<uint32_t, int64_t>,
                                  ValueDivIntIsNotInteger<int64_t, decltype(mag<123>())>>,
                       ValueDivIntIsNotInteger<uint32_t, decltype(mag<123>())>>();

    StaticAssertTypeEq<
        UpdateRisk<StaticCast<double, float>, ValueDivIntIsNotInteger<float, decltype(mag<456>())>>,
        ValueDivIntIsNotInteger<double, decltype(mag<456>())>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<float, int16_t>,
                                  ValueDivIntIsNotInteger<int16_t, decltype(mag<789>())>>,
                       ValueDivIntIsNotInteger<float, decltype(mag<789>())>>();
}

TEST(UpdateRisk, MultiplyIntByIntBeforeValueDivIntIsNotIntegerUpdatesRatio) {
    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<int16_t, decltype(mag<3>())>,
                                  ValueDivIntIsNotInteger<int16_t, decltype(mag<5>())>>,
                       ValueTimesRatioIsNotInteger<int16_t, decltype(mag<3>() / mag<5>())>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<uint32_t, decltype(mag<8>())>,
                                  ValueDivIntIsNotInteger<uint32_t, decltype(mag<12>())>>,
                       ValueTimesRatioIsNotInteger<uint32_t, decltype(mag<2>() / mag<3>())>>();
}

TEST(UpdateRisk,
     MultiplyIntByExactMultipleOfDivisorBeforeValueDivIntIsNotIntegerIsNoTruncationRisk) {
    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<uint64_t, decltype(mag<7>())>,
                                  ValueDivIntIsNotInteger<uint64_t, decltype(mag<7>())>>,
                       NoTruncationRisk<uint64_t>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<int, decltype(mag<10>())>,
                                  ValueDivIntIsNotInteger<int, decltype(mag<2>())>>,
                       NoTruncationRisk<int>>();
}

TEST(UpdateRisk, MultiplyFloatByIntBeforeValueDivIntIsNotIntegerUpdatesRatio) {
    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<float, decltype(mag<3>())>,
                                  ValueDivIntIsNotInteger<float, decltype(mag<5>())>>,
                       ValueTimesRatioIsNotInteger<float, decltype(mag<3>() / mag<5>())>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<double, decltype(mag<8>())>,
                                  ValueDivIntIsNotInteger<double, decltype(mag<12>())>>,
                       ValueTimesRatioIsNotInteger<double, decltype(mag<2>() / mag<3>())>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<long double, decltype(mag<7>())>,
                                  ValueDivIntIsNotInteger<long double, decltype(mag<7>())>>,
                       ValueIsNotInteger<long double>>();

    StaticAssertTypeEq<UpdateRisk<MultiplyTypeBy<float, decltype(mag<10>())>,
                                  ValueDivIntIsNotInteger<float, decltype(mag<2>())>>,
                       ValueTimesIntIsNotInteger<float, decltype(mag<5>())>>();
}

}  // namespace
}  // namespace detail
}  // namespace au
