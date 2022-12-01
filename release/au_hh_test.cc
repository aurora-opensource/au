// Copyright 2022 Aurora Operations, Inc.

#include <chrono>

#include "docs/au.hh"
#include "gtest/gtest.h"

// clang-format off
//
// This file textually includes all of the test cases which every ready-made
// single-file release should pass.  It needs to go after all other includes.
#include "release/common_test_cases.hh"
// clang-format on

namespace au {

namespace {
template <typename T>
std::string stream_to_string(const T &x) {
    std::ostringstream oss;
    oss << x;
    return oss.str();
}
}  // namespace

TEST(AuHh, PrintsValueAndUnitLabel) {
    EXPECT_EQ(stream_to_string(meters(3)), "3 m");
    EXPECT_EQ(stream_to_string((meters / milli(second))(1.25)), "1.25 m / ms");
}

TEST(AuHh, DistinguishesPointFromQuantityByAtSign) {
    EXPECT_EQ(stream_to_string(kelvins(20)), "20 K");
    EXPECT_EQ(stream_to_string(kelvins_pt(20)), "@(20 K)");
}

TEST(AuHh, PrintsZero) { EXPECT_EQ(stream_to_string(ZERO), "0"); }

}  // namespace au
