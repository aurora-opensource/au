// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/moles.hh"
#include "au/units/seconds.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct KatalsLabel {
    static constexpr const char label[] = "kat";
};
template <typename T>
constexpr const char KatalsLabel<T>::label[];
struct Katals : decltype(Moles{} / Seconds{}), KatalsLabel<void> {
    using KatalsLabel<void>::label;
};
constexpr auto katal = SingularNameFor<Katals>{};
constexpr auto katals = QuantityMaker<Katals>{};

}  // namespace au
