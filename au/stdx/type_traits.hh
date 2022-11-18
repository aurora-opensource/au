// Copyright 2022 Aurora Operations, Inc.

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
template <class B1>
struct conjunction<B1> : B1 {};
template <class B1, class... Bn>
struct conjunction<B1, Bn...> : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/disjunction).
template <class...>
struct disjunction : std::false_type {};
template <class B1>
struct disjunction<B1> : B1 {};
template <class B1, class... Bn>
struct disjunction<B1, Bn...> : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};

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
