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
