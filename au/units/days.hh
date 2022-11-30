// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/hours.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct DaysLabel {
    static constexpr const char label[] = "d";
};
template <typename T>
constexpr const char DaysLabel<T>::label[];
struct Days : decltype(Hours{} * mag<24>()), DaysLabel<void> {
    using DaysLabel<void>::label;
};
constexpr auto day = SingularNameFor<Days>{};
constexpr auto days = QuantityMaker<Days>{};

}  // namespace au
