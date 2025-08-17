// Copyright 2022 Aurora Operations, Inc.
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

#include "au/chrono_policy_validation.hh"

#include "au/prefix.hh"
#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace au {

using ::testing::IsFalse;
using ::testing::IsTrue;

struct Meters : UnitImpl<Length> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Individual operations to validate.

struct Assignment {
    template <typename T, typename U>
    constexpr auto operator()(T t, U u) const
        -> std::remove_reference_t<decltype(t = u)> {  // Trailing return enables SFINAE.
        return (t = u);
    }
};

struct Equality {
    template <typename T, typename U>
    constexpr auto operator()(T t, U u) const
        -> decltype(t == u) {  // Trailing return enables SFINAE.
        return (t == u);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Tests for the machinery introduced in this file.

TEST(MapToUnitsLib, CreatesEquivalentType) {
    EXPECT_THAT(map_to_au(8s), QuantityEquivalent(some_units((8s).count())));

    EXPECT_THAT(map_to_au(std::chrono::duration<double>(3.4)), QuantityEquivalent(some_units(3.4)));

    EXPECT_THAT(map_to_au(3ms), QuantityEquivalent(milli(some_units)((3ms).count())));
}

TEST(MapToUnitsLib, IsIdentityForNonChronoDurationObjects) {
    EXPECT_THAT(map_to_au(true), SameTypeAndValue(true));
    EXPECT_THAT(map_to_au(3.14), SameTypeAndValue(3.14));
    EXPECT_THAT(map_to_au('c'), SameTypeAndValue('c'));
}

TEST(HasOp, DetectsOpExistence) {
    EXPECT_THAT((HasOp<Assignment, std::chrono::milliseconds, std::chrono::seconds>::value),
                IsTrue());
    EXPECT_THAT((HasOp<Assignment, std::chrono::milliseconds, std::chrono::nanoseconds>::value),
                IsFalse());
}

TEST(BothPermit, TrueWhenBothOperationsPermittedAndCompatible) {
    EXPECT_THAT(both_permit<Assignment>(std::chrono::seconds{}, 4s), IsTrue());
}

TEST(BothPermit, IfExpectedValueSuppliedWeCheckBothTypeAndValue) {
    EXPECT_THAT(both_permit<Assignment>(std::chrono::seconds{}, 4s, 4s), IsTrue());
    EXPECT_THAT(both_permit<Assignment>(std::chrono::seconds{}, 4s, 5s), IsFalse());
    EXPECT_THAT(both_permit<Assignment>(std::chrono::seconds{}, 4s, 4000ms), IsFalse());
}

TEST(BothForbid, TrueWhenBothOperationsForbidden) {
    // Assigning Milli<X> to Kilo<X> will generally lose precision when using integral types.
    EXPECT_THAT(both_forbid<Assignment>(std::chrono::duration<int, std::kilo>(1),
                                        std::chrono::duration<size_t, std::milli>(1)),
                IsTrue());
}

TEST(ChronoPermitsButAuForbids, TrueWhenWeAreMoreRestrictive) {
    // Assigning X to Milli<X> in a 16-bit Rep won't lose information, but _will_ run a risk of
    // overflow which the au library considers too great to permit.
    EXPECT_THAT(
        chrono_permits_but_au_forbids<Assignment>(std::chrono::duration<uint16_t, std::milli>(1),
                                                  std::chrono::duration<uint16_t>(1)),
        IsTrue());
}

}  // namespace au
