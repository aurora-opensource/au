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

// Compile time benchmark: prefixed unit literals (`micro(..._s)`), built and nothing else.
//
// The literals themselves live in `literal_list.hh`, shared with the other benchmarks in this
// folder.

#include "au/prefix.hh"
#include "au/units/literals/seconds.hh"

namespace au {
namespace benchmarks {
using namespace au_literals;

// Accepts anything, so that we can make an array out of literals of different types.  This lets us
// build each literal without doing anything else with it.
struct Sink {
    template <typename T>
    constexpr Sink(T) {}
};

#define AU_BENCHMARK_LITERAL(x) micro(x##_s),
extern const Sink PREFIXED_UNIT_LITERALS[];
constexpr Sink PREFIXED_UNIT_LITERALS[] = {
#include "benchmarks/literals/literal_list.hh"
};
#undef AU_BENCHMARK_LITERAL

static_assert(sizeof(PREFIXED_UNIT_LITERALS) / sizeof(PREFIXED_UNIT_LITERALS[0]) == 64u,
              "Expected 64 literals");

}  // namespace benchmarks
}  // namespace au
