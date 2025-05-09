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
namespace auimpl {

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

template <typename T, typename U>
struct SameTypeIgnoringCvref : std::is_same<stdx::remove_cvref_t<T>, stdx::remove_cvref_t<U>> {};

template <typename T, typename U>
constexpr bool same_type_ignoring_cvref(T, U) {
    return SameTypeIgnoringCvref<T, U>::value;
}

template <typename... Ts>
struct AlwaysFalse : std::false_type {};

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
                       auimpl::PrependT<DropAll<T, Pack<Ts...>>, H>> {};

}  // namespace auimpl
}  // namespace au
