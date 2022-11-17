// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

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
