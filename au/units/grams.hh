// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct GramsLabel {
    static constexpr const char label[] = "g";
};
template <typename T>
constexpr const char GramsLabel<T>::label[];
struct Grams : UnitImpl<Mass>, GramsLabel<void> {
    using GramsLabel<void>::label;
};
constexpr auto gram = SingularNameFor<Grams>{};
constexpr auto grams = QuantityMaker<Grams>{};

}  // namespace au
