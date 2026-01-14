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

#include <iostream>

#include "au/au.hh"
#include "au/io.hh"
#include "au/units/hours.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"

using ::au::symbols::h;
using ::au::symbols::m;
using ::au::symbols::s;
constexpr auto km = ::au::kilo(m);

int main(int argc, char **argv) {
    constexpr auto v1 = 1 * m / s;
    constexpr auto v2 = 1 * km / h;
    std::cout << "(" << v1 << ") + (" << v2 << ") = " << (v1 + v2) << std::endl;
    return 0;
}
