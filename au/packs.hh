// Copyright 2022 Aurora Operations, Inc.
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

#include <cstdint>
#include <ratio>
#include <utility>

#include "au/fwd.hh"
#include "au/stdx/experimental/is_detected.hh"
#include "au/stdx/type_traits.hh"
#include "au/utility/type_traits.hh"

// Products of base powers are the foundation of au.  We use them for:
//
//   - The Dimension of a Unit.
//   - The Magnitude of a Unit.
//   - Making *compound* Units (products of powers, e.g., m^1 * s^(-2)).

namespace au {

// A base type B raised to an integer exponent N.
template <typename B, std::intmax_t N>
struct Pow;

// A base type B raised to a rational exponent (N/D).
template <typename B, std::intmax_t N, std::intmax_t D>
struct RatioPow;

// Type trait for the "base" of a type, interpreted as a base power.
//
// Any type can act as a base, with an implicit power of 1.  `Pow<B, N>` can represent integer
// powers of a base type `B`, and `RatioPow<B, N, D>` can represent rational powers of `B` (where
// the power is `(N/D)`).
template <typename T>
struct BaseImpl : stdx::type_identity<T> {};
template <typename T>
using Base = typename BaseImpl<T>::type;
template <typename T>
using BaseT = Base<T>;

// Type trait for the rational exponent of a type, interpreted as a base power.
template <typename T>
struct ExpImpl : stdx::type_identity<std::ratio<1>> {};
template <typename T>
using Exp = typename ExpImpl<T>::type;
template <typename T>
using ExpT = Exp<T>;

// Type trait for treating an arbitrary type as a given type of pack.
//
// This should be the identity for anything that is already a pack of this type, and otherwise
// should wrap it in this type of pack.
template <template <class... Ts> class Pack, typename T>
struct AsPackImpl : stdx::type_identity<Pack<T>> {};
template <template <class... Ts> class Pack, typename T>
using AsPack = typename AsPackImpl<Pack, T>::type;
template <template <class... Ts> class Pack, typename T>
using AsPackT = AsPack<Pack, T>;

// Type trait to remove a Pack enclosing a single item.
//
// Defined only if T is Pack<Ts...> for some typelist.  Always the identity, unless sizeof...(Ts) is
// exactly 1, in which case, it returns the (sole) element.
template <template <class... Ts> class Pack, typename T>
struct UnpackIfSoloImpl;
template <template <class... Ts> class Pack, typename T>
using UnpackIfSolo = typename UnpackIfSoloImpl<Pack, T>::type;
template <template <class... Ts> class Pack, typename T>
using UnpackIfSoloT = UnpackIfSolo<Pack, T>;

// Trait to define whether two types are in order, based on the total ordering for some pack.
//
// Each pack should individually define its desired total ordering.  For these implementations,
// prefer to inherit from LexicographicTotalOrdering (below), because it guards against the most
// common way to fail to achieve a strict total ordering (namely, by having two types which are not
// identical nevertheless compare equal).
template <template <class...> class Pack, typename A, typename B>
struct InOrderFor;

// A strict total ordering which combines strict partial orderings serially, using the first which
// distinguishes A and B.
template <typename A, typename B, template <class, class> class... Orderings>
struct LexicographicTotalOrdering;

// A (somewhat arbitrary) total ordering on _packs themselves_.
//
// Built on top of the total ordering for the _bases_ of the packs.
template <typename T, typename U>
struct InStandardPackOrder;

// Insert an element in a list, using the ordering for a specific (possibly different) pack.
//
// A precondition is that the list must already be sorted by the given ordering.
template <template <class...> class PackForOrdering, typename T, typename ListT>
struct InsertUsingOrderingForImpl;
template <template <class...> class PackForOrdering, typename T, typename ListT>
using InsertUsingOrderingFor = typename InsertUsingOrderingForImpl<PackForOrdering, T, ListT>::type;

// Sort a type list using the ordering for a specific (possibly different) pack.
template <template <class...> class PackForOrdering, typename ListT>
struct SortAsImpl;
template <template <class...> class PackForOrdering, typename ListT>
using SortAs = typename SortAsImpl<PackForOrdering, ListT>::type;

// Make a List of deduplicated, sorted types.
//
// The result will always be List<...>, and the elements will be sorted according to the total
// ordering for List, with duplicates removed.  It will be "flattened" in that any elements which
// are already `List<Ts...>` will be effectively replaced by `Ts...`.
//
// A precondition for `FlatDedupedTypeListT` is that any inputs which are already of type
// `List<...>`, respect the _ordering_ for `List`, with no duplicates.  Otherwise, behaviour is
// undefined.  (This precondition will automatically be satisfied if *every* instance of `List<...>`
// arises as the result of a call to `FlatDedupedTypeListT<...>`.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeListImpl;
template <template <class...> class List, typename... Ts>
using FlatDedupedTypeList = typename FlatDedupedTypeListImpl<List, AsPack<List, Ts>...>::type;
template <template <class...> class List, typename... Ts>
using FlatDedupedTypeListT = FlatDedupedTypeList<List, Ts...>;

namespace detail {
// Express a base power in its simplest form (base alone if power is 1, or Pow if exp is integral).
template <typename T>
struct SimplifyBasePowersImpl;
template <typename T>
using SimplifyBasePowers = typename SimplifyBasePowersImpl<T>::type;
}  // namespace detail

// Compute the product between two power packs.
template <template <class...> class Pack, typename... Ts>
struct PackProduct;
template <template <class...> class Pack, typename... Ts>
using PackProductT = detail::SimplifyBasePowers<typename PackProduct<Pack, Ts...>::type>;

// Compute a rational power of a pack.
template <template <class...> class Pack, typename T, typename E>
struct PackPower;
template <template <class...> class Pack,
          typename T,
          std::intmax_t ExpNum,
          std::intmax_t ExpDen = 1>
using PackPowerT =
    detail::SimplifyBasePowers<typename PackPower<Pack, T, std::ratio<ExpNum, ExpDen>>::type>;

// Compute the inverse of a power pack.
template <template <class...> class Pack, typename T>
using PackInverseT = PackPowerT<Pack, T, -1>;

// Compute the quotient of two power packs.
template <template <class...> class Pack, typename T, typename U>
using PackQuotientT = PackProductT<Pack, T, PackInverseT<Pack, U>>;

namespace detail {
// Pull out all of the elements in a Pack whose exponents are positive.
template <typename T>
struct NumeratorPartImpl;
template <typename T>
using NumeratorPart = typename NumeratorPartImpl<T>::type;

// Pull out all of the elements in a Pack whose exponents are negative.
template <typename T>
struct DenominatorPartImpl;
template <typename T>
using DenominatorPart = typename DenominatorPartImpl<T>::type;
}  // namespace detail

// A validator for a pack of Base Powers.
//
// `IsValidPack<Pack, T>::value` is `true` iff `T` is an instance of the variadic `Pack<...>`, and
// its parameters fulfill all of the appropriate type traits, namely:
//
// - `AreBasesInOrder<Pack, T>`
template <template <class...> class Pack, typename T>
struct IsValidPack;

// Assuming that `T` is an instance of `Pack<BPs...>`, validates that every consecutive pair from
// `Base<BPs>...` satisfies the strict total ordering `InOrderFor<Pack, ...>` for `Pack`.
template <template <class...> class Pack, typename T>
struct AreBasesInOrder;

// Assuming that `T` is an instance of `Pack<BPs...>`, validates that every consecutive pair from
// `BPs...` satisfies the strict total ordering `InOrderFor<Pack, ...>` for `Pack`.
//
// This is very similar to AreBasesInOrder, but is intended for packs that _don't_ represent
// products-of-powers.
template <template <class...> class Pack, typename T>
struct AreElementsInOrder;

// Assuming `T` is an instance of `Pack<BPs...>`, validates that `Exp<BPs>...` is always nonzero.
template <template <class...> class Pack, typename T>
struct AreAllPowersNonzero;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

// These forward declarations and traits go here to enable us to treat Pow and RatioPow (below) as
// full-fledged "Units".  A "Unit" is any type U where `DimT<U>` gives a valid Dimension, and
// `MagT<U>` gives a valid Magnitude.  Even though we can't define Dimension and Magnitude precisely
// in this file, we'll take advantage of the fact that we know they're going to be parameter packs.

template <typename... BPs>
struct Dimension;

template <typename... BPs>
struct Magnitude;

namespace detail {

// The default dimension, `DimT<U>`, of a type `U`, is the `::Dim` typedef (or `void` if none).
//
// Users can customize by specializing `DimImpl<U>` and setting the `type` member variable.
template <typename U>
using DimMemberT = typename U::Dim;
template <typename U>
struct DimImpl : stdx::experimental::detected_or<void, DimMemberT, U> {};
template <typename U>
using DimT = typename DimImpl<U>::type;

// The default magnitude, `MagT<U>`, of a type `U`, is the `::Mag` typedef (or `void` if none).
//
// Users can customize by specializing `MagImpl<U>` and setting the `type` member variable.
template <typename U>
using MagMemberT = typename U::Mag;
template <typename U>
struct MagImpl : stdx::experimental::detected_or<void, MagMemberT, U> {};
template <typename U>
using MagT = typename MagImpl<U>::type;

}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `Pow` implementation.

template <typename B, std::intmax_t N>
struct Pow {
    // TODO(#40): Clean up relationship between Dim/Mag and Pow, if compile times are OK.
    using Dim = PackPowerT<Dimension, AsPack<Dimension, detail::DimT<B>>, N>;
    using Mag = PackPowerT<Magnitude, AsPack<Magnitude, detail::MagT<B>>, N>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `RatioPow` implementation.

// A base type B raised to a rational exponent (N/D).
template <typename B, std::intmax_t N, std::intmax_t D>
struct RatioPow {
    // TODO(#40): Clean up relationship between Dim/Mag and RatioPow, if compile times are OK.
    using Dim = PackPowerT<Dimension, AsPack<Dimension, detail::DimT<B>>, N, D>;
    using Mag = PackPowerT<Magnitude, AsPack<Magnitude, detail::MagT<B>>, N, D>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `Base` implementation.

template <typename T, std::intmax_t N>
struct BaseImpl<Pow<T, N>> : stdx::type_identity<T> {};

template <typename T, std::intmax_t N, std::intmax_t D>
struct BaseImpl<RatioPow<T, N, D>> : stdx::type_identity<T> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `Exp` implementation.

template <typename T, std::intmax_t N>
struct ExpImpl<Pow<T, N>> : stdx::type_identity<std::ratio<N>> {};

template <typename T, std::intmax_t N, std::intmax_t D>
struct ExpImpl<RatioPow<T, N, D>> : stdx::type_identity<std::ratio<N, D>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AsPack` implementation.

template <template <class... Ts> class Pack, typename... Ts>
struct AsPackImpl<Pack, Pack<Ts...>> : stdx::type_identity<Pack<Ts...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnpackIfSolo` implementation.

// Null pack case: do not unpack.
template <template <class... Ts> class Pack>
struct UnpackIfSoloImpl<Pack, Pack<>> : stdx::type_identity<Pack<>> {};

// Non-null pack case: unpack only if there is nothing after the head element.
template <template <class... Ts> class Pack, typename T, typename... Ts>
struct UnpackIfSoloImpl<Pack, Pack<T, Ts...>>
    : std::conditional<(sizeof...(Ts) == 0u), T, Pack<T, Ts...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `LexicographicTotalOrdering` implementation.

// Base case: if there is no ordering, then the inputs are not in order.
template <typename A, typename B>
struct LexicographicTotalOrdering<A, B> : std::false_type {
    // LexicographicTotalOrdering is for strict total orderings only.  If two types compare equal,
    // then they must be the same type; otherwise, we have not created a strict total ordering among
    // all types being used in some pack.
    static_assert(std::is_same<A, B>::value,
                  "Broken strict total ordering: distinct input types compare equal");
};

// Recursive case.
template <typename A,
          typename B,
          template <class, class>
          class PrimaryOrdering,
          template <class, class>
          class... Tiebreakers>
struct LexicographicTotalOrdering<A, B, PrimaryOrdering, Tiebreakers...> :

    // Short circuit for when the inputs are the same.
    //
    // This can prevent us from instantiating a tiebreaker which doesn't exist for a given type.
    std::conditional_t<
        (std::is_same<A, B>::value),
        std::false_type,

        // If A and B are properly ordered by the primary criterion, they are definitely ordered.
        std::conditional_t<(PrimaryOrdering<A, B>::value),
                           std::true_type,

                           // If B and A are properly ordered by the primary criterion, then A and B
                           // are definitely _not_ properly ordered.
                           std::conditional_t<(PrimaryOrdering<B, A>::value),
                                              std::false_type,

                                              // Fall back to the remaining orderings as
                                              // tiebreakers.
                                              LexicographicTotalOrdering<A, B, Tiebreakers...>>>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `InStandardPackOrder` implementation.

namespace detail {
// Helper: check that the lead bases are in order.
template <typename T, typename U>
struct LeadBasesInOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct LeadBasesInOrder<P<H1, T1...>, P<H2, T2...>> : InOrderFor<P, Base<H1>, Base<H2>> {};

// Helper: check that the lead exponents are in order.
template <typename T, typename U>
struct LeadExpsInOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct LeadExpsInOrder<P<H1, T1...>, P<H2, T2...>>
    : stdx::bool_constant<(std::ratio_subtract<Exp<H1>, Exp<H2>>::num < 0)> {};

// Helper: apply InStandardPackOrder to tails.
template <typename T, typename U>
struct TailsInStandardPackOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct TailsInStandardPackOrder<P<H1, T1...>, P<H2, T2...>>
    : InStandardPackOrder<P<T1...>, P<T2...>> {};
}  // namespace detail

// Base case: left pack is null.
template <template <class...> class P, typename... Ts>
struct InStandardPackOrder<P<>, P<Ts...>> : stdx::bool_constant<(sizeof...(Ts) > 0)> {};

// Base case: right pack (only) is null.
template <template <class...> class P, typename H, typename... T>
struct InStandardPackOrder<P<H, T...>, P<>> : std::false_type {};

// Recursive case: try ordering the heads, and fall back to the tails.
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct InStandardPackOrder<P<H1, T1...>, P<H2, T2...>>
    : LexicographicTotalOrdering<P<H1, T1...>,
                                 P<H2, T2...>,
                                 detail::LeadBasesInOrder,
                                 detail::LeadExpsInOrder,
                                 detail::TailsInStandardPackOrder> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `InsertUsingOrderingFor` implementation.

// Base case.
template <template <class...> class PackForOrdering, typename T, template <class...> class Pack>
struct InsertUsingOrderingForImpl<PackForOrdering, T, Pack<>> : stdx::type_identity<Pack<T>> {};

// Recursive case: simply prepend if it's already in order, or else recurse past the first element,
// and then prepend the old first element.
template <template <class...> class PackForOrdering,
          typename T,
          template <class...>
          class Pack,
          typename U,
          typename... Us>
struct InsertUsingOrderingForImpl<PackForOrdering, T, Pack<U, Us...>>
    : std::conditional<
          InOrderFor<PackForOrdering, T, U>::value,
          Pack<T, U, Us...>,
          detail::Prepend<InsertUsingOrderingFor<PackForOrdering, T, Pack<Us...>>, U>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `SortAs` implementation.

// Base case.
template <template <class...> class PackForOrdering, template <class...> class Pack>
struct SortAsImpl<PackForOrdering, Pack<>> : stdx::type_identity<Pack<>> {};

// Recursive case.
template <template <class...> class PackForOrdering,
          template <class...>
          class Pack,
          typename T,
          typename... Ts>
struct SortAsImpl<PackForOrdering, Pack<T, Ts...>>
    : stdx::type_identity<
          InsertUsingOrderingFor<PackForOrdering, T, SortAs<PackForOrdering, Pack<Ts...>>>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `FlatDedupedTypeListT` implementation.

// 1-ary Base case: a list with a single element is already done.
//
// (We explicitly assumed that any `List<...>` inputs would already be in sorted order.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeListImpl<List, List<Ts...>> : stdx::type_identity<List<Ts...>> {};

// 2-ary base case: if we exhaust elements in the second list, the first list is the answer.
//
// (Again: this relies on the explicit assumption that any `List<...>` inputs are already in order.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeListImpl<List, List<Ts...>, List<>> : stdx::type_identity<List<Ts...>> {};

// 2-ary recursive case, single-element head.
//
// This use case also serves as the core "insertion logic", inserting `T` into the proper place
// within `List<H, Ts...>`.
template <template <class...> class List, typename T, typename H, typename... Ts>
struct FlatDedupedTypeListImpl<List, List<T>, List<H, Ts...>> :

    // If the candidate element exactly equals the head, disregard it (de-dupe!).
    std::conditional<
        (std::is_same<T, H>::value),
        List<H, Ts...>,

        // If the candidate element is strictly before the head, prepend it.
        std::conditional_t<(InOrderFor<List, T, H>::value),
                           List<T, H, Ts...>,

                           // If we're here, we know the candidate comes after the head.  So, try
                           // inserting it (recursively) in the tail, and then prepend the old Head
                           // (because we know it comes first).
                           detail::Prepend<FlatDedupedTypeListT<List, List<T>, List<Ts...>>, H>>> {
};

// 2-ary recursive case, multi-element head: insert head of second element, and recurse.
template <template <class...> class List,
          typename H1,
          typename N1,
          typename... T1,
          typename H2,
          typename... T2>
struct FlatDedupedTypeListImpl<List, List<H1, N1, T1...>, List<H2, T2...>>
    : FlatDedupedTypeListImpl<List,
                              // Put H2 first so we can use single-element-head case from above.
                              FlatDedupedTypeListT<List, List<H2>, List<H1, N1, T1...>>,
                              List<T2...>> {};

// N-ary case, multi-element head: peel off tail-of-head, and recurse.
//
// Note that this also handles the 2-ary case where the head list has more than one element.
template <template <class...> class List, typename L1, typename L2, typename L3, typename... Ls>
struct FlatDedupedTypeListImpl<List, L1, L2, L3, Ls...>
    : FlatDedupedTypeListImpl<List, FlatDedupedTypeListT<List, L1, L2>, L3, Ls...> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `PackProductT` implementation.

// 0-ary case:
template <template <class...> class Pack>
struct PackProduct<Pack> : stdx::type_identity<Pack<>> {};

// 1-ary case:
template <template <class...> class Pack, typename... Ts>
struct PackProduct<Pack, Pack<Ts...>> : stdx::type_identity<Pack<Ts...>> {};

// 2-ary Base case: two null packs.
template <template <class...> class Pack>
struct PackProduct<Pack, Pack<>, Pack<>> : stdx::type_identity<Pack<>> {};

// 2-ary Base case: only left pack is null.
template <template <class...> class Pack, typename T, typename... Ts>
struct PackProduct<Pack, Pack<>, Pack<T, Ts...>> : stdx::type_identity<Pack<T, Ts...>> {};

// 2-ary Base case: only right pack is null.
template <template <class...> class Pack, typename T, typename... Ts>
struct PackProduct<Pack, Pack<T, Ts...>, Pack<>> : stdx::type_identity<Pack<T, Ts...>> {};

namespace detail {
template <typename B, typename E1, typename E2>
struct ComputeRationalPowerImpl {
    using E = std::ratio_add<E1, E2>;
    using type = RatioPow<B, E::num, E::den>;
};
template <typename B, typename E1, typename E2>
using ComputeRationalPower = typename ComputeRationalPowerImpl<B, E1, E2>::type;
}  // namespace detail

// 2-ary Recursive case: two non-null packs.
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct PackProduct<P, P<H1, T1...>, P<H2, T2...>> :

    // If the bases for H1 and H2 are in-order, prepend H1 to the product of the remainder.
    std::conditional<
        (InOrderFor<P, Base<H1>, Base<H2>>::value),
        detail::Prepend<PackProductT<P, P<T1...>, P<H2, T2...>>, H1>,

        // If the bases for H2 and H1 are in-order, prepend H2 to the product of the remainder.
        std::conditional_t<
            (InOrderFor<P, Base<H2>, Base<H1>>::value),
            detail::Prepend<PackProductT<P, P<T2...>, P<H1, T1...>>, H2>,

            // If the bases have the same position, assume they really _are_ the same (because
            // InOrderFor will verify this if it uses LexicographicTotalOrdering), and add the
            // exponents.  (If the exponents add to zero, omit the term.)
            std::conditional_t<
                (std::ratio_add<Exp<H1>, Exp<H2>>::num == 0),
                PackProductT<P, P<T1...>, P<T2...>>,
                detail::Prepend<PackProductT<P, P<T2...>, P<T1...>>,
                                detail::ComputeRationalPower<Base<H1>, Exp<H1>, Exp<H2>>>>>> {};

// N-ary case, N > 2: recurse.
template <template <class...> class P,
          typename... T1s,
          typename... T2s,
          typename... T3s,
          typename... Ps>
struct PackProduct<P, P<T1s...>, P<T2s...>, P<T3s...>, Ps...>
    : PackProduct<P, P<T1s...>, PackProductT<P, P<T2s...>, P<T3s...>, Ps...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `PackPowerT` implementation.

namespace detail {
template <typename T, typename E>
using MultiplyExpFor = std::ratio_multiply<Exp<T>, E>;
}

template <template <class...> class P, typename... Ts, typename E>
struct PackPower<P, P<Ts...>, E>
    : std::conditional<(E::num == 0),
                       P<>,
                       P<RatioPow<Base<Ts>,
                                  detail::MultiplyExpFor<Ts, E>::num,
                                  detail::MultiplyExpFor<Ts, E>::den>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `IsValidPack` implementation.

namespace detail {
template <template <class...> class Pack, typename T>
struct IsPackOf : std::false_type {};

template <template <class...> class Pack, typename... Ts>
struct IsPackOf<Pack, Pack<Ts...>> : std::true_type {};
}  // namespace detail

template <template <class...> class Pack, typename T>
struct IsValidPack : stdx::conjunction<detail::IsPackOf<Pack, T>,
                                       AreBasesInOrder<Pack, T>,
                                       AreAllPowersNonzero<Pack, T>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreElementsInOrder` implementation.

template <template <class...> class Pack>
struct AreElementsInOrder<Pack, Pack<>> : std::true_type {};

template <template <class...> class Pack, typename T>
struct AreElementsInOrder<Pack, Pack<T>> : std::true_type {};

template <template <class...> class Pack, typename T1, typename T2, typename... Ts>
struct AreElementsInOrder<Pack, Pack<T1, T2, Ts...>>
    : stdx::conjunction<InOrderFor<Pack, T1, T2>, AreElementsInOrder<Pack, Pack<T2, Ts...>>> {};

namespace detail {

constexpr bool all_true() { return true; }

template <typename... Predicates>
constexpr bool all_true(Predicates &&...values) {
    // The reason we bother to make an array is so that we can iterate over it.
    const bool value_array[] = {values...};

    for (auto i = 0u; i < sizeof...(Predicates); ++i) {
        if (!value_array[i]) {
            return false;
        }
    }

    return true;
}
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreBasesInOrder` implementation.

template <template <class...> class Pack, typename... Ts>
struct AreBasesInOrder<Pack, Pack<Ts...>> : AreElementsInOrder<Pack, Pack<Base<Ts>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreAllPowersNonzero` implementation.

template <template <class...> class Pack, typename... Ts>
struct AreAllPowersNonzero<Pack, Pack<Ts...>>
    : stdx::bool_constant<detail::all_true((Exp<Ts>::num != 0)...)> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `SimplifyBasePowers` implementation.

namespace detail {
// To simplify an individual base power, by default, do nothing.
template <typename T>
struct SimplifyBasePowerImpl : stdx::type_identity<T> {};
template <typename T>
using SimplifyBasePower = typename SimplifyBasePowerImpl<T>::type;

// To simplify an integer power of a base, give the base alone if the exponent is 1; otherwise, do
// nothing.
template <typename B, std::intmax_t N>
struct SimplifyBasePowerImpl<Pow<B, N>> : std::conditional<(N == 1), B, Pow<B, N>> {};

// To simplify a rational power of a base, simplify the integer power if the exponent is an integer
// (i.e., if its denominator is 1); else, do nothing.
template <typename B, std::intmax_t N, std::intmax_t D>
struct SimplifyBasePowerImpl<RatioPow<B, N, D>>
    : std::conditional<(D == 1), SimplifyBasePower<Pow<B, N>>, RatioPow<B, N, D>> {};

// To simplify the base powers in a pack, give the pack with each base power simplified.
template <template <class...> class Pack, typename... BPs>
struct SimplifyBasePowersImpl<Pack<BPs...>> : stdx::type_identity<Pack<SimplifyBasePower<BPs>...>> {
};
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `NumeratorPart` and `DenominatorPart` implementation.

namespace detail {
template <typename BP>
struct IsInNumerator : stdx::bool_constant<(Exp<BP>::num > 0)> {};

template <typename BP>
struct IsInDenominator : stdx::bool_constant<(Exp<BP>::num < 0)> {};

// A generic helper for both numerator and denominator.
template <template <class> class Pred, typename T>
struct PullOutMatchingPowers;

// Base case: empty pack.
template <template <class> class Pred, template <class...> class Pack>
struct PullOutMatchingPowers<Pred, Pack<>> : stdx::type_identity<Pack<>> {};

// Recursive case: non-empty pack.
template <template <class> class Pred, template <class...> class Pack, typename H, typename... Ts>
struct PullOutMatchingPowers<Pred, Pack<H, Ts...>>
    : std::conditional<(Pred<H>::value),
                       detail::Prepend<typename PullOutMatchingPowers<Pred, Pack<Ts...>>::type, H>,
                       typename PullOutMatchingPowers<Pred, Pack<Ts...>>::type> {};

template <typename T>
struct NumeratorPartImpl : PullOutMatchingPowers<IsInNumerator, T> {};

template <template <class...> class Pack, typename... Ts>
struct DenominatorPartImpl<Pack<Ts...>>
    : stdx::type_identity<
          PackInverseT<Pack, typename PullOutMatchingPowers<IsInDenominator, Pack<Ts...>>::type>> {
};

}  // namespace detail

}  // namespace au
