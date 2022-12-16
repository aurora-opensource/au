// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/quantity_point.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct KelvinsLabel {
    static constexpr const char label[] = "K";
};
template <typename T>
constexpr const char KelvinsLabel<T>::label[];
struct Kelvins : UnitImpl<Temperature>, KelvinsLabel<void> {
    using KelvinsLabel<void>::label;
};
constexpr auto kelvin = SingularNameFor<Kelvins>{};
constexpr auto kelvins = QuantityMaker<Kelvins>{};
constexpr auto kelvins_pt = QuantityPointMaker<Kelvins>{};

}  // namespace au
