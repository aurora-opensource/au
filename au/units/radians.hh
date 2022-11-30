// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct RadiansLabel {
    static constexpr const char label[] = "rad";
};
template <typename T>
constexpr const char RadiansLabel<T>::label[];
struct Radians : UnitImpl<Angle>, RadiansLabel<void> {
    using RadiansLabel<void>::label;
};
constexpr auto radian = SingularNameFor<Radians>{};
constexpr auto radians = QuantityMaker<Radians>{};

}  // namespace au
