// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/feet.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct YardsLabel {
    static constexpr const char label[] = "yd";
};
template <typename T>
constexpr const char YardsLabel<T>::label[];
struct Yards : decltype(Feet{} * mag<3>()), YardsLabel<void> {
    using YardsLabel<void>::label;
};
constexpr auto yard = SingularNameFor<Yards>{};
constexpr auto yards = QuantityMaker<Yards>{};

}  // namespace au
