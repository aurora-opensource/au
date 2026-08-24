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

#include "au/constant.hh"
#include "au/magnitude.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/unit_symbol.hh"
#include "au/zero.hh"

namespace au {

namespace detail {
// Unary `+` promotes a char-like rep (e.g. `int8_t`), so that `<<` prints a number rather than a
// character.  Not every rep has one --- an Eigen vector does not --- so promote only where we can.
template <typename T>
constexpr auto promote_for_streaming(const T &x, int) -> decltype(+x) {
    return +x;
}
template <typename T>
constexpr const T &promote_for_streaming(const T &x, ...) {
    return x;
}
}  // namespace detail

// Streaming output support for Quantity types.
template <typename U, typename R>
std::ostream &operator<<(std::ostream &out, const Quantity<U, R> &q) {
    out << detail::promote_for_streaming(q.in(U{}), 0) << " " << unit_label(U{});
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

// Streaming support for Magnitude: print the magnitude label.
template <typename... BPs>
std::ostream &operator<<(std::ostream &out, Magnitude<BPs...> m) {
    return (out << mag_label(m));
}

// Streaming support for Constant: print the unit label.
template <typename U>
std::ostream &operator<<(std::ostream &out, Constant<U>) {
    return (out << unit_label(U{}));
}

// Streaming support for unit symbols: print the unit label.
template <typename U>
std::ostream &operator<<(std::ostream &out, SymbolFor<U>) {
    return (out << unit_label(U{}));
}

}  // namespace au
