// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/pounds_mass.hh"
#include "au/units/standard_gravity.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct PoundsForceLabel {
    static constexpr const char label[] = "lbf";
};
template <typename T>
constexpr const char PoundsForceLabel<T>::label[];
struct PoundsForce : decltype(PoundsMass{} * StandardGravity{}), PoundsForceLabel<void> {
    using PoundsForceLabel<void>::label;
};
constexpr auto pound_force = SingularNameFor<PoundsForce>{};
constexpr auto pounds_force = QuantityMaker<PoundsForce>{};

}  // namespace au
