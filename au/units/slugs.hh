// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/feet.hh"
#include "au/units/pounds_force.hh"
#include "au/units/seconds.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct SlugsLabel {
    static constexpr const char label[] = "slug";
};
template <typename T>
constexpr const char SlugsLabel<T>::label[];
struct Slugs : decltype(PoundsForce{} * squared(Seconds{}) / Feet{}), SlugsLabel<void> {
    using SlugsLabel<void>::label;
};
constexpr auto slug = SingularNameFor<Slugs>{};
constexpr auto slugs = QuantityMaker<Slugs>{};

}  // namespace au
