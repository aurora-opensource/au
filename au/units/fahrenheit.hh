// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/units/kelvins.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-tech.github.io/au/howto/new-units).
template <typename T>
struct FahrenheitLabel {
    static constexpr const char label[] = "F";
};
template <typename T>
constexpr const char FahrenheitLabel<T>::label[];
struct Rankines : decltype(Kelvins{} * mag<5>() / mag<9>()) {};
constexpr auto rankines = QuantityMaker<Rankines>{};
struct Fahrenheit : Rankines, FahrenheitLabel<void> {
    using FahrenheitLabel<void>::label;
    static constexpr auto origin() { return centi(rankines)(459'67); }
};
constexpr auto fahrenheit = QuantityMaker<Fahrenheit>{};
constexpr auto fahrenheit_pt = QuantityPointMaker<Fahrenheit>{};

}  // namespace au
