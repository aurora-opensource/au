// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/meters.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct NauticalMilesLabel {
    static constexpr const char label[] = "nmi";
};
template <typename T>
constexpr const char NauticalMilesLabel<T>::label[];
struct NauticalMiles : decltype(Meters{} * mag<1'852>()), NauticalMilesLabel<void> {
    using NauticalMilesLabel<void>::label;
};
constexpr auto nautical_mile = SingularNameFor<NauticalMiles>{};
constexpr auto nautical_miles = QuantityMaker<NauticalMiles>{};

}  // namespace au
