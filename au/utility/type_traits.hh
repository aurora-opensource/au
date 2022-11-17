// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.

template <template <typename...> class Pack, typename T, typename... Us>
struct Prepend<Pack<Us...>, T> {
    using type = Pack<T, Us...>;
};

}  // namespace detail
}  // namespace au
