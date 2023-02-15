// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/feet.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct FathomsLabel {
    static constexpr const char label[] = "ftm";
};
template <typename T>
constexpr const char FathomsLabel<T>::label[];
struct Fathoms : decltype(Feet{} * mag<6>()), FathomsLabel<void> {
    using FathomsLabel<void>::label;
};
constexpr auto fathom = SingularNameFor<Fathoms>{};
constexpr auto fathoms = QuantityMaker<Fathoms>{};

}  // namespace au
