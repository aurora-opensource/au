// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include "au/quantity.hh"
#include "au/units/bits.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/howto/new-units).
template <typename T>
struct BytesLabel {
    static constexpr const char label[] = "B";
};
template <typename T>
constexpr const char BytesLabel<T>::label[];
struct Bytes : decltype(Bits{} * mag<8>()), BytesLabel<void> {
    using BytesLabel<void>::label;
};
constexpr auto byte = SingularNameFor<Bytes>{};
constexpr auto bytes = QuantityMaker<Bytes>{};

}  // namespace au
