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
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;
using ::testing::Not;

namespace detail {

std::ostream &operator<<(std::ostream &out, Equal) { return out << " == "; }
std::ostream &operator<<(std::ostream &out, NotEqual) { return out << " != "; }
std::ostream &operator<<(std::ostream &out, Less) { return out << " < "; }
std::ostream &operator<<(std::ostream &out, LessEqual) { return out << " <= "; }
std::ostream &operator<<(std::ostream &out, Greater) { return out << " > "; }
std::ostream &operator<<(std::ostream &out, GreaterEqual) { return out << " >= "; }

template <typename Comparison, typename T, typename U, typename Listener>
auto cmp(Comparison compare, T t, U u, Listener *listener) {
    auto result = compare(t, u);
    *listener << t << compare << u << ": " << (result ? "true" : "false");
    return result;
}

MATCHER_P(OpEqual, target, "") { return cmp(equal, arg, target, result_listener); }
MATCHER_P(OpNotEqual, target, "") { return cmp(not_equal, arg, target, result_listener); }
MATCHER_P(OpLess, target, "") { return cmp(less, arg, target, result_listener); }
MATCHER_P(OpLessEqual, target, "") { return cmp(less_equal, arg, target, result_listener); }
MATCHER_P(OpGreater, target, "") { return cmp(greater, arg, target, result_listener); }
MATCHER_P(OpGreaterEqual, target, "") { return cmp(greater_equal, arg, target, result_listener); }

template <typename T>
auto OpConsistentlyLessThan(T target) {
    return ::testing::AllOf(OpNotEqual(target),
                            OpLess(target),
                            OpLessEqual(target),

                            Not(OpEqual(target)),
                            Not(OpGreater(target)),
                            Not(OpGreaterEqual(target)));
}

template <typename T>
auto OpConsistentlyGreaterThan(T target) {
    return ::testing::AllOf(OpNotEqual(target),
                            OpGreater(target),
                            OpGreaterEqual(target),

                            Not(OpEqual(target)),
                            Not(OpLess(target)),
                            Not(OpLessEqual(target)));
}

template <typename T>
auto OpConsistentlyEqual(T target) {
    return ::testing::AllOf(OpEqual(target),
                            OpLessEqual(target),
                            OpGreaterEqual(target),

                            Not(OpNotEqual(target)),
                            Not(OpLess(target)),
                            Not(OpGreater(target)));
}

template <typename T>
void expect_comparators_work(T a, T b) {
    EXPECT_THAT(equal(a, b), Eq(a == b));
    EXPECT_THAT(not_equal(a, b), Eq(a != b));
    EXPECT_THAT(less(a, b), Eq(a < b));
    EXPECT_THAT(less_equal(a, b), Eq(a <= b));
    EXPECT_THAT(greater(a, b), Eq(a > b));
    EXPECT_THAT(greater_equal(a, b), Eq(a >= b));
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

TEST(Comparators, HandleInternetComparisonCorrectly) {
    EXPECT_THAT(-1, OpConsistentlyLessThan(1u));

    // This specific test case adds more meaningful coverage for equality operators.
    EXPECT_THAT(int32_t{-1}, OpConsistentlyLessThan(uint32_t{0} - 1u));
}

TEST(Arithmetic, ResultsMatchUnderlyingOperatorForSameTypes) {
    expect_arithmetic_works(1, 2.5);
    expect_arithmetic_works(3.3, -8.9);
    expect_arithmetic_works(int8_t{5}, int8_t{10});
}

}  // namespace detail
}  // namespace au
