// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/seconds.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct MinutesLabel {
    static constexpr const char label[] = "min";
};
template <typename T>
constexpr const char MinutesLabel<T>::label[];
struct Minutes : decltype(Seconds{} * mag<60>()), MinutesLabel<void> {
    using MinutesLabel<void>::label;
};
constexpr auto minute = SingularNameFor<Minutes>{};
constexpr auto minutes = QuantityMaker<Minutes>{};

}  // namespace au
