// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/amperes.hh"
#include "au/units/watts.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct VoltsLabel {
    static constexpr const char label[] = "V";
};
template <typename T>
constexpr const char VoltsLabel<T>::label[];
struct Volts : decltype(Watts{} / Amperes{}), VoltsLabel<void> {
    using VoltsLabel<void>::label;
};
constexpr auto volt = SingularNameFor<Volts>{};
constexpr auto volts = QuantityMaker<Volts>{};

}  // namespace au
