// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/radians.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct DegreesLabel {
    static constexpr const char label[] = "deg";
};
template <typename T>
constexpr const char DegreesLabel<T>::label[];
struct Degrees : decltype(Radians{} * PI / mag<180>()), DegreesLabel<void> {
    using DegreesLabel<void>::label;
};
constexpr auto degree = SingularNameFor<Degrees>{};
constexpr auto degrees = QuantityMaker<Degrees>{};

}  // namespace au
