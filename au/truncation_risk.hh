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

template <typename Op>
struct TruncationRiskForImpl;
template <typename Op>
using TruncationRiskFor = typename TruncationRiskForImpl<Op>::type;

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
// `StaticCast<T, U>` section:

// (A) -> (A)
template <typename T, typename U>
struct TruncationRiskForStaticCastFromArithmeticToArithmetic
    : std::conditional<stdx::conjunction<std::is_floating_point<T>, std::is_integral<U>>::value,
                       ValueIsNotInteger<T>,
                       NoTruncationRisk<T>> {};

// (A) -> (X)
template <typename T, typename U>
struct TruncationRiskForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         TruncationRiskForStaticCastFromArithmeticToArithmetic<T, U>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T>>> {};

// (X) -> (X)
template <typename T, typename U>
struct TruncationRiskForStaticCastAssumingScalar
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForStaticCastFromArithmetic<T, U>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T>>> {};

template <typename T, typename U>
struct TruncationRiskForImpl<StaticCast<T, U>>
    : TruncationRiskForStaticCastAssumingScalar<RealPart<T>, RealPart<U>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` section:

template <typename T, typename M>
struct TruncationRiskForMultiplyArithmeticByIrrational
    : std::conditional<std::is_integral<T>::value, ValueIsNotZero<T>, NoTruncationRisk<T>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyByIrrational
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForMultiplyArithmeticByIrrational<T, M>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T>>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyArithmeticByRationalNontrivialDenominator
    : std::conditional<(get_value_result<RealPart<T>>(DenominatorT<M>{}).outcome ==
                        MagRepresentationOutcome::ERR_CANNOT_FIT),
                       ValueIsNotZero<T>,
                       ValueTimesRatioIsNotInteger<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyArithmeticByRational
    : std::conditional_t<stdx::disjunction<IsInteger<M>, std::is_floating_point<T>>::value,
                         stdx::type_identity<NoTruncationRisk<T>>,
                         TruncationRiskForMultiplyArithmeticByRationalNontrivialDenominator<T, M>> {
};

template <typename T, typename M>
struct TruncationRiskForMultiplyByRational
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForMultiplyArithmeticByRational<T, M>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T>>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyByAssumingScalar
    : std::conditional_t<IsRational<M>::value,
                         TruncationRiskForMultiplyByRational<T, M>,
                         TruncationRiskForMultiplyByIrrational<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForImpl<MultiplyTypeBy<T, M>>
    : TruncationRiskForMultiplyByAssumingScalar<RealPart<T>, M> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger<T, M>` section:

template <typename T, typename M>
struct TruncationRiskForDivideNonArithmeticByInteger
    : stdx::type_identity<CannotAssessTruncationRiskFor<T>> {};

template <typename T, typename M>
struct TruncationRiskForDivideIntegralByInteger
    : std::conditional<(get_value_result<T>(M{}).outcome ==
                        MagRepresentationOutcome::ERR_CANNOT_FIT),
                       ValueIsNotZero<T>,
                       ValueTimesRatioIsNotInteger<T, MagInverseT<M>>> {};

template <typename T, typename M>
struct TruncationRiskForDivideArithmeticByInteger
    : std::conditional_t<std::is_floating_point<T>::value,
                         stdx::type_identity<NoTruncationRisk<T>>,
                         TruncationRiskForDivideIntegralByInteger<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForDivideByIntAssumingScalar
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForDivideArithmeticByInteger<T, M>,
                         TruncationRiskForDivideNonArithmeticByInteger<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForImpl<DivideTypeByInteger<T, M>>
    : TruncationRiskForDivideByIntAssumingScalar<RealPart<T>, M> {};

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
