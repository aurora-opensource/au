// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/amperes.hh"
#include "au/units/volts.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct OhmsLabel {
    static constexpr const char label[] = "ohm";
};
template <typename T>
constexpr const char OhmsLabel<T>::label[];
struct Ohms : decltype(Volts{} / Amperes{}), OhmsLabel<void> {
    using OhmsLabel<void>::label;
};
constexpr auto ohm = SingularNameFor<Ohms>{};
constexpr auto ohms = QuantityMaker<Ohms>{};

}  // namespace au
