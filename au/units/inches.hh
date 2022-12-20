// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/units/meters.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct InchesLabel {
    static constexpr const char label[] = "in";
};
template <typename T>
constexpr const char InchesLabel<T>::label[];
struct Inches : decltype(Centi<Meters>{} * mag<254>() / mag<100>()), InchesLabel<void> {
    using InchesLabel<void>::label;
};
constexpr auto inch = SingularNameFor<Inches>{};
constexpr auto inches = QuantityMaker<Inches>{};

}  // namespace au
