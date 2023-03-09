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

#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace au {
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
    EXPECT_TRUE(WrappedInt{1} > WrappedInt{0});
    EXPECT_FALSE(WrappedInt{0} > WrappedInt{1});
    EXPECT_FALSE(WrappedInt{1} > WrappedInt{1});

    EXPECT_TRUE(WrappedInt{1} < WrappedInt{2});
    EXPECT_FALSE(WrappedInt{2} < WrappedInt{1});
    EXPECT_FALSE(WrappedInt{2} < WrappedInt{2});

    EXPECT_TRUE(WrappedInt{1} == WrappedInt{1});
    EXPECT_FALSE(WrappedInt{2} == WrappedInt{1});
    EXPECT_FALSE(WrappedInt{1} == WrappedInt{2});
}

TEST(Zero, MinusZeroIsZero) {
    constexpr auto zero_minus_zero = ZERO - ZERO;
    EXPECT_EQ(ZERO, zero_minus_zero);
}

TEST(Zero, PlusZeroIsZero) {
    constexpr auto zero_plus_zero = ZERO + ZERO;
    EXPECT_EQ(ZERO, zero_plus_zero);
}

TEST(Zero, ComparableToArbitraryQuantities) {
    EXPECT_EQ(ZERO, WrappedInt{0});
    EXPECT_LT(ZERO, WrappedInt{1});
    EXPECT_GT(ZERO, WrappedInt{-1});

    EXPECT_EQ(ZERO, WrappedInt{0});
    EXPECT_LT(ZERO, WrappedInt{1});
    EXPECT_GT(ZERO, WrappedInt{-1});
}

TEST(Zero, ComparesEqualToZero) {
    EXPECT_TRUE(ZERO == ZERO);
    EXPECT_TRUE(ZERO >= ZERO);
    EXPECT_TRUE(ZERO <= ZERO);

    EXPECT_FALSE(ZERO != ZERO);
    EXPECT_FALSE(ZERO > ZERO);
    EXPECT_FALSE(ZERO < ZERO);
}

TEST(Zero, ImplicitlyConvertsToNumericTypes) {
    constexpr int zero_i = ZERO;
    EXPECT_EQ(zero_i, 0);

    constexpr float zero_f = ZERO;
    EXPECT_EQ(zero_f, 0.f);
}

TEST(Zero, ImplicitlyConvertsToChronoDuration) {
    constexpr std::chrono::nanoseconds zero_ns = ZERO;
    EXPECT_EQ(zero_ns, 0ns);

    constexpr std::chrono::duration<float, std::milli> zero_ms_f = ZERO;
    EXPECT_EQ(zero_ms_f, (std::chrono::duration<float, std::milli>{0.f}));
}

}  // namespace au
