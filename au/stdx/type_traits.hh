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

namespace au {
namespace stdx {

// Source: adapted from (https://en.cppreference.com/w/cpp/types/type_identity).
template <class T>
struct type_identity {
    using type = T;
};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/integral_constant).
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

// Source: adapted from (https://en.cppreference.com/w/cpp/types/conjunction).
template <class...>
struct conjunction : std::true_type {};
template <class B>
struct conjunction<B> : B {};
template <class B, class... Bn>
struct conjunction<B, Bn...> : std::conditional_t<bool(B::value), conjunction<Bn...>, B> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/disjunction).
template <class...>
struct disjunction : std::false_type {};
template <class B>
struct disjunction<B> : B {};
template <class B, class... Bn>
struct disjunction<B, Bn...> : std::conditional_t<bool(B::value), B, disjunction<Bn...>> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/negation).
template <class B>
struct negation : stdx::bool_constant<!static_cast<bool>(B::value)> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/remove_cvref).
template <class T>
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};
template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

// Source: adapted from (https://en.cppreference.com/w/cpp/types/void_t).
template <class...>
using void_t = void;

}  // namespace stdx
}  // namespace au
