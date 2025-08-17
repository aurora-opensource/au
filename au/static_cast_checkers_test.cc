// Copyright 2024 Aurora Operations, Inc.
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

#include "au/overflow_boundary.hh"
#include "au/truncation_risk.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::IsFalse;
using ::testing::IsTrue;

namespace detail {

//
// This file held the unit tests for an older library, `:static_cast_checkers`, which we no longer
// need. We have retained the test file so that we can still get value out of all of the
// pre-existing test cases.  The first part of this file simply re-implements the functionality of
// the old library very concisely, in a few lines of code that use the replacement libraries.
//

template <typename U, typename T>
constexpr bool will_static_cast_overflow(const T &value) {
    return would_value_overflow<StaticCast<T, U>>(value);
}

template <typename U, typename T>
constexpr bool will_static_cast_truncate(const T &value) {
    return TruncationRiskFor<StaticCast<T, U>>::would_value_truncate(value);
}

TEST(WillStaticCastOverflow, DependsOnValueForUnsignedToNonContainingSigned) {
    EXPECT_THAT(will_static_cast_overflow<int8_t>(uint8_t{127}), IsFalse());
    EXPECT_THAT(will_static_cast_overflow<int8_t>(uint8_t{128}), IsTrue());
}

TEST(WillStaticCastOverflow, AlwaysFalseForUnsignedToContainingSigned) {
    EXPECT_THAT(will_static_cast_overflow<int>(uint8_t{124}), IsFalse());
    EXPECT_THAT(will_static_cast_overflow<int>(uint8_t{125}), IsFalse());
}

TEST(WillStaticCastOverflow, ChecksLimitForNonContainingSameSignedness) {
    EXPECT_THAT(will_static_cast_overflow<int8_t>(127), IsFalse());
    EXPECT_THAT(will_static_cast_overflow<int8_t>(128), IsTrue());
}

TEST(WillStaticCastOverflow, TrueForNegativeInputAndUnsignedDestination) {
    EXPECT_THAT(will_static_cast_overflow<uint8_t>(-1), IsTrue());
    EXPECT_THAT(will_static_cast_overflow<unsigned int>(int8_t{-1}), IsTrue());
}

TEST(WillStaticCastOverflow, FalseWhenDestBoundsContainsSourceBounds) {
    EXPECT_THAT(will_static_cast_overflow<float>(std::numeric_limits<uint64_t>::max()), IsFalse());
}

TEST(WillStaticCastOverflow, DependsOnTypeLimitsForFloatToInt) {
    EXPECT_THAT(will_static_cast_overflow<uint8_t>(-0.0001), IsTrue());
    EXPECT_THAT(will_static_cast_overflow<uint8_t>(0.0000), IsFalse());
    EXPECT_THAT(will_static_cast_overflow<uint8_t>(0.0001), IsFalse());

    EXPECT_THAT(will_static_cast_overflow<uint8_t>(254.9999), IsFalse());
    EXPECT_THAT(will_static_cast_overflow<uint8_t>(255.0000), IsFalse());
    EXPECT_THAT(will_static_cast_overflow<uint8_t>(255.0001), IsTrue());
}

TEST(WillStaticCastOverflow, TrueForReallyBigDoubleGoingToFloat) {
    EXPECT_THAT(will_static_cast_overflow<float>(1e200), IsTrue());
}

TEST(WillStaticCastTruncate, IntToFloatFalseForIntTypeThatCanFitInFloat) {
    EXPECT_THAT(will_static_cast_truncate<float>(uint8_t{124}), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<double>(124), IsFalse());

    static_assert(std::numeric_limits<double>::digits >= std::numeric_limits<int32_t>::digits, "");
    EXPECT_THAT(will_static_cast_truncate<double>(std::numeric_limits<int32_t>::max()), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<double>(std::numeric_limits<int32_t>::max() - 1),
                IsFalse());

    static_assert(std::numeric_limits<double>::digits >= std::numeric_limits<uint32_t>::digits, "");
    EXPECT_THAT(will_static_cast_truncate<double>(std::numeric_limits<uint32_t>::max()), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<double>(std::numeric_limits<uint32_t>::max() - 1),
                IsFalse());
}

TEST(WillStaticCastTruncate, IntToFloatFalseByConvention) {
    static_assert(std::numeric_limits<float>::radix == 2, "Test assumes binary");

    constexpr auto first_unrepresentable = (1 << std::numeric_limits<float>::digits) + 1;
    EXPECT_THAT(will_static_cast_truncate<float>(first_unrepresentable - 2), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<float>(first_unrepresentable - 1), IsFalse());

    // This is actually non-representable, but we call it "non-truncating" by convention.
    EXPECT_THAT(will_static_cast_truncate<float>(first_unrepresentable + 0), IsFalse());

    EXPECT_THAT(will_static_cast_truncate<float>(first_unrepresentable + 1), IsFalse());

    // This is actually non-representable, but we call it "non-truncating" by convention.
    EXPECT_THAT(will_static_cast_truncate<float>(first_unrepresentable + 2), IsFalse());
}

TEST(WillStaticCastTruncate, AutomaticallyFalseForIntegralToIntegral) {
    EXPECT_THAT(will_static_cast_truncate<int8_t>(uint8_t{127}), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<int8_t>(uint8_t{128}), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<int8_t>(128), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<int8_t>(uint64_t{9876543210u}), IsFalse());
}

TEST(WillStaticCastTruncate, TrueForFloatToIntIffInputHasAFractionalPart) {
    EXPECT_THAT(will_static_cast_truncate<uint8_t>(-0.1), IsTrue());
    EXPECT_THAT(will_static_cast_truncate<uint8_t>(0.0), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<uint8_t>(0.1), IsTrue());

    EXPECT_THAT(will_static_cast_truncate<uint8_t>(254.9), IsTrue());
    EXPECT_THAT(will_static_cast_truncate<uint8_t>(255.0), IsFalse());
    EXPECT_THAT(will_static_cast_truncate<uint8_t>(255.1), IsTrue());
}

TEST(WillStaticCastTruncate, IgnoresLimitsOfDestinationType) {
    // Yes, this would be lossy, but we would chalk it up to "overflow", not "truncation".
    EXPECT_THAT(will_static_cast_truncate<uint8_t>(9999999.0), IsFalse());
}

}  // namespace detail
}  // namespace au
