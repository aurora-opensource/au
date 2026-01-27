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

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
#include <compare>
#endif

#include "au/stdx/type_traits.hh"
#include "au/stdx/utility.hh"

// This file provides alternatives to certain standard library function objects for comparison and
// arithmetic: `std::less<void>`, `std::plus<void>`, etc.
//
// These are _not_ intended as _fully general_ replacements.  They are _only_ intended for certain
// specific use cases in this library.  External user code should not use these utilities: their
// contract is subject to change at any time to suit the needs of Au.
//
// The biggest change is that these function objects produce mathematically correct results when
// comparing built-in integral types with mixed signedness.  As a concrete example: in the C++
// language, `-1 < 1u` is `false`, because the common type of the input types is `unsigned int`, and
// the `int` input `-1` gets converted to a (very large) `unsigned int` value.  However, using these
// types, `Lt{}(-1, 1u)` will correctly return `true`!
//
// There were two initial motivations to roll our own versions instead of just using the ones from
// the standard library (as we had done earlier).  First, the `<functional>` header is moderately
// expensive to include---using these alternatives could save 100 ms or more on every file.  Second,
// certain compilers (such as the Green Hills compiler) struggle with the trailing return types in,
// say, `std::less<void>::operator()`, but work correctly with our alternatives.

namespace au {
namespace detail {

// These tag types act as a kind of "compile time enum".
struct CompareBuiltInIntegers {};
struct DefaultComparison {};

// `ComparisonCategory<T, U>` acts like a function which takes two _types_, and returns the correct
// instance of the above "compile time enum".
template <typename T, typename U>
using ComparisonCategory =
    std::conditional_t<stdx::conjunction<std::is_integral<T>, std::is_integral<U>>::value,
                       CompareBuiltInIntegers,
                       DefaultComparison>;

//
// Comparison operators.
//

struct Equal {
    template <typename T, typename U>
    constexpr bool operator()(const T &a, const U &b) const {
        return op_impl(ComparisonCategory<T, U>{}, a, b);
    }

    template <typename T, typename U>
    constexpr bool op_impl(DefaultComparison, const T &a, const U &b) const {
        return a == b;
    }

    template <typename T, typename U>
    constexpr bool op_impl(CompareBuiltInIntegers, const T &a, const U &b) const {
        return stdx::cmp_equal(a, b);
    }
};
constexpr auto equal = Equal{};

struct NotEqual {
    template <typename T, typename U>
    constexpr bool operator()(const T &a, const U &b) const {
        return op_impl(ComparisonCategory<T, U>{}, a, b);
    }

    template <typename T, typename U>
    constexpr bool op_impl(DefaultComparison, const T &a, const U &b) const {
        return a != b;
    }

    template <typename T, typename U>
    constexpr bool op_impl(CompareBuiltInIntegers, const T &a, const U &b) const {
        return stdx::cmp_not_equal(a, b);
    }
};
constexpr auto not_equal = NotEqual{};

struct Greater {
    template <typename T, typename U>
    constexpr bool operator()(const T &a, const U &b) const {
        return op_impl(ComparisonCategory<T, U>{}, a, b);
    }

    template <typename T, typename U>
    constexpr bool op_impl(DefaultComparison, const T &a, const U &b) const {
        return a > b;
    }

    template <typename T, typename U>
    constexpr bool op_impl(CompareBuiltInIntegers, const T &a, const U &b) const {
        return stdx::cmp_greater(a, b);
    }
};
constexpr auto greater = Greater{};

struct Less {
    template <typename T, typename U>
    constexpr bool operator()(const T &a, const U &b) const {
        return op_impl(ComparisonCategory<T, U>{}, a, b);
    }

    template <typename T, typename U>
    constexpr bool op_impl(DefaultComparison, const T &a, const U &b) const {
        return a < b;
    }

    template <typename T, typename U>
    constexpr bool op_impl(CompareBuiltInIntegers, const T &a, const U &b) const {
        return stdx::cmp_less(a, b);
    }
};
constexpr auto less = Less{};

struct GreaterEqual {
    template <typename T, typename U>
    constexpr bool operator()(const T &a, const U &b) const {
        return op_impl(ComparisonCategory<T, U>{}, a, b);
    }

    template <typename T, typename U>
    constexpr bool op_impl(DefaultComparison, const T &a, const U &b) const {
        return a >= b;
    }

    template <typename T, typename U>
    constexpr bool op_impl(CompareBuiltInIntegers, const T &a, const U &b) const {
        return stdx::cmp_greater_equal(a, b);
    }
};
constexpr auto greater_equal = GreaterEqual{};

struct LessEqual {
    template <typename T, typename U>
    constexpr bool operator()(const T &a, const U &b) const {
        return op_impl(ComparisonCategory<T, U>{}, a, b);
    }

    template <typename T, typename U>
    constexpr bool op_impl(DefaultComparison, const T &a, const U &b) const {
        return a <= b;
    }

    template <typename T, typename U>
    constexpr bool op_impl(CompareBuiltInIntegers, const T &a, const U &b) const {
        return stdx::cmp_less_equal(a, b);
    }
};
constexpr auto less_equal = LessEqual{};

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
struct ThreeWayCompare {
    template <typename T, typename U>
    constexpr auto operator()(const T &a, const U &b) const {
        // Note that we do not need special treatment for the case where `T` and `U` are both
        // integral types, because the C++ language already prohibits narrowing conversions (such as
        // `int` to `uint`) for `operator<=>`.  We can rely on this implicit warning to induce users
        // to fix their code.
        return a <=> b;
    }
};
constexpr auto three_way_compare = ThreeWayCompare{};
#endif

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
