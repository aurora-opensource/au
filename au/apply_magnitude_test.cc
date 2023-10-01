// Copyright 2023 Aurora Operations, Inc.
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

#include "au/apply_magnitude.hh"

#include "gtest/gtest.h"

namespace au {
namespace detail {

TEST(CategorizeMagnitude, FindsIntegerMultiplyInstances) {
    EXPECT_EQ(categorize_magnitude(mag<2>()), ApplyAs::INTEGER_MULTIPLY);
    EXPECT_EQ(categorize_magnitude(mag<35>()), ApplyAs::INTEGER_MULTIPLY);

    EXPECT_EQ(categorize_magnitude(mag<35>() / mag<7>()), ApplyAs::INTEGER_MULTIPLY);
}

TEST(CategorizeMagnitude, FindsIntegerDivideInstances) {
    EXPECT_EQ(categorize_magnitude(ONE / mag<2>()), ApplyAs::INTEGER_DIVIDE);
    EXPECT_EQ(categorize_magnitude(ONE / mag<35>()), ApplyAs::INTEGER_DIVIDE);

    EXPECT_EQ(categorize_magnitude(mag<7>() / mag<35>()), ApplyAs::INTEGER_DIVIDE);
}

TEST(CategorizeMagnitude, FindsRationalMultiplyInstances) {
    EXPECT_EQ(categorize_magnitude(mag<5>() / mag<2>()), ApplyAs::RATIONAL_MULTIPLY);
}

TEST(CategorizeMagnitude, FindsIrrationalMultiplyInstances) {
    EXPECT_EQ(categorize_magnitude(sqrt(mag<2>())), ApplyAs::IRRATIONAL_MULTIPLY);
    EXPECT_EQ(categorize_magnitude(PI), ApplyAs::IRRATIONAL_MULTIPLY);
}

}  // namespace detail
}  // namespace au
