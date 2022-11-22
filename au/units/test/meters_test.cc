// Copyright 2022 Aurora Operations, Inc.

#include "au/units/meters.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "au/units/inches.hh"
#include "gtest/gtest.h"

namespace au {

TEST(Meters, HasExpectedLabel) { expect_label<Meters>("m"); }

TEST(Meters, HasExpectedRelationshipsWithInches) { EXPECT_EQ(centi(meters)(254), inches(100)); }

}  // namespace au
