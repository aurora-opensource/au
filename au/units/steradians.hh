// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/radians.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct SteradiansLabel {
    static constexpr const char label[] = "sr";
};
template <typename T>
constexpr const char SteradiansLabel<T>::label[];
struct Steradians : decltype(squared(Radians{})), SteradiansLabel<void> {
    using SteradiansLabel<void>::label;
};
constexpr auto steradian = SingularNameFor<Steradians>{};
constexpr auto steradians = QuantityMaker<Steradians>{};

}  // namespace au
