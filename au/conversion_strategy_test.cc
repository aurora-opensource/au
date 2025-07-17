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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {
namespace detail {

using ::testing::IsFalse;
using ::testing::StaticAssertTypeEq;

TEST(ConversionForRepsAndFactor, SameRepAndNonPromotingTypeIsJustMultiplyByDefault) {
    StaticAssertTypeEq<ConversionForRepsAndFactor<int32_t, int32_t, decltype(mag<15>())>,
                       MultiplyTypeBy<int32_t, decltype(mag<15>())>>();
}

TEST(ConversionForRepsAndFactor, SameRepAndNonPromotingTypeWithInverseIntegerIsJustDivideBy) {
    StaticAssertTypeEq<ConversionForRepsAndFactor<int32_t, int32_t, decltype(mag<1>() / mag<16>())>,
                       DivideTypeByInteger<int32_t, decltype(mag<16>())>>();

    StaticAssertTypeEq<ConversionForRepsAndFactor<double, double, decltype(mag<1>() / mag<3456>())>,
                       DivideTypeByInteger<double, decltype(mag<3456>())>>();
}

TEST(ConversionForRepsAndFactor, SameRepForPromotingTypeHasStaticCastAtBeginningAndEnd) {
    using T = uint16_t;
    using Promoted = PromotedType<T>;
    ASSERT_THAT((std::is_same<T, Promoted>::value), IsFalse());

    StaticAssertTypeEq<ConversionForRepsAndFactor<T, T, decltype(mag<15>())>,
                       OpSequence<StaticCast<T, Promoted>,
                                  MultiplyTypeBy<Promoted, decltype(mag<15>())>,
                                  StaticCast<Promoted, T>>>();
}

TEST(ConversionForRepsAndFactor, ApplyingNontrivialRationalToIntegralTypeIsMultiplyThenDivide) {
    StaticAssertTypeEq<
        ConversionForRepsAndFactor<uint64_t, uint64_t, decltype(mag<3>() / mag<4>())>,
        OpSequence<MultiplyTypeBy<uint64_t, decltype(mag<3>())>,
                   DivideTypeByInteger<uint64_t, decltype(mag<4>())>>>();
}

TEST(ConversionForRepsAndFactor, ApplyingNontrivialRationalToFloatingPointIsSingleMultiply) {
    StaticAssertTypeEq<ConversionForRepsAndFactor<double, double, decltype(mag<3>() / mag<4>())>,
                       MultiplyTypeBy<double, decltype(mag<3>() / mag<4>())>>();
}

TEST(ConversionForRepsAndFactor, WhenTargetIsPromotedTypeSkipFinalStaticCast) {
    using T = uint16_t;
    using Promoted = PromotedType<T>;
    ASSERT_THAT((std::is_same<T, Promoted>::value), IsFalse());

    StaticAssertTypeEq<
        ConversionForRepsAndFactor<T, Promoted, decltype(mag<15>())>,
        OpSequence<StaticCast<T, Promoted>, MultiplyTypeBy<Promoted, decltype(mag<15>())>>>();
}

TEST(ConversionForRepsAndFactor, WhenOldRepIsPromotedCommonSkipInitialStaticCast) {
    StaticAssertTypeEq<ConversionForRepsAndFactor<float, int, decltype(mag<13>() / mag<15>())>,
                       OpSequence<MultiplyTypeBy<float, decltype(mag<13>() / mag<15>())>,
                                  StaticCast<float, int>>>();
}

}  // namespace detail
}  // namespace au
