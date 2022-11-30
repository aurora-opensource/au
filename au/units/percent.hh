// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/unos.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct PercentLabel {
    static constexpr const char label[] = "%";
};
template <typename T>
constexpr const char PercentLabel<T>::label[];
struct Percent : decltype(Unos{} / mag<100>()), PercentLabel<void> {
    using PercentLabel<void>::label;
};
constexpr auto percent = QuantityMaker<Percent>{};

}  // namespace au
