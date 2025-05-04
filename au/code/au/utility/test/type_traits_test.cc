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
struct Pack;

TEST(Prepend, PrependsToPack) {
    StaticAssertTypeEq<PrependT<Pack<>, int>, Pack<int>>();
    StaticAssertTypeEq<PrependT<Pack<double, char>, int>, Pack<int, double, char>>();
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

TEST(AlwaysFalse, IsAlwaysFalse) {
    EXPECT_FALSE(AlwaysFalse<int>::value);
    EXPECT_FALSE(AlwaysFalse<void>::value);
    EXPECT_FALSE(AlwaysFalse<>::value);
    EXPECT_FALSE((AlwaysFalse<int, char, double>::value));
}

TEST(DropAll, IdentityWhenTargetAbsent) {
    StaticAssertTypeEq<DropAll<void, Pack<>>, Pack<>>();
    StaticAssertTypeEq<DropAll<void, Pack<int>>, Pack<int>>();
    StaticAssertTypeEq<DropAll<void, Pack<int, char, double>>, Pack<int, char, double>>();
}

TEST(DropAll, DropsAllInstancesOfTarget) {
    StaticAssertTypeEq<DropAll<int, Pack<int>>, Pack<>>();
    StaticAssertTypeEq<DropAll<int, Pack<int, int>>, Pack<>>();
    StaticAssertTypeEq<DropAll<int, Pack<int, char, int>>, Pack<char>>();
    StaticAssertTypeEq<DropAll<int, Pack<char, int, char>>, Pack<char, char>>();
    StaticAssertTypeEq<DropAll<int, Pack<int, char, int, double>>, Pack<char, double>>();
}

TEST(IncludeInPackIf, MakesPackOfEverythingThatMatches) {
    StaticAssertTypeEq<
        IncludeInPackIf<std::is_unsigned, Pack, int32_t, uint8_t, double, char, uint64_t, int16_t>,
        Pack<uint8_t, uint64_t>>();
}

}  // namespace detail
}  // namespace au
