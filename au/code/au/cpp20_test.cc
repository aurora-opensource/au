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

#include "au/quantity.hh"
#include "au/testing.hh"
#include "au/units/meters.hh"

namespace au {

using symbols::m;

struct Foo {
    auto operator<=>(const Foo &) const = default;

    QuantityD<Meters> thickness;
};

TEST(Quantity, SupportsSpaceship) { EXPECT_LT(Foo{5 * m}, Foo{6 * m}); }

}  // namespace au
