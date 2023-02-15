// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/miles.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct FurlongsLabel {
    static constexpr const char label[] = "fur";
};
template <typename T>
constexpr const char FurlongsLabel<T>::label[];
struct Furlongs : decltype(Miles{} / mag<8>()), FurlongsLabel<void> {
    using FurlongsLabel<void>::label;
};
constexpr auto furlong = SingularNameFor<Furlongs>{};
constexpr auto furlongs = QuantityMaker<Furlongs>{};

}  // namespace au
