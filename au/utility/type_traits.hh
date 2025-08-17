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

#include <type_traits>

#include "au/stdx/type_traits.hh"

namespace au {
namespace detail {

template <typename PackT, typename T>
struct Prepend;
template <typename PackT, typename T>
using PrependT = typename Prepend<PackT, T>::type;

template <template <class> class Condition, template <class...> class Pack, typename... Ts>
struct IncludeInPackIfImpl;
template <template <class> class Condition, template <class...> class Pack, typename... Ts>
using IncludeInPackIf = typename IncludeInPackIfImpl<Condition, Pack, Ts...>::type;

template <typename T, typename Pack>
struct DropAllImpl;
template <typename T, typename Pack>
using DropAll = typename DropAllImpl<T, Pack>::type;

template <template <class...> class Pack, typename... Ts>
struct FlattenAsImpl;
template <template <class...> class Pack, typename... Ts>
using FlattenAs = typename FlattenAsImpl<Pack, Ts...>::type;

template <typename T, typename U>
struct SameTypeIgnoringCvref : std::is_same<stdx::remove_cvref_t<T>, stdx::remove_cvref_t<U>> {};

template <typename T, typename U>
constexpr bool same_type_ignoring_cvref(T, U) {
    return SameTypeIgnoringCvref<T, U>::value;
}

template <typename... Ts>
struct AlwaysFalse : std::false_type {};

template <typename R1, typename R2>
struct CommonTypeButPreserveIntSignednessImpl;
template <typename R1, typename R2>
using CommonTypeButPreserveIntSignedness =
    typename CommonTypeButPreserveIntSignednessImpl<R1, R2>::type;

//
// `PromotedType<T>` is the result type for arithmetic operations involving `T`.  Of course, this is
// normally just `T`, but integer promotion for small integral types can change this.
//
template <typename T>
struct PromotedTypeImpl;
template <typename T>
using PromotedType = typename PromotedTypeImpl<T>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `Prepend` implementation.

template <template <typename...> class Pack, typename T, typename... Us>
struct Prepend<Pack<Us...>, T> {
    using type = Pack<T, Us...>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `IncludeInPackIf` implementation.

// Helper: change the pack.  This lets us do our work in one kind of pack, and then swap it out for
// another pack at the end.
template <template <class...> class NewPack, typename PackT>
struct ChangePackToImpl;
template <template <class...> class NewPack, typename PackT>
using ChangePackTo = typename ChangePackToImpl<NewPack, PackT>::type;
template <template <class...> class NewPack, template <class...> class OldPack, typename... Ts>
struct ChangePackToImpl<NewPack, OldPack<Ts...>> : stdx::type_identity<NewPack<Ts...>> {};

// A generic typelist with no constraints on members or ordering.  Intended as a type to hold
// intermediate work.
template <typename... Ts>
struct GenericTypeList;

template <template <class> class Condition, typename PackT>
struct ListMatchingTypesImpl;
template <template <class> class Condition, typename PackT>
using ListMatchingTypes = typename ListMatchingTypesImpl<Condition, PackT>::type;

// Base case:
template <template <class> class Condition>
struct ListMatchingTypesImpl<Condition, GenericTypeList<>>
    : stdx::type_identity<GenericTypeList<>> {};

// Recursive case:
template <template <class> class Condition, typename H, typename... Ts>
struct ListMatchingTypesImpl<Condition, GenericTypeList<H, Ts...>>
    : std::conditional<Condition<H>::value,
                       PrependT<ListMatchingTypes<Condition, GenericTypeList<Ts...>>, H>,
                       ListMatchingTypes<Condition, GenericTypeList<Ts...>>> {};

template <template <class> class Condition, template <class...> class Pack, typename... Ts>
struct IncludeInPackIfImpl
    : stdx::type_identity<
          ChangePackTo<Pack, ListMatchingTypes<Condition, GenericTypeList<Ts...>>>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DropAll` implementation.

// Base case.
template <typename T, template <class...> class Pack>
struct DropAllImpl<T, Pack<>> : stdx::type_identity<Pack<>> {};

// Recursive case:
template <typename T, template <class...> class Pack, typename H, typename... Ts>
struct DropAllImpl<T, Pack<H, Ts...>>
    : std::conditional<std::is_same<T, H>::value,
                       DropAll<T, Pack<Ts...>>,
                       detail::PrependT<DropAll<T, Pack<Ts...>>, H>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `FlattenAs` implementation.

template <typename P1, typename P2>
struct ConcatImpl;
template <typename P1, typename P2>
using Concat = typename ConcatImpl<P1, P2>::type;

template <template <class...> class Pack, typename... T1s, typename... T2s>
struct ConcatImpl<Pack<T1s...>, Pack<T2s...>> : stdx::type_identity<Pack<T1s..., T2s...>> {};

template <template <class...> class Pack, typename ResultPack, typename... Ts>
struct FlattenAsImplHelper;

template <template <class...> class Pack, typename ResultPack>
struct FlattenAsImplHelper<Pack, ResultPack> : stdx::type_identity<ResultPack> {};

// Skip empty packs.
template <template <class...> class Pack, typename ResultPack, typename... Us>
struct FlattenAsImplHelper<Pack, ResultPack, Pack<>, Us...>
    : FlattenAsImplHelper<Pack, ResultPack, Us...> {};

template <template <class...> class Pack,
          typename ResultPack,
          typename T,
          typename... Ts,
          typename... Us>
struct FlattenAsImplHelper<Pack, ResultPack, Pack<T, Ts...>, Us...>
    : FlattenAsImplHelper<Pack, ResultPack, T, Pack<Ts...>, Us...> {};

template <template <class...> class Pack, typename ResultPack, typename T, typename... Us>
struct FlattenAsImplHelper<Pack, ResultPack, T, Us...>
    : FlattenAsImplHelper<Pack, Concat<ResultPack, Pack<T>>, Us...> {};

template <template <class...> class Pack, typename... Ts>
struct FlattenAsImpl : FlattenAsImplHelper<Pack, Pack<>, Ts...> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonTypeButPreserveIntSignedness` implementation.

// `CopySignednessIfIntType<X, T>` has a `type` member that is always `T`, unless `T` is an integral
// type: in which case, it's the signed version of `T` if `X` is signed, and the unsigned version of
// `T` if `X` is unsigned.
template <typename SignednessSource, typename T, bool IsTIntegral>
struct CopySignednessIfIntTypeHelper;
template <typename SignednessSource, typename T>
struct CopySignednessIfIntTypeHelper<SignednessSource, T, true>
    : std::conditional<std::is_unsigned<SignednessSource>::value,
                       std::make_unsigned_t<T>,
                       std::make_signed_t<T>> {};
template <typename SignednessSource, typename T>
struct CopySignednessIfIntTypeHelper<SignednessSource, T, false> : stdx::type_identity<T> {};

template <typename SignednessSource, typename T>
struct CopySignednessIfIntType
    : CopySignednessIfIntTypeHelper<SignednessSource, T, std::is_integral<T>::value> {};

template <typename R1, typename R2>
struct CommonTypeButPreserveIntSignednessImpl
    : CopySignednessIfIntType<R1, std::common_type_t<R1, R2>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `PromotedType<T>` implementation.

template <typename T>
struct PromotedTypeImpl {
    using type = decltype(std::declval<T>() * std::declval<T>());

    static_assert(std::is_same<type, typename PromotedTypeImpl<type>::type>::value,
                  "We explicitly assume that promoted types are not again promotable");
};

}  // namespace detail
}  // namespace au
