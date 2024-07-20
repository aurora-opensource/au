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

#include "au/utility/type_traits.hh"

#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {

template <typename... Ts>
struct TestingPack;

TEST(Prepend, PrependsToPack) {
    StaticAssertTypeEq<PrependT<TestingPack<>, int>, TestingPack<int>>();
    StaticAssertTypeEq<PrependT<TestingPack<double, char>, int>, TestingPack<int, double, char>>();
}

TEST(SameTypeIgnoringCvref, IgnoresCvrefQualifiers) {
    EXPECT_TRUE((SameTypeIgnoringCvref<int, int &>::value));
    EXPECT_TRUE((SameTypeIgnoringCvref<const int &&, volatile int>::value));
}

TEST(SameTypeIgnoringCvref, FalseForDifferentBases) {
    EXPECT_FALSE((SameTypeIgnoringCvref<int, char>::value));
    EXPECT_FALSE((SameTypeIgnoringCvref<const double &, const float &>::value));
}

TEST(SameTypeIgnoringCvref, CanTakeInstances) {
    EXPECT_TRUE(same_type_ignoring_cvref(1, 2));
    EXPECT_FALSE(same_type_ignoring_cvref(1.0, 2.0f));
}

}  // namespace detail
}  // namespace au
