// Copyright 2026 Aurora Operations, Inc.
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

#include "au/au.hh"

#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::Eq;

TEST(CudaCompilation, Vacuous) {
    // This test intentionally does nothing.
    // Its purpose is to verify that Au headers compile successfully with nvcc.
    EXPECT_THAT(1, Eq(1));
}

}  // namespace
}  // namespace au
