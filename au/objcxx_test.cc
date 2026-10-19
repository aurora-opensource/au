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

// On clang, this file is compiled as Objective-C++ (see `BUILD.bazel`); elsewhere, as plain C++.
// The point is to make sure that Au's headers stay compatible with the identifiers and macros that
// Objective-C++ users will have in scope, so we define them _before_ including any Au headers.

#if defined(__clang__) && !defined(__OBJC__)
#error "On clang, this test must be compiled in Objective-C++ mode"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// A minimal stand-in for `<objc/objc.h>`, so that we can test Au in Objective-C++ mode without
// depending on Apple's SDK or the GNUstep runtime.
//
// We mirror the identifiers and macros that the real header provides, because these are the only
// way that Objective-C++ can break otherwise-valid C++ code.  In particular, `YES` and `NO` are
// macros, not keywords, so any Au identifier with those names would get clobbered.

typedef struct objc_class *Class;
typedef struct objc_object *id;
typedef struct objc_selector *SEL;
typedef id (*IMP)(id, SEL, ...);
typedef bool BOOL;

#if defined(__has_feature)
#if __has_feature(objc_bool)
#define AU_OBJCXX_TEST_HAS_OBJC_BOOL
#endif
#endif

#if defined(AU_OBJCXX_TEST_HAS_OBJC_BOOL)
#define YES __objc_yes
#define NO __objc_no
#else
#define YES ((BOOL)1)
#define NO ((BOOL)0)
#endif

#define nil nullptr
#define Nil nullptr
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "au/au.hh"
#include "au/io.hh"
#include "au/testing.hh"
#include "au/units/feet.hh"
#include "au/units/inches.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::Eq;
using ::testing::Lt;

TEST(ObjectiveCpp, ObjcMacrosAreInScope) {
    const BOOL yes = YES;
    const BOOL no = NO;
    EXPECT_THAT(yes, Eq(true));
    EXPECT_THAT(no, Eq(false));
}

TEST(ObjectiveCpp, BasicQuantityOperationsWork) {
    constexpr auto distance = meters(10.0);
    constexpr auto time = seconds(2.0);
    EXPECT_THAT(distance / time, SameTypeAndValue((meters / second)(5.0)));
}

TEST(ObjectiveCpp, ConversionsWork) {
    EXPECT_THAT(feet(3).in(inches), Eq(36));
    EXPECT_THAT(meters(1.0), IsNear(feet(3.28084), micro(meters)(1)));
}

TEST(ObjectiveCpp, ComparisonsAcrossUnitsWork) { EXPECT_THAT(feet(3), Lt(meters(1))); }

}  // namespace
}  // namespace au
