// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/minutes.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct HoursLabel {
    static constexpr const char label[] = "h";
};
template <typename T>
constexpr const char HoursLabel<T>::label[];
struct Hours : decltype(Minutes{} * mag<60>()), HoursLabel<void> {
    using HoursLabel<void>::label;
};
constexpr auto hour = SingularNameFor<Hours>{};
constexpr auto hours = QuantityMaker<Hours>{};

}  // namespace au
