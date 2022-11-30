// Copyright 2022 Aurora Operations, Inc.

#include "au/units/bits.hh"

#include "au/testing.hh"
#include "au/units/bytes.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Bits, HasExpectedLabel) { expect_label<Bits>("b"); }

TEST(Bits, OneEighthOfAByte) { EXPECT_EQ(bits(1.0), bytes(1.0 / 8.0)); }

}  // namespace au
