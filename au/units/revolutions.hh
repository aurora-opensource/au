// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/degrees.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct RevolutionsLabel {
    static constexpr const char label[] = "rev";
};
template <typename T>
constexpr const char RevolutionsLabel<T>::label[];
struct Revolutions : decltype(Degrees{} * mag<360>()), RevolutionsLabel<void> {
    using RevolutionsLabel<void>::label;
};
constexpr auto revolution = SingularNameFor<Revolutions>{};
constexpr auto revolutions = QuantityMaker<Revolutions>{};

}  // namespace au
