// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct UnosLabel {
    static constexpr const char label[] = "U";
};
template <typename T>
constexpr const char UnosLabel<T>::label[];
struct Unos : UnitProductT<>, UnosLabel<void> {
    using UnosLabel<void>::label;
};
constexpr auto unos = QuantityMaker<Unos>{};

}  // namespace au
