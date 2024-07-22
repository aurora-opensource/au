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

// This file provides drop-in replacements for certain standard library function objects for
// comparison and arithmetic: `std::less<void>`, `std::plus<void>`, etc.
//
// These are _not_ intended as _fully general_ replacements.  They are _only_ intended for certain
// specific use cases in this library, where we can ensure certain preconditions are met before they
// are called.  For example, these utilities don't handle comparing signed and unsigned integral
// types, because we only ever use them in places where we've already explicitly cast our quantities
// to the same Rep.
//
// There are two main reasons we rolled our own versions instead of just using the ones from the
// standard library (as we had initially done).  First, the `<functional>` header is moderately
// expensive to include---using these alternatives could save 100 ms or more on every file.  Second,
// certain compilers (such as the Green Hills compiler) struggle with the trailing return types in,
// say, `std::less<void>::operator()`, but work correctly with our alternatives.

namespace au {
namespace detail {

//
// Comparison operators.
//

struct Equal {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a == b;
    }
};
constexpr auto equal = Equal{};

struct NotEqual {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a != b;
    }
};
constexpr auto not_equal = NotEqual{};

struct Greater {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a > b;
    }
};
constexpr auto greater = Greater{};

struct Less {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a < b;
    }
};
constexpr auto less = Less{};

struct GreaterEqual {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a >= b;
    }
};
constexpr auto greater_equal = GreaterEqual{};

struct LessEqual {
    template <typename T>
    constexpr bool operator()(const T &a, const T &b) const {
        return a <= b;
    }
};
constexpr auto less_equal = LessEqual{};

//
// Arithmetic operators.
//

struct Plus {
    template <typename T, typename U>
    constexpr auto operator()(const T &a, const U &b) const {
        return a + b;
    }
};
constexpr auto plus = Plus{};

struct Minus {
    template <typename T, typename U>
    constexpr auto operator()(const T &a, const U &b) const {
        return a - b;
    }
};
constexpr auto minus = Minus{};

}  // namespace detail
}  // namespace au
