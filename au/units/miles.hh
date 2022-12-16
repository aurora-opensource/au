// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/feet.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct MilesLabel {
    static constexpr const char label[] = "mi";
};
template <typename T>
constexpr const char MilesLabel<T>::label[];
struct Miles : decltype(Feet{} * mag<5'280>()), MilesLabel<void> {
    using MilesLabel<void>::label;
};
constexpr auto mile = SingularNameFor<Miles>{};
constexpr auto miles = QuantityMaker<Miles>{};

}  // namespace au
