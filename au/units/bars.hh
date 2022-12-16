// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/units/pascals.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct BarsLabel {
    static constexpr const char label[] = "bar";
};
template <typename T>
constexpr const char BarsLabel<T>::label[];
struct Bars : decltype(Kilo<Pascals>{} * mag<100>()), BarsLabel<void> {
    using BarsLabel<void>::label;
};
constexpr auto bar = SingularNameFor<Bars>{};
constexpr auto bars = QuantityMaker<Bars>{};

}  // namespace au
