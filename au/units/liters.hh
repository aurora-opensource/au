// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/units/meters.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct LitersLabel {
    static constexpr const char label[] = "L";
};
template <typename T>
constexpr const char LitersLabel<T>::label[];
struct Liters : decltype(cubed(Deci<Meters>{})), LitersLabel<void> {
    using LitersLabel<void>::label;
};
constexpr auto liter = SingularNameFor<Liters>{};
constexpr auto liters = QuantityMaker<Liters>{};

}  // namespace au
