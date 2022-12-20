// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/seconds.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct HertzLabel {
    static constexpr const char label[] = "Hz";
};
template <typename T>
constexpr const char HertzLabel<T>::label[];
struct Hertz : UnitInverseT<Seconds>, HertzLabel<void> {
    using HertzLabel<void>::label;
};
constexpr auto hertz = QuantityMaker<Hertz>{};

}  // namespace au
