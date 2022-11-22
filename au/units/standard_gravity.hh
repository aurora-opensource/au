// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct StandardGravityLabel {
    static constexpr const char label[] = "g_0";
};
template <typename T>
constexpr const char StandardGravityLabel<T>::label[];
struct StandardGravity
    : decltype((Meters{} / squared(Seconds{})) * (mag<980'665>() / mag<100'000>())),
      StandardGravityLabel<void> {
    using StandardGravityLabel<void>::label;
};
constexpr auto standard_gravity = QuantityMaker<StandardGravity>{};

}  // namespace au
