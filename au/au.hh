// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include <chrono>

#include "au/math.hh"

namespace au {

// Define 1:1 mapping between duration types of chrono library and our library.
template <typename RepT, typename Period>
struct CorrespondingQuantity<std::chrono::duration<RepT, Period>> {
    using Unit = decltype(Seconds{} * (mag<Period::num>() / mag<Period::den>()));
    using Rep = RepT;

    using ChronoDuration = std::chrono::duration<Rep, Period>;

    static constexpr Rep extract_value(ChronoDuration d) { return d.count(); }
    static constexpr ChronoDuration construct_from_value(Rep x) { return ChronoDuration{x}; }
};

}  // namespace au
