// Copyright 2022 Aurora Operations, Inc.

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
