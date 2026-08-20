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

#include "examples/atomic_units/atomic_units.hh"

// --8<-- [start:labels]
// C++14 needs these out-of-line definitions for the unit labels.  In C++17 or later, declare the
// labels `static constexpr inline` in the header instead, and delete this file.
namespace atomic_units {

constexpr const char ElectronMasses::label[];
constexpr const char Hartrees::label[];
constexpr const char BohrRadii::label[];
constexpr const char AtomicTimeUnits::label[];

}  // namespace atomic_units

// --8<-- [end:labels]
