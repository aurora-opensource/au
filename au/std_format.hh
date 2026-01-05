// Copyright 2025 Aurora Operations, Inc.
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

#pragma once

#include <format>

#include "au/quantity.hh"
#include "au/quantity_point.hh"

namespace std {
template <typename U, typename R>
struct formatter<au::Quantity<U, R>> : ::au::QuantityFormatter<U, R, ::std::formatter> {};

template <typename U, typename R>
struct formatter<au::QuantityPoint<U, R>> : ::au::QuantityPointFormatter<U, R, ::std::formatter> {};
}  // namespace std
