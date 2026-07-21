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

#include "au/version.hh"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

using ::testing::AnyOf;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::Lt;

TEST(Version, ComponentMacrosAreDefined) {
#ifndef AU_VERSION_MAJOR
    FAIL() << "AU_VERSION_MAJOR is not defined";
#endif
#ifndef AU_VERSION_MINOR
    FAIL() << "AU_VERSION_MINOR is not defined";
#endif
#ifndef AU_VERSION_PATCH
    FAIL() << "AU_VERSION_PATCH is not defined";
#endif
    // Each component must fit in the three decimal digits that `AU_VERSION_NUMBER` allots it.
    EXPECT_THAT(AU_VERSION_MAJOR, Lt(1000));
    EXPECT_THAT(AU_VERSION_MINOR, Lt(1000));
    EXPECT_THAT(AU_VERSION_PATCH, Lt(1000));
}

TEST(Version, CombinedVersionMatchesComponents) {
    EXPECT_THAT(AU_VERSION,
                Eq(AU_VERSION_MAJOR * 1000000 + AU_VERSION_MINOR * 1000 + AU_VERSION_PATCH));
}

TEST(Version, VersionNumberOrdersLikeSemanticVersioning) {
    // Patch increments increase the number.
    EXPECT_THAT(AU_VERSION_NUMBER(0, 5, 1), Gt(AU_VERSION_NUMBER(0, 5, 0)));
    // Minor increments outweigh any patch difference.
    EXPECT_THAT(AU_VERSION_NUMBER(0, 6, 0), Gt(AU_VERSION_NUMBER(0, 5, 999)));
    // Major increments outweigh any minor difference.
    EXPECT_THAT(AU_VERSION_NUMBER(1, 0, 0), Gt(AU_VERSION_NUMBER(0, 999, 999)));
}

TEST(Version, IncludingAnyAuHeaderDefinesVersion) {
    // This is the "was Au included?" use case: `AU_VERSION` is truthy whenever Au is in scope.
    EXPECT_THAT(AU_VERSION, Ge(AU_VERSION_NUMBER(0, 5, 0)));
}

TEST(Version, IsReleaseFlagIsDefinedAndBoolean) {
#ifndef AU_VERSION_IS_RELEASE
    FAIL() << "AU_VERSION_IS_RELEASE is not defined";
#endif
    // Note: we deliberately do _not_ assert that this is `0`.  It is `0` on `main`, but `1` on the
    // release branch (where CI also runs), so pinning the value would break release-branch builds.
    // The "`main` must be `0`" invariant is a process guarantee (see `RELEASE.md`), not a test.
    EXPECT_THAT(AU_VERSION_IS_RELEASE, AnyOf(Eq(0), Eq(1)));
}

}  // namespace
