// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/joules.hh"
#include "au/units/seconds.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct WattsLabel {
    static constexpr const char label[] = "W";
};
template <typename T>
constexpr const char WattsLabel<T>::label[];
struct Watts : decltype(Joules{} / Seconds{}), WattsLabel<void> {
    using WattsLabel<void>::label;
};
constexpr auto watt = SingularNameFor<Watts>{};
constexpr auto watts = QuantityMaker<Watts>{};

}  // namespace au
