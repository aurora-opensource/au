// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct AmperesLabel {
    static constexpr const char label[] = "A";
};
template <typename T>
constexpr const char AmperesLabel<T>::label[];
struct Amperes : UnitImpl<Current>, AmperesLabel<void> {
    using AmperesLabel<void>::label;
};
constexpr auto ampere = SingularNameFor<Amperes>{};
constexpr auto amperes = QuantityMaker<Amperes>{};

}  // namespace au
