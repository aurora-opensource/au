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

#include "au/chrono_interop.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace au {

TEST(DurationQuantity, InterconvertsWithExactlyEquivalentChronoDuration) {
    constexpr QuantityD<Seconds> from_chrono = std::chrono::duration<double>{1.23};
    EXPECT_THAT(from_chrono, SameTypeAndValue(seconds(1.23)));

    constexpr auto val = std::chrono::nanoseconds::rep{456};
    constexpr std::chrono::nanoseconds from_au = nano(seconds)(val);
    EXPECT_THAT(from_au.count(), SameTypeAndValue(val));
}

TEST(DurationQuantity, InterconvertsWithIndirectlyEquivalentChronoDuration) {
    constexpr QuantityD<Seconds> from_chrono = as_quantity(1234ms);
    EXPECT_THAT(from_chrono, SameTypeAndValue(seconds(1.234)));
}

TEST(AsChronoDuration, ProducesExpectedResults) {
    constexpr auto original = milli(seconds)(12.3f);
    constexpr auto result = as_chrono_duration(original);
    EXPECT_THAT(result.count(), SameTypeAndValue(12.3f));
    EXPECT_THAT(as_quantity(result), QuantityEquivalent(original));
}

}  // namespace au
