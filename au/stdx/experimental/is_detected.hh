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

namespace auimpl {
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

}  // namespace auimpl

template <template <class...> class Op, class... Args>
using is_detected = typename auimpl::detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename auimpl::detector<nonesuch, void, Op, Args...>::type;

template <class Default, template <class...> class Op, class... Args>
using detected_or = auimpl::detector<Default, void, Op, Args...>;

template <class Default, template <class...> class Op, class... Args>
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

}  // namespace experimental
}  // namespace stdx
}  // namespace au
