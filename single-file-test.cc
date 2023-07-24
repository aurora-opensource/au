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

#include <algorithm>
#include <iostream>
#include <vector>

#include "au.hh"

// This file builds all of the code in the single-file package of Au, and runs some basic tests.
//
// If we find any failures not covered by this file, we can add more test cases as needed.
//
// Note that this file will *not* be built with bazel.  It's therefore important that we avoid all
// dependencies outside of the C++14 standard library, and the single-file package of Au itself.

using namespace au;

// This ad hoc utility is a stand-in for GTEST, which we can't use here.
template <typename ExpectedT, typename ActualT>
bool expect_equal(ExpectedT expected, ActualT actual) {
    if (expected != actual) {
        std::cerr << "Failure!  Expected (" << expected << "); Actual (" << actual << ")"
                  << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    const std::vector<bool> results{
        {
            expect_equal((meters / second)(5) * seconds(6), meters(30)),
        },
    };
    return std::all_of(std::begin(results), std::end(results), [](auto x) { return x; }) ? 0 : 1;
}
