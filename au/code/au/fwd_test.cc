// Copyright 2024 Aurora Operations, Inc.
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

#include "au/fwd_test_lib.hh"
#include "au/quantity.hh"
#include "au/units/meters.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::StrEq;

namespace au {

TEST(Fwd, CanCallFunctionDeclaredWithOnlyFwdFiles) {
    EXPECT_THAT(print_to_string(meters(1)), StrEq("1 m"));
}

}  // namespace au
