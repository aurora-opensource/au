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

template <template <typename...> class Pack, typename T, typename... Us>
struct Prepend<Pack<Us...>, T> {
    using type = Pack<T, Us...>;
};

}  // namespace detail
}  // namespace au
