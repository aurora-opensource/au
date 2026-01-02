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

#include "au/abstract_operations.hh"

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {
namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast` section:

TEST(StaticCast, HasExpectedInputAndOutputTypes) {
    StaticAssertTypeEq<OpInput<StaticCast<int16_t, float>>, int16_t>();
    StaticAssertTypeEq<OpOutput<StaticCast<int16_t, float>>, float>();
}

TEST(StaticCast, PerformsStaticCast) {
    EXPECT_THAT((StaticCast<int16_t, float>::apply_to(int16_t{123})), SameTypeAndValue(123.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ImplicitConversion` section:

TEST(ImplicitConversion, HasExpectedInputAndOutputTypes) {
    StaticAssertTypeEq<OpInput<ImplicitConversion<int16_t, double>>, int16_t>();
    StaticAssertTypeEq<OpOutput<ImplicitConversion<int16_t, double>>, double>();
}

TEST(ImplicitConversion, PerformsImplicitConversion) {
    EXPECT_THAT((ImplicitConversion<int16_t, double>::apply_to(int16_t{123})),
                SameTypeAndValue(123.0));
    EXPECT_THAT((ImplicitConversion<float, double>::apply_to(1.25f)), SameTypeAndValue(1.25));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy` section:

TEST(MultiplyTypeBy, InputTypeIsTypeParameter) {
    StaticAssertTypeEq<OpInput<MultiplyTypeBy<int16_t, decltype(mag<2>())>>, int16_t>();
    StaticAssertTypeEq<OpInput<MultiplyTypeBy<uint32_t, decltype(mag<3>() / mag<4>())>>,
                       uint32_t>();
}

TEST(MultiplyTypeBy, OutputTypeIsTypeParameter) {
    StaticAssertTypeEq<OpOutput<MultiplyTypeBy<int16_t, decltype(mag<2>())>>, int16_t>();
    StaticAssertTypeEq<OpOutput<MultiplyTypeBy<double, decltype(mag<3>() / mag<4>())>>, double>();
}

TEST(MultiplyTypeBy, IntegerTypeCanBeMultipliedByIntegerMag) {
    EXPECT_THAT((MultiplyTypeBy<int16_t, decltype(mag<2>())>::apply_to(int16_t{3})),
                SameTypeAndValue(int16_t{6}));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger` section:

TEST(DivideTypeByInteger, InputTypeIsTypeParameter) {
    StaticAssertTypeEq<OpInput<DivideTypeByInteger<int16_t, decltype(mag<2>())>>, int16_t>();
    StaticAssertTypeEq<OpInput<DivideTypeByInteger<uint32_t, decltype(mag<3>() / mag<4>())>>,
                       uint32_t>();
}

TEST(DivideTypeByInteger, OutputTypeIsTypeParameter) {
    StaticAssertTypeEq<OpOutput<DivideTypeByInteger<int16_t, decltype(mag<2>())>>, int16_t>();
    StaticAssertTypeEq<OpOutput<DivideTypeByInteger<double, decltype(mag<3>() / mag<4>())>>,
                       double>();
}

TEST(DivideTypeByInteger, IntegerTypeCanBeDividedByIntegerMag) {
    EXPECT_THAT((DivideTypeByInteger<uint16_t, decltype(mag<3>())>::apply_to(uint16_t{16})),
                SameTypeAndValue(uint16_t{5}));
}

TEST(DivideTypeByInteger, IntegerTypeDividedByIntegerTooBigToRepresentGivesZero) {
    EXPECT_THAT((DivideTypeByInteger<uint8_t, decltype(mag<256>())>::apply_to(uint8_t{1})),
                SameTypeAndValue(uint8_t{0}));
}

TEST(DivideTypeByInteger, IntegerTypeDividedByIntegerMagGreaterThanDividendGivesZero) {
    EXPECT_THAT((DivideTypeByInteger<uint8_t, decltype(mag<2>())>::apply_to(uint8_t{1})),
                SameTypeAndValue(uint8_t{0}));

    EXPECT_THAT((DivideTypeByInteger<int, decltype(mag<2025>())>::apply_to(int{2024})),
                SameTypeAndValue(int{0}));
}

TEST(DivideTypeByInteger, FloatingPointTypeDividedByNumberTooBigToRepresentGivesZero) {
    EXPECT_THAT((DivideTypeByInteger<float, decltype(-pow<40>(mag<10>()))>::apply_to(float{1.0f})),
                SameTypeAndValue(0.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence` section:

TEST(OpSequence, InputTypeIsInputTypeOfFirstOperation) {
    StaticAssertTypeEq<OpInput<OpSequence<MultiplyTypeBy<uint32_t, decltype(mag<3>() / mag<4>())>>>,
                       uint32_t>();

    StaticAssertTypeEq<OpInput<OpSequence<StaticCast<int16_t, uint16_t>,
                                          MultiplyTypeBy<uint16_t, decltype(mag<2>())>>>,
                       int16_t>();
}

TEST(OpSequence, OutputTypeIsOutputTypeOfLastOperation) {
    StaticAssertTypeEq<
        OpOutput<OpSequence<MultiplyTypeBy<uint32_t, decltype(mag<3>() / mag<4>())>>>,
        uint32_t>();

    StaticAssertTypeEq<OpOutput<OpSequence<StaticCast<int16_t, uint16_t>,
                                           MultiplyTypeBy<uint16_t, decltype(mag<2>())>,
                                           StaticCast<uint16_t, double>>>,
                       double>();
}

TEST(OpSequence, AppliesOperationsInSequence) {
    EXPECT_THAT((OpSequence<StaticCast<float, int>,
                            MultiplyTypeBy<int, decltype(mag<3>())>,
                            StaticCast<int, double>>::apply_to(2.9f)),
                SameTypeAndValue(6.0));
}

TEST(OpSequence, EliminatesRedundantOperations) {
    StaticAssertTypeEq<
        OpSequence<StaticCast<int, float>,
                   OpSequence<OpSequence<OpSequence<>>>,
                   OpSequence<MultiplyTypeBy<float, decltype(mag<2>())>>,
                   OpSequence<>>,
        OpSequence<StaticCast<int, float>, MultiplyTypeBy<float, decltype(mag<2>())>>>();
}

}  // namespace
}  // namespace detail
}  // namespace au
