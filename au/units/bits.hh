// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct BitsLabel {
    static constexpr const char label[] = "b";
};
template <typename T>
constexpr const char BitsLabel<T>::label[];
struct Bits : UnitImpl<Information>, BitsLabel<void> {
    using BitsLabel<void>::label;
};
constexpr auto bit = SingularNameFor<Bits>{};
constexpr auto bits = QuantityMaker<Bits>{};

}  // namespace au
