// Copyright 2022 Aurora Operations, Inc.

#include "au/units/bytes.hh"

#include "au/testing.hh"
#include "au/units/bits.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Bytes, HasExpectedLabel) { expect_label<Bytes>("B"); }

TEST(Bytes, EquivalentTo8Bits) { EXPECT_EQ(bytes(1), bits(8)); }

}  // namespace au
