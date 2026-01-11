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

template <MagKind>
struct MagKindHolder {};

template <typename M>
struct MagKindForImpl
    : std::conditional<
          stdx::conjunction<IsRational<M>,
                            stdx::negation<std::is_same<DenominatorT<M>, Magnitude<>>>>::value,
          std::conditional_t<std::is_same<Abs<NumeratorT<M>>, Magnitude<>>::value,
                             MagKindHolder<MagKind::INTEGER_DIVIDE>,
                             MagKindHolder<MagKind::NONTRIVIAL_RATIONAL>>,
          MagKindHolder<MagKind::DEFAULT>> {};
template <typename M>
using MagKindFor = typename MagKindForImpl<M>::type;

template <typename T, typename Mag, typename MagKindValue>
struct ApplicationStrategyForImpl : stdx::type_identity<MultiplyTypeBy<T, Mag>> {};
template <typename T, typename Mag>
using ApplicationStrategyFor = typename ApplicationStrategyForImpl<T, Mag, MagKindFor<Mag>>::type;

template <typename T, typename Mag>
struct ApplicationStrategyForImpl<T, Mag, MagKindHolder<MagKind::INTEGER_DIVIDE>>
    : stdx::type_identity<DivideTypeByInteger<T, MagProduct<Sign<Mag>, DenominatorT<Mag>>>> {};

template <typename T, typename Mag>
struct ApplicationStrategyForImpl<T, Mag, MagKindHolder<MagKind::NONTRIVIAL_RATIONAL>>
    : std::conditional<
          std::is_integral<RealPart<T>>::value,
          OpSequence<MultiplyTypeBy<T, NumeratorT<Mag>>, DivideTypeByInteger<T, DenominatorT<Mag>>>,
          MultiplyTypeBy<T, Mag>> {};

//
// `ConversionRep<OldRep, NewRep>` is the rep we should use when applying the conversion factor.
//
template <typename OldRep, typename NewRep>
struct ConversionRepImpl;
template <typename OldRep, typename NewRep>
using ConversionRep = typename ConversionRepImpl<OldRep, NewRep>::type;

template <typename OldRep, typename NewRep>
struct IsRealToComplex
    : stdx::conjunction<std::is_same<OldRep, RealPart<OldRep>>,
                        stdx::experimental::is_detected<TypeOfRealMember, NewRep>> {};

template <typename OldRep, typename NewRep>
struct ConversionRepImpl
    : std::conditional<IsRealToComplex<OldRep, NewRep>::value,
                       PromotedType<std::common_type_t<RealPart<OldRep>, RealPart<NewRep>>>,
                       PromotedType<std::common_type_t<OldRep, NewRep>>> {};

//
// `StaticCastSequence<T, U>` is the sequence of operations that gets us from `T` to `U`.
//
// Normally, of course, this is just `StaticCast<T, U>`.  But we have weird edge cases like going
// from `double` to `std::complex<int>`, which require an intermediate step of static casting to
// `int`.
//

template <typename T, typename U>
struct StaticCastSequenceImpl
    : std::conditional<stdx::conjunction<IsRealToComplex<T, U>,
                                         stdx::negation<std::is_same<T, RealPart<U>>>>::value,
                       OpSequence<StaticCast<T, RealPart<U>>, StaticCast<RealPart<U>, U>>,
                       StaticCast<T, U>> {};
template <typename T, typename U>
using StaticCastSequence = typename StaticCastSequenceImpl<T, U>::type;

//
// `FullConversionImpl<OldRep, ConversionRepT, NewRep, Factor>` should resolve to the most efficient
// sequence of operations for a conversion from `OldRep` to `NewRep`, with a magnitude `Factor`,
// where `ConversionRepT` is the promoted type of the common type of `OldRep` and `NewRep`.
//

template <typename OldRep, typename ConversionRepT, typename NewRep, typename Factor>
struct FullConversionImpl
    : stdx::type_identity<OpSequence<StaticCastSequence<OldRep, ConversionRepT>,
                                     ApplicationStrategyFor<ConversionRepT, Factor>,
                                     StaticCastSequence<ConversionRepT, NewRep>>> {};

template <typename OldRepIsConversionRep, typename NewRep, typename Factor>
struct FullConversionImpl<OldRepIsConversionRep, OldRepIsConversionRep, NewRep, Factor>
    : stdx::type_identity<OpSequence<ApplicationStrategyFor<OldRepIsConversionRep, Factor>,
                                     StaticCastSequence<OldRepIsConversionRep, NewRep>>> {};

template <typename OldRep, typename NewRepIsConversionRep, typename Factor>
struct FullConversionImpl<OldRep, NewRepIsConversionRep, NewRepIsConversionRep, Factor>
    : stdx::type_identity<OpSequence<StaticCastSequence<OldRep, NewRepIsConversionRep>,
                                     ApplicationStrategyFor<NewRepIsConversionRep, Factor>>> {};

template <typename Rep, typename Factor>
struct FullConversionImpl<Rep, Rep, Rep, Factor>
    : stdx::type_identity<ApplicationStrategyFor<Rep, Factor>> {};

// To implement `ConversionForRepsAndFactor`, delegate to `FullConversionImpl`.
template <typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl
    : FullConversionImpl<OldRep, ConversionRep<OldRep, NewRep>, NewRep, Factor> {};

}  // namespace detail
}  // namespace au
