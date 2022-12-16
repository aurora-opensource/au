// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct CandelasLabel {
    static constexpr const char label[] = "cd";
};
template <typename T>
constexpr const char CandelasLabel<T>::label[];
struct Candelas : UnitImpl<LuminousIntensity>, CandelasLabel<void> {
    using CandelasLabel<void>::label;
};
constexpr auto candela = SingularNameFor<Candelas>{};
constexpr auto candelas = QuantityMaker<Candelas>{};

}  // namespace au
