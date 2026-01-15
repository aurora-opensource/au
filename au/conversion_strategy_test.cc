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

#include "au/conversion_strategy.hh"

#include <complex>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {
namespace detail {

using ::testing::IsFalse;
using ::testing::StaticAssertTypeEq;

template <typename T, typename U, typename Factor>
struct CastStrategyIndependentConversionForRepsAndFactorImpl {
    using ViaStaticCast = ConversionForRepsAndFactor<UseStaticCast, T, U, Factor>;
    using ViaImplicitConversion = ConversionForRepsAndFactor<UseImplicitConversion, T, U, Factor>;
    static_assert(std::is_same<ViaStaticCast, ViaImplicitConversion>::value,
                  "Must be independent of cast strategy to use this");
    using type = ViaStaticCast;
};
template <typename T, typename U, typename Factor>
using CastStrategyIndependentConversionForRepsAndFactor =
    typename CastStrategyIndependentConversionForRepsAndFactorImpl<T, U, Factor>::type;

TEST(ConversionForRepsAndFactor, SameRepAndNonPromotingTypeIsJustMultiplyByDefault) {
    StaticAssertTypeEq<
        CastStrategyIndependentConversionForRepsAndFactor<int32_t, int32_t, decltype(mag<15>())>,
        MultiplyTypeBy<int32_t, decltype(mag<15>())>>();
}

TEST(ConversionForRepsAndFactor, SameRepAndNonPromotingTypeWithInverseIntegerIsJustDivideBy) {
    StaticAssertTypeEq<
        CastStrategyIndependentConversionForRepsAndFactor<int32_t,
                                                          int32_t,
                                                          decltype(mag<1>() / mag<16>())>,
        DivideTypeByInteger<int32_t, decltype(mag<16>())>>();

    StaticAssertTypeEq<
        CastStrategyIndependentConversionForRepsAndFactor<double,
                                                          double,
                                                          decltype(mag<1>() / mag<3456>())>,
        DivideTypeByInteger<double, decltype(mag<3456>())>>();
}

TEST(ConversionForRepsAndFactor, SameRepForPromotingTypeHasCastAtBeginningAndEnd) {
    using T = uint16_t;
    using Promoted = PromotedType<T>;
    ASSERT_THAT((std::is_same<T, Promoted>::value), IsFalse());

    StaticAssertTypeEq<ConversionForRepsAndFactor<UseStaticCast, T, T, decltype(mag<15>())>,
                       OpSequence<StaticCast<T, Promoted>,
                                  MultiplyTypeBy<Promoted, decltype(mag<15>())>,
                                  StaticCast<Promoted, T>>>();
    StaticAssertTypeEq<ConversionForRepsAndFactor<UseImplicitConversion, T, T, decltype(mag<15>())>,
                       OpSequence<ImplicitConversion<T, Promoted>,
                                  MultiplyTypeBy<Promoted, decltype(mag<15>())>,
                                  ImplicitConversion<Promoted, T>>>();
}

TEST(ConversionForRepsAndFactor, ApplyingNontrivialRationalToIntegralTypeIsMultiplyThenDivide) {
    StaticAssertTypeEq<
        CastStrategyIndependentConversionForRepsAndFactor<uint64_t,
                                                          uint64_t,
                                                          decltype(mag<3>() / mag<4>())>,
        OpSequence<MultiplyTypeBy<uint64_t, decltype(mag<3>())>,
                   DivideTypeByInteger<uint64_t, decltype(mag<4>())>>>();
}

TEST(ConversionForRepsAndFactor, ApplyingNontrivialRationalToFloatingPointIsSingleMultiply) {
    StaticAssertTypeEq<
        CastStrategyIndependentConversionForRepsAndFactor<double,
                                                          double,
                                                          decltype(mag<3>() / mag<4>())>,
        MultiplyTypeBy<double, decltype(mag<3>() / mag<4>())>>();
}

TEST(ConversionForRepsAndFactor,
     ApplyingNontrivialRationalToComplexIntegralTypeIsMultiplyThenDivide) {
    StaticAssertTypeEq<
        CastStrategyIndependentConversionForRepsAndFactor<std::complex<int>,
                                                          std::complex<int>,
                                                          decltype(mag<3>() / mag<4>())>,
        OpSequence<MultiplyTypeBy<std::complex<int>, decltype(mag<3>())>,
                   DivideTypeByInteger<std::complex<int>, decltype(mag<4>())>>>();
}

TEST(ConversionForRepsAndFactor, WhenTargetIsPromotedTypeSkipFinalStaticCast) {
    using T = uint16_t;
    using Promoted = PromotedType<T>;
    ASSERT_THAT((std::is_same<T, Promoted>::value), IsFalse());

    StaticAssertTypeEq<
        ConversionForRepsAndFactor<UseStaticCast, T, Promoted, decltype(mag<15>())>,
        OpSequence<StaticCast<T, Promoted>, MultiplyTypeBy<Promoted, decltype(mag<15>())>>>();
    StaticAssertTypeEq<
        ConversionForRepsAndFactor<UseImplicitConversion, T, Promoted, decltype(mag<15>())>,
        OpSequence<ImplicitConversion<T, Promoted>,
                   MultiplyTypeBy<Promoted, decltype(mag<15>())>>>();
}

TEST(ConversionForRepsAndFactor, WhenOldRepIsPromotedCommonSkipInitialStaticCast) {
    StaticAssertTypeEq<
        ConversionForRepsAndFactor<UseStaticCast, float, int, decltype(mag<13>() / mag<15>())>,
        OpSequence<MultiplyTypeBy<float, decltype(mag<13>() / mag<15>())>,
                   StaticCast<float, int>>>();
    StaticAssertTypeEq<ConversionForRepsAndFactor<UseImplicitConversion,
                                                  float,
                                                  int,
                                                  decltype(mag<13>() / mag<15>())>,
                       OpSequence<MultiplyTypeBy<float, decltype(mag<13>() / mag<15>())>,
                                  ImplicitConversion<float, int>>>();
}

TEST(ConversionForRepsAndFactor,
     StaticCastToScalarOfComplexAfterMultiplyForScalarFloatToComplexInt) {
    StaticAssertTypeEq<
        ConversionForRepsAndFactor<UseStaticCast, double, std::complex<int>, decltype(mag<12>())>,
        OpSequence<MultiplyTypeBy<double, decltype(mag<12>())>,
                   StaticCast<double, int>,
                   StaticCast<int, std::complex<int>>>>();
    StaticAssertTypeEq<ConversionForRepsAndFactor<UseImplicitConversion,
                                                  double,
                                                  std::complex<int>,
                                                  decltype(mag<12>())>,
                       OpSequence<MultiplyTypeBy<double, decltype(mag<12>())>,
                                  ImplicitConversion<double, int>,
                                  ImplicitConversion<int, std::complex<int>>>>();
}

TEST(ConversionForRepsAndFactor,
     StaticCastToScalarOfComplexBeforeMultiplyForScalarIntToComplexFloat) {
    StaticAssertTypeEq<
        ConversionForRepsAndFactor<UseStaticCast, int, std::complex<double>, decltype(mag<12>())>,
        OpSequence<StaticCast<int, double>,
                   MultiplyTypeBy<double, decltype(mag<12>())>,
                   StaticCast<double, std::complex<double>>>>();
    StaticAssertTypeEq<ConversionForRepsAndFactor<UseImplicitConversion,
                                                  int,
                                                  std::complex<double>,
                                                  decltype(mag<12>())>,
                       OpSequence<ImplicitConversion<int, double>,
                                  MultiplyTypeBy<double, decltype(mag<12>())>,
                                  ImplicitConversion<double, std::complex<double>>>>();
}

TEST(ConversionForRepsAndFactor,
     PerformsConversionInHighestFidelityComplexForTwoComplexFloatTypes) {
    StaticAssertTypeEq<ConversionForRepsAndFactor<UseStaticCast,
                                                  std::complex<float>,
                                                  std::complex<double>,
                                                  decltype(mag<12>())>,
                       OpSequence<StaticCast<std::complex<float>, std::complex<double>>,
                                  MultiplyTypeBy<std::complex<double>, decltype(mag<12>())>>>();
    StaticAssertTypeEq<ConversionForRepsAndFactor<UseImplicitConversion,
                                                  std::complex<float>,
                                                  std::complex<double>,
                                                  decltype(mag<12>())>,
                       OpSequence<ImplicitConversion<std::complex<float>, std::complex<double>>,
                                  MultiplyTypeBy<std::complex<double>, decltype(mag<12>())>>>();

    StaticAssertTypeEq<ConversionForRepsAndFactor<UseStaticCast,
                                                  std::complex<double>,
                                                  std::complex<float>,
                                                  decltype(mag<12>())>,
                       OpSequence<MultiplyTypeBy<std::complex<double>, decltype(mag<12>())>,
                                  StaticCast<std::complex<double>, std::complex<float>>>>();
    StaticAssertTypeEq<ConversionForRepsAndFactor<UseImplicitConversion,
                                                  std::complex<double>,
                                                  std::complex<float>,
                                                  decltype(mag<12>())>,
                       OpSequence<MultiplyTypeBy<std::complex<double>, decltype(mag<12>())>,
                                  ImplicitConversion<std::complex<double>, std::complex<float>>>>();
}

}  // namespace detail
}  // namespace au
