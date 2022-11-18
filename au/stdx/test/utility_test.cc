// Copyright 2022 Aurora Operations, Inc.

#include "au/stdx/utility.hh"

#include "gtest/gtest.h"

namespace au {
namespace stdx {

TEST(CmpLess, HandlesMixedSignedUnsigned) {
    EXPECT_TRUE(cmp_less(-1, 1u));
    EXPECT_FALSE(cmp_less(1u, -1));
    EXPECT_FALSE(cmp_less(1u, 1));
    EXPECT_TRUE(cmp_less(1u, 2));
}

TEST(CmpEqual, HandlesMixedSignedUnsigned) {
    EXPECT_FALSE(cmp_equal(-1, 1u));
    EXPECT_FALSE(cmp_equal(1u, -1));
    EXPECT_TRUE(cmp_equal(1u, 1));
    EXPECT_FALSE(cmp_equal(1u, 2));
}

}  // namespace stdx
}  // namespace au
