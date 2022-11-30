// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/meters.hh"
#include "au/units/newtons.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct JoulesLabel {
    static constexpr const char label[] = "J";
};
template <typename T>
constexpr const char JoulesLabel<T>::label[];
struct Joules : decltype(Newtons{} * Meters{}), JoulesLabel<void> {
    using JoulesLabel<void>::label;
};
constexpr auto joule = SingularNameFor<Joules>{};
constexpr auto joules = QuantityMaker<Joules>{};

}  // namespace au
