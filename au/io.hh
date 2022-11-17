// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#pragma once

#include <ostream>

#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/zero.hh"

namespace au {

// Streaming output support for Quantity types.
template <typename U, typename R>
std::ostream &operator<<(std::ostream &out, const Quantity<U, R> &q) {
    out << q.in(U{}) << " " << unit_label(U{});
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
