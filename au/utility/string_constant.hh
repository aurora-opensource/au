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

#include <cstdint>
#include <utility>

namespace au {
namespace detail {

//
// A constexpr-compatible string constant class of a given size.
//
// The point is to make it easy to build up compile-time strings by using "join" and "concatenate"
// operations.
//
// Beware that this is not one type, but a family of types, one for each length!  If you're in a
// context where you can't use `auto` (say, because you're making a member variable), you'll need to
// know the length in order to name the type.
//
template <std::size_t Strlen>
class StringConstant;

//
// `as_string_constant()`: Create StringConstant<N>, of correct length, corresponding to the input.
//
// Possible inputs include char-arrays, or `StringConstant` (for which this is the identity).
//
template <std::size_t N>
constexpr StringConstant<N - 1> as_string_constant(const char (&c_string)[N]) {
    return {c_string};
}
template <std::size_t N>
constexpr StringConstant<N> as_string_constant(const StringConstant<N> &x) {
    return x;
}

//
// Create StringConstant which concatenates all arguments.
//
// Each argument will be treated as a StringConstant.  The final length will automatically be
// computed from the lengths of the inputs.
//
template <typename... Ts>
constexpr auto concatenate(const Ts &...ts);

//
// Join arbitrarily many arguments into a new StringConstant, using the first argument as separator.
//
// Each argument will be treated as a StringConstant.  The final length will automatically be
// computed from the lengths of the inputs.
//
// As usual for the join algorithm, the separator will not appear in the output unless there are at
// least two arguments (apart from the separator) being joined.
//
template <typename SepT, typename... StringTs>
constexpr auto join_by(const SepT &sep, const StringTs &...ts);

//
// Wrap a StringConstant in parentheses () if the supplied template param is true.
//
template <bool Enable, typename StringT>
constexpr auto parens_if(const StringT &s);

//
// A constexpr-compatible utility to generate compile-time string representations of integers.
//
// `IToA<N>::value` is a `StringConstant<Len>` of whatever appropriate length `Len` is needed to
// represent `N`.  This includes handling the negative sign (if any).
//
template <int64_t N>
struct IToA;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

// The string-length needed to hold a representation of this integer.
constexpr std::size_t string_size(int64_t x) {
    if (x < 0) {
        return string_size(-x) + 1;
    }

    std::size_t digits = 1;
    while (x > 9) {
        x /= 10;
        ++digits;
    }
    return digits;
}

// The sum of the template parameters.
template <std::size_t... Ns>
constexpr std::size_t sum() {
    std::size_t result{0};
    std::size_t values[] = {Ns...};
    for (std::size_t i = 0; i < sizeof...(Ns); ++i) {
        result += values[i];
    }
    return result;
}

template <std::size_t Strlen>
class StringConstant {
 public:
    constexpr StringConstant(const char (&c_string)[Strlen + 1])
        : StringConstant{c_string, std::make_index_sequence<Strlen>{}} {}

    static constexpr std::size_t length = Strlen;

    // Get a C-string representation of this constant.
    //
    // (Note that the constructors have guaranteed a correct placement of the '\0'.)
    constexpr const char *c_str() const { return data_array_; }
    constexpr operator const char *() const { return c_str(); }

    // Get a (sizeof()-compatible) char array reference to the data.
    constexpr auto char_array() const -> const char (&)[Strlen + 1] { return data_array_; }

    // The string-length of this string (i.e., NOT including the null terminator).
    constexpr std::size_t size() const { return Strlen; }

    // Whether this string is empty (i.e., has size zero).
    constexpr bool empty() const { return size() == 0u; }

    // Join multiple `StringConstant<Ns>` inputs, using `*this` as the separator.
    template <std::size_t... Ns>
    constexpr auto join(const StringConstant<Ns> &...items) const {
        constexpr std::size_t N =
            sum<Ns...>() + Strlen * (sizeof...(items) > 0 ? (sizeof...(items) - 1) : 0);
        char result[N + 1]{'\0'};
        join_impl(result, items...);
        return StringConstant<N>{result};
    }

 private:
    // This would be unsafe if called with arbitrary pointers and/or integer sequences.  However,
    // this is a private constructor of this class, called only by its public constructor(s), and we
    // know they satisfy the conditions needed to call this function safely.
    template <std::size_t... Is>
    constexpr StringConstant(const char *data, std::index_sequence<Is...>)
        : data_array_{data[Is]..., '\0'} {
        (void)data;  // Suppress unused-var error when `Is` is empty in platform-independent way.
    }

    // Base case for the join algorithm.
    constexpr void join_impl(char *) const {}

    // Recursive case for the join algorithm.
    template <std::size_t N, std::size_t... Ns>
    constexpr void join_impl(char *out_iter,
                             const StringConstant<N> &head,
                             const StringConstant<Ns> &...tail) const {
        // Definitely copy data from the head element.
        //
        // The `static_cast<int>` incantation mollifies certain "helpful" compilers, which notice
        // that the comparison is always false when `Strlen` is `0`, and disregard best practices
        // for generic programming by failing the build for this.
        for (std::size_t i = 0; static_cast<int>(i) < static_cast<int>(N); ++i) {
            *out_iter++ = head.c_str()[i];
        }

        // If there are tail elements, copy out the separator, and recurse.
        if (sizeof...(tail) > 0) {
            // The `static_cast<int>` incantation mollifies certain "helpful" compilers, which
            // notice that the comparison is always false when `Strlen` is `0`, and disregard best
            // practices for generic programming by failing the build for this.
            for (std::size_t i = 0; static_cast<int>(i) < static_cast<int>(Strlen); ++i) {
                *out_iter++ = data_array_[i];
            }
            join_impl(out_iter, tail...);
        }
    }

    // Data storage for the string constant.
    const char data_array_[Strlen + 1];
};

template <std::size_t Strlen>
constexpr std::size_t StringConstant<Strlen>::length;

template <typename... Ts>
constexpr auto concatenate(const Ts &...ts) {
    return join_by("", ts...);
}

template <typename SepT, typename... StringTs>
constexpr auto join_by(const SepT &sep, const StringTs &...ts) {
    return as_string_constant(sep).join(as_string_constant(ts)...);
}

template <int64_t N>
struct IToA {
 private:
    static constexpr auto print_to_array() {
        char data[length + 1] = {'\0'};

        int num = N;
        if (num < 0) {
            data[0] = '-';
            num = -num;
        }

        std::size_t i = length - 1;
        do {
            data[i--] = '0' + static_cast<char>(num % 10);
            num /= 10;
        } while (num > 0);

        return StringConstant<length>{data};
    }

 public:
    static constexpr std::size_t length = string_size(N);

    static constexpr StringConstant<length> value = print_to_array();
};

// Definitions for IToA<N>::value.  (Needed to prevent linker errors.)
template <int64_t N>
constexpr std::size_t IToA<N>::length;
template <int64_t N>
constexpr StringConstant<IToA<N>::length> IToA<N>::value;

template <bool Enable>
struct ParensIf;

template <>
struct ParensIf<true> {
    static constexpr StringConstant<1> open() { return as_string_constant("("); }
    static constexpr StringConstant<1> close() { return as_string_constant(")"); }
};

template <>
struct ParensIf<false> {
    static constexpr StringConstant<0> open() { return as_string_constant(""); }
    static constexpr StringConstant<0> close() { return as_string_constant(""); }
};

template <bool Enable, typename StringT>
constexpr auto parens_if(const StringT &s) {
    return concatenate(ParensIf<Enable>::open(), s, ParensIf<Enable>::close());
}

}  // namespace detail
}  // namespace au
