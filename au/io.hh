// Copyright 2022 Aurora Operations, Inc.
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

#include <ostream>

#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/zero.hh"

namespace au {

// Streaming output support for Quantity types.
template <typename U, typename R>
std::ostream &operator<<(std::ostream &out, const Quantity<U, R> &q) {
    out << +q.in(U{}) << " " << unit_label(U{});
    return out;
}

// Streaming output support for QuantityPoint types.
template <typename U, typename R>
std::ostream &operator<<(std::ostream &out, const QuantityPoint<U, R> &p) {
    out << "@(" << (p - rep_cast<R>(make_quantity_point<U>(0))) << ")";
    return out;
}

// Streaming output support for Zero.  (Useful for printing in unit test failures.)
inline std::ostream &operator<<(std::ostream &out, Zero) {
    out << "0";
    return out;
}

}  // namespace au
