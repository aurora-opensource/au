// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/candelas.hh"
#include "au/units/steradians.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct LumensLabel {
    static constexpr const char label[] = "lm";
};
template <typename T>
constexpr const char LumensLabel<T>::label[];
struct Lumens : decltype(Candelas{} * Steradians{}), LumensLabel<void> {
    using LumensLabel<void>::label;
};
constexpr auto lumen = SingularNameFor<Lumens>{};
constexpr auto lumens = QuantityMaker<Lumens>{};

}  // namespace au
