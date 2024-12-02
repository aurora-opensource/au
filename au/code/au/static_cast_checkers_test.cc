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

#include "au/static_cast_checkers.hh"

#include "gtest/gtest.h"

namespace au {
namespace detail {

TEST(WillStaticCastOverflow, DependsOnValueForUnsignedToNonContainingSigned) {
    EXPECT_FALSE(will_static_cast_overflow<int8_t>(uint8_t{127}));
    EXPECT_TRUE(will_static_cast_overflow<int8_t>(uint8_t{128}));
}

TEST(WillStaticCastOverflow, AlwaysFalseForUnsignedToContainingSigned) {
    EXPECT_FALSE(will_static_cast_overflow<int>(uint8_t{124}));
    EXPECT_FALSE(will_static_cast_overflow<int>(uint8_t{125}));
}

TEST(WillStaticCastOverflow, ChecksLimitForNonContainingSameSignedness) {
    EXPECT_FALSE(will_static_cast_overflow<int8_t>(127));
    EXPECT_TRUE(will_static_cast_overflow<int8_t>(128));
}

TEST(WillStaticCastOverflow, TrueForNegativeInputAndUnsignedDestination) {
    EXPECT_TRUE(will_static_cast_overflow<uint8_t>(-1));
    EXPECT_TRUE(will_static_cast_overflow<unsigned int>(int8_t{-1}));
}

TEST(WillStaticCastTruncate, AutomaticallyFalseForIntegralToIntegral) {
    EXPECT_FALSE(will_static_cast_truncate<int8_t>(uint8_t{127}));
    EXPECT_FALSE(will_static_cast_truncate<int8_t>(uint8_t{128}));
    EXPECT_FALSE(will_static_cast_truncate<int8_t>(128));
    EXPECT_FALSE(will_static_cast_truncate<int8_t>(uint64_t{9876543210u}));
}

}  // namespace detail
}  // namespace au
