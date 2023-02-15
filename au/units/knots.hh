// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/hours.hh"
#include "au/units/nautical_miles.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct KnotsLabel {
    static constexpr const char label[] = "kn";
};
template <typename T>
constexpr const char KnotsLabel<T>::label[];
struct Knots : decltype(NauticalMiles{} / Hours{}), KnotsLabel<void> {
    using KnotsLabel<void>::label;
};
constexpr auto knot = SingularNameFor<Knots>{};
constexpr auto knots = QuantityMaker<Knots>{};

}  // namespace au
