// Copyright 2022 Aurora Operations, Inc.

#pragma once

#include <utility>

namespace au {
namespace stdx {

// Source: adapted from (https://en.cppreference.com/w/cpp/utility/functional/identity)
struct identity {
    template <class T>
    constexpr T &&operator()(T &&t) const noexcept {
        return std::forward<T>(t);
    }
};

}  // namespace stdx
}  // namespace au
