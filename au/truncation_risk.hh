// Copyright 2025 Aurora Operations, Inc.
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

#include "au/abstract_operations.hh"

namespace au {
namespace detail {

template <typename T>
struct NoTruncationRisk {
    static constexpr bool would_value_truncate(const T &) { return false; }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImpl;
template <typename T, typename M>
struct ValueTimesRatioIsNotInteger : ValueTimesRatioIsNotIntegerImpl<T, M> {};

template <typename T>
using ValueIsNotInteger = ValueTimesRatioIsNotInteger<T, Magnitude<>>;

template <typename T>
struct ValueIsNotZero {
    static constexpr bool would_value_truncate(const T &x) { return x != T{0}; }
};

template <typename T>
struct CannotAssessTruncationRiskFor {
    static constexpr bool would_value_truncate(const T &) { return true; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS (`truncation_risk.hh`):
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ValueTimesRatioIsNotInteger` section:

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForIntWhereDenominatorDoesNotFit {
    static constexpr bool would_value_truncate(const T &value) { return value != T{0}; }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForIntWhereDenominatorFits {
    static constexpr bool would_value_truncate(const T &value) {
        return (value % get_value<RealPart<T>>(DenominatorT<M>{})) != T{0};
    }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForInt
    : std::conditional_t<get_value_result<RealPart<T>>(DenominatorT<M>{}).outcome ==
                             MagRepresentationOutcome::ERR_CANNOT_FIT,
                         ValueTimesRatioIsNotIntegerImplForIntWhereDenominatorDoesNotFit<T, M>,
                         ValueTimesRatioIsNotIntegerImplForIntWhereDenominatorFits<T, M>> {};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForFloatGeneric {
    static constexpr bool would_value_truncate(const T &value) {
        const auto result = value * get_value<RealPart<T>>(M{});
        return std::trunc(result) != result;
    }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForFloatDivideByInteger {
    static constexpr bool would_value_truncate(const T &value) {
        const auto result = value / get_value<RealPart<T>>(MagInverseT<M>{});
        return std::trunc(result) != result;
    }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForFloat
    : std::conditional_t<IsInteger<MagInverseT<M>>::value,
                         ValueTimesRatioIsNotIntegerImplForFloatDivideByInteger<T, M>,
                         ValueTimesRatioIsNotIntegerImplForFloatGeneric<T, M>> {};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImpl
    : std::conditional_t<std::is_integral<T>::value,
                         ValueTimesRatioIsNotIntegerImplForInt<T, M>,
                         ValueTimesRatioIsNotIntegerImplForFloat<T, M>> {};

}  // namespace detail
}  // namespace au
