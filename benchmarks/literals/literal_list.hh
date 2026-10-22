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

// The literals for the benchmarks in this folder, as a list of `AU_BENCHMARK_LITERAL(x)` entries.
// Each benchmark defines `AU_BENCHMARK_LITERAL` to turn the bare literal `x` into whatever it wants
// to measure (say, `x##_mag,`), and then includes this file inside a braced list.
//
// The literals cover the different forms a literal can take.  Each has a distinct value, so each
// produces its own magnitude (and, for unit literals, its own unit types), which is what makes them
// expensive.
//
// There is no `#pragma once`, because this file is meant to be included where it is used.

#ifndef AU_BENCHMARK_LITERAL
#error "Define AU_BENCHMARK_LITERAL(x) before including this file"
#endif

// Decimal integers, with and without digit separators.
AU_BENCHMARK_LITERAL(7)
AU_BENCHMARK_LITERAL(42)
AU_BENCHMARK_LITERAL(365)
AU_BENCHMARK_LITERAL(8'191)
AU_BENCHMARK_LITERAL(86'400)
AU_BENCHMARK_LITERAL(1'000'003)
AU_BENCHMARK_LITERAL(31'556'952)
AU_BENCHMARK_LITERAL(299'792'458)

// Decimals.
AU_BENCHMARK_LITERAL(0.5)
AU_BENCHMARK_LITERAL(2.75)
AU_BENCHMARK_LITERAL(12.34)
AU_BENCHMARK_LITERAL(3.14159)
AU_BENCHMARK_LITERAL(98.6)
AU_BENCHMARK_LITERAL(1234.567)
AU_BENCHMARK_LITERAL(0.001234)
AU_BENCHMARK_LITERAL(60.0221)

// Decimals with unusual spellings: no integer part, no fractional part, digit separators.
AU_BENCHMARK_LITERAL(.25)
AU_BENCHMARK_LITERAL(.0078)
AU_BENCHMARK_LITERAL(17.)
AU_BENCHMARK_LITERAL(4096.)
AU_BENCHMARK_LITERAL(1'234.567'8)
AU_BENCHMARK_LITERAL(0.000'001'5)
AU_BENCHMARK_LITERAL(9'806.65)
AU_BENCHMARK_LITERAL(012.5)

// Scientific notation, positive exponent.
AU_BENCHMARK_LITERAL(1e3)
AU_BENCHMARK_LITERAL(34e6)
AU_BENCHMARK_LITERAL(2E9)
AU_BENCHMARK_LITERAL(6.022e23)
AU_BENCHMARK_LITERAL(1.5e+2)
AU_BENCHMARK_LITERAL(9.81e1)
AU_BENCHMARK_LITERAL(4.2E7)
AU_BENCHMARK_LITERAL(1'000e3)

// Scientific notation, negative exponent.
AU_BENCHMARK_LITERAL(5e-3)
AU_BENCHMARK_LITERAL(1e-9)
AU_BENCHMARK_LITERAL(2.5e-4)
AU_BENCHMARK_LITERAL(6.62607015e-34)
AU_BENCHMARK_LITERAL(1.602e-19)
AU_BENCHMARK_LITERAL(3E-6)
AU_BENCHMARK_LITERAL(7.5e-12)
AU_BENCHMARK_LITERAL(01e-2)

// Hexadecimal integers.
AU_BENCHMARK_LITERAL(0x1F)
AU_BENCHMARK_LITERAL(0xFF)
AU_BENCHMARK_LITERAL(0XAB)
AU_BENCHMARK_LITERAL(0x1e3)
AU_BENCHMARK_LITERAL(0xDEAD'BEEF)
AU_BENCHMARK_LITERAL(0x7FFF)
AU_BENCHMARK_LITERAL(0xC0FFEE)
AU_BENCHMARK_LITERAL(0x10000)

// Binary integers.
AU_BENCHMARK_LITERAL(0b1)
AU_BENCHMARK_LITERAL(0b11)
AU_BENCHMARK_LITERAL(0b1011)
AU_BENCHMARK_LITERAL(0B1101'0110)
AU_BENCHMARK_LITERAL(0b1'0000'0001)
AU_BENCHMARK_LITERAL(0b111'1111)
AU_BENCHMARK_LITERAL(0b1001'1001)
AU_BENCHMARK_LITERAL(0b10'1010'1010)

// Octal integers.
AU_BENCHMARK_LITERAL(017)
AU_BENCHMARK_LITERAL(0755)
AU_BENCHMARK_LITERAL(0644)
AU_BENCHMARK_LITERAL(01'000)
AU_BENCHMARK_LITERAL(0123)
AU_BENCHMARK_LITERAL(076'543)
AU_BENCHMARK_LITERAL(0404)
AU_BENCHMARK_LITERAL(02'000'001)
