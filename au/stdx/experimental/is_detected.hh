// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#pragma once

#include <type_traits>

#include "au/stdx/type_traits.hh"

namespace au {
namespace stdx {
namespace experimental {

////////////////////////////////////////////////////////////////////////////////////////////////////
// `nonesuch`: adapted from (https://en.cppreference.com/w/cpp/experimental/nonesuch).

struct nonesuch {
    ~nonesuch() = delete;
    nonesuch(nonesuch const &) = delete;
    void operator=(nonesuch const &) = delete;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `is_detected` and friends: adapted from
// (https://en.cppreference.com/w/cpp/experimental/is_detected).

namespace detail {
template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, stdx::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

}  // namespace detail

template <template <class...> class Op, class... Args>
using is_detected = typename detail::detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename detail::detector<nonesuch, void, Op, Args...>::type;

template <class Default, template <class...> class Op, class... Args>
using detected_or = detail::detector<Default, void, Op, Args...>;

template <class Default, template <class...> class Op, class... Args>
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

}  // namespace experimental
}  // namespace stdx
}  // namespace au
