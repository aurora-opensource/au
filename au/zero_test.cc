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

#include "au/zero.hh"

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace au {

using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Lt;

// Example for supporting implicit construction from Zero.
class WrappedInt {
 public:
    constexpr WrappedInt(int x) : x_{x} {}
    constexpr WrappedInt(Zero) : WrappedInt{0} {}

    constexpr friend bool operator==(WrappedInt a, WrappedInt b) { return a.x_ == b.x_; }
    constexpr friend bool operator<(WrappedInt a, WrappedInt b) { return a.x_ < b.x_; }
    constexpr friend bool operator>(WrappedInt a, WrappedInt b) { return a.x_ > b.x_; }

 private:
    int x_;
};

TEST(WrappedInt, BasicInterfaceWorksAsExpected) {
    EXPECT_THAT(WrappedInt{1} > WrappedInt{0}, IsTrue());
    EXPECT_THAT(WrappedInt{0} > WrappedInt{1}, IsFalse());
    EXPECT_THAT(WrappedInt{1} > WrappedInt{1}, IsFalse());

    EXPECT_THAT(WrappedInt{1} < WrappedInt{2}, IsTrue());
    EXPECT_THAT(WrappedInt{2} < WrappedInt{1}, IsFalse());
    EXPECT_THAT(WrappedInt{2} < WrappedInt{2}, IsFalse());

    EXPECT_THAT(WrappedInt{1} == WrappedInt{1}, IsTrue());
    EXPECT_THAT(WrappedInt{2} == WrappedInt{1}, IsFalse());
    EXPECT_THAT(WrappedInt{1} == WrappedInt{2}, IsFalse());
}

TEST(Zero, MinusZeroIsZero) {
    constexpr auto zero_minus_zero = ZERO - ZERO;
    EXPECT_THAT(zero_minus_zero, Eq(ZERO));
}

TEST(Zero, PlusZeroIsZero) {
    constexpr auto zero_plus_zero = ZERO + ZERO;
    EXPECT_THAT(zero_plus_zero, Eq(ZERO));
}

TEST(Zero, ComparableToArbitraryQuantities) {
    EXPECT_THAT(ZERO, Eq(WrappedInt{0}));
    EXPECT_THAT(ZERO, Lt(WrappedInt{1}));
    EXPECT_THAT(ZERO, Gt(WrappedInt{-1}));
}

TEST(Zero, ComparesEqualToZero) {
    EXPECT_THAT(ZERO == ZERO, IsTrue());
    EXPECT_THAT(ZERO >= ZERO, IsTrue());
    EXPECT_THAT(ZERO <= ZERO, IsTrue());

    EXPECT_THAT(ZERO != ZERO, IsFalse());
    EXPECT_THAT(ZERO > ZERO, IsFalse());
    EXPECT_THAT(ZERO < ZERO, IsFalse());
}

TEST(Zero, ImplicitlyConvertsToNumericTypes) {
    constexpr int zero_i = ZERO;
    EXPECT_THAT(zero_i, Eq(0));

    constexpr float zero_f = ZERO;
    EXPECT_THAT(zero_f, Eq(0.f));
}

TEST(Zero, ImplicitlyConvertsToChronoDuration) {
    constexpr std::chrono::nanoseconds zero_ns = ZERO;
    EXPECT_THAT(zero_ns, Eq(0ns));

    constexpr std::chrono::duration<float, std::milli> zero_ms_f = ZERO;
    EXPECT_THAT(zero_ms_f, Eq(std::chrono::duration<float, std::milli>{0.f}));
}

TEST(ValueOfZero, ProducesValueOfZero) {
    EXPECT_THAT(ValueOfZero<float>::value(), SameTypeAndValue(0.f));
}

}  // namespace au
