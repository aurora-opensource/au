// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/quantity_point.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct MetersLabel {
    static constexpr const char label[] = "m";
};
template <typename T>
constexpr const char MetersLabel<T>::label[];
struct Meters : UnitImpl<Length>, MetersLabel<void> {
    using MetersLabel<void>::label;
};
constexpr auto meter = SingularNameFor<Meters>{};
constexpr auto meters = QuantityMaker<Meters>{};
constexpr auto meters_pt = QuantityPointMaker<Meters>{};

}  // namespace au
