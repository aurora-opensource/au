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
#include "au/magnitude.hh"
#include "au/stdx/type_traits.hh"

namespace au {
namespace detail {

//
// `ConversionForRepsAndFactor<OldRep, NewRep, Factor>` is the operation that takes a value of
// `OldRep`, and produces the product of that value with magnitude `Factor` in the type `NewRep`.
//
template <typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl;
template <typename OldRep, typename NewRep, typename Factor>
using ConversionForRepsAndFactor =
    typename ConversionForRepsAndFactorImpl<OldRep, NewRep, Factor>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details (`conversion_strategy.hh`):
////////////////////////////////////////////////////////////////////////////////////////////////////

//
// `ApplicationStrategyFor<T, Mag>` tells us how we should apply a magnitude `Mag` to a type `T`.
//

enum class MagKind {
    DEFAULT,
    INTEGER_DIVIDE,
    NONTRIVIAL_RATIONAL,
};

template <typename M>
constexpr MagKind mag_kind_for(M) {
    if (stdx::conjunction<IsRational<M>,
                          stdx::negation<std::is_same<DenominatorT<M>, Magnitude<>>>>::value) {
        return std::is_same<Abs<NumeratorT<M>>, Magnitude<>>::value ? MagKind::INTEGER_DIVIDE
                                                                    : MagKind::NONTRIVIAL_RATIONAL;
    }
    return MagKind::DEFAULT;
}

template <typename T, typename Mag, MagKind>
struct ApplicationStrategyForImpl : stdx::type_identity<MultiplyTypeBy<T, Mag>> {};
template <typename T, typename Mag>
using ApplicationStrategyFor =
    typename ApplicationStrategyForImpl<T, Mag, mag_kind_for(Mag{})>::type;

template <typename T, typename Mag>
struct ApplicationStrategyForImpl<T, Mag, MagKind::INTEGER_DIVIDE>
    : stdx::type_identity<DivideTypeByInteger<T, MagProductT<Sign<Mag>, DenominatorT<Mag>>>> {};

template <typename T, typename Mag>
struct ApplicationStrategyForImpl<T, Mag, MagKind::NONTRIVIAL_RATIONAL>
    : std::conditional<
          std::is_integral<T>::value,
          OpSequence<MultiplyTypeBy<T, NumeratorT<Mag>>, DivideTypeByInteger<T, DenominatorT<Mag>>>,
          MultiplyTypeBy<T, Mag>> {};

//
// `FullConversionImpl<OldRep, PromotedCommon, NewRep, Factor>` should resolve to the most efficient
// sequence of operations for a conversion from `OldRep` to `NewRep`, with a magnitude `Factor`,
// where `PromotedCommon` is the promoted type of the common type of `OldRep` and `NewRep`.
//

template <typename OldRep, typename PromotedCommon, typename NewRep, typename Factor>
struct FullConversionImpl
    : stdx::type_identity<OpSequence<StaticCast<OldRep, PromotedCommon>,
                                     ApplicationStrategyFor<PromotedCommon, Factor>,
                                     StaticCast<PromotedCommon, NewRep>>> {};

template <typename OldRepIsPromotedCommon, typename NewRep, typename Factor>
struct FullConversionImpl<OldRepIsPromotedCommon, OldRepIsPromotedCommon, NewRep, Factor>
    : stdx::type_identity<OpSequence<ApplicationStrategyFor<OldRepIsPromotedCommon, Factor>,
                                     StaticCast<OldRepIsPromotedCommon, NewRep>>> {};

template <typename OldRep, typename NewRepIsPromotedCommon, typename Factor>
struct FullConversionImpl<OldRep, NewRepIsPromotedCommon, NewRepIsPromotedCommon, Factor>
    : stdx::type_identity<OpSequence<StaticCast<OldRep, NewRepIsPromotedCommon>,
                                     ApplicationStrategyFor<NewRepIsPromotedCommon, Factor>>> {};

template <typename Rep, typename Factor>
struct FullConversionImpl<Rep, Rep, Rep, Factor>
    : stdx::type_identity<ApplicationStrategyFor<Rep, Factor>> {};

// To implement `ConversionForRepsAndFactor`, delegate to `FullConversionImpl`.
template <typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl
    : FullConversionImpl<OldRep, PromotedType<std::common_type_t<OldRep, NewRep>>, NewRep, Factor> {
};

}  // namespace detail
}  // namespace au
