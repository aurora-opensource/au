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
// `ConversionForRepsAndFactor<CastType, OldRep, NewRep, Factor>` is the operation that takes a
// value of `OldRep`, and produces the product of that value with magnitude `Factor`.
//
// If `NewRep` is `void`, the operation omits the final cast and returns the "natural" result type
// of applying the conversion factor (after any required promotion).
//
// Otherwise, the result is cast to `NewRep`, using casting operations according to `CastType`
// (`static_cast` or implicit conversions).
//
template <typename CastType, typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl;
template <typename CastType, typename OldRep, typename NewRep, typename Factor>
using ConversionForRepsAndFactor =
    typename ConversionForRepsAndFactorImpl<CastType, OldRep, NewRep, Factor>::type;

// Provide `UseStaticCast` as the first parameter to `ConversionForRepsAndFactor` to use
// `static_cast` to convert between representations.
struct UseStaticCast {};

// Provide `UseImplicitConversion` as the first parameter to `ConversionForRepsAndFactor` to use
// implicit conversions to convert between representations.
struct UseImplicitConversion {};

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
                            stdx::negation<std::is_same<Denominator<M>, Magnitude<>>>>::value,
          std::conditional_t<std::is_same<Abs<Numerator<M>>, Magnitude<>>::value,
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
    : stdx::type_identity<DivideTypeByInteger<T, MagProduct<Sign<Mag>, Denominator<Mag>>>> {};

template <typename T, typename Mag>
struct ApplicationStrategyForImpl<T, Mag, MagKindHolder<MagKind::NONTRIVIAL_RATIONAL>>
    : std::conditional<std::is_integral<RealPart<T>>::value,
                       OpSequence<MultiplyTypeBy<T, Numerator<Mag>>,
                                  DivideTypeByInteger<OpOutput<MultiplyTypeBy<T, Numerator<Mag>>>,
                                                      Denominator<Mag>>>,
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
// `HasConversionRep<OldRep, NewRep>` tells us (SFINAE-friendly) whether `ConversionRep<OldRep,
// NewRep>` is well-formed.
//
// The conversion arithmetic is hosted in `std::common_type` of the two reps, but some reps have no
// common type at all --- most notably, two *distinct* Eigen expression templates.  Asking
// `std::common_type` for such a pair is a hard error rather than a soft one, which would otherwise
// blow up any conversion *policy* check (e.g. the implicit-constructor's SFINAE guard) that merely
// needs to answer "is this conversion permitted?" with `false`.  This trait lets those callers
// short-circuit to `false` instead.
//
template <typename A, typename B>
using CommonTypeMemberT = typename std::common_type<A, B>::type;
template <typename A, typename B>
struct HasCommonType : stdx::experimental::is_detected<CommonTypeMemberT, A, B> {};

template <typename OldRep, typename NewRep>
struct HasConversionRep
    : std::conditional_t<IsRealToComplex<OldRep, NewRep>::value,
                         HasCommonType<RealPart<OldRep>, RealPart<NewRep>>,
                         HasCommonType<OldRep, NewRep>> {};

//
// `CastStep<CastType, T, U>` is a single step of casting from type `T` to type `U`, using the
// appropriate operation based on `CastType`.
//
template <typename CastType, typename T, typename U>
struct CastStepImpl;
template <typename CastType, typename T, typename U>
using CastStep = typename CastStepImpl<CastType, T, U>::type;

template <typename T, typename U>
struct CastStepImpl<UseStaticCast, T, U> : stdx::type_identity<StaticCast<T, U>> {};

template <typename T, typename U>
struct CastStepImpl<UseImplicitConversion, T, U> : stdx::type_identity<ImplicitConversion<T, U>> {};

//
// `CastSequence<CastType, T, U>` is the sequence of operations that gets us from `T` to `U`, using
// `CastStep<CastType, T, U>` for each step.
//
// Normally, of course, this is just a single step of `CastStep<CastType, T, U>`.  But we have weird
// edge cases like going from `double` to `std::complex<int>`, which require an intermediate step of
// casting to `int`.
//

template <typename CastType, typename T, typename U>
struct CastSequenceImpl
    : std::conditional<
          stdx::conjunction<IsRealToComplex<T, U>,
                            stdx::negation<std::is_same<T, RealPart<U>>>>::value,
          OpSequence<CastStep<CastType, T, RealPart<U>>, CastStep<CastType, RealPart<U>, U>>,
          CastStep<CastType, T, U>> {};
template <typename CastType, typename T, typename U>
using CastSequence = typename CastSequenceImpl<CastType, T, U>::type;

//
// `FullConversionImpl<CastType, OldRep, ConversionRepT, NewRep, Factor>` should resolve to the most
// efficient sequence of operations for a conversion from `OldRep` to `NewRep`, with a magnitude
// `Factor`, where `ConversionRepT` is the promoted type of the common type of `OldRep` and
// `NewRep`.  `CastType` discriminates between `static_cast` and implicit conversions.
//

// Helper to get the output type after applying the conversion factor.
template <typename Rep, typename Factor>
using ApplicationOutputFor = OpOutput<ApplicationStrategyFor<Rep, Factor>>;

template <typename CastType,
          typename OldRep,
          typename ConversionRepT,
          typename NewRep,
          typename Factor>
struct FullConversionImpl
    : stdx::type_identity<OpSequence<
          CastSequence<CastType, OldRep, ConversionRepT>,
          ApplicationStrategyFor<ConversionRepT, Factor>,
          CastSequence<CastType, ApplicationOutputFor<ConversionRepT, Factor>, NewRep>>> {};

template <typename CastType, typename OldRepIsConversionRep, typename NewRep, typename Factor>
struct FullConversionImpl<CastType, OldRepIsConversionRep, OldRepIsConversionRep, NewRep, Factor>
    : stdx::type_identity<OpSequence<
          ApplicationStrategyFor<OldRepIsConversionRep, Factor>,
          CastSequence<CastType, ApplicationOutputFor<OldRepIsConversionRep, Factor>, NewRep>>> {};

template <typename CastType, typename OldRep, typename NewRepIsConversionRep, typename Factor>
struct FullConversionImpl<CastType, OldRep, NewRepIsConversionRep, NewRepIsConversionRep, Factor>
    : stdx::type_identity<OpSequence<CastSequence<CastType, OldRep, NewRepIsConversionRep>,
                                     ApplicationStrategyFor<NewRepIsConversionRep, Factor>>> {};

// When OldRep == ConversionRep == NewRep and the application output matches Rep, no cast needed.
template <typename CastType, typename Rep, typename Factor>
struct FullConversionImpl<CastType, Rep, Rep, Rep, Factor>
    : std::conditional<std::is_same<ApplicationOutputFor<Rep, Factor>, Rep>::value,
                       ApplicationStrategyFor<Rep, Factor>,
                       OpSequence<ApplicationStrategyFor<Rep, Factor>,
                                  CastSequence<CastType, ApplicationOutputFor<Rep, Factor>, Rep>>> {
};

// To implement `ConversionForRepsAndFactor`, delegate to `FullConversionImpl`.
template <typename CastType, typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl
    : FullConversionImpl<CastType, OldRep, ConversionRep<OldRep, NewRep>, NewRep, Factor> {};

// Identity factor: just cast, no arithmetic.
template <typename CastType, typename OldRep, typename NewRep>
struct ConversionForRepsAndFactorImpl<CastType, OldRep, NewRep, Magnitude<>>
    : FullConversionImpl<CastType, OldRep, ConversionRep<OldRep, NewRep>, NewRep, Magnitude<>> {};

// Specialization for `void`: apply the conversion factor with proper promotion,
// but don't add a final cast to force a specific output type.
template <typename CastType, typename OldRep, typename Factor>
struct ConversionForRepsAndFactorImpl<CastType, OldRep, void, Factor>
    : FullConversionImpl<CastType,
                         OldRep,
                         PromotedType<OldRep>,
                         ApplicationOutputFor<PromotedType<OldRep>, Factor>,
                         Factor> {};

// Identity factor, implicit rep: no conversion at all.
template <typename CastType, typename OldRep>
struct ConversionForRepsAndFactorImpl<CastType, OldRep, void, Magnitude<>>
    : FullConversionImpl<CastType, OldRep, OldRep, OldRep, Magnitude<>> {};

}  // namespace detail
}  // namespace au
