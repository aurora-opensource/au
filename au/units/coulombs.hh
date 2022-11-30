// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/amperes.hh"
#include "au/units/seconds.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct CoulombsLabel {
    static constexpr const char label[] = "C";
};
template <typename T>
constexpr const char CoulombsLabel<T>::label[];
struct Coulombs : decltype(Amperes{} * Seconds{}), CoulombsLabel<void> {
    using CoulombsLabel<void>::label;
};
constexpr auto coulomb = SingularNameFor<Coulombs>{};
constexpr auto coulombs = QuantityMaker<Coulombs>{};

}  // namespace au
