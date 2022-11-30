// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/units/grams.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct NewtonsLabel {
    static constexpr const char label[] = "N";
};
template <typename T>
constexpr const char NewtonsLabel<T>::label[];
struct Newtons : decltype(Kilo<Grams>{} * Meters{} / squared(Seconds{})), NewtonsLabel<void> {
    using NewtonsLabel<void>::label;
};
constexpr auto newton = SingularNameFor<Newtons>{};
constexpr auto newtons = QuantityMaker<Newtons>{};

}  // namespace au
