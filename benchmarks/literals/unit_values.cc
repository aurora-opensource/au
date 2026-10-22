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

// Compile time benchmark: unit literals (`_s`), converted to nanoseconds as `double`.
//
// The literals themselves live in `literal_list.hh`, shared with the other benchmarks in this
// folder.

#include "au/prefix.hh"
#include "au/units/literals/seconds.hh"
#include "au/units/seconds.hh"

namespace au {
namespace benchmarks {
using namespace au_literals;

#define AU_BENCHMARK_LITERAL(x) (x##_s).in<double>(nano(seconds)),
extern const double UNIT_LITERAL_VALUES_IN_NS[];
constexpr double UNIT_LITERAL_VALUES_IN_NS[] = {
#include "benchmarks/literals/literal_list.hh"
};
#undef AU_BENCHMARK_LITERAL

static_assert(sizeof(UNIT_LITERAL_VALUES_IN_NS) / sizeof(UNIT_LITERAL_VALUES_IN_NS[0]) == 64u,
              "Expected 64 literals");

}  // namespace benchmarks
}  // namespace au
