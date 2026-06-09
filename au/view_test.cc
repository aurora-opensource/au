// Copyright 2026 Aurora Operations, Inc.
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

#include "au/view.hh"

#include <array>

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

TEST(View, ImplicitlyConvertsToUnderlyingValue) {
    double x = 3.5;
    View<double> v{x};
    double result = v;
    EXPECT_THAT(result, Eq(3.5));
}

TEST(View, AssignmentWritesThroughToUnderlying) {
    double x = 1.0;
    View<double> v{x};
    v = 2.5;
    EXPECT_THAT(x, Eq(2.5));
}

TEST(View, CopyAssignmentWritesThroughRatherThanRebinding) {
    double x = 1.0;
    double y = 2.0;
    View<double> vx{x};
    View<double> vy{y};
    vx = vy;
    EXPECT_THAT(x, Eq(2.0));

    y = 99.0;
    EXPECT_THAT(static_cast<double>(vx), Eq(2.0));
}

TEST(View, ElementAccessReturnsViewOfElement) {
    std::array<double, 3> arr = {1.0, 2.0, 3.0};
    View<std::array<double, 3>> v{arr};
    auto elem = v[1u];
    StaticAssertTypeEq<decltype(elem), View<double>>();
    EXPECT_THAT(static_cast<double>(elem), Eq(2.0));
}

TEST(View, ElementAccessWritesThrough) {
    std::array<double, 3> arr = {1.0, 2.0, 3.0};
    View<std::array<double, 3>> v{arr};
    v[1u] = 42.0;
    EXPECT_THAT(arr[1u], Eq(42.0));
}

TEST(View, AdditionDereferencesAndReturnsValue) {
    double a = 1.5;
    double b = 2.5;
    View<double> va{a};
    View<double> vb{b};

    EXPECT_THAT(va + vb, SameTypeAndValue(4.0));
    EXPECT_THAT(va + 10.0, SameTypeAndValue(11.5));
    EXPECT_THAT(10.0 + va, SameTypeAndValue(11.5));
}

TEST(View, SubtractionDereferencesAndReturnsValue) {
    double a = 5.0;
    double b = 2.0;
    View<double> va{a};
    View<double> vb{b};

    EXPECT_THAT(va - vb, SameTypeAndValue(3.0));
    EXPECT_THAT(va - 1.0, SameTypeAndValue(4.0));
    EXPECT_THAT(10.0 - va, SameTypeAndValue(5.0));
}

TEST(View, ScalarMultiplicationWorks) {
    double x = 3.0;
    View<double> v{x};

    EXPECT_THAT(v * 2.0, SameTypeAndValue(6.0));
    EXPECT_THAT(2.0 * v, SameTypeAndValue(6.0));
}

TEST(View, ScalarDivisionWorks) {
    double x = 6.0;
    View<double> v{x};

    EXPECT_THAT(v / 2.0, SameTypeAndValue(3.0));
    EXPECT_THAT(12.0 / v, SameTypeAndValue(2.0));
}

TEST(View, CompoundAdditionWritesThrough) {
    double x = 1.0;
    View<double> v{x};
    v += 2.5;
    EXPECT_THAT(x, Eq(3.5));
}

TEST(View, CompoundSubtractionWritesThrough) {
    double x = 5.0;
    View<double> v{x};
    v -= 1.5;
    EXPECT_THAT(x, Eq(3.5));
}

TEST(View, CompoundMultiplicationWritesThrough) {
    double x = 3.0;
    View<double> v{x};
    v *= 4.0;
    EXPECT_THAT(x, Eq(12.0));
}

TEST(View, CompoundDivisionWritesThrough) {
    double x = 12.0;
    View<double> v{x};
    v /= 3.0;
    EXPECT_THAT(x, Eq(4.0));
}

TEST(View, ArithmeticDoesNotModifyUnderlying) {
    double a = 3.0;
    double b = 2.0;
    View<double> va{a};
    View<double> vb{b};

    EXPECT_THAT(va + vb, Eq(5.0));
    EXPECT_THAT(a, Eq(3.0));
    EXPECT_THAT(b, Eq(2.0));
}

TEST(MakeView, CreatesViewOfValue) {
    int x = 42;
    auto v = make_view(x);
    StaticAssertTypeEq<decltype(v), View<int>>();
    EXPECT_THAT(static_cast<int>(v), Eq(42));
}

TEST(IsView, TrueForViewTypes) {
    EXPECT_THAT(IsView<View<double>>::value, IsTrue());
    EXPECT_THAT(IsView<View<int>>::value, IsTrue());
}

TEST(IsView, FalseForNonViewTypes) {
    EXPECT_THAT(IsView<double>::value, IsFalse());
    EXPECT_THAT(IsView<int>::value, IsFalse());
}

TEST(UnderlyingType, StripsViewWrapper) {
    StaticAssertTypeEq<detail::UnderlyingType<View<double>>, double>();
    StaticAssertTypeEq<detail::UnderlyingType<View<int>>, int>();
}

TEST(UnderlyingType, PassesThroughNonViewTypes) {
    StaticAssertTypeEq<detail::UnderlyingType<double>, double>();
    StaticAssertTypeEq<detail::UnderlyingType<int>, int>();
}

struct HasScalar {
    using Scalar = float;
};

TEST(ScalarOfTrait, ViewDelegatesToUnderlyingType) {
    StaticAssertTypeEq<ScalarOf<View<HasScalar>>, float>();
}

TEST(RealPartImpl, ViewDelegatesToUnderlyingType) {
    StaticAssertTypeEq<detail::RealPart<View<double>>, double>();
}

}  // namespace au
