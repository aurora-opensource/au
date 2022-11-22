// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct SecondsLabel {
    static constexpr const char label[] = "s";
};
template <typename T>
constexpr const char SecondsLabel<T>::label[];
struct Seconds : UnitImpl<Time>, SecondsLabel<void> {
    using SecondsLabel<void>::label;
};
constexpr auto second = SingularNameFor<Seconds>{};
constexpr auto seconds = QuantityMaker<Seconds>{};

}  // namespace au
