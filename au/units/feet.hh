// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/inches.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct FeetLabel {
    static constexpr const char label[] = "ft";
};
template <typename T>
constexpr const char FeetLabel<T>::label[];
struct Feet : decltype(Inches{} * mag<12>()), FeetLabel<void> {
    using FeetLabel<void>::label;
};
constexpr auto foot = SingularNameFor<Feet>{};
constexpr auto feet = QuantityMaker<Feet>{};

}  // namespace au
