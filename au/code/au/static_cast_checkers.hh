// Copyright 2024 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <exception>
#include <limits>
#include <type_traits>

namespace au {

struct Inches;

namespace detail {

template <typename Source, typename Dest>
struct StaticCastChecker;

template <typename Dest, typename Source>
constexpr bool will_static_cast_overflow(Source x) {
    return StaticCastChecker<Source, Dest>::will_static_cast_overflow(x);
}

template <typename Dest, typename Source>
constexpr bool will_static_cast_truncate(Source x) {
    return StaticCastChecker<Source, Dest>::will_static_cast_truncate(x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Overflow checking:

// We should categorize the situation as the FIRST match: each category's implementation may assume
// that no higher category is a match.
enum class OverflowSituation {
    DEST_IS_STRICT_SUPERSET,
    UNSIGNED_TO_INTEGRAL,
    SIGNED_TO_UNSIGNED,
    SIGNED_TO_SIGNED,
    UNEXPLORED,
};

template <typename Source, typename Dest>
constexpr OverflowSituation categorize_overflow_situation() {
    if (std::is_same<Source, Dest>::value) {
        return OverflowSituation::DEST_IS_STRICT_SUPERSET;
    }

    if (std::is_integral<Source>::value && std::is_integral<Dest>::value) {
        if ((std::is_signed<Source>::value == std::is_signed<Dest>::value) &&
            (sizeof(Source) < sizeof(Dest))) {
            return OverflowSituation::DEST_IS_STRICT_SUPERSET;
        }

        if (std::is_unsigned<Source>::value) {
            return OverflowSituation::UNSIGNED_TO_INTEGRAL;
        }

        return std::is_unsigned<Dest>::value ? OverflowSituation::SIGNED_TO_UNSIGNED
                                             : OverflowSituation::SIGNED_TO_SIGNED;
    }

    return OverflowSituation::UNEXPLORED;
}

template <typename Source, typename Dest, OverflowSituation Cat>
struct StaticCastOverflowImpl {
    static constexpr bool will_static_cast_overflow(Source x) { return true; }
};

template <typename Source, typename Dest>
struct StaticCastOverflowImpl<Source, Dest, OverflowSituation::UNSIGNED_TO_INTEGRAL> {
    static constexpr bool will_static_cast_overflow(Source x) {
        return x > static_cast<Source>(std::numeric_limits<Dest>::max());
    }
};

template <typename Source, typename Dest>
struct StaticCastOverflowImpl<Source, Dest, OverflowSituation::SIGNED_TO_UNSIGNED> {
    static constexpr bool will_static_cast_overflow(Source x) {
        return (x < 0) ||
               (static_cast<std::make_unsigned_t<Source>>(x) >
                static_cast<std::make_unsigned_t<Source>>(std::numeric_limits<Dest>::max()));
    }
};

template <typename Source, typename Dest>
struct StaticCastOverflowImpl<Source, Dest, OverflowSituation::SIGNED_TO_SIGNED> {
    static constexpr bool will_static_cast_overflow(Source x) {
        return (x < static_cast<Source>(std::numeric_limits<Dest>::lowest())) ||
               (x > static_cast<Source>(std::numeric_limits<Dest>::max()));
    }
};

template <typename Source, typename Dest>
struct StaticCastOverflowImpl<Source, Dest, OverflowSituation::DEST_IS_STRICT_SUPERSET> {
    static constexpr bool will_static_cast_overflow(Source x) { return false; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Truncation checking:

enum class TruncationSituation {
    INTEGRAL_TO_INTEGRAL,
    UNEXPLORED,
};

template <typename Source, typename Dest>
constexpr TruncationSituation categorize_truncation_situation() {
    if (std::is_integral<Source>::value && std::is_integral<Dest>::value) {
        return TruncationSituation::INTEGRAL_TO_INTEGRAL;
    }

    return TruncationSituation::UNEXPLORED;
}

template <typename Source, typename Dest, TruncationSituation Cat>
struct StaticCastTruncateImpl {
    static constexpr bool will_static_cast_truncate(Source x) { return true; }
};

template <typename Source, typename Dest>
struct StaticCastTruncateImpl<Source, Dest, TruncationSituation::INTEGRAL_TO_INTEGRAL> {
    static constexpr bool will_static_cast_truncate(Source) { return false; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Main implementation:

template <typename Source, typename Dest>
struct StaticCastChecker
    : StaticCastOverflowImpl<Source, Dest, categorize_overflow_situation<Source, Dest>()>,
      StaticCastTruncateImpl<Source, Dest, categorize_truncation_situation<Source, Dest>()> {};

}  // namespace detail
}  // namespace au
