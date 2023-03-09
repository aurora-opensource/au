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

#include "au/operators.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {
namespace detail {

template <typename T>
void expect_comparators_work(T a, T b) {
    EXPECT_EQ(equal(a, b), (a == b));
    EXPECT_EQ(not_equal(a, b), (a != b));
    EXPECT_EQ(less(a, b), (a < b));
    EXPECT_EQ(less_equal(a, b), (a <= b));
    EXPECT_EQ(greater(a, b), (a > b));
    EXPECT_EQ(greater_equal(a, b), (a >= b));
}

template <typename T, typename U>
void expect_arithmetic_works(T t, U u) {
    EXPECT_THAT(plus(t, u), SameTypeAndValue(t + u));
    EXPECT_THAT(minus(t, u), SameTypeAndValue(t - u));
}

TEST(Comparators, ResultsMatchUnderlyingOperatorForSameType) {
    expect_comparators_work(1, 2);
    expect_comparators_work(1.5, 1.49999999999);
}

TEST(Arithmetic, ResultsMatchUnderlyingOperatorForSameTypes) {
    expect_arithmetic_works(1, 2.5);
    expect_arithmetic_works(3.3, -8.9);
    expect_arithmetic_works(int8_t{5}, int8_t{10});
}

}  // namespace detail
}  // namespace au
