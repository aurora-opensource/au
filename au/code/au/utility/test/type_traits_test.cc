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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

namespace detail {

template <typename... Ts>
struct Pack;

TEST(Prepend, PrependsToPack) {
    StaticAssertTypeEq<PrependT<Pack<>, int>, Pack<int>>();
    StaticAssertTypeEq<PrependT<Pack<double, char>, int>, Pack<int, double, char>>();
}

TEST(SameTypeIgnoringCvref, IgnoresCvrefQualifiers) {
    EXPECT_THAT((SameTypeIgnoringCvref<int, int &>::value), IsTrue());
    EXPECT_THAT((SameTypeIgnoringCvref<const int &&, volatile int>::value), IsTrue());
}

TEST(SameTypeIgnoringCvref, FalseForDifferentBases) {
    EXPECT_THAT((SameTypeIgnoringCvref<int, char>::value), IsFalse());
    EXPECT_THAT((SameTypeIgnoringCvref<const double &, const float &>::value), IsFalse());
}

TEST(SameTypeIgnoringCvref, CanTakeInstances) {
    EXPECT_THAT(same_type_ignoring_cvref(1, 2), IsTrue());
    EXPECT_THAT(same_type_ignoring_cvref(1.0, 2.0f), IsFalse());
}

TEST(AlwaysFalse, IsAlwaysFalse) {
    EXPECT_THAT(AlwaysFalse<int>::value, IsFalse());
    EXPECT_THAT(AlwaysFalse<void>::value, IsFalse());
    EXPECT_THAT(AlwaysFalse<>::value, IsFalse());
    EXPECT_THAT((AlwaysFalse<int, char, double>::value), IsFalse());
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

TEST(CommonTypeButPreserveIntSignedness, CommonTypeIfItIsNotIntegral) {
    StaticAssertTypeEq<CommonTypeButPreserveIntSignedness<int, double>, double>();
    StaticAssertTypeEq<CommonTypeButPreserveIntSignedness<float, int>, float>();
    StaticAssertTypeEq<CommonTypeButPreserveIntSignedness<double, float>, double>();
}

TEST(CommonTypeButPreserveIntSignedness, UsesSignOfFirstArgumentIfCommonTypeIsIntegral) {
    StaticAssertTypeEq<CommonTypeButPreserveIntSignedness<int, uint8_t>, int>();
    StaticAssertTypeEq<CommonTypeButPreserveIntSignedness<uint8_t, int>, uint>();
}

}  // namespace detail
}  // namespace au
