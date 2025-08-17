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

template <int N>
struct TruncationRiskClass {
    static constexpr int truncation_risk_class() { return N; }
};

template <typename T>
struct NoTruncationRisk : TruncationRiskClass<0> {
    static constexpr bool would_value_truncate(const T &) { return false; }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImpl;
template <typename T, typename M>
struct ValueTimesRatioIsNotInteger : ValueTimesRatioIsNotIntegerImpl<T, M>,
                                     TruncationRiskClass<10> {};

template <typename T>
using ValueIsNotInteger = ValueTimesRatioIsNotInteger<T, Magnitude<>>;

template <typename T>
struct ValueIsNotZero : TruncationRiskClass<20> {
    static constexpr bool would_value_truncate(const T &x) { return x != T{0}; }
};

template <typename T>
struct CannotAssessTruncationRiskFor : TruncationRiskClass<1000> {
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
// `OpSequence<...>` section:

// A little helper to simplify instances of `ValueTimesRatioIsNotInteger` that turn out to be
// trivial (because their type is integral, so they can never produce truncating values).
template <typename T, typename M>
struct ReduceValueTimesRatioIsNotIntegerImpl
    : std::conditional<stdx::conjunction<IsInteger<M>, std::is_integral<T>>::value,
                       NoTruncationRisk<T>,
                       ValueTimesRatioIsNotInteger<T, M>> {};
template <typename T, typename M>
using ReduceValueTimesRatioIsNotInteger =
    typename ReduceValueTimesRatioIsNotIntegerImpl<T, M>::type;

//
// `UpdateRisk<Op, Risk>` adapts a "downstream" risk to the "upstream" interface.
//
// At minimum, this updates the input type to `OpInput<Op>`.  But it may also tweak the parameters
// (e.g., for `ValuesNotSomeIntegerTimes`), or even change the risk type entirely.
//
template <typename Op, typename Risk>
struct UpdateRiskImpl;
template <typename Op, typename Risk>
using UpdateRisk = typename UpdateRiskImpl<Op, Risk>::type;

template <template <class> class Risk, typename T, typename U>
struct UpdateRiskImpl<StaticCast<T, U>, Risk<RealPart<U>>>
    : stdx::type_identity<Risk<RealPart<T>>> {};

template <typename T, typename U, typename M>
struct UpdateRiskImpl<StaticCast<T, U>, ValueTimesRatioIsNotInteger<RealPart<U>, M>>
    : std::conditional<stdx::conjunction<IsInteger<M>, std::is_integral<T>>::value,
                       NoTruncationRisk<RealPart<T>>,
                       ReduceValueTimesRatioIsNotInteger<RealPart<T>, M>> {};

template <template <class> class Risk, typename T, typename M>
struct UpdateRiskImpl<MultiplyTypeBy<T, M>, Risk<RealPart<T>>>
    : stdx::type_identity<Risk<RealPart<T>>> {};

template <template <class> class Risk, typename T, typename M>
struct UpdateRiskImpl<DivideTypeByInteger<T, M>, Risk<RealPart<T>>>
    : stdx::type_identity<Risk<RealPart<T>>> {};

template <typename T, typename M1, typename M2>
struct UpdateRiskImpl<MultiplyTypeBy<T, M1>, ValueTimesRatioIsNotInteger<RealPart<T>, M2>>
    : std::conditional<IsRational<M1>::value,
                       ReduceValueTimesRatioIsNotInteger<RealPart<T>, MagProductT<M1, M2>>,
                       ValueIsNotZero<RealPart<T>>> {};

template <typename T, typename M1, typename M2>
struct UpdateRiskImpl<DivideTypeByInteger<T, M1>, ValueTimesRatioIsNotInteger<RealPart<T>, M2>>
    : stdx::type_identity<ReduceValueTimesRatioIsNotInteger<RealPart<T>, MagQuotientT<M2, M1>>> {};

//
// `BiggestRiskImpl<Risk1, Risk2>` is a helper that computes the "biggest" risk between two risks.
//

template <typename Risk1, typename Risk2>
struct TruncationRisks {};

template <typename Risk1, typename Risk2>
struct OrderByTruncationRiskClass
    : stdx::bool_constant<(Risk1::truncation_risk_class() < Risk2::truncation_risk_class())> {};

template <typename Risk>
struct DenominatorOfRatioImpl : stdx::type_identity<Magnitude<>> {};
template <typename T, typename M>
struct DenominatorOfRatioImpl<ValueTimesRatioIsNotInteger<T, M>>
    : stdx::type_identity<DenominatorT<M>> {};
template <typename Risk>
using DenominatorOfRatio = typename DenominatorOfRatioImpl<Risk>::type;

template <typename Risk1, typename Risk2>
struct OrderByDenominatorOfRatio
    : stdx::bool_constant<(get_value<uint64_t>(DenominatorOfRatio<Risk1>{}) <
                           get_value<uint64_t>(DenominatorOfRatio<Risk2>{}))> {};

}  // namespace detail

// Must be in `::au` namespace:
template <typename Risk1, typename Risk2>
struct InOrderFor<detail::TruncationRisks, Risk1, Risk2>
    : LexicographicTotalOrdering<Risk1,
                                 Risk2,
                                 detail::OrderByTruncationRiskClass,
                                 detail::OrderByDenominatorOfRatio> {};

namespace detail {

template <typename Risk1, typename Risk2>
struct BiggestRiskImpl
    : std::conditional<InOrderFor<TruncationRisks, Risk1, Risk2>::value, Risk2, Risk1> {};

//
// Full `TruncationRiskFor` implementation for `OpSequence<Op>`:
//

template <typename Op>
struct TruncationRiskForImpl<OpSequenceImpl<Op>> : TruncationRiskForImpl<Op> {};

template <typename Op, typename... Ops>
struct TruncationRiskForImpl<OpSequenceImpl<Op, Ops...>>
    : BiggestRiskImpl<UpdateRisk<Op, TruncationRiskFor<OpSequenceImpl<Ops...>>>,
                      TruncationRiskFor<Op>> {};

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
