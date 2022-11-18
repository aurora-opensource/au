// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include <limits>
#include <type_traits>

namespace au {
namespace stdx {

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
//
// For C++14 compatibility, we needed to change `if constexpr` to SFINAE.
template <typename T, typename U, typename Enable = void>
struct CmpEqualImpl;
template <class T, class U>
constexpr bool cmp_equal(T t, U u) noexcept {
    return CmpEqualImpl<T, U>{}(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
template <class T, class U>
constexpr bool cmp_not_equal(T t, U u) noexcept {
    return !cmp_equal(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
//
// For C++14 compatibility, we needed to change `if constexpr` to SFINAE.
template <typename T, typename U, typename Enable = void>
struct CmpLessImpl;
template <class T, class U>
constexpr bool cmp_less(T t, U u) noexcept {
    return CmpLessImpl<T, U>{}(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
template <class T, class U>
constexpr bool cmp_greater(T t, U u) noexcept {
    return cmp_less(u, t);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
template <class T, class U>
constexpr bool cmp_less_equal(T t, U u) noexcept {
    return !cmp_greater(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/intcmp).
template <class T, class U>
constexpr bool cmp_greater_equal(T t, U u) noexcept {
    return !cmp_less(t, u);
}

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/in_range).
template <class R, class T>
constexpr bool in_range(T t) noexcept {
    return cmp_greater_equal(t, std::numeric_limits<R>::min()) &&
           cmp_less_equal(t, std::numeric_limits<R>::max());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, typename U>
struct CmpEqualImpl<T, U, std::enable_if_t<std::is_signed<T>::value == std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return t == u; }
};

template <typename T, typename U>
struct CmpEqualImpl<T, U, std::enable_if_t<std::is_signed<T>::value && !std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return t < 0 ? false : std::make_unsigned_t<T>(t) == u; }
};

template <typename T, typename U>
struct CmpEqualImpl<T, U, std::enable_if_t<!std::is_signed<T>::value && std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return u < 0 ? false : t == std::make_unsigned_t<U>(u); }
};

template <typename T, typename U>
struct CmpLessImpl<T, U, std::enable_if_t<std::is_signed<T>::value == std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return t < u; }
};

template <typename T, typename U>
struct CmpLessImpl<T, U, std::enable_if_t<std::is_signed<T>::value && !std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return t < 0 ? true : std::make_unsigned_t<T>(t) < u; }
};

template <typename T, typename U>
struct CmpLessImpl<T, U, std::enable_if_t<!std::is_signed<T>::value && std::is_signed<U>::value>> {
    constexpr bool operator()(T t, U u) { return u < 0 ? false : t < std::make_unsigned_t<U>(u); }
};

}  // namespace stdx
}  // namespace au
