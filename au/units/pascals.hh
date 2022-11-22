// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/units/meters.hh"
#include "au/units/newtons.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct PascalsLabel {
    static constexpr const char label[] = "Pa";
};
template <typename T>
constexpr const char PascalsLabel<T>::label[];
struct Pascals : decltype(Newtons{} / squared(Meters{})), PascalsLabel<void> {
    using PascalsLabel<void>::label;
};
constexpr auto pascal = SingularNameFor<Pascals>{};
constexpr auto pascals = QuantityMaker<Pascals>{};
constexpr QuantityPointMaker<Pascals> pascals_pt{};

}  // namespace au
