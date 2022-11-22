// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/units/grams.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct PoundsMassLabel {
    static constexpr const char label[] = "lb";
};
template <typename T>
constexpr const char PoundsMassLabel<T>::label[];
struct PoundsMass : decltype(Micro<Grams>{} * mag<453'592'370>()), PoundsMassLabel<void> {
    using PoundsMassLabel<void>::label;
};
constexpr auto pound_mass = SingularNameFor<PoundsMass>{};
constexpr auto pounds_mass = QuantityMaker<PoundsMass>{};

}  // namespace au
