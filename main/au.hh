// Copyright 2025 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <ostream>
#include <ratio>
#include <type_traits>
#include <utility>

// Version identifier: 0.5.0-base-35-gf2ab254
// <iostream> support: INCLUDED
// <format> support: EXCLUDED
// List of included units:
//   amperes
//   bits
//   candelas
//   grams
//   kelvins
//   meters
//   moles
//   radians
//   seconds
//   unos
// List of included constants:


namespace au {

struct Zero;

template <typename B, std::intmax_t N>
struct Pow;

template <typename B, std::intmax_t N, std::intmax_t D>
struct RatioPow;

template <typename... BPs>
struct Dimension;

template <typename... BPs>
struct Magnitude;

struct Negative;

template <typename UnitT>
struct QuantityMaker;

template <typename Unit>
struct SingularNameFor;

template <typename UnitT>
struct QuantityPointMaker;

template <typename UnitT, typename RepT>
class Quantity;

//
// Machinery for forward-declaring a unit product.
//
// To use, make an alias with the correct unit powers in the correct order, in the `_fwd.hh` file.
// In the `.hh` file, call `is_forward_declared_unit_valid(...)` (defined in `unit_of_measure.hh`)
// on an instance of that alias.
//
template <typename... UnitPowers>
struct UnitProduct;
template <typename... UnitPowers>
struct ForwardDeclareUnitProduct {
    using unit_type = UnitProduct<UnitPowers...>;
};

//
// Machinery for forward-declaring a unit power.
//
// To use, make an alias with the same unit and power(s) that `UnitPowerT` would produce, in the
// `_fwd.hh` file.  In the `.hh` file, call `is_forward_declared_unit_valid(...)` (defined in
// `unit_of_measure.hh`) on that alias.
//
template <typename U, std::intmax_t N, std::intmax_t D = 1>
struct ForwardDeclareUnitPow {
    using unit_type = RatioPow<U, N, D>;
};
template <typename U, std::intmax_t N>
struct ForwardDeclareUnitPow<U, N, 1> {
    using unit_type = Pow<U, N>;
};

//
// Quantity aliases to set a particular Rep.
//
// This presents a less cumbersome interface for end users.
//
template <typename UnitT>
using QuantityD = Quantity<UnitT, double>;
template <typename UnitT>
using QuantityF = Quantity<UnitT, float>;
template <typename UnitT>
using QuantityI = Quantity<UnitT, int>;
template <typename UnitT>
using QuantityU = Quantity<UnitT, unsigned int>;
template <typename UnitT>
using QuantityI32 = Quantity<UnitT, int32_t>;
template <typename UnitT>
using QuantityU32 = Quantity<UnitT, uint32_t>;
template <typename UnitT>
using QuantityI64 = Quantity<UnitT, int64_t>;
template <typename UnitT>
using QuantityU64 = Quantity<UnitT, uint64_t>;

template <typename T>
struct CorrespondingQuantity;

template <typename UnitT, typename RepT>
class QuantityPoint;

//
// QuantityPoint aliases to set a particular Rep.
//
// This presents a less cumbersome interface for end users.
//
template <typename UnitT>
using QuantityPointD = QuantityPoint<UnitT, double>;
template <typename UnitT>
using QuantityPointF = QuantityPoint<UnitT, float>;
template <typename UnitT>
using QuantityPointI = QuantityPoint<UnitT, int>;
template <typename UnitT>
using QuantityPointU = QuantityPoint<UnitT, unsigned int>;
template <typename UnitT>
using QuantityPointI32 = QuantityPoint<UnitT, int32_t>;
template <typename UnitT>
using QuantityPointU32 = QuantityPoint<UnitT, uint32_t>;
template <typename UnitT>
using QuantityPointI64 = QuantityPoint<UnitT, int64_t>;
template <typename UnitT>
using QuantityPointU64 = QuantityPoint<UnitT, uint64_t>;

template <typename Unit>
struct Constant;

template <typename Unit>
struct SymbolFor;

template <template <class U> class Prefix>
struct PrefixApplier;

// SI Prefixes.
template <typename U>
struct Quetta;
template <typename U>
struct Ronna;
template <typename U>
struct Yotta;
template <typename U>
struct Zetta;
template <typename U>
struct Exa;
template <typename U>
struct Peta;
template <typename U>
struct Tera;
template <typename U>
struct Giga;
template <typename U>
struct Mega;
template <typename U>
struct Kilo;
template <typename U>
struct Hecto;
template <typename U>
struct Deka;
template <typename U>
struct Deci;
template <typename U>
struct Centi;
template <typename U>
struct Milli;
template <typename U>
struct Micro;
template <typename U>
struct Nano;
template <typename U>
struct Pico;
template <typename U>
struct Femto;
template <typename U>
struct Atto;
template <typename U>
struct Zepto;
template <typename U>
struct Yocto;
template <typename U>
struct Ronto;
template <typename U>
struct Quecto;

// Binary Prefixes.
template <typename U>
struct Yobi;
template <typename U>
struct Zebi;
template <typename U>
struct Exbi;
template <typename U>
struct Pebi;
template <typename U>
struct Tebi;
template <typename U>
struct Gibi;
template <typename U>
struct Mebi;
template <typename U>
struct Kibi;

}  // namespace au


namespace au {
namespace stdx {

// Source: adapted from (https://en.cppreference.com/w/cpp/types/type_identity).
template <class T>
struct type_identity {
    using type = T;
};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/integral_constant).
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

// Source: adapted from (https://en.cppreference.com/w/cpp/types/conjunction).
template <class...>
struct conjunction : std::true_type {};
template <class B>
struct conjunction<B> : B {};
template <class B, class... Bn>
struct conjunction<B, Bn...> : std::conditional_t<bool(B::value), conjunction<Bn...>, B> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/disjunction).
template <class...>
struct disjunction : std::false_type {};
template <class B>
struct disjunction<B> : B {};
template <class B, class... Bn>
struct disjunction<B, Bn...> : std::conditional_t<bool(B::value), B, disjunction<Bn...>> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/negation).
template <class B>
struct negation : stdx::bool_constant<!static_cast<bool>(B::value)> {};

// Source: adapted from (https://en.cppreference.com/w/cpp/types/remove_cvref).
template <class T>
struct remove_cvref {
    typedef std::remove_cv_t<std::remove_reference_t<T>> type;
};
template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

// Source: adapted from (https://en.cppreference.com/w/cpp/types/void_t).
template <class...>
using void_t = void;

}  // namespace stdx
}  // namespace au



namespace au {
namespace detail {

template <typename PackT, typename T>
struct Prepend;
template <typename PackT, typename T>
using PrependT = typename Prepend<PackT, T>::type;

template <template <class> class Condition, template <class...> class Pack, typename... Ts>
struct IncludeInPackIfImpl;
template <template <class> class Condition, template <class...> class Pack, typename... Ts>
using IncludeInPackIf = typename IncludeInPackIfImpl<Condition, Pack, Ts...>::type;

template <typename T, typename Pack>
struct DropAllImpl;
template <typename T, typename Pack>
using DropAll = typename DropAllImpl<T, Pack>::type;

template <template <class...> class Pack, typename... Ts>
struct FlattenAsImpl;
template <template <class...> class Pack, typename... Ts>
using FlattenAs = typename FlattenAsImpl<Pack, Ts...>::type;

template <typename T, typename U>
struct SameTypeIgnoringCvref : std::is_same<stdx::remove_cvref_t<T>, stdx::remove_cvref_t<U>> {};

template <typename T, typename U>
constexpr bool same_type_ignoring_cvref(T, U) {
    return SameTypeIgnoringCvref<T, U>::value;
}

template <typename... Ts>
struct AlwaysFalse : std::false_type {};

template <typename R1, typename R2>
struct CommonTypeButPreserveIntSignednessImpl;
template <typename R1, typename R2>
using CommonTypeButPreserveIntSignedness =
    typename CommonTypeButPreserveIntSignednessImpl<R1, R2>::type;

//
// `PromotedType<T>` is the result type for arithmetic operations involving `T`.  Of course, this is
// normally just `T`, but integer promotion for small integral types can change this.
//
template <typename T>
struct PromotedTypeImpl;
template <typename T>
using PromotedType = typename PromotedTypeImpl<T>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `Prepend` implementation.

template <template <typename...> class Pack, typename T, typename... Us>
struct Prepend<Pack<Us...>, T> {
    using type = Pack<T, Us...>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `IncludeInPackIf` implementation.

// Helper: change the pack.  This lets us do our work in one kind of pack, and then swap it out for
// another pack at the end.
template <template <class...> class NewPack, typename PackT>
struct ChangePackToImpl;
template <template <class...> class NewPack, typename PackT>
using ChangePackTo = typename ChangePackToImpl<NewPack, PackT>::type;
template <template <class...> class NewPack, template <class...> class OldPack, typename... Ts>
struct ChangePackToImpl<NewPack, OldPack<Ts...>> : stdx::type_identity<NewPack<Ts...>> {};

// A generic typelist with no constraints on members or ordering.  Intended as a type to hold
// intermediate work.
template <typename... Ts>
struct GenericTypeList;

template <template <class> class Condition, typename PackT>
struct ListMatchingTypesImpl;
template <template <class> class Condition, typename PackT>
using ListMatchingTypes = typename ListMatchingTypesImpl<Condition, PackT>::type;

// Base case:
template <template <class> class Condition>
struct ListMatchingTypesImpl<Condition, GenericTypeList<>>
    : stdx::type_identity<GenericTypeList<>> {};

// Recursive case:
template <template <class> class Condition, typename H, typename... Ts>
struct ListMatchingTypesImpl<Condition, GenericTypeList<H, Ts...>>
    : std::conditional<Condition<H>::value,
                       PrependT<ListMatchingTypes<Condition, GenericTypeList<Ts...>>, H>,
                       ListMatchingTypes<Condition, GenericTypeList<Ts...>>> {};

template <template <class> class Condition, template <class...> class Pack, typename... Ts>
struct IncludeInPackIfImpl
    : stdx::type_identity<
          ChangePackTo<Pack, ListMatchingTypes<Condition, GenericTypeList<Ts...>>>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DropAll` implementation.

// Base case.
template <typename T, template <class...> class Pack>
struct DropAllImpl<T, Pack<>> : stdx::type_identity<Pack<>> {};

// Recursive case:
template <typename T, template <class...> class Pack, typename H, typename... Ts>
struct DropAllImpl<T, Pack<H, Ts...>>
    : std::conditional<std::is_same<T, H>::value,
                       DropAll<T, Pack<Ts...>>,
                       detail::PrependT<DropAll<T, Pack<Ts...>>, H>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `FlattenAs` implementation.

template <typename P1, typename P2>
struct ConcatImpl;
template <typename P1, typename P2>
using Concat = typename ConcatImpl<P1, P2>::type;

template <template <class...> class Pack, typename... T1s, typename... T2s>
struct ConcatImpl<Pack<T1s...>, Pack<T2s...>> : stdx::type_identity<Pack<T1s..., T2s...>> {};

template <template <class...> class Pack, typename ResultPack, typename... Ts>
struct FlattenAsImplHelper;

template <template <class...> class Pack, typename ResultPack>
struct FlattenAsImplHelper<Pack, ResultPack> : stdx::type_identity<ResultPack> {};

// Skip empty packs.
template <template <class...> class Pack, typename ResultPack, typename... Us>
struct FlattenAsImplHelper<Pack, ResultPack, Pack<>, Us...>
    : FlattenAsImplHelper<Pack, ResultPack, Us...> {};

template <template <class...> class Pack,
          typename ResultPack,
          typename T,
          typename... Ts,
          typename... Us>
struct FlattenAsImplHelper<Pack, ResultPack, Pack<T, Ts...>, Us...>
    : FlattenAsImplHelper<Pack, ResultPack, T, Pack<Ts...>, Us...> {};

template <template <class...> class Pack, typename ResultPack, typename T, typename... Us>
struct FlattenAsImplHelper<Pack, ResultPack, T, Us...>
    : FlattenAsImplHelper<Pack, Concat<ResultPack, Pack<T>>, Us...> {};

template <template <class...> class Pack, typename... Ts>
struct FlattenAsImpl : FlattenAsImplHelper<Pack, Pack<>, Ts...> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonTypeButPreserveIntSignedness` implementation.

// `CopySignednessIfIntType<X, T>` has a `type` member that is always `T`, unless `T` is an integral
// type: in which case, it's the signed version of `T` if `X` is signed, and the unsigned version of
// `T` if `X` is unsigned.
template <typename SignednessSource, typename T, bool IsTIntegral>
struct CopySignednessIfIntTypeHelper;
template <typename SignednessSource, typename T>
struct CopySignednessIfIntTypeHelper<SignednessSource, T, true>
    : std::conditional<std::is_unsigned<SignednessSource>::value,
                       std::make_unsigned_t<T>,
                       std::make_signed_t<T>> {};
template <typename SignednessSource, typename T>
struct CopySignednessIfIntTypeHelper<SignednessSource, T, false> : stdx::type_identity<T> {};

template <typename SignednessSource, typename T>
struct CopySignednessIfIntType
    : CopySignednessIfIntTypeHelper<SignednessSource, T, std::is_integral<T>::value> {};

template <typename R1, typename R2>
struct CommonTypeButPreserveIntSignednessImpl
    : CopySignednessIfIntType<R1, std::common_type_t<R1, R2>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `PromotedType<T>` implementation.

template <typename T>
struct PromotedTypeImpl {
    using type = decltype(std::declval<T>() * std::declval<T>());

    static_assert(std::is_same<type, typename PromotedTypeImpl<type>::type>::value,
                  "We explicitly assume that promoted types are not again promotable");
};

}  // namespace detail
}  // namespace au



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

// The absolute value of a signed integer, as an unsigned integer.
//
// This handles the special case where the lowest `int64_t` cannot be directly negated: it would be
// too big to fit into `int64_t`.
template <typename S>
constexpr std::make_unsigned_t<S> abs_as_unsigned(S x) {
    static_assert(stdx::conjunction<std::is_integral<S>, std::is_signed<S>>::value,
                  "Only designed for signed integral types");
    using U = std::make_unsigned_t<S>;
    constexpr auto SMAX = static_cast<U>(std::numeric_limits<S>::max());
    constexpr auto UMAX = std::numeric_limits<U>::max();

    auto result = static_cast<U>(x);
    return (result > SMAX) ? static_cast<U>(UMAX - result + 1u) : result;
}

// The string-length needed to hold a representation of this unsigned integer.
constexpr std::size_t string_size_unsigned(uint64_t x) {
    std::size_t digits = 1;
    while (x > 9) {
        x /= 10;
        ++digits;
    }
    return digits;
}

// The string-length needed to hold a representation of this integer.
constexpr std::size_t string_size(int64_t x) {
    std::size_t sign_length = 0u;
    if (x < 0) {
        ++sign_length;
    }
    return string_size_unsigned(abs_as_unsigned(x)) + sign_length;
}

// The sum of the template parameters.
template <std::size_t... Ns>
constexpr std::size_t sum() {
    std::size_t result{0};
    std::size_t values[] = {0u, Ns...};  // Dummy `0u` avoids empty array.
    for (std::size_t i = 0; i < sizeof...(Ns); ++i) {
        result += values[i + 1u];  // "+ 1u" to skip the dummy value.
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

template <uint64_t N>
struct UIToA {
 private:
    static constexpr auto print_to_array() {
        char data[length + 1u] = {'\0'};

        uint64_t num = N;
        std::size_t i = length - 1;
        do {
            data[i--] = '0' + static_cast<char>(num % 10u);
            num /= 10u;
        } while (num > 0u);

        return StringConstant<length>{data};
    }

 public:
    static constexpr std::size_t length = string_size_unsigned(N);

    static constexpr StringConstant<length> value = print_to_array();
};

// Definitions for UIToA<N>::value.  (Needed to prevent linker errors.)
template <uint64_t N>
constexpr std::size_t UIToA<N>::length;
template <uint64_t N>
constexpr StringConstant<UIToA<N>::length> UIToA<N>::value;

template <bool IsPositive>
struct SignIfPositiveIs {
    static constexpr StringConstant<0> value() { return StringConstant<0>{""}; }
};
template <>
struct SignIfPositiveIs<false> {
    static constexpr StringConstant<1> value() { return StringConstant<1>{"-"}; }
};

template <int64_t N>
struct IToA {
    static constexpr std::size_t length = string_size(N);

    static constexpr StringConstant<length> value =
        concatenate(SignIfPositiveIs<(N >= 0)>::value(), UIToA<abs_as_unsigned(N)>::value);
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

template <std::size_t N>
constexpr auto as_char_array(const char (&x)[N]) -> const char (&)[N] {
    return x;
}

template <std::size_t N>
constexpr auto as_char_array(const StringConstant<N> &x) -> const char (&)[N + 1] {
    return x.char_array();
}

}  // namespace detail
}  // namespace au


namespace au {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic mathematical convenience functions.
//
// The reason these exist is to be able to make unit expressions easier to read in common cases.
// They also work for dimensions and magnitudes.

//
// This section works around an error:
//
//    warning: use of function template name with no prior declaration in function call with
//    explicit template arguments is a C++20 extension [-Wc++20-extensions]
//
// We work around it by providing declarations, even though those declarations are never used.
//
namespace no_prior_declaration_workaround {
struct Dummy;
}  // namespace no_prior_declaration_workaround
template <std::intmax_t N>
auto root(no_prior_declaration_workaround::Dummy);
template <std::intmax_t N>
auto pow(no_prior_declaration_workaround::Dummy);

// Make "inverse" an alias for "pow<-1>" when the latter exists (for anything).
template <typename T>
constexpr auto inverse(T x) -> decltype(pow<-1>(x)) {
    return pow<-1>(x);
}
template <typename T>
using Inverse = decltype(inverse(std::declval<T>()));

// Make "squared" an alias for "pow<2>" when the latter exists (for anything).
template <typename T>
constexpr auto squared(T x) -> decltype(pow<2>(x)) {
    return pow<2>(x);
}
template <typename T>
using Squared = decltype(squared(std::declval<T>()));

// Make "cubed" an alias for "pow<3>" when the latter exists (for anything).
template <typename T>
constexpr auto cubed(T x) -> decltype(pow<3>(x)) {
    return pow<3>(x);
}
template <typename T>
using Cubed = decltype(cubed(std::declval<T>()));

// Make "sqrt" an alias for "root<2>" when the latter exists (for anything).
template <typename T>
constexpr auto sqrt(T x) -> decltype(root<2>(x)) {
    return root<2>(x);
}
template <typename T>
using Sqrt = decltype(sqrt(std::declval<T>()));

// Make "cbrt" an alias for "root<3>" when the latter exists (for anything).
template <typename T>
constexpr auto cbrt(T x) -> decltype(root<3>(x)) {
    return root<3>(x);
}
template <typename T>
using Cbrt = decltype(cbrt(std::declval<T>()));

}  // namespace au


namespace au {
namespace detail {

// (a + b) % n
//
// Precondition: (a < n).
// Precondition: (b < n).
constexpr uint64_t add_mod(uint64_t a, uint64_t b, uint64_t n) {
    if (a >= n - b) {
        return a - (n - b);
    } else {
        return a + b;
    }
}

// (a - b) % n
//
// Precondition: (a < n).
// Precondition: (b < n).
constexpr uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t n) {
    if (a >= b) {
        return a - b;
    } else {
        return n - (b - a);
    }
}

// (a * b) % n
//
// Precondition: (a < n).
// Precondition: (b < n).
constexpr uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t n) {
    // Start by trying the simplest case, where everything "fits".
    if (b == 0u || a < std::numeric_limits<uint64_t>::max() / b) {
        return (a * b) % n;
    }

    // We know the "negative" result is smaller, because we've taken as many copies of `a` as will
    // fit into `n`.  So, do the reduced calculation in "negative space", and then transform the
    // result back at the end.
    uint64_t chunk_size = n / a;
    uint64_t num_chunks = b / chunk_size;
    uint64_t negative_chunk = n - (a * chunk_size);  // == n % a  (but this should be cheaper)
    uint64_t chunk_result = n - mul_mod(negative_chunk, num_chunks, n);

    // Compute the leftover.  (We don't need to recurse, because we know it will fit.)
    uint64_t leftover = b - num_chunks * chunk_size;
    uint64_t leftover_result = (a * leftover) % n;

    return add_mod(chunk_result, leftover_result, n);
}

// (a / 2) % n
//
// Precondition: (a < n).
// Precondition: (n is odd).
//
// If `a` is even, this is of course simply `a / 2` (because `(a < n)` as a precondition).
// Otherwise, we give the result one would obtain by first adding `n` (guaranteeing an even number,
// since `n` is also odd as a precondition), and _then_ dividing by `2`.
constexpr uint64_t half_mod_odd(uint64_t a, uint64_t n) {
    return (a / 2u) + ((a % 2u == 0u) ? 0u : (n / 2u + 1u));
}

// (base ^ exp) % n
constexpr uint64_t pow_mod(uint64_t base, uint64_t exp, uint64_t n) {
    uint64_t result = 1u;
    base %= n;

    while (exp > 0u) {
        if (exp % 2u == 1u) {
            result = mul_mod(result, base, n);
        }

        exp /= 2u;
        base = mul_mod(base, base, n);
    }

    return result;
}

}  // namespace detail
}  // namespace au


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



namespace au {

//
// A type trait that determines if a type is a valid representation type for `Quantity` or
// `QuantityPoint`.
//
template <typename T>
struct IsValidRep;

//
// A type trait to indicate whether the product of two types is a valid rep.
//
// Will validly return `false` if the product does not exist.
//
template <typename T, typename U>
struct IsProductValidRep;

//
// A type trait to indicate whether the quotient of two types is a valid rep.
//
// Will validly return `false` if the quotient does not exist.
//
template <typename T, typename U>
struct IsQuotientValidRep;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {
template <typename T>
struct IsAuType : std::false_type {};

template <typename U, typename R>
struct IsAuType<::au::Quantity<U, R>> : std::true_type {};

template <typename U, typename R>
struct IsAuType<::au::QuantityPoint<U, R>> : std::true_type {};

template <typename T>
using CorrespondingUnit = typename CorrespondingQuantity<T>::Unit;

template <typename T>
using CorrespondingRep = typename CorrespondingQuantity<T>::Rep;

template <typename T>
struct HasCorrespondingQuantity
    : stdx::conjunction<stdx::experimental::is_detected<CorrespondingUnit, T>,
                        stdx::experimental::is_detected<CorrespondingRep, T>> {};

template <typename T>
using LooksLikeAuOrOtherQuantity = stdx::disjunction<IsAuType<T>, HasCorrespondingQuantity<T>>;

// We need a way to form an "operation on non-quantity types only".  That is: it's some operation,
// but _if either input is a quantity_, then we _don't even form the type_.
//
// The reason this very specific machinery lives in `rep.hh` is because when we're dealing with
// operations on "types that might be a rep", we know we can exclude quantity types right away.
// (Note that we're using the term "quantity" in an expansive sense, which includes not just
// `au::Quantity`, but also `au::QuantityPoint`, and "quantity-like" types from other libraries
// (which we consider as "anything that has a `CorrespondingQuantity`".
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantity;
template <template <class...> class Op, typename... Ts>
using ResultIfNoneAreQuantityT = typename ResultIfNoneAreQuantity<Op, Ts...>::type;

// Default implementation where we know that none are quantities.
template <bool AreAnyQuantity, template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityImpl : stdx::type_identity<Op<Ts...>> {};

// Implementation if any of the types are quantities.
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityImpl<true, Op, Ts...> : stdx::type_identity<void> {};

// The main implementation.
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantity
    : ResultIfNoneAreQuantityImpl<stdx::disjunction<LooksLikeAuOrOtherQuantity<Ts>...>::value,
                                  Op,
                                  Ts...> {};

// The `std::is_empty` is a good way to catch all of the various unit and other monovalue types in
// our library, which have little else in common.  It's also just intrinsically true that it
// wouldn't make much sense to use an empty type as a rep.
template <typename T>
struct IsKnownInvalidRep
    : stdx::disjunction<std::is_empty<T>, LooksLikeAuOrOtherQuantity<T>, std::is_same<void, T>> {};

// The type of the product of two types.
template <typename T, typename U>
using ProductType = decltype(std::declval<T>() * std::declval<U>());

template <typename T, typename U>
using ProductTypeOrVoid = stdx::experimental::detected_or_t<void, ProductType, T, U>;

// The type of the quotient of two types.
template <typename T, typename U>
using QuotientType = decltype(std::declval<T>() / std::declval<U>());

template <typename T, typename U>
using QuotientTypeOrVoid = stdx::experimental::detected_or_t<void, QuotientType, T, U>;
}  // namespace detail

// Implementation for `IsValidRep`.
//
// For now, we'll accept anything that isn't explicitly known to be invalid.  We may tighten this up
// later, but this seems like a reasonable starting point.
template <typename T>
struct IsValidRep : stdx::negation<detail::IsKnownInvalidRep<T>> {};

template <typename T, typename U>
struct IsProductValidRep
    : IsValidRep<detail::ResultIfNoneAreQuantityT<detail::ProductTypeOrVoid, T, U>> {};

template <typename T, typename U>
struct IsQuotientValidRep
    : IsValidRep<detail::ResultIfNoneAreQuantityT<detail::QuotientTypeOrVoid, T, U>> {};

}  // namespace au


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

namespace au {

struct Unos;

}  // namespace au

namespace au {

struct Bits;

}  // namespace au

namespace au {

struct Radians;

}  // namespace au

namespace au {

struct Candelas;

}  // namespace au

namespace au {

struct Moles;

}  // namespace au

namespace au {

struct Amperes;

}  // namespace au

namespace au {

struct Kelvins;

}  // namespace au

namespace au {

struct Grams;

}  // namespace au

namespace au {

struct Seconds;

}  // namespace au

namespace au {

struct Meters;

}  // namespace au

namespace au {

struct Minutes;

}  // namespace au

namespace au {

struct Hours;

}  // namespace au



namespace au {

// A type representing a quantity of "zero" in any units.
//
// Zero is special: it's the only number that we can meaningfully compare or assign to a Quantity of
// _any_ dimension.  Giving it a special type (and a predefined constant of that type, `ZERO`,
// defined below) lets our code be both concise and readable.
//
// For example, we can zero-initialize any arbitrary Quantity, even if it doesn't have a
// user-defined literal, and even if it's in a header file so we couldn't use the literals anyway:
//
//   struct PathPoint {
//       QuantityD<RadiansPerMeter> curvature = ZERO;
//   };
struct Zero {
    // Implicit conversion to arithmetic types.
    template <typename T, typename Enable = std::enable_if_t<std::is_arithmetic<T>::value>>
    constexpr operator T() const {
        return 0;
    }

    // Implicit conversion to chrono durations.
    template <typename Rep, typename Period>
    constexpr operator std::chrono::duration<Rep, Period>() const {
        return std::chrono::duration<Rep, Period>{0};
    }
};

// A value of Zero.
//
// This exists purely for convenience, so people don't have to call the initializer.  i.e., it lets
// us write `ZERO` instead of `Zero{}`.
static constexpr auto ZERO = Zero{};

// Addition, subtraction, and comparison of Zero are well defined.
inline constexpr Zero operator+(Zero, Zero) { return ZERO; }
inline constexpr Zero operator-(Zero, Zero) { return ZERO; }
inline constexpr bool operator==(Zero, Zero) { return true; }
inline constexpr bool operator>=(Zero, Zero) { return true; }
inline constexpr bool operator<=(Zero, Zero) { return true; }
inline constexpr bool operator!=(Zero, Zero) { return false; }
inline constexpr bool operator>(Zero, Zero) { return false; }
inline constexpr bool operator<(Zero, Zero) { return false; }

// Implementation helper for "a type where value() returns 0".
template <typename T>
struct ValueOfZero {
    static constexpr T value() { return ZERO; }
};

}  // namespace au



namespace au {
namespace detail {

//
// The possible results of a probable prime test.
//
enum class PrimeResult {
    COMPOSITE,
    PROBABLY_PRIME,
    BAD_INPUT,
};

//
// Decompose a number by factoring out all powers of 2: `n = 2^power_of_two * odd_remainder`.
//
struct NumberDecomposition {
    uint64_t power_of_two;
    uint64_t odd_remainder;
};

//
// Express any positive `n` as `(2^s * d)`, where `d` is odd.
//
// Preconditions: `n` is positive.
constexpr NumberDecomposition decompose(uint64_t n) {
    NumberDecomposition result{0u, n};
    while (result.odd_remainder % 2u == 0u) {
        result.odd_remainder /= 2u;
        ++result.power_of_two;
    }
    return result;
}

//
// Perform a Miller-Rabin primality test on `n` using base `a`.
//
// Preconditions: `n` is odd, and at least as big as `a + 2`.  Also, `2` is the smallest allowable
// value for `a`.  We will return `BAD_INPUT` if these preconditions are violated.  Otherwise, we
// will return `PROBABLY_PRIME` for all prime inputs, and also all composite inputs which are
// pseudoprime to base `a`, returning `COMPOSITE` for all other inputs (which are definitely known
// to be composite).
//
constexpr PrimeResult miller_rabin(std::size_t a, uint64_t n) {
    if (a < 2u || n < a + 2u || n % 2u == 0u) {
        return PrimeResult::BAD_INPUT;
    }

    const auto params = decompose(n - 1u);
    const auto &s = params.power_of_two;
    const auto &d = params.odd_remainder;

    uint64_t x = pow_mod(a, d, n);
    if (x == 1u) {
        return PrimeResult::PROBABLY_PRIME;
    }

    const auto minus_one = n - 1u;
    for (auto r = 0u; r < s; ++r) {
        if (x == minus_one) {
            return PrimeResult::PROBABLY_PRIME;
        }
        x = mul_mod(x, x, n);
    }
    return PrimeResult::COMPOSITE;
}

//
// Test whether the number is a perfect square.
//
constexpr bool is_perfect_square(uint64_t n) {
    if (n < 2u) {
        return true;
    }

    uint64_t prev = n / 2u;
    while (true) {
        const uint64_t curr = (prev + n / prev) / 2u;
        if (curr * curr == n) {
            return true;
        }
        if (curr >= prev) {
            return false;
        }
        prev = curr;
    }
}

constexpr uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0u) {
        const auto remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

// Map `true` onto `1`, and `false` onto `0`.
//
// The conversions `true` -> `1` and `false` -> `0` are guaranteed by the standard.  This is a
// branchless implementation, which should generally be faster.
constexpr int bool_sign(bool x) { return x - (!x); }

//
// The Jacobi symbol (a/n) is defined for odd positive `n` and any integer `a` as the product of the
// Legendre symbols (a/p) for all prime factors `p` of n.  There are several rules that make this
// easier to calculate, including:
//
//  1. (a/n) = (b/n) whenever (a % n) == (b % n).
//
//  2. (2a/n) = (a/n) if n is congruent to 1 or 7 (mod 8), and -(a/n) if n is congruent to 3 or 5.
//
//  3. (1/n) = 1 for all n.
//
//  4. (a/n) = 0 whenever a and n have a nontrivial common factor.
//
//  5. (a/n) = (n/a) * (-1)^x if a and n are both odd, positive, and coprime.  Here, x is 0 if
//     either a or n is congruent to 1 (mod 4), and 1 otherwise.
//
constexpr int jacobi_symbol_positive_numerator(uint64_t a, uint64_t n, int start) {
    int result = start;

    while (a != 0u) {
        // Handle even numbers in the "numerator".
        const uint64_t rem_8 = n % 8u;
        const int sign_for_even = bool_sign(rem_8 == 1u || rem_8 == 7u);
        while (a % 2u == 0u) {
            a /= 2u;
            result *= sign_for_even;
        }

        // `jacobi_symbol(1, n)` is `1` for all `n`.
        if (a == 1u) {
            return result;
        }

        // `jacobi_symbol(a, n)` is `0` whenever `a` and `n` have a common factor.
        if (gcd(a, n) != 1u) {
            return 0;
        }

        // At this point, `a` and `n` are odd, positive, and coprime.  We can use the reciprocity
        // relationship to "flip" them, and modular arithmetic to reduce them.

        // First, compute the sign change from the flip.
        result *= bool_sign((a % 4u == 1u) || (n % 4u == 1u));

        // Now, do the flip-and-reduce.
        const uint64_t new_a = n % a;
        n = a;
        a = new_a;
    }
    return 0;
}
constexpr int jacobi_symbol(int64_t raw_a, uint64_t n) {
    // Degenerate case: n = 1.
    if (n == 1u) {
        return 1;
    }

    // Starting conditions: transform `a` to strictly non-negative values, setting `result` to the
    // sign we pick up from this operation (if any).
    int result = bool_sign((raw_a >= 0) || (n % 4u == 1u));
    auto a = static_cast<uint64_t>(raw_a * bool_sign(raw_a >= 0)) % n;

    // Delegate to an implementation which can only handle positive numbers.
    return jacobi_symbol_positive_numerator(a, n, result);
}

// The "D" parameter in the Strong Lucas probable prime test.
//
// Default construction produces the first value to try according to Selfridge's parameter
// selection.  Calling `increment()` on this will successively produce the next parameter to try.
struct LucasDParameter {
    uint64_t mag = 5u;
    bool is_positive = true;

    friend constexpr int as_int(const LucasDParameter &D) {
        return bool_sign(D.is_positive) * static_cast<int>(D.mag);
    }
    friend constexpr void increment(LucasDParameter &D) {
        D.mag += 2u;
        D.is_positive = !D.is_positive;
    }
};

//
// The first `D` in the infinite sequence {5, -7, 9, -11, ...} whose Jacobi symbol is (-1) is the
// `D` we want to use for the Strong Lucas Probable Prime test.
//
// Requires that `n` is *not* a perfect square.
//
constexpr LucasDParameter find_first_D_with_jacobi_symbol_neg_one(uint64_t n) {
    LucasDParameter D{};
    while (jacobi_symbol(as_int(D), n) != -1) {
        increment(D);
    }
    return D;
}

//
// Elements of the Lucas sequence.
//
// The default values give the first element (i.e., k=1) of the sequence.
//
struct LucasSequenceElement {
    uint64_t U = 1u;
    uint64_t V = 1u;
};

// Produce the Lucas element whose index is twice the input element's index.
constexpr LucasSequenceElement double_strong_lucas_index(const LucasSequenceElement &element,
                                                         uint64_t n,
                                                         LucasDParameter D) {
    const auto &U = element.U;
    const auto &V = element.V;

    uint64_t V_squared = mul_mod(V, V, n);
    uint64_t D_U_squared = mul_mod(D.mag, mul_mod(U, U, n), n);
    uint64_t V2 =
        D.is_positive ? add_mod(V_squared, D_U_squared, n) : sub_mod(V_squared, D_U_squared, n);
    V2 = half_mod_odd(V2, n);

    return LucasSequenceElement{
        mul_mod(U, V, n),
        V2,
    };
}

// Find the next element in the Lucas sequence, using parameters for strong Lucas probable primes.
constexpr LucasSequenceElement increment_strong_lucas_index(const LucasSequenceElement &element,
                                                            uint64_t n,
                                                            LucasDParameter D) {
    const auto &U = element.U;
    const auto &V = element.V;

    auto U2 = half_mod_odd(add_mod(U, V, n), n);

    const auto D_U = mul_mod(D.mag, U, n);
    auto V2 = D.is_positive ? add_mod(V, D_U, n) : sub_mod(V, D_U, n);
    V2 = half_mod_odd(V2, n);

    return LucasSequenceElement{U2, V2};
}

// Compute the strong Lucas sequence element at index `i`.
constexpr LucasSequenceElement find_strong_lucas_element(uint64_t i,
                                                         uint64_t n,
                                                         LucasDParameter D) {
    LucasSequenceElement element{};

    bool bits[64] = {};
    std::size_t n_bits = 0u;
    while (i > 1u) {
        bits[n_bits++] = (i & 1u);
        i >>= 1;
    }

    for (std::size_t j = n_bits; j > 0u; --j) {
        element = double_strong_lucas_index(element, n, D);
        if (bits[j - 1u]) {
            element = increment_strong_lucas_index(element, n, D);
        }
    }

    return element;
}

//
// Perform a strong Lucas primality test on `n`.
//
constexpr PrimeResult strong_lucas(uint64_t n) {
    if (n < 2u || n % 2u == 0u) {
        return PrimeResult::BAD_INPUT;
    }

    if (is_perfect_square(n)) {
        return PrimeResult::COMPOSITE;
    }

    const auto D = find_first_D_with_jacobi_symbol_neg_one(n);

    const auto params = decompose(n + 1u);
    const auto &s = params.power_of_two;
    const auto &d = params.odd_remainder;

    auto element = find_strong_lucas_element(d, n, D);
    if (element.U == 0u) {
        return PrimeResult::PROBABLY_PRIME;
    }

    for (std::size_t i = 0u; i < s; ++i) {
        if (element.V == 0u) {
            return PrimeResult::PROBABLY_PRIME;
        }
        element = double_strong_lucas_index(element, n, D);
    }

    return PrimeResult::COMPOSITE;
}

//
// Perform the Baillie-PSW test for primality.
//
// Returns `BAD_INPUT` for any number less than 2, `COMPOSITE` for any larger number that is _known_
// to be prime, and `PROBABLY_PRIME` for any larger number that is deemed "probably prime", which
// includes all prime numbers.
//
// Actually, the Baillie-PSW test is known to be completely accurate for all 64-bit numbers;
// therefore, since our input type is `uint64_t`, the output will be `PROBABLY_PRIME` if and only if
// the input is prime.
//
constexpr PrimeResult baillie_psw(uint64_t n) {
    if (n < 2u) {
        return PrimeResult::BAD_INPUT;
    }
    if (n < 4u) {
        return PrimeResult::PROBABLY_PRIME;
    }
    if (n % 2u == 0u) {
        return PrimeResult::COMPOSITE;
    }

    if (miller_rabin(2u, n) == PrimeResult::COMPOSITE) {
        return PrimeResult::COMPOSITE;
    }

    return strong_lucas(n);
}

}  // namespace detail
}  // namespace au



// Products of base powers are the foundation of au.  We use them for:
//
//   - The Dimension of a Unit.
//   - The Magnitude of a Unit.
//   - Making *compound* Units (products of powers, e.g., m^1 * s^(-2)).

namespace au {

// A base type B raised to an integer exponent N.
template <typename B, std::intmax_t N>
struct Pow;

// A base type B raised to a rational exponent (N/D).
template <typename B, std::intmax_t N, std::intmax_t D>
struct RatioPow;

// Type trait for the "base" of a type, interpreted as a base power.
//
// Any type can act as a base, with an implicit power of 1.  `Pow<B, N>` can represent integer
// powers of a base type `B`, and `RatioPow<B, N, D>` can represent rational powers of `B` (where
// the power is `(N/D)`).
template <typename T>
struct Base : stdx::type_identity<T> {};
template <typename T>
using BaseT = typename Base<T>::type;

// Type trait for the rational exponent of a type, interpreted as a base power.
template <typename T>
struct Exp : stdx::type_identity<std::ratio<1>> {};
template <typename T>
using ExpT = typename Exp<T>::type;

// Type trait for treating an arbitrary type as a given type of pack.
//
// This should be the identity for anything that is already a pack of this type, and otherwise
// should wrap it in this type of pack.
template <template <class... Ts> class Pack, typename T>
struct AsPack : stdx::type_identity<Pack<T>> {};
template <template <class... Ts> class Pack, typename T>
using AsPackT = typename AsPack<Pack, T>::type;

// Type trait to remove a Pack enclosing a single item.
//
// Defined only if T is Pack<Ts...> for some typelist.  Always the identity, unless sizeof...(Ts) is
// exactly 1, in which case, it returns the (sole) element.
template <template <class... Ts> class Pack, typename T>
struct UnpackIfSolo;
template <template <class... Ts> class Pack, typename T>
using UnpackIfSoloT = typename UnpackIfSolo<Pack, T>::type;

// Trait to define whether two types are in order, based on the total ordering for some pack.
//
// Each pack should individually define its desired total ordering.  For these implementations,
// prefer to inherit from LexicographicTotalOrdering (below), because it guards against the most
// common way to fail to achieve a strict total ordering (namely, by having two types which are not
// identical nevertheless compare equal).
template <template <class...> class Pack, typename A, typename B>
struct InOrderFor;

// A strict total ordering which combines strict partial orderings serially, using the first which
// distinguishes A and B.
template <typename A, typename B, template <class, class> class... Orderings>
struct LexicographicTotalOrdering;

// A (somewhat arbitrary) total ordering on _packs themselves_.
//
// Built on top of the total ordering for the _bases_ of the packs.
template <typename T, typename U>
struct InStandardPackOrder;

// Insert an element in a list, using the ordering for a specific (possibly different) pack.
//
// A precondition is that the list must already be sorted by the given ordering.
template <template <class...> class PackForOrdering, typename T, typename ListT>
struct InsertUsingOrderingForImpl;
template <template <class...> class PackForOrdering, typename T, typename ListT>
using InsertUsingOrderingFor = typename InsertUsingOrderingForImpl<PackForOrdering, T, ListT>::type;

// Sort a type list using the ordering for a specific (possibly different) pack.
template <template <class...> class PackForOrdering, typename ListT>
struct SortAsImpl;
template <template <class...> class PackForOrdering, typename ListT>
using SortAs = typename SortAsImpl<PackForOrdering, ListT>::type;

// Make a List of deduplicated, sorted types.
//
// The result will always be List<...>, and the elements will be sorted according to the total
// ordering for List, with duplicates removed.  It will be "flattened" in that any elements which
// are already `List<Ts...>` will be effectively replaced by `Ts...`.
//
// A precondition for `FlatDedupedTypeListT` is that any inputs which are already of type
// `List<...>`, respect the _ordering_ for `List`, with no duplicates.  Otherwise, behaviour is
// undefined.  (This precondition will automatically be satisfied if *every* instance of `List<...>`
// arises as the result of a call to `FlatDedupedTypeListT<...>`.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeList;
template <template <class...> class List, typename... Ts>
using FlatDedupedTypeListT = typename FlatDedupedTypeList<List, AsPackT<List, Ts>...>::type;

namespace detail {
// Express a base power in its simplest form (base alone if power is 1, or Pow if exp is integral).
template <typename T>
struct SimplifyBasePowers;
template <typename T>
using SimplifyBasePowersT = typename SimplifyBasePowers<T>::type;
}  // namespace detail

// Compute the product between two power packs.
template <template <class...> class Pack, typename... Ts>
struct PackProduct;
template <template <class...> class Pack, typename... Ts>
using PackProductT = detail::SimplifyBasePowersT<typename PackProduct<Pack, Ts...>::type>;

// Compute a rational power of a pack.
template <template <class...> class Pack, typename T, typename E>
struct PackPower;
template <template <class...> class Pack,
          typename T,
          std::intmax_t ExpNum,
          std::intmax_t ExpDen = 1>
using PackPowerT =
    detail::SimplifyBasePowersT<typename PackPower<Pack, T, std::ratio<ExpNum, ExpDen>>::type>;

// Compute the inverse of a power pack.
template <template <class...> class Pack, typename T>
using PackInverseT = PackPowerT<Pack, T, -1>;

// Compute the quotient of two power packs.
template <template <class...> class Pack, typename T, typename U>
using PackQuotientT = PackProductT<Pack, T, PackInverseT<Pack, U>>;

namespace detail {
// Pull out all of the elements in a Pack whose exponents are positive.
template <typename T>
struct NumeratorPart;
template <typename T>
using NumeratorPartT = typename NumeratorPart<T>::type;

// Pull out all of the elements in a Pack whose exponents are negative.
template <typename T>
struct DenominatorPart;
template <typename T>
using DenominatorPartT = typename DenominatorPart<T>::type;
}  // namespace detail

// A validator for a pack of Base Powers.
//
// `IsValidPack<Pack, T>::value` is `true` iff `T` is an instance of the variadic `Pack<...>`, and
// its parameters fulfill all of the appropriate type traits, namely:
//
// - `AreBasesInOrder<Pack, T>`
template <template <class...> class Pack, typename T>
struct IsValidPack;

// Assuming that `T` is an instance of `Pack<BPs...>`, validates that every consecutive pair from
// `BaseT<BPs>...` satisfies the strict total ordering `InOrderFor<Pack, ...>` for `Pack`.
template <template <class...> class Pack, typename T>
struct AreBasesInOrder;

// Assuming that `T` is an instance of `Pack<BPs...>`, validates that every consecutive pair from
// `BPs...` satisfies the strict total ordering `InOrderFor<Pack, ...>` for `Pack`.
//
// This is very similar to AreBasesInOrder, but is intended for packs that _don't_ represent
// products-of-powers.
template <template <class...> class Pack, typename T>
struct AreElementsInOrder;

// Assuming `T` is an instance of `Pack<BPs...>`, validates that `Exp<BPs>...` is always nonzero.
template <template <class...> class Pack, typename T>
struct AreAllPowersNonzero;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

// These forward declarations and traits go here to enable us to treat Pow and RatioPow (below) as
// full-fledged "Units".  A "Unit" is any type U where `DimT<U>` gives a valid Dimension, and
// `MagT<U>` gives a valid Magnitude.  Even though we can't define Dimension and Magnitude precisely
// in this file, we'll take advantage of the fact that we know they're going to be parameter packs.

template <typename... BPs>
struct Dimension;

template <typename... BPs>
struct Magnitude;

namespace detail {

// The default dimension, `DimT<U>`, of a type `U`, is the `::Dim` typedef (or `void` if none).
//
// Users can customize by specializing `DimImpl<U>` and setting the `type` member variable.
template <typename U>
using DimMemberT = typename U::Dim;
template <typename U>
struct DimImpl : stdx::experimental::detected_or<void, DimMemberT, U> {};
template <typename U>
using DimT = typename DimImpl<U>::type;

// The default magnitude, `MagT<U>`, of a type `U`, is the `::Mag` typedef (or `void` if none).
//
// Users can customize by specializing `MagImpl<U>` and setting the `type` member variable.
template <typename U>
using MagMemberT = typename U::Mag;
template <typename U>
struct MagImpl : stdx::experimental::detected_or<void, MagMemberT, U> {};
template <typename U>
using MagT = typename MagImpl<U>::type;

}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `Pow` implementation.

template <typename B, std::intmax_t N>
struct Pow {
    // TODO(#40): Clean up relationship between Dim/Mag and Pow, if compile times are OK.
    using Dim = PackPowerT<Dimension, AsPackT<Dimension, detail::DimT<B>>, N>;
    using Mag = PackPowerT<Magnitude, AsPackT<Magnitude, detail::MagT<B>>, N>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `RatioPow` implementation.

// A base type B raised to a rational exponent (N/D).
template <typename B, std::intmax_t N, std::intmax_t D>
struct RatioPow {
    // TODO(#40): Clean up relationship between Dim/Mag and RatioPow, if compile times are OK.
    using Dim = PackPowerT<Dimension, AsPackT<Dimension, detail::DimT<B>>, N, D>;
    using Mag = PackPowerT<Magnitude, AsPackT<Magnitude, detail::MagT<B>>, N, D>;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `BaseT` implementation.

template <typename T, std::intmax_t N>
struct Base<Pow<T, N>> : stdx::type_identity<T> {};

template <typename T, std::intmax_t N, std::intmax_t D>
struct Base<RatioPow<T, N, D>> : stdx::type_identity<T> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ExpT` implementation.

template <typename T, std::intmax_t N>
struct Exp<Pow<T, N>> : stdx::type_identity<std::ratio<N>> {};

template <typename T, std::intmax_t N, std::intmax_t D>
struct Exp<RatioPow<T, N, D>> : stdx::type_identity<std::ratio<N, D>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AsPackT` implementation.

template <template <class... Ts> class Pack, typename... Ts>
struct AsPack<Pack, Pack<Ts...>> : stdx::type_identity<Pack<Ts...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnpackIfSoloT` implementation.

// Null pack case: do not unpack.
template <template <class... Ts> class Pack>
struct UnpackIfSolo<Pack, Pack<>> : stdx::type_identity<Pack<>> {};

// Non-null pack case: unpack only if there is nothing after the head element.
template <template <class... Ts> class Pack, typename T, typename... Ts>
struct UnpackIfSolo<Pack, Pack<T, Ts...>>
    : std::conditional<(sizeof...(Ts) == 0u), T, Pack<T, Ts...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `LexicographicTotalOrdering` implementation.

// Base case: if there is no ordering, then the inputs are not in order.
template <typename A, typename B>
struct LexicographicTotalOrdering<A, B> : std::false_type {
    // LexicographicTotalOrdering is for strict total orderings only.  If two types compare equal,
    // then they must be the same type; otherwise, we have not created a strict total ordering among
    // all types being used in some pack.
    static_assert(std::is_same<A, B>::value,
                  "Broken strict total ordering: distinct input types compare equal");
};

// Recursive case.
template <typename A,
          typename B,
          template <class, class>
          class PrimaryOrdering,
          template <class, class>
          class... Tiebreakers>
struct LexicographicTotalOrdering<A, B, PrimaryOrdering, Tiebreakers...> :

    // Short circuit for when the inputs are the same.
    //
    // This can prevent us from instantiating a tiebreaker which doesn't exist for a given type.
    std::conditional_t<
        (std::is_same<A, B>::value),
        std::false_type,

        // If A and B are properly ordered by the primary criterion, they are definitely ordered.
        std::conditional_t<(PrimaryOrdering<A, B>::value),
                           std::true_type,

                           // If B and A are properly ordered by the primary criterion, then A and B
                           // are definitely _not_ properly ordered.
                           std::conditional_t<(PrimaryOrdering<B, A>::value),
                                              std::false_type,

                                              // Fall back to the remaining orderings as
                                              // tiebreakers.
                                              LexicographicTotalOrdering<A, B, Tiebreakers...>>>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `InStandardPackOrder` implementation.

namespace detail {
// Helper: check that the lead bases are in order.
template <typename T, typename U>
struct LeadBasesInOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct LeadBasesInOrder<P<H1, T1...>, P<H2, T2...>> : InOrderFor<P, BaseT<H1>, BaseT<H2>> {};

// Helper: check that the lead exponents are in order.
template <typename T, typename U>
struct LeadExpsInOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct LeadExpsInOrder<P<H1, T1...>, P<H2, T2...>>
    : stdx::bool_constant<(std::ratio_subtract<ExpT<H1>, ExpT<H2>>::num < 0)> {};

// Helper: apply InStandardPackOrder to tails.
template <typename T, typename U>
struct TailsInStandardPackOrder;
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct TailsInStandardPackOrder<P<H1, T1...>, P<H2, T2...>>
    : InStandardPackOrder<P<T1...>, P<T2...>> {};
}  // namespace detail

// Base case: left pack is null.
template <template <class...> class P, typename... Ts>
struct InStandardPackOrder<P<>, P<Ts...>> : stdx::bool_constant<(sizeof...(Ts) > 0)> {};

// Base case: right pack (only) is null.
template <template <class...> class P, typename H, typename... T>
struct InStandardPackOrder<P<H, T...>, P<>> : std::false_type {};

// Recursive case: try ordering the heads, and fall back to the tails.
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct InStandardPackOrder<P<H1, T1...>, P<H2, T2...>>
    : LexicographicTotalOrdering<P<H1, T1...>,
                                 P<H2, T2...>,
                                 detail::LeadBasesInOrder,
                                 detail::LeadExpsInOrder,
                                 detail::TailsInStandardPackOrder> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `InsertUsingOrderingFor` implementation.

// Base case.
template <template <class...> class PackForOrdering, typename T, template <class...> class Pack>
struct InsertUsingOrderingForImpl<PackForOrdering, T, Pack<>> : stdx::type_identity<Pack<T>> {};

// Recursive case: simply prepend if it's already in order, or else recurse past the first element,
// and then prepend the old first element.
template <template <class...> class PackForOrdering,
          typename T,
          template <class...>
          class Pack,
          typename U,
          typename... Us>
struct InsertUsingOrderingForImpl<PackForOrdering, T, Pack<U, Us...>>
    : std::conditional<
          InOrderFor<PackForOrdering, T, U>::value,
          Pack<T, U, Us...>,
          detail::PrependT<InsertUsingOrderingFor<PackForOrdering, T, Pack<Us...>>, U>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `SortAs` implementation.

// Base case.
template <template <class...> class PackForOrdering, template <class...> class Pack>
struct SortAsImpl<PackForOrdering, Pack<>> : stdx::type_identity<Pack<>> {};

// Recursive case.
template <template <class...> class PackForOrdering,
          template <class...>
          class Pack,
          typename T,
          typename... Ts>
struct SortAsImpl<PackForOrdering, Pack<T, Ts...>>
    : stdx::type_identity<
          InsertUsingOrderingFor<PackForOrdering, T, SortAs<PackForOrdering, Pack<Ts...>>>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `FlatDedupedTypeListT` implementation.

// 1-ary Base case: a list with a single element is already done.
//
// (We explicitly assumed that any `List<...>` inputs would already be in sorted order.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeList<List, List<Ts...>> : stdx::type_identity<List<Ts...>> {};

// 2-ary base case: if we exhaust elements in the second list, the first list is the answer.
//
// (Again: this relies on the explicit assumption that any `List<...>` inputs are already in order.)
template <template <class...> class List, typename... Ts>
struct FlatDedupedTypeList<List, List<Ts...>, List<>> : stdx::type_identity<List<Ts...>> {};

// 2-ary recursive case, single-element head.
//
// This use case also serves as the core "insertion logic", inserting `T` into the proper place
// within `List<H, Ts...>`.
template <template <class...> class List, typename T, typename H, typename... Ts>
struct FlatDedupedTypeList<List, List<T>, List<H, Ts...>> :

    // If the candidate element exactly equals the head, disregard it (de-dupe!).
    std::conditional<
        (std::is_same<T, H>::value),
        List<H, Ts...>,

        // If the candidate element is strictly before the head, prepend it.
        std::conditional_t<(InOrderFor<List, T, H>::value),
                           List<T, H, Ts...>,

                           // If we're here, we know the candidate comes after the head.  So, try
                           // inserting it (recursively) in the tail, and then prepend the old Head
                           // (because we know it comes first).
                           detail::PrependT<FlatDedupedTypeListT<List, List<T>, List<Ts...>>, H>>> {
};

// 2-ary recursive case, multi-element head: insert head of second element, and recurse.
template <template <class...> class List,
          typename H1,
          typename N1,
          typename... T1,
          typename H2,
          typename... T2>
struct FlatDedupedTypeList<List, List<H1, N1, T1...>, List<H2, T2...>>
    : FlatDedupedTypeList<List,
                          // Put H2 first so we can use single-element-head case from above.
                          FlatDedupedTypeListT<List, List<H2>, List<H1, N1, T1...>>,
                          List<T2...>> {};

// N-ary case, multi-element head: peel off tail-of-head, and recurse.
//
// Note that this also handles the 2-ary case where the head list has more than one element.
template <template <class...> class List, typename L1, typename L2, typename L3, typename... Ls>
struct FlatDedupedTypeList<List, L1, L2, L3, Ls...>
    : FlatDedupedTypeList<List, FlatDedupedTypeListT<List, L1, L2>, L3, Ls...> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `PackProductT` implementation.

// 0-ary case:
template <template <class...> class Pack>
struct PackProduct<Pack> : stdx::type_identity<Pack<>> {};

// 1-ary case:
template <template <class...> class Pack, typename... Ts>
struct PackProduct<Pack, Pack<Ts...>> : stdx::type_identity<Pack<Ts...>> {};

// 2-ary Base case: two null packs.
template <template <class...> class Pack>
struct PackProduct<Pack, Pack<>, Pack<>> : stdx::type_identity<Pack<>> {};

// 2-ary Base case: only left pack is null.
template <template <class...> class Pack, typename T, typename... Ts>
struct PackProduct<Pack, Pack<>, Pack<T, Ts...>> : stdx::type_identity<Pack<T, Ts...>> {};

// 2-ary Base case: only right pack is null.
template <template <class...> class Pack, typename T, typename... Ts>
struct PackProduct<Pack, Pack<T, Ts...>, Pack<>> : stdx::type_identity<Pack<T, Ts...>> {};

namespace detail {
template <typename B, typename E1, typename E2>
struct ComputeRationalPower {
    using E = std::ratio_add<E1, E2>;
    using type = RatioPow<B, E::num, E::den>;
};
template <typename B, typename E1, typename E2>
using ComputeRationalPowerT = typename ComputeRationalPower<B, E1, E2>::type;
}  // namespace detail

// 2-ary Recursive case: two non-null packs.
template <template <class...> class P, typename H1, typename... T1, typename H2, typename... T2>
struct PackProduct<P, P<H1, T1...>, P<H2, T2...>> :

    // If the bases for H1 and H2 are in-order, prepend H1 to the product of the remainder.
    std::conditional<
        (InOrderFor<P, BaseT<H1>, BaseT<H2>>::value),
        detail::PrependT<PackProductT<P, P<T1...>, P<H2, T2...>>, H1>,

        // If the bases for H2 and H1 are in-order, prepend H2 to the product of the remainder.
        std::conditional_t<
            (InOrderFor<P, BaseT<H2>, BaseT<H1>>::value),
            detail::PrependT<PackProductT<P, P<T2...>, P<H1, T1...>>, H2>,

            // If the bases have the same position, assume they really _are_ the same (because
            // InOrderFor will verify this if it uses LexicographicTotalOrdering), and add the
            // exponents.  (If the exponents add to zero, omit the term.)
            std::conditional_t<
                (std::ratio_add<ExpT<H1>, ExpT<H2>>::num == 0),
                PackProductT<P, P<T1...>, P<T2...>>,
                detail::PrependT<PackProductT<P, P<T2...>, P<T1...>>,
                                 detail::ComputeRationalPowerT<BaseT<H1>, ExpT<H1>, ExpT<H2>>>>>> {
};

// N-ary case, N > 2: recurse.
template <template <class...> class P,
          typename... T1s,
          typename... T2s,
          typename... T3s,
          typename... Ps>
struct PackProduct<P, P<T1s...>, P<T2s...>, P<T3s...>, Ps...>
    : PackProduct<P, P<T1s...>, PackProductT<P, P<T2s...>, P<T3s...>, Ps...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `PackPowerT` implementation.

namespace detail {
template <typename T, typename E>
using MultiplyExpFor = std::ratio_multiply<ExpT<T>, E>;
}

template <template <class...> class P, typename... Ts, typename E>
struct PackPower<P, P<Ts...>, E>
    : std::conditional<(E::num == 0),
                       P<>,
                       P<RatioPow<BaseT<Ts>,
                                  detail::MultiplyExpFor<Ts, E>::num,
                                  detail::MultiplyExpFor<Ts, E>::den>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `IsValidPack` implementation.

namespace detail {
template <template <class...> class Pack, typename T>
struct IsPackOf : std::false_type {};

template <template <class...> class Pack, typename... Ts>
struct IsPackOf<Pack, Pack<Ts...>> : std::true_type {};
}  // namespace detail

template <template <class...> class Pack, typename T>
struct IsValidPack : stdx::conjunction<detail::IsPackOf<Pack, T>,
                                       AreBasesInOrder<Pack, T>,
                                       AreAllPowersNonzero<Pack, T>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreElementsInOrder` implementation.

template <template <class...> class Pack>
struct AreElementsInOrder<Pack, Pack<>> : std::true_type {};

template <template <class...> class Pack, typename T>
struct AreElementsInOrder<Pack, Pack<T>> : std::true_type {};

template <template <class...> class Pack, typename T1, typename T2, typename... Ts>
struct AreElementsInOrder<Pack, Pack<T1, T2, Ts...>>
    : stdx::conjunction<InOrderFor<Pack, T1, T2>, AreElementsInOrder<Pack, Pack<T2, Ts...>>> {};

namespace detail {

constexpr bool all_true() { return true; }

template <typename... Predicates>
constexpr bool all_true(Predicates &&...values) {
    // The reason we bother to make an array is so that we can iterate over it.
    const bool value_array[] = {values...};

    for (auto i = 0u; i < sizeof...(Predicates); ++i) {
        if (!value_array[i]) {
            return false;
        }
    }

    return true;
}
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreBasesInOrder` implementation.

template <template <class...> class Pack, typename... Ts>
struct AreBasesInOrder<Pack, Pack<Ts...>> : AreElementsInOrder<Pack, Pack<BaseT<Ts>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreAllPowersNonzero` implementation.

template <template <class...> class Pack, typename... Ts>
struct AreAllPowersNonzero<Pack, Pack<Ts...>>
    : stdx::bool_constant<detail::all_true((ExpT<Ts>::num != 0)...)> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `SimplifyBasePowersT` implementation.

namespace detail {
// To simplify an individual base power, by default, do nothing.
template <typename T>
struct SimplifyBasePower : stdx::type_identity<T> {};
template <typename T>
using SimplifyBasePowerT = typename SimplifyBasePower<T>::type;

// To simplify an integer power of a base, give the base alone if the exponent is 1; otherwise, do
// nothing.
template <typename B, std::intmax_t N>
struct SimplifyBasePower<Pow<B, N>> : std::conditional<(N == 1), B, Pow<B, N>> {};

// To simplify a rational power of a base, simplify the integer power if the exponent is an integer
// (i.e., if its denominator is 1); else, do nothing.
template <typename B, std::intmax_t N, std::intmax_t D>
struct SimplifyBasePower<RatioPow<B, N, D>>
    : std::conditional<(D == 1), SimplifyBasePowerT<Pow<B, N>>, RatioPow<B, N, D>> {};

// To simplify the base powers in a pack, give the pack with each base power simplified.
template <template <class...> class Pack, typename... BPs>
struct SimplifyBasePowers<Pack<BPs...>> : stdx::type_identity<Pack<SimplifyBasePowerT<BPs>...>> {};
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `NumeratorPartT` and `DenominatorPartT` implementation.

namespace detail {
template <typename BP>
struct IsInNumerator : stdx::bool_constant<(ExpT<BP>::num > 0)> {};

template <typename BP>
struct IsInDenominator : stdx::bool_constant<(ExpT<BP>::num < 0)> {};

// A generic helper for both numerator and denominator.
template <template <class> class Pred, typename T>
struct PullOutMatchingPowers;

// Base case: empty pack.
template <template <class> class Pred, template <class...> class Pack>
struct PullOutMatchingPowers<Pred, Pack<>> : stdx::type_identity<Pack<>> {};

// Recursive case: non-empty pack.
template <template <class> class Pred, template <class...> class Pack, typename H, typename... Ts>
struct PullOutMatchingPowers<Pred, Pack<H, Ts...>>
    : std::conditional<(Pred<H>::value),
                       detail::PrependT<typename PullOutMatchingPowers<Pred, Pack<Ts...>>::type, H>,
                       typename PullOutMatchingPowers<Pred, Pack<Ts...>>::type> {};

template <typename T>
struct NumeratorPart : PullOutMatchingPowers<IsInNumerator, T> {};

template <template <class...> class Pack, typename... Ts>
struct DenominatorPart<Pack<Ts...>>
    : stdx::type_identity<
          PackInverseT<Pack, typename PullOutMatchingPowers<IsInDenominator, Pack<Ts...>>::type>> {
};

}  // namespace detail

}  // namespace au


namespace au {

template <typename... BPs>
struct Dimension {
    // Having separate `static_assert` instances for the individual conditions produces more
    // readable errors if we fail.
    static_assert(AreAllPowersNonzero<Dimension, Dimension<BPs...>>::value,
                  "All powers must be nonzero");
    static_assert(AreBasesInOrder<Dimension, Dimension<BPs...>>::value,
                  "Bases must be listed in ascending order");

    // We also want to use the "full" validity check.  This should be equivalent to the above
    // conditions, but if we add more conditions later, we want them to get picked up here
    // automatically.
    static_assert(IsValidPack<Dimension, Dimension<BPs...>>::value, "Ill-formed Dimension");
};

// Define readable operations for product, quotient, power, inverse on Dimensions.
template <typename... BPs>
using DimProductT = PackProductT<Dimension, BPs...>;
template <typename T, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using DimPowerT = PackPowerT<Dimension, T, ExpNum, ExpDen>;
template <typename T, typename U>
using DimQuotientT = PackQuotientT<Dimension, T, U>;
template <typename T>
using DimInverseT = PackInverseT<Dimension, T>;

template <typename... BP1s, typename... BP2s>
constexpr auto operator*(Dimension<BP1s...>, Dimension<BP2s...>) {
    return DimProductT<Dimension<BP1s...>, Dimension<BP2s...>>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator/(Dimension<BP1s...>, Dimension<BP2s...>) {
    return DimQuotientT<Dimension<BP1s...>, Dimension<BP2s...>>{};
}

// Roots and powers for Dimension instances.
template <std::intmax_t N, typename... BPs>
constexpr DimPowerT<Dimension<BPs...>, N> pow(Dimension<BPs...>) {
    return {};
}
template <std::intmax_t N, typename... BPs>
constexpr DimPowerT<Dimension<BPs...>, 1, N> root(Dimension<BPs...>) {
    return {};
}

template <typename... Dims>
struct CommonDimension;
template <typename... Dims>
using CommonDimensionT = typename CommonDimension<Dims...>::type;

template <typename... BaseDims>
struct CommonDimension<Dimension<BaseDims...>> : stdx::type_identity<Dimension<BaseDims...>> {};
template <typename Head, typename... Tail>
struct CommonDimension<Head, Tail...> : CommonDimension<Tail...> {
    static_assert(std::is_same<Head, CommonDimensionT<Tail...>>::value,
                  "Common dimension only defined when all dimensions are identical");
};

namespace base_dim {

template <int64_t I>
struct BaseDimension {
    static constexpr int64_t base_dim_index = I;
};
template <int64_t I>
constexpr int64_t BaseDimension<I>::base_dim_index;

template <typename T, typename U>
struct OrderByBaseDimIndex : stdx::bool_constant<(T::base_dim_index < U::base_dim_index)> {};

struct Length : BaseDimension<-99> {};
struct Mass : BaseDimension<-98> {};
struct Time : BaseDimension<-97> {};
struct Current : BaseDimension<-96> {};
struct Temperature : BaseDimension<-95> {};
struct Angle : BaseDimension<-94> {};
struct Information : BaseDimension<-93> {};
struct AmountOfSubstance : BaseDimension<-92> {};
struct LuminousIntensity : BaseDimension<-91> {};

}  // namespace base_dim

template <typename A, typename B>
struct InOrderFor<Dimension, A, B>
    : LexicographicTotalOrdering<A, B, base_dim::OrderByBaseDimIndex> {};

// The types we want to expose to the rest of the library internals are the full-fledged Dimensions,
// not the Base Dimensions, because Dimensions are easier to work with (we can take products,
// quotients, powers, etc.).
using Length = Dimension<base_dim::Length>;
using Mass = Dimension<base_dim::Mass>;
using Time = Dimension<base_dim::Time>;
using Current = Dimension<base_dim::Current>;
using Temperature = Dimension<base_dim::Temperature>;
using Angle = Dimension<base_dim::Angle>;
using Information = Dimension<base_dim::Information>;
using AmountOfSubstance = Dimension<base_dim::AmountOfSubstance>;
using LuminousIntensity = Dimension<base_dim::LuminousIntensity>;

}  // namespace au



namespace au {
namespace detail {

// Check whether a number is prime.
constexpr bool is_prime(std::uintmax_t n) {
    static_assert(sizeof(std::uintmax_t) <= sizeof(std::uint64_t),
                  "Baillie-PSW only strictly guaranteed for 64-bit numbers");

    return baillie_psw(n) == PrimeResult::PROBABLY_PRIME;
}

// Compute the next step for Pollard's rho algorithm factoring `n`, with parameter `t`.
constexpr std::uintmax_t x_squared_plus_t_mod_n(std::uintmax_t x,
                                                std::uintmax_t t,
                                                std::uintmax_t n) {
    return add_mod(mul_mod(x, x, n), t, n);
}

constexpr std::uintmax_t absolute_diff(std::uintmax_t a, std::uintmax_t b) {
    return a > b ? a - b : b - a;
}

// Pollard's rho algorithm, using Brent's cycle detection method.
//
// Precondition: `n` is known to be composite.
constexpr std::uintmax_t find_pollard_rho_factor(std::uintmax_t n) {
    // The outer loop tries separate _parameterizations_ of Pollard's rho.  We try a finite number
    // of them just to guarantee that we terminate.  But in practice, the vast overwhelming majority
    // will succeed on the first iteration, and we don't expect that any will _ever_ come anywhere
    // _near_ to hitting this limit.
    for (std::uintmax_t t = 1u; t < n / 2u; ++t) {
        std::size_t max_cycle_length = 1u;
        std::size_t cycle_length = 1u;
        std::uintmax_t tortoise = 2u;
        std::uintmax_t hare = x_squared_plus_t_mod_n(tortoise, t, n);

        std::uintmax_t factor = gcd(n, absolute_diff(tortoise, hare));
        while (factor == 1u) {
            if (max_cycle_length == cycle_length) {
                tortoise = hare;
                max_cycle_length *= 2u;
                cycle_length = 0u;
            }
            hare = x_squared_plus_t_mod_n(hare, t, n);
            ++cycle_length;
            factor = gcd(n, absolute_diff(tortoise, hare));
        }
        if (factor < n) {
            return factor;
        }
    }
    // Failure case: we think this should be unreachable (in practice) with any composite `n`.
    return n;
}

template <typename T = void>
struct FirstPrimesImpl {
    static constexpr std::array<uint16_t, 100u> values = {
        2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,  59,
        61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131, 137, 139,
        149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
        239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337,
        347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433, 439,
        443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521, 523, 541};
};
template <typename T>
constexpr std::array<uint16_t, 100u> FirstPrimesImpl<T>::values;
using FirstPrimes = FirstPrimesImpl<>;

// Find the smallest factor which divides n.
//
// Undefined unless (n > 1).
constexpr std::uintmax_t find_prime_factor(std::uintmax_t n) {
    // First, do trial division against the first N primes.
    //
    // Note that range-for isn't supported until C++17, so we need to use an index.
    for (auto i = 0u; i < FirstPrimes::values.size(); ++i) {
        const std::uintmax_t p = FirstPrimes::values[i];

        if (n % p == 0u) {
            return p;
        }

        if (p * p > n) {
            return n;
        }
    }

    // If we got this far, and haven't found a factor nor terminated, do a fast primality check.
    if (is_prime(n)) {
        return n;
    }

    auto factor = find_pollard_rho_factor(n);
    while (!is_prime(factor)) {
        factor = find_pollard_rho_factor(factor);
    }
    return factor;
}

// Find the largest power of `factor` which divides `n`.
//
// Undefined unless n > 0, and factor > 1.
constexpr std::uintmax_t multiplicity(std::uintmax_t factor, std::uintmax_t n) {
    std::uintmax_t m = 0u;
    while (n % factor == 0u) {
        ++m;
        n /= factor;
    }
    return m;
}

template <typename T>
constexpr T square(T n) {
    return n * n;
}

// Raise a base to an integer power.
//
// Undefined behavior if base^exp overflows T.
template <typename T>
constexpr T int_pow(T base, std::uintmax_t exp) {
    if (exp == 0u) {
        return T{1};
    }

    if (exp % 2u == 1u) {
        return base * int_pow(base, exp - 1u);
    }

    return square(int_pow(base, exp / 2u));
}

}  // namespace detail
}  // namespace au



// "Magnitude" is a collection of templated types, representing positive real numbers.
//
// The key design goal is to support products and rational powers _exactly_, including for many
// irrational numbers, such as Pi, or sqrt(2).
//
// Even though there is only one possible value for each type, we encourage users to use these
// values wherever possible, because they interact correctly via standard `*`, `/`, `==`, and `!=`
// operations, and this leads to more readable code.

namespace au {

template <typename... BPs>
struct Magnitude {
    // Having separate `static_assert` instances for the individual conditions produces more
    // readable errors if we fail.
    static_assert(AreAllPowersNonzero<Magnitude, Magnitude<BPs...>>::value,
                  "All powers must be nonzero");
    static_assert(AreBasesInOrder<Magnitude, Magnitude<BPs...>>::value,
                  "Bases must be listed in ascending order");

    // We also want to use the "full" validity check.  This should be equivalent to the above
    // conditions, but if we add more conditions later, we want them to get picked up here
    // automatically.
    static_assert(IsValidPack<Magnitude, Magnitude<BPs...>>::value, "Ill-formed Magnitude");
};

// Define readable operations for product, quotient, power, inverse on Magnitudes.
template <typename... BPs>
using MagProductT = PackProductT<Magnitude, BPs...>;
template <typename T, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using MagPowerT = PackPowerT<Magnitude, T, ExpNum, ExpDen>;
template <typename T, typename U>
using MagQuotientT = PackQuotientT<Magnitude, T, U>;
template <typename T>
using MagInverseT = PackInverseT<Magnitude, T>;

// Enable negative magnitudes with a type representing (-1) that appears/disappears under powers.
struct Negative {};
template <typename... BPs, std::intmax_t ExpNum, std::intmax_t ExpDen>
struct PackPower<Magnitude, Magnitude<Negative, BPs...>, std::ratio<ExpNum, ExpDen>>
    : std::conditional<
          (std::ratio<ExpNum, ExpDen>::num % 2 == 0),

          // Even powers of (-1) are 1 for any root.
          PackPowerT<Magnitude, Magnitude<BPs...>, ExpNum, ExpDen>,

          // At this point, we know we're taking the D'th root of (-1), which is (-1)
          // if D is odd, and a hard compiler error if D is even.
          MagProductT<Magnitude<Negative>, MagPowerT<Magnitude<BPs...>, ExpNum, ExpDen>>>
// Implement the hard error for raising to (odd / even) power:
{
    static_assert(std::ratio<ExpNum, ExpDen>::den % 2 == 1,
                  "Cannot take even root of negative magnitude");
};
template <typename... LeftBPs, typename... RightBPs>
struct PackProduct<Magnitude, Magnitude<Negative, LeftBPs...>, Magnitude<Negative, RightBPs...>>
    : stdx::type_identity<MagProductT<Magnitude<LeftBPs...>, Magnitude<RightBPs...>>> {};

// Define negation.
template <typename... BPs>
constexpr auto operator-(Magnitude<Negative, BPs...>) {
    return Magnitude<BPs...>{};
}
template <typename... BPs>
constexpr auto operator-(Magnitude<BPs...>) {
    return Magnitude<Negative, BPs...>{};
}

// A printable label to indicate the Magnitude for human readers.
template <typename MagT>
struct MagnitudeLabel;

// A sizeof()-compatible API to get the label for a Magnitude.
template <typename MagT>
constexpr const auto &mag_label(MagT = MagT{});

// A helper function to create a Magnitude from an integer constant.
template <std::uintmax_t N>
constexpr auto mag();

// A base type for prime numbers.
template <std::uintmax_t N>
struct Prime {
    static_assert(detail::is_prime(N), "Prime<N> requires that N is prime");

    static constexpr std::uintmax_t value() { return N; }
};

// A base type for pi.
struct Pi {
    // The reason we define this manually, rather than using something like `M_PIl`, is because the
    // latter is not available on certain architectures.  We do test against `M_PIl`.  Those tests
    // are not run on architectures that don't support `M_PIl`, but as long as they are run on any
    // architectures at all, that's enough to give confidence in this value.
    //
    // Source for value: http://www.pi-world-ranking-list.com/lists/details/hogg.html
    static constexpr long double value() { return 3.14159265358979323846264338327950288419716939L; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Define the lexicographic ordering of bases for Magnitude.

namespace detail {
template <typename T, typename U>
struct OrderByValue : stdx::bool_constant<(T::value() < U::value())> {};

template <typename T>
struct OrderByValue<Negative, T> : std::true_type {};

template <typename T>
struct OrderByValue<T, Negative> : std::false_type {};

template <>
struct OrderByValue<Negative, Negative> : std::false_type {};
}  // namespace detail

template <typename A, typename B>
struct InOrderFor<Magnitude, A, B> : LexicographicTotalOrdering<A, B, detail::OrderByValue> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type trait based interface for Magnitude.

template <typename MagT>
struct IntegerPartImpl;
template <typename MagT>
using IntegerPartT = typename IntegerPartImpl<MagT>::type;

template <typename MagT>
struct AbsImpl;
template <typename MagT>
using Abs = typename AbsImpl<MagT>::type;

template <typename MagT>
struct SignImpl;
template <typename MagT>
using Sign = typename SignImpl<MagT>::type;

template <typename MagT>
struct NumeratorImpl;
template <typename MagT>
using NumeratorT = typename NumeratorImpl<MagT>::type;

template <typename MagT>
using DenominatorT = NumeratorT<MagInverseT<Abs<MagT>>>;

template <typename MagT>
struct IsPositive : std::true_type {};
template <typename... BPs>
struct IsPositive<Magnitude<Negative, BPs...>> : std::false_type {};

template <typename MagT>
struct IsRational
    : std::is_same<MagT,
                   MagQuotientT<IntegerPartT<NumeratorT<MagT>>, IntegerPartT<DenominatorT<MagT>>>> {
};

template <typename MagT>
struct IsInteger : std::is_same<MagT, IntegerPartT<MagT>> {};

// The "common magnitude" of two Magnitudes is the largest Magnitude that evenly divides both.
//
// This is possible only if the quotient of the inputs is rational.  If it's not, then the "common
// magnitude" is one that is related to both inputs, and symmetrical under a change in order (to
// fulfill the requirements of a `std::common_type` specialization).
template <typename... Ms>
struct CommonMagnitude;
template <typename... Ms>
using CommonMagnitudeT = typename CommonMagnitude<Ms...>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Value based interface for Magnitude.

static constexpr auto ONE = Magnitude<>{};

template <typename... BP1s, typename... BP2s>
constexpr auto operator*(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return MagProductT<Magnitude<BP1s...>, Magnitude<BP2s...>>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator/(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return MagQuotientT<Magnitude<BP1s...>, Magnitude<BP2s...>>{};
}

template <int E, typename... BPs>
constexpr auto pow(Magnitude<BPs...>) {
    return MagPowerT<Magnitude<BPs...>, E>{};
}

template <int N, typename... BPs>
constexpr auto root(Magnitude<BPs...>) {
    return MagPowerT<Magnitude<BPs...>, 1, N>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator==(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return std::is_same<Magnitude<BP1s...>, Magnitude<BP2s...>>::value;
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator!=(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    return !(m1 == m2);
}

template <typename... BPs>
constexpr auto integer_part(Magnitude<BPs...>) {
    return IntegerPartT<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr auto abs(Magnitude<BPs...>) {
    return Abs<Magnitude<BPs...>>{};
}
constexpr auto abs(Zero z) { return z; }

template <typename... BPs>
constexpr auto sign(Magnitude<BPs...>) {
    return Sign<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr auto numerator(Magnitude<BPs...>) {
    return NumeratorT<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr auto denominator(Magnitude<BPs...>) {
    return DenominatorT<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr bool is_positive(Magnitude<BPs...>) {
    return IsPositive<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool is_rational(Magnitude<BPs...>) {
    return IsRational<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool is_integer(Magnitude<BPs...>) {
    return IsInteger<Magnitude<BPs...>>::value;
}

// Get the value of this Magnitude in a "traditional" numeric type T.
//
// If T is an integral type, then the Magnitude must be integral as well.
template <typename T, typename... BPs>
constexpr T get_value(Magnitude<BPs...>);

// Value-based interface around CommonMagnitude.
template <typename... Ms>
constexpr auto common_magnitude(Ms...) {
    return CommonMagnitudeT<Ms...>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `mag<N>()` implementation.

namespace detail {

// Helper to perform prime factorization.
template <std::uintmax_t N>
struct PrimeFactorization;
template <std::uintmax_t N>
using PrimeFactorizationT = typename PrimeFactorization<N>::type;

// Base case: factorization of 1.
template <>
struct PrimeFactorization<1u> : stdx::type_identity<Magnitude<>> {};

template <std::uintmax_t N>
struct PrimeFactorization {
    static_assert(N > 0, "Can only factor positive integers");

    static constexpr std::uintmax_t base = find_prime_factor(N);
    static constexpr std::uintmax_t power = multiplicity(base, N);
    static constexpr std::uintmax_t remainder = N / int_pow(base, power);

    using type = MagProductT<Magnitude<Pow<Prime<base>, static_cast<std::intmax_t>(power)>>,
                             PrimeFactorizationT<remainder>>;
};

}  // namespace detail

template <std::uintmax_t N>
constexpr auto mag() {
    return detail::PrimeFactorizationT<N>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `integer_part()` implementation.

template <typename B, typename P>
struct IntegerPartOfBasePower : stdx::type_identity<Magnitude<>> {};

// Raise B to the largest natural number power which won't exceed (N/D), or 0 if there isn't one.
template <std::uintmax_t B, std::intmax_t N, std::intmax_t D>
struct IntegerPartOfBasePower<Prime<B>, std::ratio<N, D>>
    : stdx::type_identity<MagPowerT<Magnitude<Prime<B>>, ((N >= D) ? (N / D) : 0)>> {};

template <typename... BPs>
struct IntegerPartImpl<Magnitude<BPs...>>
    : stdx::type_identity<
          MagProductT<typename IntegerPartOfBasePower<BaseT<BPs>, ExpT<BPs>>::type...>> {};

template <typename... BPs>
struct IntegerPartImpl<Magnitude<Negative, BPs...>>
    : stdx::type_identity<MagProductT<Magnitude<Negative>, IntegerPartT<Magnitude<BPs...>>>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `abs()` implementation.

template <typename... BPs>
struct AbsImpl<Magnitude<Negative, BPs...>> : stdx::type_identity<Magnitude<BPs...>> {};

template <typename... BPs>
struct AbsImpl<Magnitude<BPs...>> : stdx::type_identity<Magnitude<BPs...>> {};

template <>
struct AbsImpl<Zero> : stdx::type_identity<Zero> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `sign()` implementation.

template <typename... BPs>
struct SignImpl<Magnitude<BPs...>> : stdx::type_identity<Magnitude<>> {};

template <typename... BPs>
struct SignImpl<Magnitude<Negative, BPs...>> : stdx::type_identity<Magnitude<Negative>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `numerator()` implementation.

template <typename... BPs>
struct NumeratorImpl<Magnitude<BPs...>>
    : stdx::type_identity<
          MagProductT<std::conditional_t<(ExpT<BPs>::num > 0), Magnitude<BPs>, Magnitude<>>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `get_value<T>(Magnitude)` implementation.

namespace detail {

enum class MagRepresentationOutcome {
    OK,
    ERR_NON_INTEGER_IN_INTEGER_TYPE,
    ERR_NEGATIVE_NUMBER_IN_UNSIGNED_TYPE,
    ERR_INVALID_ROOT,
    ERR_CANNOT_FIT,
};

template <typename T>
struct MagRepresentationOrError {
    MagRepresentationOutcome outcome;

    // Only valid/meaningful if `outcome` is `OK`.
    T value = {0};
};

// The widest arithmetic type in the same category.
//
// Used for intermediate computations.
template <typename T>
using Widen = std::conditional_t<
    std::is_arithmetic<T>::value,
    std::conditional_t<std::is_floating_point<T>::value,
                       long double,
                       std::conditional_t<std::is_signed<T>::value, std::intmax_t, std::uintmax_t>>,
    T>;

template <typename T>
constexpr MagRepresentationOrError<T> checked_int_pow(T base, std::uintmax_t exp) {
    MagRepresentationOrError<T> result = {MagRepresentationOutcome::OK, T{1}};
    while (exp > 0u) {
        if (exp % 2u == 1u) {
            if (base > std::numeric_limits<T>::max() / result.value) {
                return MagRepresentationOrError<T>{MagRepresentationOutcome::ERR_CANNOT_FIT};
            }
            result.value *= base;
        }

        exp /= 2u;

        if (base > std::numeric_limits<T>::max() / base) {
            return (exp == 0u)
                       ? result
                       : MagRepresentationOrError<T>{MagRepresentationOutcome::ERR_CANNOT_FIT};
        }
        base *= base;
    }
    return result;
}

template <typename T>
using IsKnownToBeInteger = stdx::bool_constant<(std::numeric_limits<T>::is_specialized &&
                                                std::numeric_limits<T>::is_integer)>;

template <typename T>
struct NontrivialRootForInt {
    constexpr MagRepresentationOrError<T> operator()(T, std::uintmax_t) const {
        // There exist input values where a valid answer exists.  If this were a fully general root
        // finding function, we would want to support them.  However, those situations can't arise
        // in this instance.  We would never take a non-trivial root that returns an integer,
        // because all inputs are products of rational powers of basis numbers.  If the result were
        // an integer, then this would be made from rational powers of primes, and those rational
        // exponents would have been converted to lowest terms already.
        return {MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE};
    }
};

template <typename T>
struct GeneralNontrivialRoot;

template <typename T, bool IsTKnownToBeInteger>
struct NontrivialRootImpl
    : std::conditional_t<IsTKnownToBeInteger, NontrivialRootForInt<T>, GeneralNontrivialRoot<T>> {
    static_assert(IsKnownToBeInteger<T>::value == IsTKnownToBeInteger, "Internal library error");
};

template <typename T>
constexpr MagRepresentationOrError<T> root(T x, std::uintmax_t n) {
    // The "zeroth root" would be mathematically undefined.
    if (n == 0) {
        return {MagRepresentationOutcome::ERR_INVALID_ROOT};
    }

    // The "first root" is trivial.
    if (n == 1) {
        return {MagRepresentationOutcome::OK, x};
    }

    // Handle special cases of zero and one.
    if (x == 0 || x == 1) {
        return {MagRepresentationOutcome::OK, x};
    }

    return NontrivialRootImpl<T, IsKnownToBeInteger<T>::value>{}(x, n);
}

template <typename T>
struct GeneralNontrivialRoot {
    constexpr MagRepresentationOrError<T> operator()(T x, std::uintmax_t n) const {
        // Handle negative numbers: only odd roots are allowed.
        if (x < 0) {
            if (n % 2 == 0) {
                return {MagRepresentationOutcome::ERR_INVALID_ROOT};
            }

            const auto negative_result = root(-x, n);
            if (negative_result.outcome != MagRepresentationOutcome::OK) {
                return {negative_result.outcome};
            }

            return {MagRepresentationOutcome::OK, static_cast<T>(-negative_result.value)};
        }

        // Handle numbers bewtween 0 and 1.
        if (x < 1) {
            const auto inverse_result = root(T{1} / x, n);
            if (inverse_result.outcome != MagRepresentationOutcome::OK) {
                return {inverse_result.outcome};
            }
            return {MagRepresentationOutcome::OK, static_cast<T>(T{1} / inverse_result.value)};
        }

        //
        // At this point, error conditions are finished, and we can proceed with the "core"
        // algorithm.
        //

        // Always use `long double` for intermediate computations.  We don't ever expect people to
        // be calling this at runtime, so we want maximum accuracy.
        long double lo = 1.0;
        long double hi = static_cast<long double>(x);

        // Do a binary search to find the closest value such that `checked_int_pow` recovers the
        // input.
        //
        // Because we know `n > 1`, and `x > 1`, and x^n is monotonically increasing, we know that
        // `checked_int_pow(lo, n) < x < checked_int_pow(hi, n)`.  We will preserve this as an
        // invariant.
        while (lo < hi) {
            long double mid = lo + (hi - lo) / 2;

            auto result = checked_int_pow(mid, n);

            if (result.outcome != MagRepresentationOutcome::OK) {
                return {result.outcome};
            }

            // Early return if we get lucky with an exact answer.
            if (result.value == x) {
                return {MagRepresentationOutcome::OK, static_cast<T>(mid)};
            }

            // Check for stagnation.
            if (mid == lo || mid == hi) {
                break;
            }

            // Preserve the invariant that `checked_int_pow(lo, n) < x < checked_int_pow(hi, n)`.
            if (result.value < x) {
                lo = mid;
            } else {
                hi = mid;
            }
        }

        // Pick whichever one gets closer to the target.
        const auto lo_diff = x - checked_int_pow(lo, n).value;
        const auto hi_diff = checked_int_pow(hi, n).value - x;
        return {MagRepresentationOutcome::OK, static_cast<T>(lo_diff < hi_diff ? lo : hi)};
    }
};
enum class SignOfExponent { POSITIVE_SIGN, NEGATIVE_SIGN };

template <typename T, std::uintmax_t N, std::uintmax_t D, typename B, SignOfExponent>
struct BasePowerValueImpl;

template <typename T, std::intmax_t N, std::uintmax_t D, typename B>
constexpr MagRepresentationOrError<Widen<T>> base_power_value(B base) {
    return BasePowerValueImpl<T,
                              static_cast<std::uintmax_t>(N < 0 ? -N : N),
                              D,
                              B,
                              (N < 0 ? SignOfExponent::NEGATIVE_SIGN
                                     : SignOfExponent::POSITIVE_SIGN)>{}(base);
}

template <typename T, std::uintmax_t N, std::uintmax_t D, typename B>
struct BasePowerValueImpl<T, N, D, B, SignOfExponent::NEGATIVE_SIGN> {
    constexpr MagRepresentationOrError<Widen<T>> operator()(B base) const {
        const auto inverse_result =
            BasePowerValueImpl<T, N, D, B, SignOfExponent::POSITIVE_SIGN>{}(base);
        if (inverse_result.outcome != MagRepresentationOutcome::OK) {
            return inverse_result;
        }
        return {
            MagRepresentationOutcome::OK,
            Widen<T>{1} / inverse_result.value,
        };
    }
};

template <typename T, std::uintmax_t N, std::uintmax_t D, typename B>
struct BasePowerValueImpl<T, N, D, B, SignOfExponent::POSITIVE_SIGN> {
    constexpr MagRepresentationOrError<Widen<T>> operator()(B base) const {
        const auto power_result = checked_int_pow(static_cast<Widen<T>>(base), N);
        if (power_result.outcome != MagRepresentationOutcome::OK) {
            return {power_result.outcome};
        }
        return (D > 1) ? root(power_result.value, D) : power_result;
    }
};

template <typename T, std::size_t N>
constexpr MagRepresentationOrError<T> product(const MagRepresentationOrError<T> (&values)[N]) {
    for (const auto &x : values) {
        if (x.outcome != MagRepresentationOutcome::OK) {
            return x;
        }
    }

    T result{1};
    for (const auto &x : values) {
        if ((x.value > 1) && (result > std::numeric_limits<T>::max() / x.value)) {
            return {MagRepresentationOutcome::ERR_CANNOT_FIT};
        }
        result *= x.value;
    }
    return {MagRepresentationOutcome::OK, result};
}

template <std::size_t N>
constexpr bool all(const bool (&values)[N]) {
    for (const auto &x : values) {
        if (!x) {
            return false;
        }
    }
    return true;
}

// `RealPart<T>` is `T` itself, unless that type has a `.real()` member.
template <typename T>
using TypeOfRealMember = decltype(std::declval<T>().real());
template <typename T>
// `RealPartImpl` is basically equivalent to the `detected_or<T, TypeOfRealMember, T>` part at the
// end.  But we special-case `is_arithmetic` to get a fast short-circuit for the overwhelmingly most
// common case.
struct RealPartImpl : std::conditional<std::is_arithmetic<T>::value,
                                       T,
                                       stdx::experimental::detected_or_t<T, TypeOfRealMember, T>> {
};
template <typename T>
using RealPart = typename RealPartImpl<T>::type;

template <typename Target, typename Enable = void>
struct SafeCastingChecker {
    template <typename T>
    constexpr bool operator()(T x) {
        return stdx::cmp_less_equal(std::numeric_limits<RealPart<Target>>::lowest(), x) &&
               stdx::cmp_greater_equal(std::numeric_limits<RealPart<Target>>::max(), x);
    }
};

template <typename Target>
struct SafeCastingChecker<Target, std::enable_if_t<std::is_integral<Target>::value>> {
    template <typename T>
    constexpr bool operator()(T x) {
        return std::is_integral<T>::value &&
               stdx::cmp_less_equal(std::numeric_limits<RealPart<Target>>::lowest(), x) &&
               stdx::cmp_greater_equal(std::numeric_limits<RealPart<Target>>::max(), x);
    }
};

template <typename T, typename InputT>
constexpr bool safe_to_cast_to(InputT x) {
    return SafeCastingChecker<T>{}(x);
}

template <typename T, typename MagT>
struct GetValueResultImplForNonIntegerInIntegralType {
    constexpr MagRepresentationOrError<T> operator()() {
        return {MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE};
    }
};

template <typename T, typename MagT>
struct GetValueResultImplForDefaultCase;
template <typename T, typename... BPs>
struct GetValueResultImplForDefaultCase<T, Magnitude<BPs...>> {
    constexpr MagRepresentationOrError<T> operator()() {
        // Force the expression to be evaluated in a constexpr context.
        constexpr auto widened_result =
            product({base_power_value<RealPart<T>,
                                      ExpT<BPs>::num,
                                      static_cast<std::uintmax_t>(ExpT<BPs>::den)>(
                BaseT<BPs>::value())...});

        if ((widened_result.outcome != MagRepresentationOutcome::OK) ||
            !safe_to_cast_to<T>(widened_result.value)) {
            return {MagRepresentationOutcome::ERR_CANNOT_FIT};
        }

        return {MagRepresentationOutcome::OK, static_cast<T>(widened_result.value)};
    }
};

template <typename T, typename MagT>
struct GetValueResultImpl
    : std::conditional_t<
          stdx::conjunction<std::is_integral<T>, stdx::negation<IsInteger<MagT>>>::value,
          GetValueResultImplForNonIntegerInIntegralType<T, MagT>,
          GetValueResultImplForDefaultCase<T, MagT>> {};

template <typename T, typename... BPs>
constexpr MagRepresentationOrError<T> get_value_result(Magnitude<BPs...>) {
    constexpr auto result = GetValueResultImpl<T, Magnitude<BPs...>>{}();
    return result;
}

// This simple overload avoids edge cases with creating and passing zero-sized arrays.
template <typename T>
constexpr MagRepresentationOrError<T> get_value_result(Magnitude<>) {
    return {MagRepresentationOutcome::OK, static_cast<T>(1)};
}

template <typename T, typename MagT, bool IsCandidate>
struct IsExactlyLowestOfSignedIntegral : std::false_type {};
template <typename T, typename... BPs>
struct IsExactlyLowestOfSignedIntegral<T, Magnitude<Negative, BPs...>, true>
    : std::is_same<
          decltype(mag<static_cast<std::make_unsigned_t<T>>(std::numeric_limits<T>::max()) + 1u>()),
          Magnitude<BPs...>> {};
template <typename T, typename... BPs>
constexpr bool is_exactly_lowest_of_signed_integral(Magnitude<BPs...>) {
    return IsExactlyLowestOfSignedIntegral<
        T,
        Magnitude<BPs...>,
        stdx::conjunction<std::is_integral<T>, std::is_signed<T>>::value>::value;
}

template <typename T, typename... BPs>
constexpr MagRepresentationOrError<T> get_value_result(Magnitude<Negative, BPs...> m) {
    if (std::is_unsigned<T>::value) {
        return {MagRepresentationOutcome::ERR_NEGATIVE_NUMBER_IN_UNSIGNED_TYPE};
    }

    if (is_exactly_lowest_of_signed_integral<T>(m)) {
        return {MagRepresentationOutcome::OK, std::numeric_limits<T>::lowest()};
    }

    const auto result = get_value_result<T>(Magnitude<BPs...>{});
    if (result.outcome != MagRepresentationOutcome::OK) {
        return result;
    }
    return {MagRepresentationOutcome::OK, static_cast<T>(-result.value)};
}
}  // namespace detail

template <typename T, typename... BPs>
constexpr bool representable_in(Magnitude<BPs...> m) {
    using namespace detail;

    return get_value_result<T>(m).outcome == MagRepresentationOutcome::OK;
}

template <typename T, typename... BPs>
constexpr T get_value(Magnitude<BPs...> m) {
    using namespace detail;

    constexpr auto result = get_value_result<T>(m);

    static_assert(result.outcome != MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE,
                  "Cannot represent non-integer in integral destination type");
    static_assert(result.outcome != MagRepresentationOutcome::ERR_INVALID_ROOT,
                  "Could not compute root for rational power of base");
    static_assert(result.outcome != MagRepresentationOutcome::ERR_CANNOT_FIT,
                  "Value outside range of destination type");

    static_assert(result.outcome == MagRepresentationOutcome::OK, "Unknown error occurred");
    return result.value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MagnitudeLabel` implementation.

namespace detail {
enum class MagLabelCategory {
    INTEGER,
    RATIONAL,
    UNSUPPORTED,
};

template <typename... BPs>
constexpr MagLabelCategory categorize_mag_label(Magnitude<BPs...> m) {
    // This unsightly "nested ternary" approach makes this entire function into --- _technically_
    // --- a one-liner, which appeases the Green Hills compiler.
    return IsInteger<Magnitude<BPs...>>::value
               ? (get_value_result<std::uintmax_t>(m).outcome == MagRepresentationOutcome::OK
                      ? MagLabelCategory::INTEGER
                      : MagLabelCategory::UNSUPPORTED)
               : (IsRational<Magnitude<BPs...>>::value ? MagLabelCategory::RATIONAL
                                                       : MagLabelCategory::UNSUPPORTED);
}

template <typename MagT, MagLabelCategory Category>
struct MagnitudeLabelImplementation {
    static constexpr const char value[] = "(UNLABELED SCALE FACTOR)";

    static constexpr const bool has_exposed_slash = false;
};
template <typename MagT, MagLabelCategory Category>
constexpr const char MagnitudeLabelImplementation<MagT, Category>::value[];
template <typename MagT, MagLabelCategory Category>
constexpr const bool MagnitudeLabelImplementation<MagT, Category>::has_exposed_slash;

template <typename MagT>
struct MagnitudeLabelImplementation<MagT, MagLabelCategory::INTEGER>
    : detail::UIToA<get_value<std::uintmax_t>(MagT{})> {
    static constexpr const bool has_exposed_slash = false;
};
template <typename MagT>
constexpr const bool
    MagnitudeLabelImplementation<MagT, MagLabelCategory::INTEGER>::has_exposed_slash;

// Analogous to `detail::ExtendedLabel`, but for magnitudes.
//
// This makes it easier to name the exact type for compound labels.
template <std::size_t ExtensionStrlen, typename... Mags>
using ExtendedMagLabel =
    StringConstant<concatenate(MagnitudeLabel<Mags>::value...).size() + ExtensionStrlen>;

template <typename MagT>
struct MagnitudeLabelImplementation<MagT, MagLabelCategory::RATIONAL> {
    using LabelT = ExtendedMagLabel<3u, NumeratorT<MagT>, DenominatorT<MagT>>;
    static constexpr LabelT value = join_by(
        " / ", MagnitudeLabel<NumeratorT<MagT>>::value, MagnitudeLabel<DenominatorT<MagT>>::value);

    static constexpr const bool has_exposed_slash = true;
};
template <typename MagT>
constexpr typename MagnitudeLabelImplementation<MagT, MagLabelCategory::RATIONAL>::LabelT
    MagnitudeLabelImplementation<MagT, MagLabelCategory::RATIONAL>::value;
template <typename MagT>
constexpr const bool
    MagnitudeLabelImplementation<MagT, MagLabelCategory::RATIONAL>::has_exposed_slash;

}  // namespace detail

template <typename... BPs>
struct MagnitudeLabel<Magnitude<BPs...>>
    : detail::MagnitudeLabelImplementation<Magnitude<BPs...>,
                                           detail::categorize_mag_label(Magnitude<BPs...>{})> {};

template <typename... BPs>
struct MagnitudeLabel<Magnitude<Negative, BPs...>> :
    // Inherit for "has exposed slash".
    MagnitudeLabel<Magnitude<BPs...>> {
    using LabelT = detail::ExtendedMagLabel<1u, Magnitude<BPs...>>;
    static constexpr LabelT value =
        detail::concatenate("-", MagnitudeLabel<Magnitude<BPs...>>::value);
};
template <typename... BPs>
constexpr typename MagnitudeLabel<Magnitude<Negative, BPs...>>::LabelT
    MagnitudeLabel<Magnitude<Negative, BPs...>>::value;

template <typename MagT>
constexpr const auto &mag_label(MagT) {
    return detail::as_char_array(MagnitudeLabel<MagT>::value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonMagnitude` implementation.

namespace detail {
// Helper: prepend a base power, but only if the Exp is negative.
template <typename BP, typename MagT>
struct PrependIfExpNegative;
template <typename BP, typename MagT>
using PrependIfExpNegativeT = typename PrependIfExpNegative<BP, MagT>::type;
template <typename BP, typename... Ts>
struct PrependIfExpNegative<BP, Magnitude<Ts...>>
    : std::conditional<(ExpT<BP>::num < 0), Magnitude<BP, Ts...>, Magnitude<Ts...>> {};

// Remove all positive powers from M.
template <typename M>
using NegativePowers = MagQuotientT<M, NumeratorPartT<M>>;
}  // namespace detail

// 1-ary case: identity.
template <typename M>
struct CommonMagnitude<M> : stdx::type_identity<M> {};

// 2-ary base case: both Magnitudes null.
template <>
struct CommonMagnitude<Magnitude<>, Magnitude<>> : stdx::type_identity<Magnitude<>> {};

// 2-ary base case: only left Magnitude is null.
template <typename Head, typename... Tail>
struct CommonMagnitude<Magnitude<>, Magnitude<Head, Tail...>>
    : stdx::type_identity<detail::NegativePowers<Magnitude<Head, Tail...>>> {};

// 2-ary base case: only right Magnitude is null.
template <typename Head, typename... Tail>
struct CommonMagnitude<Magnitude<Head, Tail...>, Magnitude<>>
    : stdx::type_identity<detail::NegativePowers<Magnitude<Head, Tail...>>> {};

// 2-ary recursive case: two non-null Magnitudes.
template <typename H1, typename... T1, typename H2, typename... T2>
struct CommonMagnitude<Magnitude<H1, T1...>, Magnitude<H2, T2...>> :

    // If the bases for H1 and H2 are in-order, prepend H1-if-negative to the remainder.
    std::conditional<
        (InOrderFor<Magnitude, BaseT<H1>, BaseT<H2>>::value),
        detail::PrependIfExpNegativeT<H1, CommonMagnitudeT<Magnitude<T1...>, Magnitude<H2, T2...>>>,

        // If the bases for H2 and H1 are in-order, prepend H2-if-negative to the remainder.
        std::conditional_t<
            (InOrderFor<Magnitude, BaseT<H2>, BaseT<H1>>::value),
            detail::PrependIfExpNegativeT<H2,
                                          CommonMagnitudeT<Magnitude<T2...>, Magnitude<H1, T1...>>>,

            // If we got here, the bases must be the same.  (We can assume that `InOrderFor` does
            // proper checking to guard against equivalent-but-not-identical bases, which would
            // violate total ordering.)
            std::conditional_t<
                (std::ratio_subtract<ExpT<H1>, ExpT<H2>>::num < 0),
                detail::PrependT<CommonMagnitudeT<Magnitude<T1...>, Magnitude<T2...>>, H1>,
                detail::PrependT<CommonMagnitudeT<Magnitude<T1...>, Magnitude<T2...>>, H2>>>> {};

// N-ary case: recurse.
template <typename M1, typename M2, typename... Tail>
struct CommonMagnitude<M1, M2, Tail...> : CommonMagnitude<M1, CommonMagnitudeT<M2, Tail...>> {};

// Zero is always ignored.
template <typename M>
struct CommonMagnitude<M, Zero> : stdx::type_identity<M> {};
template <typename M>
struct CommonMagnitude<Zero, M> : stdx::type_identity<M> {};
template <>
struct CommonMagnitude<Zero, Zero> : stdx::type_identity<Zero> {};

}  // namespace  au


namespace au {
namespace detail {

//
// `OpInput<Op>` and `OpOutput<Op>` are the input and output types of an operation.
//
template <typename Op>
struct OpInputImpl;
template <typename Op>
using OpInput = typename OpInputImpl<Op>::type;

template <typename Op>
struct OpOutputImpl;
template <typename Op>
using OpOutput = typename OpOutputImpl<Op>::type;

//
// `StaticCast<T, U>` represents an operation that converts from `T` to `U` via `static_cast`.
//
template <typename T, typename U>
struct StaticCast;

//
// `MultiplyTypeBy<T, M>` represents an operation that multiplies a value of type `T` by the
// magnitude `M`.
//
// Note that this operation does *not* model integer promotion.  It will always force the result to
// be `T`.  To model integer promotion, form a compound operation with `OpSequence` that includes
// appropriate `StaticCast`.
//
template <typename T, typename M>
struct MultiplyTypeBy;

//
// `DivideTypeByInteger<T, M>` represents an operation that divides a value of type `T` by the
// magnitude `M`.
//
// Note that this operation does *not* model integer promotion.  It will always force the result to
// be `T`.  To model integer promotion, form a compound operation with `OpSequence` that includes
// appropriate `StaticCast`.
//
template <typename T, typename M>
struct DivideTypeByInteger;

//
// `OpSequence<Ops...>` represents an ordered sequence of operations.
//
// We require that the output type of each operation is the same as the input type of the next one
// (see below for `OpInput` and `OpOutput`).
//
template <typename... Ops>
struct OpSequenceImpl;
template <typename... Ops>
using OpSequence = FlattenAs<OpSequenceImpl, Ops...>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS (`abstract_operations.hh`):
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename U>
struct OpInputImpl<StaticCast<T, U>> : stdx::type_identity<T> {};
template <typename T, typename U>
struct OpOutputImpl<StaticCast<T, U>> : stdx::type_identity<U> {};

// `StaticCast<T, U>` operation:
template <typename T, typename U>
struct StaticCast {
    static constexpr U apply_to(T value) { return static_cast<U>(value); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename M>
struct OpInputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};
template <typename T, typename M>
struct OpOutputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};

// `MultiplyTypeBy<T, M>` operation:
template <typename T, typename Mag>
struct MultiplyTypeBy {
    static constexpr T apply_to(T value) {
        return static_cast<T>(value * get_value<RealPart<T>>(Mag{}));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger<T, M>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename M>
struct OpInputImpl<DivideTypeByInteger<T, M>> : stdx::type_identity<T> {};
template <typename T, typename M>
struct OpOutputImpl<DivideTypeByInteger<T, M>> : stdx::type_identity<T> {};

template <typename T, typename M, MagRepresentationOutcome MagOutcome>
struct DivideTypeByIntegerImpl {
    static constexpr T apply_to(T value) {
        static_assert(MagOutcome == MagRepresentationOutcome::OK, "Internal library error");
        return static_cast<T>(value / get_value<RealPart<T>>(M{}));
    }
};

template <typename T, typename M>
struct DivideTypeByIntegerImpl<T, M, MagRepresentationOutcome::ERR_CANNOT_FIT> {
    // If a number is too big to fit in the type, then dividing by it should produce 0.
    static constexpr T apply_to(T) { return T{0}; }
};

template <typename T, typename M>
struct DivideTypeByInteger
    : DivideTypeByIntegerImpl<T, M, get_value_result<RealPart<T>>(M{}).outcome> {
    static_assert(IsInteger<M>::value,
                  "Internal library error: inappropriate operation"
                  " (use `MultiplyTypeBy` with inverse instead)");
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<Ops...>` implementation.

// `OpInput`:
template <typename Op, typename... Ops>
struct OpInputImpl<OpSequenceImpl<Op, Ops...>> : stdx::type_identity<OpInput<Op>> {};

// `OpOutput`:
template <typename Op, typename... Ops>
struct OpOutputImpl<OpSequenceImpl<Op, Ops...>>
    : stdx::type_identity<OpOutput<OpSequence<Ops...>>> {};
template <typename OnlyOp>
struct OpOutputImpl<OpSequenceImpl<OnlyOp>> : stdx::type_identity<OpOutput<OnlyOp>> {};

template <typename Op>
struct OpSequenceImpl<Op> {
    static constexpr auto apply_to(OpInput<OpSequenceImpl> value) { return Op::apply_to(value); }
};

template <typename Op, typename... Ops>
struct OpSequenceImpl<Op, Ops...> {
    static constexpr auto apply_to(OpInput<OpSequenceImpl> value) {
        return OpSequenceImpl<Ops...>::apply_to(Op::apply_to(value));
    }
};

}  // namespace detail
}  // namespace au


namespace au {
namespace detail {

//
// `ConversionForRepsAndFactor<OldRep, NewRep, Factor>` is the operation that takes a value of
// `OldRep`, and produces the product of that value with magnitude `Factor` in the type `NewRep`.
//
template <typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl;
template <typename OldRep, typename NewRep, typename Factor>
using ConversionForRepsAndFactor =
    typename ConversionForRepsAndFactorImpl<OldRep, NewRep, Factor>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details (`conversion_strategy.hh`):
////////////////////////////////////////////////////////////////////////////////////////////////////

//
// `ApplicationStrategyFor<T, Mag>` tells us how we should apply a magnitude `Mag` to a type `T`.
//

enum class MagKind {
    DEFAULT,
    INTEGER_DIVIDE,
    NONTRIVIAL_RATIONAL,
};

template <MagKind>
struct MagKindHolder {};

template <typename M>
struct MagKindForImpl
    : std::conditional<
          stdx::conjunction<IsRational<M>,
                            stdx::negation<std::is_same<DenominatorT<M>, Magnitude<>>>>::value,
          std::conditional_t<std::is_same<Abs<NumeratorT<M>>, Magnitude<>>::value,
                             MagKindHolder<MagKind::INTEGER_DIVIDE>,
                             MagKindHolder<MagKind::NONTRIVIAL_RATIONAL>>,
          MagKindHolder<MagKind::DEFAULT>> {};
template <typename M>
using MagKindFor = typename MagKindForImpl<M>::type;

template <typename T, typename Mag, typename MagKindValue>
struct ApplicationStrategyForImpl : stdx::type_identity<MultiplyTypeBy<T, Mag>> {};
template <typename T, typename Mag>
using ApplicationStrategyFor = typename ApplicationStrategyForImpl<T, Mag, MagKindFor<Mag>>::type;

template <typename T, typename Mag>
struct ApplicationStrategyForImpl<T, Mag, MagKindHolder<MagKind::INTEGER_DIVIDE>>
    : stdx::type_identity<DivideTypeByInteger<T, MagProductT<Sign<Mag>, DenominatorT<Mag>>>> {};

template <typename T, typename Mag>
struct ApplicationStrategyForImpl<T, Mag, MagKindHolder<MagKind::NONTRIVIAL_RATIONAL>>
    : std::conditional<
          std::is_integral<RealPart<T>>::value,
          OpSequence<MultiplyTypeBy<T, NumeratorT<Mag>>, DivideTypeByInteger<T, DenominatorT<Mag>>>,
          MultiplyTypeBy<T, Mag>> {};

//
// `ConversionRep<OldRep, NewRep>` is the rep we should use when applying the conversion factor.
//
template <typename OldRep, typename NewRep>
struct ConversionRepImpl;
template <typename OldRep, typename NewRep>
using ConversionRep = typename ConversionRepImpl<OldRep, NewRep>::type;

template <typename OldRep, typename NewRep>
struct IsRealToComplex
    : stdx::conjunction<std::is_same<OldRep, RealPart<OldRep>>,
                        stdx::experimental::is_detected<TypeOfRealMember, NewRep>> {};

template <typename OldRep, typename NewRep>
struct ConversionRepImpl
    : std::conditional<IsRealToComplex<OldRep, NewRep>::value,
                       PromotedType<std::common_type_t<RealPart<OldRep>, RealPart<NewRep>>>,
                       PromotedType<std::common_type_t<OldRep, NewRep>>> {};

//
// `StaticCastSequence<T, U>` is the sequence of operations that gets us from `T` to `U`.
//
// Normally, of course, this is just `StaticCast<T, U>`.  But we have weird edge cases like going
// from `double` to `std::complex<int>`, which require an intermediate step of static casting to
// `int`.
//

template <typename T, typename U>
struct StaticCastSequenceImpl
    : std::conditional<stdx::conjunction<IsRealToComplex<T, U>,
                                         stdx::negation<std::is_same<T, RealPart<U>>>>::value,
                       OpSequence<StaticCast<T, RealPart<U>>, StaticCast<RealPart<U>, U>>,
                       StaticCast<T, U>> {};
template <typename T, typename U>
using StaticCastSequence = typename StaticCastSequenceImpl<T, U>::type;

//
// `FullConversionImpl<OldRep, ConversionRepT, NewRep, Factor>` should resolve to the most efficient
// sequence of operations for a conversion from `OldRep` to `NewRep`, with a magnitude `Factor`,
// where `ConversionRepT` is the promoted type of the common type of `OldRep` and `NewRep`.
//

template <typename OldRep, typename ConversionRepT, typename NewRep, typename Factor>
struct FullConversionImpl
    : stdx::type_identity<OpSequence<StaticCastSequence<OldRep, ConversionRepT>,
                                     ApplicationStrategyFor<ConversionRepT, Factor>,
                                     StaticCastSequence<ConversionRepT, NewRep>>> {};

template <typename OldRepIsConversionRep, typename NewRep, typename Factor>
struct FullConversionImpl<OldRepIsConversionRep, OldRepIsConversionRep, NewRep, Factor>
    : stdx::type_identity<OpSequence<ApplicationStrategyFor<OldRepIsConversionRep, Factor>,
                                     StaticCastSequence<OldRepIsConversionRep, NewRep>>> {};

template <typename OldRep, typename NewRepIsConversionRep, typename Factor>
struct FullConversionImpl<OldRep, NewRepIsConversionRep, NewRepIsConversionRep, Factor>
    : stdx::type_identity<OpSequence<StaticCastSequence<OldRep, NewRepIsConversionRep>,
                                     ApplicationStrategyFor<NewRepIsConversionRep, Factor>>> {};

template <typename Rep, typename Factor>
struct FullConversionImpl<Rep, Rep, Rep, Factor>
    : stdx::type_identity<ApplicationStrategyFor<Rep, Factor>> {};

// To implement `ConversionForRepsAndFactor`, delegate to `FullConversionImpl`.
template <typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl
    : FullConversionImpl<OldRep, ConversionRep<OldRep, NewRep>, NewRep, Factor> {};

}  // namespace detail
}  // namespace au



// These utilities help assess overflow risk for an operation `Op` by finding the minimum and
// maximum values in the "scalar type" of `OpInput<Op>` that are guaranteed to not overflow.
//
// The "scalar type" of `T` is usually just `T`, but if `T` is something like `std::complex<U>`, or
// `Eigen::Vector<U, N>`, then it would be `U`.

namespace au {
namespace detail {

//
// `MinPossible<Op>::value()` is the smallest representable value in the "scalar type" for
// `OpInput<Op>` (see above comments for definition of "scalar type").
//
// This exists to give us an interface for `numeric_limits<T>::lowest()` that is as easy as possible
// to use with `MinGood<Op, Limits>`.  That means it automatically applies to the scalar type, and
// that it stores the result behind a `::value()` interface.
//
template <typename Op>
struct MinPossibleImpl;
template <typename Op>
using MinPossible = typename MinPossibleImpl<Op>::type;

//
// `MaxPossible<Op>::value()` is the largest representable value in the "scalar type" for
// `OpInput<Op>` (see above comments for definition of "scalar type").
//
template <typename Op>
struct MaxPossibleImpl;
template <typename Op>
using MaxPossible = typename MaxPossibleImpl<Op>::type;

//
// `MinGood<Op>::value()` is a constexpr constant of the "scalar type" for `OpInput<Op>` that is the
// minimum value that does not overflow.
//
// IMPORTANT: the result must always be non-positive.  The code is structured on this assumption.
//
template <typename Op, typename Limits>
struct MinGoodImpl;
template <typename Op, typename Limits = void>
using MinGood = typename MinGoodImpl<Op, Limits>::type;

//
// `MaxGood<Op>::value()` is a constexpr constant of the "scalar type" for `OpInput<Op>` that is the
// maximum value that does not overflow.
//
// IMPORTANT: the result must always be non-negative.  The code is structured on this assumption.
//
template <typename Op, typename Limits = void>
struct MaxGoodImpl;
template <typename Op, typename Limits = void>
using MaxGood = typename MaxGoodImpl<Op, Limits>::type;

//
// `CanOverflowBelow<Op>::value` is `true` if there is any value in `OpInput<Op>` that can cause the
// operation to exceed its bounds.
//
template <typename Op>
struct CanOverflowBelow;

//
// `CanOverflowAbove<Op>::value` is `true` if there is any value in `OpInput<Op>` that can cause the
// operation to exceed its bounds.
//
template <typename Op>
struct CanOverflowAbove;

// `MinValueChecker<Op>::is_too_small(x)` checks whether the value `x` is small enough to overflow
// the bounds of the operation.
template <typename Op>
struct MinValueChecker;

// `MaxValueChecker<Op>::is_too_large(x)` checks whether the value `x` is large enough to overflow
// the bounds of the operation.
template <typename Op>
struct MaxValueChecker;

// `would_value_overflow<Op>(x)` checks whether the value `x` would exceed the bounds of the
// operation at any stage.
template <typename Op>
constexpr bool would_value_overflow(const OpInput<Op> &x) {
    return MinValueChecker<Op>::is_too_small(x) || MaxValueChecker<Op>::is_too_large(x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS
////////////////////////////////////////////////////////////////////////////////////////////////////

// General note:
//
// The implementation strategy will be to decompose to increasingly specific cases, using
// `std::conditional` constructs that are _at most one layer deep_.  This should keep every
// individual piece as easy to understand as possible, although it does mean we'll tend to be
// navigating many layers deep from the top-level API to the ultimate implementation.
//
// It's easier to navigate these helpers if we put a shorthand comment at the top of each.  Here's
// the key:
//
// (A) = arithmetic (integral or floating point)
// (F) = floating point
// (I) = integral (signed or unsigned)
// (N) = non-arithmetic
// (S) = signed integral
// (U) = unsigned integral
// (X) = any type

////////////////////////////////////////////////////////////////////////////////////////////////////
// Predicate helpers

//
// `IsDefinitelyBounded<T>::value` is `true` if `T` is known to have specific min/max values.
//
template <typename T>
using IsDefinitelyBounded =
    stdx::conjunction<stdx::bool_constant<(std::numeric_limits<T>::is_specialized)>,
                      stdx::bool_constant<(std::numeric_limits<T>::is_bounded)>>;

//
// `IsDefinitelyUnsigned<T>::value` is `true` if `T` is known to be an unsigned type.
//
template <typename T>
using IsDefinitelyUnsigned =
    stdx::conjunction<stdx::bool_constant<std::numeric_limits<T>::is_specialized>,
                      stdx::bool_constant<!std::numeric_limits<T>::is_signed>>;

//
// `IsAbsProbablyBiggerThanOne<T, M>::value` is `true` if `Abs<M>` is bigger than 1.
//
template <typename T, typename M, MagRepresentationOutcome Outcome>
struct IsAbsProbablyBiggerThanOneHelper : std::false_type {};

template <typename T, typename M>
struct IsAbsProbablyBiggerThanOneHelper<T, M, MagRepresentationOutcome::OK>
    : stdx::bool_constant<(get_value<T>(Abs<M>{}) >= T{1})> {};

template <typename T, typename M>
struct IsAbsProbablyBiggerThanOneHelper<T, M, MagRepresentationOutcome::ERR_CANNOT_FIT>
    : std::true_type {};

template <typename T, typename M>
struct IsAbsProbablyBiggerThanOne
    : IsAbsProbablyBiggerThanOneHelper<T, M, get_value_result<T>(Abs<M>{}).outcome> {};

// `UpperLimit<T, Limits>::value()` returns `Limits::upper()` (assumed to be of type `T`), unless
// `Limits` is `void`, in which case it means "no limit" and we return the highest possible value.
template <typename T, typename Limits>
struct UpperLimit {
    static constexpr T value() { return Limits::upper(); }
};
template <typename T>
struct UpperLimit<T, void> {
    static constexpr T value() { return std::numeric_limits<T>::max(); }
};

// `LowerLimit<T, Limits>::value()` returns `Limits::lower()` (assumed to be of type `T`), unless
// `Limits` is `void`, in which case it means "no limit" and we return the lowest possible value.
template <typename T, typename Limits>
struct LowerLimit {
    static constexpr T value() { return Limits::lower(); }
};
template <typename T>
struct LowerLimit<T, void> {
    static constexpr T value() { return std::numeric_limits<T>::lowest(); }
};

template <typename T>
constexpr T clamped_negate(T x) {
    if (Less{}(x, T{0}) && Less{}(x, -std::numeric_limits<T>::max())) {
        return std::numeric_limits<T>::max();
    }
    if (Greater{}(x, T{0}) && Greater{}(x, clamped_negate(std::numeric_limits<T>::lowest()))) {
        return std::numeric_limits<T>::lowest();
    }
    return -x;
}

// `LimitsFor<Op, Limits>` produces a type which can be the `Limits` argument for some other op.
template <typename Op, typename Limits>
struct LimitsFor {
    static constexpr RealPart<OpInput<Op>> lower() { return MinGood<Op, Limits>::value(); }
    static constexpr RealPart<OpInput<Op>> upper() { return MaxGood<Op, Limits>::value(); }
};

// Inherit from this struct to produce a compiler error in case we try to use a combination of types
// that isn't yet supported.
template <typename T>
struct OverflowBoundaryNotYetImplemented {
    struct NotYetImplemented {};
    static_assert(std::is_same<T, NotYetImplemented>::value,
                  "Overflow boundary not yet implemented for this type.");
};

// A type whose `::value()` function returns the higher of `std::numeric_limits<T>::lowest()`, or
// `LowerLimit<U, ULimit>` expressed in `T`.  Assumes that `U` is more expansive than `T`, so that
// we can cast everything to `U` to do the comparisons.
template <typename T, typename U, typename ULimit>
struct ValueOfSourceLowestUnlessDestLimitIsHigher {
    static constexpr T value() {
        constexpr auto LOWEST_T_IN_U = static_cast<U>(std::numeric_limits<T>::lowest());
        constexpr auto U_LIMIT = LowerLimit<U, ULimit>::value();
        return (LOWEST_T_IN_U <= U_LIMIT) ? static_cast<T>(U_LIMIT)
                                          : std::numeric_limits<T>::lowest();
    }
};

// A type whose `::value()` function returns the lower of `std::numeric_limits<T>::max()`, or
// `UpperLimit<U, ULimit>` expressed in `T`.  Assumes that `U` is more expansive than `T`, so that
// we can cast everything to `U` to do the comparisons.
template <typename T, typename U, typename ULimit>
struct ValueOfSourceHighestUnlessDestLimitIsLower {
    static constexpr T value() {
        constexpr auto HIGHEST_T_IN_U = static_cast<U>(std::numeric_limits<T>::max());
        constexpr auto U_LIMIT = UpperLimit<U, ULimit>::value();
        return (HIGHEST_T_IN_U >= U_LIMIT) ? static_cast<T>(U_LIMIT)
                                           : std::numeric_limits<T>::max();
    }
};

// A type whose `::value()` function returns the lowest value of `U`, expressed in `T`.
template <typename T, typename U = T, typename ULimit = void>
struct ValueOfLowestInDestination {
    static constexpr T value() { return static_cast<T>(LowerLimit<U, ULimit>::value()); }

    static_assert(static_cast<U>(value()) == LowerLimit<U, ULimit>::value(),
                  "This utility assumes lossless round trips");
};

// A type whose `::value()` function returns the highest value of `U`, expressed in `T`.
template <typename T, typename U = T, typename ULimit = void>
struct ValueOfHighestInDestination {
    static constexpr T value() { return static_cast<T>(UpperLimit<U, ULimit>::value()); }

    static_assert(static_cast<U>(value()) == UpperLimit<U, ULimit>::value(),
                  "This utility assumes lossless round trips");
};

// A type whose `::value()` function is capped at the highest value in `Float` (assumed to be a
// floating point type) that can be cast to `Int` (assumed to be an integral type).  We need to be
// really careful in how we express this, because max int values tend not to be nice powers of 2.
// Therefore, even though we can cast the `Int` max to `Float` successfully, casting back to `Int`
// will produce a compile time error because the closest representable integer in `Float` is
// slightly _higher_ than that max.
//
// On the implementation side, keep in mind that our library supports C++14, and most common
// floating point utilities (such as `std::nextafter`) are not `constexpr` compatible in C++14.
// Therefore, we need to use alternative strategies to explore the floating point type.  These are
// always evaluated at compile time, so we are not especially concerned about the efficiency: it
// should have no runtime effect at all, and we expect even the compile time impact --- which we
// measure regularly as we land commits --- to be too small to measure.
template <typename Float, typename Int, typename IntLimit>
struct ValueOfMaxFloatNotExceedingMaxInt {
    // The `Float` value where all mantissa bits are set to `1`, and the exponent is `0`.
    static constexpr Float max_mantissa() {
        constexpr Float ONE = Float{1};
        Float x = ONE;
        Float last = x;
        while (x + ONE > x) {
            last = x;
            x += x + ONE;
        }
        return last;
    }

    // Function to do the actual computation of the value.
    static constexpr Float compute_value() {
        constexpr Float LIMIT = static_cast<Float>(std::numeric_limits<Int>::max());
        constexpr Float MAX_MANTISSA = max_mantissa();

        return (LIMIT <= MAX_MANTISSA) ? LIMIT : double_first_until_second(MAX_MANTISSA, LIMIT);
    }

    static constexpr Float double_first_until_second(Float x, Float limit) {
        while (x + x < limit) {
            x += x;
        }
        return x;
    }

    // `value()` implementation simply computes the result _once_ (caching it), and then returns it.
    static constexpr Float value() {
        constexpr Float FLOAT_LIMIT = compute_value();
        constexpr Float EXPLICIT_LIMIT = static_cast<Float>(UpperLimit<Int, IntLimit>::value());
        constexpr Float RESULT = (FLOAT_LIMIT <= EXPLICIT_LIMIT) ? FLOAT_LIMIT : EXPLICIT_LIMIT;
        return RESULT;
    }
};

template <typename T, typename MagT, MagRepresentationOutcome Outcome>
struct MagHelper {
    static constexpr bool equal(const T &, const T &) { return false; }
    static constexpr T div(const T &, const T &) {
        static_assert(Outcome == MagRepresentationOutcome::ERR_CANNOT_FIT,
                      "Internal library error");

        // Dividing by a number that is too big to fit in the type implies a result of 0.
        return T{0};
    }
};

template <typename T, typename MagT>
struct MagHelper<T, MagT, MagRepresentationOutcome::OK> {
    static constexpr bool equal(const T &x, const T &value) { return x == value; }
    static constexpr T div(const T &a, const T &b) { return a / b; }
};

template <typename T, typename... BPs>
constexpr T divide_by_mag(const T &x, Magnitude<BPs...> m) {
    constexpr auto result = get_value_result<T>(m);
    return MagHelper<T, Magnitude<BPs...>, result.outcome>::div(x, result.value);
}

// Name reads as "lowest of (limits divided by value)".  Remember that the value can be negative, so
// we just take whichever limit is smaller _after_ dividing.
//
// This utility should only be called when `Abs<M>` is greater than 1.  (We can't easily check this
// condition, so we simply assume it; all callers are library-internal anyway, and we have unit
// tests.)  Since `Abs<M>` can be assumed to be greater than one, we know that dividing by `M` will
// shrink values, so we don't risk overflow.
template <typename T, typename M, typename Limits>
struct LowestOfLimitsDividedByValue {
    static constexpr T value() {
        constexpr auto RELEVANT_LIMIT =
            IsPositive<M>::value ? LowerLimit<T, Limits>::value() : UpperLimit<T, Limits>::value();

        return divide_by_mag(RELEVANT_LIMIT, M{});
    }
};

// Name reads as "clamp lowest of (limits times inverse value)".  First, remember that the value can
// be negative, so multiplying can sometimes switch the sign: we want whichever is smaller _after_
// that operation.  Next, if clamping is relevant, that means both that the type is bounded (so
// overflow is _possible_), and that `Abs<M>` is _smaller_ than 1 (implying that its _inverse_ can
// _grow_ values, so we risk overflow).  Therefore, we have to start from the bounds of the type,
// and back out the most extreme value for the limit that will _not_ overflow.
template <typename T, typename M, typename Limits>
struct ClampLowestOfLimitsTimesInverseValue {
    static constexpr T value() {
        constexpr auto ABS_DIVISOR = MagInverseT<Abs<M>>{};

        constexpr T RELEVANT_LIMIT = IsPositive<M>::value
                                         ? LowerLimit<T, Limits>::value()
                                         : clamped_negate(UpperLimit<T, Limits>::value());

        constexpr T RELEVANT_BOUND =
            IsPositive<M>::value
                ? divide_by_mag(std::numeric_limits<T>::lowest(), ABS_DIVISOR)
                : clamped_negate(divide_by_mag(std::numeric_limits<T>::max(), ABS_DIVISOR));
        constexpr bool SHOULD_CLAMP = RELEVANT_BOUND >= RELEVANT_LIMIT;

        // This value will be meaningless if `get_value_result<T>(ABS_DIVISOR).outcome` is not `OK`,
        // but we won't end up actually using the value in those cases.
        constexpr auto ABS_DIVISOR_AS_T = get_value_result<T>(ABS_DIVISOR).value;

        return SHOULD_CLAMP ? std::numeric_limits<T>::lowest() : RELEVANT_LIMIT * ABS_DIVISOR_AS_T;
    }
};

template <typename T, typename... BPs>
constexpr bool mag_representation_equals(const T &x, Magnitude<BPs...> m) {
    constexpr auto result = get_value_result<T>(m);
    return MagHelper<T, Magnitude<BPs...>, result.outcome>::equal(x, result.value);
}

// Name reads as "highest of (limits divided by value)".  Of course, normally this is just the
// higher limit divided by the value.  But if the value is negative, then the _lower limit_ will
// give the higher result _after_ we divide.
//
// Also, `Abs<M>` can be assumed to be greater than one, or else we would have been shunted into the
// clamping variant.  This means that dividing by `M` will shrink values, so we don't risk overflow.
template <typename T, typename M, typename Limits>
struct HighestOfLimitsDividedByValue {
    static constexpr T value() {
        if (mag_representation_equals(LowerLimit<T, Limits>::value(), M{})) {
            return T{1};
        }

        return (IsPositive<M>::value)
                   ? divide_by_mag(UpperLimit<T, Limits>::value(), M{})
                   : clamped_negate(divide_by_mag(LowerLimit<T, Limits>::value(), Abs<M>{}));
    }
};

// Name reads as "clamp highest of (limits times inverse value)".  See comments for
// `ClampLowestOfLimitsTimesInverseValue` for more details on the motivation and logic.
template <typename T, typename M, typename Limits>
struct ClampHighestOfLimitsTimesInverseValue {
    static constexpr T value() {
        constexpr auto ABS_DIVISOR = MagInverseT<Abs<M>>{};

        constexpr T RELEVANT_LIMIT = IsPositive<M>::value
                                         ? UpperLimit<T, Limits>::value()
                                         : clamped_negate(LowerLimit<T, Limits>::value());

        constexpr T RELEVANT_BOUND =
            IsPositive<M>::value
                ? divide_by_mag(std::numeric_limits<T>::max(), ABS_DIVISOR)
                : clamped_negate(divide_by_mag(std::numeric_limits<T>::lowest(), ABS_DIVISOR));
        constexpr bool SHOULD_CLAMP = RELEVANT_BOUND <= RELEVANT_LIMIT;

        // This value will be meaningless if `get_value_result<T>(ABS_DIVISOR).outcome` is not `OK`,
        // but we won't end up actually using the value in those cases.
        constexpr auto ABS_DIVISOR_AS_T = get_value_result<T>(ABS_DIVISOR).value;

        return SHOULD_CLAMP ? std::numeric_limits<T>::max() : RELEVANT_LIMIT * ABS_DIVISOR_AS_T;
    }
};

constexpr bool is_ok_or_err_cannot_fit(MagRepresentationOutcome outcome) {
    return outcome == MagRepresentationOutcome::OK ||
           outcome == MagRepresentationOutcome::ERR_CANNOT_FIT;
}

template <typename T, typename M>
struct IsCompatibleApartFromMaybeOverflow
    : stdx::bool_constant<is_ok_or_err_cannot_fit(get_value_result<T>(M{}).outcome)> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MinPossible<Op>` implementation.

// Why this lazy implementation, instead of using `std::numeric_limits` directly?  Simply because we
// need a _type_ whose _`value()` method_ returns the given value.  We already built that for more
// complicated use cases (it's called `LowestOfLimitsDividedByValue`), so we can just reuse it here.
template <typename Op>
struct MinPossibleImpl
    : stdx::type_identity<LowestOfLimitsDividedByValue<RealPart<OpInput<Op>>, Magnitude<>, void>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MaxPossible<Op>` implementation.

// See `MinPossibleImpl` comments above for explanation of this lazy approach.
template <typename Op>
struct MaxPossibleImpl
    : stdx::type_identity<HighestOfLimitsDividedByValue<RealPart<OpInput<Op>>, Magnitude<>, void>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` implementation.

//
// `MinGood<StaticCast<T, U>>` implementation cluster.
//
// See comment above for meanings of (N), (X), (A), etc.
//

// (N) -> (X) (placeholder)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (A) -> (N) (placeholder)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmeticToNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (S) -> (S)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToSigned
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueOfSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>,
                       ValueOfLowestInDestination<T, U, ULimit>> {};

// (S) -> (I)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToIntegral
    : std::conditional_t<std::is_unsigned<U>::value,
                         stdx::type_identity<ValueOfZero<T>>,
                         MinGoodImplForStaticCastFromSignedToSigned<T, U, ULimit>> {};

// (S) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToArithmetic
    : std::conditional_t<
          std::is_floating_point<U>::value,
          stdx::type_identity<ValueOfSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>>,
          MinGoodImplForStaticCastFromSignedToIntegral<T, U, ULimit>> {};

// (I) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromIntegralToArithmetic
    : std::conditional_t<
          std::is_unsigned<T>::value,
          stdx::type_identity<ValueOfSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>>,
          MinGoodImplForStaticCastFromSignedToArithmetic<T, U, ULimit>> {};

// (F) -> (F)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromFloatingPointToFloatingPoint
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueOfSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>,
                       ValueOfLowestInDestination<T, U, ULimit>> {};

// (F) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromFloatingPointToArithmetic
    : std::conditional_t<std::is_floating_point<U>::value,
                         MinGoodImplForStaticCastFromFloatingPointToFloatingPoint<T, U, ULimit>,
                         stdx::type_identity<ValueOfLowestInDestination<T, U, ULimit>>> {};

// (A) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmeticToArithmetic
    : std::conditional_t<std::is_integral<T>::value,
                         MinGoodImplForStaticCastFromIntegralToArithmetic<T, U, ULimit>,
                         MinGoodImplForStaticCastFromFloatingPointToArithmetic<T, U, ULimit>> {};

// (A) -> (X)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         MinGoodImplForStaticCastFromArithmeticToArithmetic<T, U, ULimit>,
                         MinGoodImplForStaticCastFromArithmeticToNonArithmetic<T, U, ULimit>> {};

// (X) -> (X)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastUsingRealPart
    : std::conditional_t<
          std::is_arithmetic<RealPart<T>>::value,
          MinGoodImplForStaticCastFromArithmetic<RealPart<T>, RealPart<U>, ULimit>,
          MinGoodImplForStaticCastFromNonArithmetic<RealPart<T>, RealPart<U>, ULimit>> {};

template <typename T, typename U, typename ULimit>
struct MinGoodImpl<StaticCast<T, U>, ULimit> : MinGoodImplForStaticCastUsingRealPart<T, U, ULimit> {
};

//
// `MaxGood<StaticCast<T, U>>` implementation cluster.
//
// See comment above for meanings of (N), (X), (A), etc.
//

// (N) -> (X) (placeholder)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (A) -> (N) (placeholder)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmeticToNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (I) -> (I)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromIntegralToIntegral
    : std::conditional<(static_cast<std::common_type_t<T, U>>(std::numeric_limits<T>::max()) <=
                        static_cast<std::common_type_t<T, U>>(std::numeric_limits<U>::max())),
                       ValueOfSourceHighestUnlessDestLimitIsLower<T, U, ULimit>,
                       ValueOfHighestInDestination<T, U, ULimit>> {};

// (I) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromIntegralToArithmetic
    : std::conditional_t<
          std::is_integral<U>::value,
          MaxGoodImplForStaticCastFromIntegralToIntegral<T, U, ULimit>,
          stdx::type_identity<ValueOfSourceHighestUnlessDestLimitIsLower<T, U, ULimit>>> {};

// (F) -> (F)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromFloatingPointToFloatingPoint
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueOfSourceHighestUnlessDestLimitIsLower<T, U, ULimit>,
                       ValueOfHighestInDestination<T, U, ULimit>> {};

// (F) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromFloatingPointToArithmetic
    : std::conditional_t<std::is_floating_point<U>::value,
                         MaxGoodImplForStaticCastFromFloatingPointToFloatingPoint<T, U, ULimit>,
                         stdx::type_identity<ValueOfMaxFloatNotExceedingMaxInt<T, U, ULimit>>> {};

// (A) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmeticToArithmetic
    : std::conditional_t<std::is_integral<T>::value,
                         MaxGoodImplForStaticCastFromIntegralToArithmetic<T, U, ULimit>,
                         MaxGoodImplForStaticCastFromFloatingPointToArithmetic<T, U, ULimit>> {};

// (A) -> (X)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         MaxGoodImplForStaticCastFromArithmeticToArithmetic<T, U, ULimit>,
                         MaxGoodImplForStaticCastFromArithmeticToNonArithmetic<T, U, ULimit>> {};

// (X) -> (X)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastUsingRealPart
    : std::conditional_t<
          std::is_arithmetic<RealPart<T>>::value,
          MaxGoodImplForStaticCastFromArithmetic<RealPart<T>, RealPart<U>, ULimit>,
          MaxGoodImplForStaticCastFromNonArithmetic<RealPart<T>, RealPart<U>, ULimit>> {};

template <typename T, typename U, typename ULimit>
struct MaxGoodImpl<StaticCast<T, U>, ULimit> : MaxGoodImplForStaticCastUsingRealPart<T, U, ULimit> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

template <typename T, typename M>
using IsClampingRequired =
    stdx::conjunction<stdx::negation<IsAbsProbablyBiggerThanOne<T, M>>, IsDefinitelyBounded<T>>;

//
// `MinGood<MultiplyTypeBy<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyCompatibleTypeBy
    : std::conditional<IsClampingRequired<T, M>::value,
                       ClampLowestOfLimitsTimesInverseValue<T, M, Limits>,
                       LowestOfLimitsDividedByValue<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByAssumingSigned
    : std::conditional_t<IsCompatibleApartFromMaybeOverflow<T, M>::value,
                         MinGoodImplForMultiplyCompatibleTypeBy<T, M, Limits>,
                         stdx::type_identity<ValueOfZero<T>>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByUsingRealPart
    : std::conditional_t<IsDefinitelyUnsigned<T>::value,
                         stdx::type_identity<ValueOfZero<T>>,
                         MinGoodImplForMultiplyTypeByAssumingSigned<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImpl<MultiplyTypeBy<T, M>, Limits>
    : MinGoodImplForMultiplyTypeByUsingRealPart<RealPart<T>, M, Limits> {};

//
// `MaxGood<MultiplyTypeBy<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyCompatibleTypeBy
    : std::conditional<IsClampingRequired<T, M>::value,
                       ClampHighestOfLimitsTimesInverseValue<T, M, Limits>,
                       HighestOfLimitsDividedByValue<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyTypeByAssumingSignedTypeOrPositiveFactor
    : std::conditional_t<IsCompatibleApartFromMaybeOverflow<T, M>::value,
                         MaxGoodImplForMultiplyCompatibleTypeBy<T, M, Limits>,
                         stdx::type_identity<ValueOfZero<T>>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyTypeByUsingRealPart
    : std::conditional_t<
          stdx::conjunction<IsDefinitelyUnsigned<T>, stdx::negation<IsPositive<M>>>::value,
          stdx::type_identity<ValueOfZero<T>>,
          MaxGoodImplForMultiplyTypeByAssumingSignedTypeOrPositiveFactor<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImpl<MultiplyTypeBy<T, M>, Limits>
    : MaxGoodImplForMultiplyTypeByUsingRealPart<RealPart<T>, M, Limits> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger<T, M>` implementation.

//
// `MinGood<DivideTypeByInteger<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MinGoodImplForDivideTypeByIntegerAssumingSigned
    : stdx::type_identity<ClampLowestOfLimitsTimesInverseValue<T, MagInverseT<M>, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForDivideTypeByIntegerUsingRealPart
    : std::conditional_t<IsDefinitelyUnsigned<T>::value,
                         stdx::type_identity<ValueOfZero<T>>,
                         MinGoodImplForDivideTypeByIntegerAssumingSigned<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImpl<DivideTypeByInteger<T, M>, Limits>
    : MinGoodImplForDivideTypeByIntegerUsingRealPart<RealPart<T>, M, Limits> {};

//
// `MaxGood<DivideTypeByInteger<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MaxGoodImplForDivideTypeByIntegerAssumingSignedTypeOrPositiveFactor
    : stdx::type_identity<ClampHighestOfLimitsTimesInverseValue<T, MagInverseT<M>, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForDivideTypeByIntegerUsingRealPart
    : std::conditional_t<
          stdx::conjunction<IsDefinitelyUnsigned<T>, stdx::negation<IsPositive<M>>>::value,
          stdx::type_identity<ValueOfZero<T>>,
          MaxGoodImplForDivideTypeByIntegerAssumingSignedTypeOrPositiveFactor<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImpl<DivideTypeByInteger<T, M>, Limits>
    : MaxGoodImplForDivideTypeByIntegerUsingRealPart<RealPart<T>, M, Limits> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<Ops...>` implementation.

//
// `MinGood<OpSequence<Ops...>>` implementation cluster.
//

template <typename OnlyOp, typename Limits>
struct MinGoodImpl<OpSequenceImpl<OnlyOp>, Limits> : MinGoodImpl<OnlyOp, Limits> {};

template <typename Op1, typename Op2, typename... Ops, typename Limits>
struct MinGoodImpl<OpSequenceImpl<Op1, Op2, Ops...>, Limits>
    : MinGoodImpl<Op1, LimitsFor<OpSequenceImpl<Op2, Ops...>, Limits>> {
    static_assert(std::is_same<OpOutput<Op1>, OpInput<Op2>>::value,
                  "Output of each op in sequence must match input of next op");
};

//
// `MaxGood<OpSequence<Ops...>>` implementation cluster.
//

template <typename OnlyOp, typename Limits>
struct MaxGoodImpl<OpSequenceImpl<OnlyOp>, Limits> : MaxGoodImpl<OnlyOp, Limits> {};

template <typename Op1, typename Op2, typename... Ops, typename Limits>
struct MaxGoodImpl<OpSequenceImpl<Op1, Op2, Ops...>, Limits>
    : MaxGoodImpl<Op1, LimitsFor<OpSequenceImpl<Op2, Ops...>, Limits>> {
    static_assert(std::is_same<OpOutput<Op1>, OpInput<Op2>>::value,
                  "Output of each op in sequence must match input of next op");
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanOverflowBelow<Op>` implementation.

template <typename Op>
struct CanOverflowBelow : stdx::bool_constant<(MinGood<Op>::value() > MinPossible<Op>::value())> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanOverflowAbove<Op>` implementation.

template <typename Op>
struct CanOverflowAbove : stdx::bool_constant<(MaxGood<Op>::value() < MaxPossible<Op>::value())> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MinValueChecker<Op>` and `MaxValueChecker<Op>` implementation.

template <typename Op, bool IsOverflowPossible>
struct MinValueCheckerImpl {
    static constexpr bool is_too_small(const OpInput<Op> &x) { return x < MinGood<Op>::value(); }
};
template <typename Op>
struct MinValueCheckerImpl<Op, false> {
    static constexpr bool is_too_small(const OpInput<Op> &) { return false; }
};
template <typename Op>
struct MinValueChecker : MinValueCheckerImpl<Op, CanOverflowBelow<Op>::value> {};

template <typename Op, bool IsOverflowPossible>
struct MaxValueCheckerImpl {
    static constexpr bool is_too_large(const OpInput<Op> &x) { return x > MaxGood<Op>::value(); }
};
template <typename Op>
struct MaxValueCheckerImpl<Op, false> {
    static constexpr bool is_too_large(const OpInput<Op> &) { return false; }
};
template <typename Op>
struct MaxValueChecker : MaxValueCheckerImpl<Op, CanOverflowAbove<Op>::value> {};

}  // namespace detail
}  // namespace au


namespace au {

// A "unit" is any type which has:
// - a member typedef `Dim`, which is a valid Dimension; and,
// - a member typedef `Mag`, which is a valid Magnitude.
//
// These can be accessed by traits `detail::DimT` and `detail::MagT`, respectively.  The detail
// namespace is meant to discourage _end users_ from accessing these concepts directly.  For
// example, we don't want end users to ask _which dimension_ a Unit has.  We'd rather they ask
// whether it is the _same_ as some other unit.  (It's also meaningful to ask whether it is
// dimensionless.)  And we certainly don't want end users to try to reason about "the magnitude" of
// a Unit, since this is totally meaningless; rather, we want them to ask about the _relative_
// magnitude with another unit of _the same dimension_.

// A UnitImpl is one easy way (although not the only way) to make a "Unit".
template <typename D, typename M = Magnitude<>>
struct UnitImpl {
    using Dim = D;
    using Mag = M;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Printable labels for units.

// A printable label to indicate the unit for human readers.
//
// To name a unit explicitly, specialize this class template for the unit's type.  For any unit not
// manually labeled, we provide a default label so that this template is always defined.
//
// Valid ways to define the label include a C-style const char array, or a StringConstant<N>.
template <typename Unit>
struct UnitLabel;

// A sizeof()-compatible API to get the label for a unit.
template <typename Unit>
constexpr const auto &unit_label(Unit = Unit{});

// Default label for a unit which hasn't been manually labeled yet.
//
// The dummy template parameter exists to enable `au` to be a header-only library.
template <typename T = void>
struct DefaultUnitLabel {
    static constexpr const char value[] = "[UNLABELED UNIT]";
};
template <typename T>
constexpr const char DefaultUnitLabel<T>::value[];

namespace detail {
// To preserve support for C++14, we need to _name the type_ of the member variable.  However, the
// `StringConstant` template produces a different type for every length, and that length depends on
// _both_ the prefix _and_ the unit label.
//
// To minimize friction as much as possible, we create this alias, which computes the type we need
// for a given unit and prefix-length.
//
// While clunky, this approach is at least robust against errors.  If the user supplies the wrong
// prefix length, it will fail to compile, because there is no assignment operator between
// `StringConstant` instances of different lengths.
template <std::size_t ExtensionStrlen, typename... Us>
using ExtendedLabel = StringConstant<concatenate(unit_label<Us>()...).size() + ExtensionStrlen>;
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type traits.

// Type trait to detect whether a type fulfills our definition of a "Unit".
template <typename T>
struct IsUnit : stdx::conjunction<IsValidPack<Dimension, detail::DimT<T>>,
                                  IsValidPack<Magnitude, detail::MagT<T>>> {};

// Type trait to detect whether two Units have the same Dimension.
template <typename... Us>
struct HasSameDimension;

// Type trait to detect whether two Units are quantity-equivalent.
//
// In this library, Units are "quantity-equivalent" exactly when they have the same Dimension and
// Magnitude.  Quantity instances whose Units are quantity-equivalent can be freely interconverted
// with each other.
template <typename U1, typename U2>
struct AreUnitsQuantityEquivalent;

// Type trait to detect whether two Units are point-equivalent.
//
// In this library, Units are "point-equivalent" exactly when they are quantity-equivalent (see
// above), _and_ they have the same origin.  QuantityPoint instances whose Units are
// point-equivalent can be freely interconverted with each other.
template <typename U1, typename U2>
struct AreUnitsPointEquivalent;

// Type trait to detect whether U is a Unit which is dimensionless.
template <typename U>
struct IsDimensionless : std::is_same<detail::DimT<U>, Dimension<>> {};

// Type trait to detect whether a Unit is "quantity-equivalent" to "the unitless unit".
//
// The "unitless unit" is a dimensionless unit of Magnitude 1 (as opposed to, say, other
// dimensionless units such as Percent).
template <typename U>
struct IsUnitlessUnit
    : stdx::conjunction<IsDimensionless<U>, std::is_same<detail::MagT<U>, Magnitude<>>> {};

// A Magnitude representing the ratio of two same-dimensioned units.
//
// Useful in doing unit conversions.
template <typename U1, typename U2>
struct UnitRatio : stdx::type_identity<MagQuotientT<detail::MagT<U1>, detail::MagT<U2>>> {
    static_assert(HasSameDimension<U1, U2>::value,
                  "Can only compute ratio of same-dimension units");
};
template <typename U1, typename U2>
using UnitRatioT = typename UnitRatio<U1, U2>::type;

// The sign of a unit: almost always `mag<1>()`, but `-mag<1>()` for "negative" units.
template <typename U>
using UnitSign = Sign<detail::MagT<U>>;

template <typename U>
struct AssociatedUnit : stdx::type_identity<U> {};
template <typename U>
using AssociatedUnitT = typename AssociatedUnit<U>::type;

template <typename U>
struct AssociatedUnitForPoints : stdx::type_identity<U> {};
template <typename U>
using AssociatedUnitForPointsT = typename AssociatedUnitForPoints<U>::type;

// `CommonUnitT`: the largest unit that evenly divides all input units.
//
// A specialization will only exist if all input types are units.
//
// If the inputs are units, but their Dimensions aren't all identical, then the request is
// ill-formed and we will produce a hard error.
//
// It may happen that the input units have the same Dimension, but there is no unit which evenly
// divides them (because some pair of input units has an irrational quotient).  In this case, there
// is no uniquely defined answer, but the program should still produce _some_ answer.  We guarantee
// that the result is associative, and symmetric under any reordering of the input units.  The
// specific implementation choice will be driven by convenience and simplicity.
template <typename... Us>
struct ComputeCommonUnit;
template <typename... Us>
using CommonUnitT = typename ComputeCommonUnit<Us...>::type;

// `CommonPointUnitT`: the largest-magnitude, highest-origin unit which is "common" to the units of
// a collection of `QuantityPoint` instances.
//
// The key goal to keep in mind is that for a `QuantityPoint` of any unit `U` in `Us...`, converting
// its value to the common point-unit should involve only:
//
//   - multiplication by a _positive integer_
//   - addition of a _non-negative integer_
//
// This helps us support the widest range of Rep types (in particular, unsigned integers).
//
// As with `CommonUnitT`, this isn't always possible: in particular, we can't do this for units with
// irrational relative magnitudes or origin displacements.  However, we still provide _some_ answer,
// which is consistent with the above policy whenever it's achievable, and produces reasonable
// results in all other cases.
//
// A specialization will only exist if the inputs are all units, and will exist but produce a hard
// error if any two input units have different Dimensions.  We also strive to keep the result
// associative, and symmetric under interchange of any inputs.
template <typename... Us>
struct ComputeCommonPointUnit;
template <typename... Us>
using CommonPointUnitT = typename ComputeCommonPointUnit<Us...>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type traits (instance-based interface).

// `is_unit(T)`: check whether this value is an instance of some Unit type.
template <typename T>
constexpr bool is_unit(T) {
    return IsUnit<T>::value;
}

// `fits_in_unit_slot(T)`: check whether this value is valid for a unit slot.
template <typename T>
constexpr bool fits_in_unit_slot(T) {
    return IsUnit<AssociatedUnitT<T>>::value;
}

// Check whether the units associated with these objects have the same Dimension.
template <typename... Us>
constexpr bool has_same_dimension(Us...) {
    return HasSameDimension<AssociatedUnitT<Us>...>::value;
}

// Check whether two Unit types are exactly quantity-equivalent.
template <typename U1, typename U2>
constexpr bool are_units_quantity_equivalent(U1, U2) {
    return AreUnitsQuantityEquivalent<AssociatedUnitT<U1>, AssociatedUnitT<U2>>::value;
}

// Check whether two Unit types are exactly point-equivalent.
template <typename U1, typename U2>
constexpr bool are_units_point_equivalent(U1, U2) {
    return AreUnitsPointEquivalent<AssociatedUnitT<U1>, AssociatedUnitT<U2>>::value;
}

// Check whether this value is an instance of a dimensionless Unit.
template <typename U>
constexpr bool is_dimensionless(U) {
    return IsDimensionless<AssociatedUnitT<U>>::value;
}

// Type trait to detect whether a Unit is "the unitless unit".
template <typename U>
constexpr bool is_unitless_unit(U) {
    return IsUnitlessUnit<AssociatedUnitT<U>>::value;
}

// A Magnitude representing the ratio of two same-dimensioned units.
//
// Useful in doing unit conversions.
template <typename U1, typename U2>
constexpr UnitRatioT<AssociatedUnitT<U1>, AssociatedUnitT<U2>> unit_ratio(U1, U2) {
    return {};
}

// Type trait for the sign of a Unit (represented as a Magnitude).
template <typename U>
constexpr UnitSign<AssociatedUnitT<U>> unit_sign(U) {
    return {};
}

template <typename U>
constexpr auto associated_unit(U) {
    return AssociatedUnitT<U>{};
}

template <typename U>
constexpr auto associated_unit_for_points(U) {
    return AssociatedUnitForPointsT<U>{};
}

template <typename... Us>
constexpr auto common_unit(Us...) {
    return CommonUnitT<AssociatedUnitT<Us>...>{};
}

template <typename... Us>
constexpr auto common_point_unit(Us...) {
    return CommonPointUnitT<AssociatedUnitForPointsT<Us>...>{};
}

template <template <class> class Utility, typename... Us>
constexpr auto make_common(Utility<Us>...) {
    return Utility<CommonUnitT<AssociatedUnitT<Us>...>>{};
}

template <template <class> class Utility, typename... Us>
constexpr auto make_common_point(Utility<Us>...) {
    return Utility<CommonPointUnitT<AssociatedUnitForPointsT<Us>...>>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unit arithmetic traits: products, powers, and derived operations.

// A Unit, scaled by some factor.
//
// Retains all of the member variables and typedefs of the existing Unit, except that the
// `detail::MagT` trait is appropriately scaled, and the unit label is erased.
//
// NOTE: This strategy will lead to long chains of inherited types when we scale a unit multiple
// times (say, going from Meters -> Centi<Meters> -> Inches -> Feet -> Miles).  What's more, each
// element in this chain yields _two_ types: one for the named opaque typedef (e.g., `Feet`), and
// one for the anonymous scaled unit (e.g., `Inches * mag<12>()`).  We explicitly assume that this
// will not cause any performance problems, because these should all be empty classes anyway.  If we
// find out we're mistaken, we'll need to revisit this idea.
template <typename Unit, typename ScaleFactor>
struct ScaledUnit;

template <typename Unit, typename ScaleFactor>
struct ComputeScaledUnitImpl : stdx::type_identity<ScaledUnit<Unit, ScaleFactor>> {};
template <typename Unit, typename ScaleFactor>
using ComputeScaledUnit = typename ComputeScaledUnitImpl<Unit, ScaleFactor>::type;
template <typename Unit, typename ScaleFactor, typename OldScaleFactor>
struct ComputeScaledUnitImpl<ScaledUnit<Unit, OldScaleFactor>, ScaleFactor>
    : ComputeScaledUnitImpl<Unit, MagProductT<OldScaleFactor, ScaleFactor>> {};
template <typename Unit>
struct ComputeScaledUnitImpl<Unit, Magnitude<>> : stdx::type_identity<Unit> {};
// Disambiguating specialization:
template <typename Unit, typename OldScaleFactor>
struct ComputeScaledUnitImpl<ScaledUnit<Unit, OldScaleFactor>, Magnitude<>>
    : stdx::type_identity<ScaledUnit<Unit, OldScaleFactor>> {};

template <typename Unit, typename ScaleFactor>
struct ScaledUnit : Unit {
    static_assert(IsValidPack<Magnitude, ScaleFactor>::value,
                  "Can only scale by a Magnitude<...> type");
    using Dim = detail::DimT<Unit>;
    using Mag = MagProductT<detail::MagT<Unit>, ScaleFactor>;
};

// Type template to hold the product of powers of Units.
template <typename... UnitPows>
struct UnitProduct {
    using Dim = DimProductT<detail::DimT<UnitPows>...>;
    using Mag = MagProductT<detail::MagT<UnitPows>...>;
};

// Helper to make a canonicalized product of units.
//
// On the input side, we treat every input unit as a UnitProduct.  Once we get our final result, we
// simplify it using `UnpackIfSoloT`.  (The motivation is that we don't want to return, say,
// `UnitProduct<Meters>`; we'd rather just return `Meters`.)
template <typename... UnitPows>
using UnitProductT =
    UnpackIfSoloT<UnitProduct, PackProductT<UnitProduct, AsPackT<UnitProduct, UnitPows>...>>;

// Raise a Unit to a (possibly rational) Power.
template <typename U, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using UnitPowerT =
    UnpackIfSoloT<UnitProduct, PackPowerT<UnitProduct, AsPackT<UnitProduct, U>, ExpNum, ExpDen>>;

// Compute the inverse of a unit.
template <typename U>
using UnitInverseT = UnitPowerT<U, -1>;

// Compute the quotient of two units.
template <typename U1, typename U2>
using UnitQuotientT = UnitProductT<U1, UnitInverseT<U2>>;

template <typename... Us>
constexpr bool is_forward_declared_unit_valid(ForwardDeclareUnitProduct<Us...>) {
    return std::is_same<typename ForwardDeclareUnitProduct<Us...>::unit_type,
                        UnitProductT<Us...>>::value;
}

template <typename U, std::intmax_t ExpNum, std::intmax_t ExpDen>
constexpr bool is_forward_declared_unit_valid(ForwardDeclareUnitPow<U, ExpNum, ExpDen>) {
    return std::is_same<typename ForwardDeclareUnitPow<U, ExpNum, ExpDen>::unit_type,
                        UnitPowerT<U, ExpNum, ExpDen>>::value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unit arithmetic on _instances_ of Units and/or Magnitudes.

// Scale this Unit by multiplying by a Magnitude.
template <typename U, typename = std::enable_if_t<IsUnit<U>::value>, typename... BPs>
constexpr ComputeScaledUnit<U, Magnitude<BPs...>> operator*(U, Magnitude<BPs...>) {
    return {};
}

// Scale this Unit by dividing by a Magnitude.
template <typename U, typename = std::enable_if_t<IsUnit<U>::value>, typename... BPs>
constexpr ComputeScaledUnit<U, MagInverseT<Magnitude<BPs...>>> operator/(U, Magnitude<BPs...>) {
    return {};
}

// Compute the product of two unit instances.
template <typename U1,
          typename U2,
          typename = std::enable_if_t<stdx::conjunction<IsUnit<U1>, IsUnit<U2>>::value>>
constexpr UnitProductT<U1, U2> operator*(U1, U2) {
    return {};
}

// Compute the quotient of two unit instances.
template <typename U1,
          typename U2,
          typename = std::enable_if_t<stdx::conjunction<IsUnit<U1>, IsUnit<U2>>::value>>
constexpr UnitQuotientT<U1, U2> operator/(U1, U2) {
    return {};
}

// Raise a Unit to an integral power.
template <std::intmax_t Exp, typename U, typename = std::enable_if_t<IsUnit<U>::value>>
constexpr UnitPowerT<U, Exp> pow(U) {
    return {};
}

// Take the Root (of some integral degree) of a Unit.
template <std::intmax_t Deg, typename U, typename = std::enable_if_t<IsUnit<U>::value>>
constexpr UnitPowerT<U, 1, Deg> root(U) {
    return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous interfaces.

// An instance which lets us refer to a unit by its singular name.
//
// To use this, whenever you define a new unit (e.g., `struct Meters`), follow it up with a line
// like the following:
//
//     constexpr auto meter = SingularNameFor<Meters>{};
//
// This is just to help us write grammatically natural code.  Examples:
//
//   - `torque.in(newton * meters)`
//                ^^^^^^
//   - `speed.as(miles / hour)`
//                       ^^^^
template <typename Unit>
struct SingularNameFor {

    // Multiplying `SingularNameFor` instances enables compound units such as:
    // `radians / (meter * second)`.
    template <typename OtherUnit>
    constexpr auto operator*(SingularNameFor<OtherUnit>) const {
        return SingularNameFor<UnitProductT<Unit, OtherUnit>>{};
    }
};

// Support `SingularNameFor` in (quantity) unit slots.
template <typename U>
struct AssociatedUnit<SingularNameFor<U>> : stdx::type_identity<U> {};

template <int Exp, typename Unit>
constexpr auto pow(SingularNameFor<Unit>) {
    return SingularNameFor<UnitPowerT<Unit, Exp>>{};
}

//
// Specialize `UnitOrderTiebreaker<YourCustomUnit>` as below, but with a different constant, in
// order to reduce the chance of hitting "distinct input types compare equal" errors.
//
template <typename U>
struct UnitOrderTiebreaker;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Origin displacement implementation.

namespace detail {
// Callable type trait for the default origin of a unit: choose ZERO.
struct ZeroValue {
    static constexpr Zero value() { return Zero{}; }
};

template <typename U>
using OriginMemberType = decltype(U::origin());

// If any unit U has an explicit origin member, then treat that as its origin.
template <typename U>
struct OriginMember {
    static constexpr const OriginMemberType<U> value() { return U::origin(); }
};

template <typename U>
struct OriginOf : std::conditional_t<stdx::experimental::is_detected<OriginMemberType, U>::value,
                                     OriginMember<U>,
                                     ZeroValue> {};

template <typename T, typename U>
struct ValueDifference {
    static constexpr auto value() { return T::value() - U::value(); }
};
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ValueDisplacementMagnitude` utility.
namespace detail {

// `ValueDisplacementMagnitude<T1, T2>` is a type that can be instantiated, and is either a
// `Magnitude` type or else `Zero`.  It represents the magnitude of the unit that takes us from
// `T1::value()` to `T2::value()` (and is `Zero` if and only if these values are equal).
//
// This is fully encapsulated inside of the `detail` namespace because we don't want end users
// reasoning in terms of "the magnitude" of a unit.  This concept makes no sense generally.
// However, it's useful to us internally, because it helps us compute the largest possible magnitude
// of a common point unit.  Being fully encapsulated, we ourselves can be careful not to misuse it.
enum class AreValuesEqual { YES, NO };
template <typename U1, typename U2, AreValuesEqual>
struct ValueDisplacementMagnitudeImpl;
template <typename U1, typename U2>
using ValueDisplacementMagnitude = typename ValueDisplacementMagnitudeImpl<
    U1,
    U2,
    (U1::value() == U2::value() ? AreValuesEqual::YES : AreValuesEqual::NO)>::type;

// Equal values case.
template <typename U1, typename U2>
struct ValueDisplacementMagnitudeImpl<U1, U2, AreValuesEqual::YES> : stdx::type_identity<Zero> {
    static_assert(U1::value() == U2::value(), "Mismatched instantiation (internal library error)");
};

// Prep for handling unequal values: it's useful to be able to turn a signed integer into a
// Magnitude.
//
// The `bool` template parameter in the `MagSign` interface has poor callsite readability, but it
// doesn't matter because we're only using it right here.
template <bool IsNeg>
struct MagSign : stdx::type_identity<Magnitude<>> {};
template <>
struct MagSign<true> : stdx::type_identity<Magnitude<Negative>> {};
template <std::intmax_t N>
constexpr auto signed_mag() {
    constexpr auto sign = typename MagSign<(N < 0)>::type{};
    return sign * mag<static_cast<std::size_t>(N < 0 ? (-N) : N)>();
}

// Unequal values case implementation: scale up the magnitude of the diff's _unit_ by the diff's
// _value in_ that unit.
template <typename U1, typename U2>
struct ValueDisplacementMagnitudeImpl<U1, U2, AreValuesEqual::NO> {
    static_assert(U1::value() != U2::value(), "Mismatched instantiation (internal library error)");
    static constexpr auto mag() {
        constexpr auto diff = U2::value() - U1::value();
        using D = typename decltype(diff)::Unit;
        return MagT<D>{} * signed_mag<diff.in(D{})>();
    }
    using type = decltype(mag());
};

}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// `HasSameDimension` implementation.

template <typename U>
struct HasSameDimension<U> : std::true_type {};

template <typename U1, typename U2, typename... Us>
struct HasSameDimension<U1, U2, Us...>
    : stdx::conjunction<std::is_same<detail::DimT<U1>, detail::DimT<U2>>,
                        HasSameDimension<U2, Us...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreUnitsQuantityEquivalent` implementation.

namespace detail {
// We don't want to advertise this utility, because "same magnitude" is meaningless unless the units
// also have the same dimension.
template <typename U1, typename U2>
struct HasSameMagnitude : std::is_same<detail::MagT<U1>, detail::MagT<U2>> {};
}  // namespace detail

template <typename U1, typename U2>
struct AreUnitsQuantityEquivalent
    : stdx::conjunction<HasSameDimension<U1, U2>, detail::HasSameMagnitude<U1, U2>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreUnitsPointEquivalent` implementation.

namespace detail {
template <typename U1, typename U2>
struct HasSameOrigin : stdx::bool_constant<(OriginOf<U1>::value() == OriginOf<U2>::value())> {};
}  // namespace detail

template <typename U1, typename U2>
struct AreUnitsPointEquivalent
    : stdx::conjunction<AreUnitsQuantityEquivalent<U1, U2>, detail::HasSameOrigin<U1, U2>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonUnit` helper implementation.

// This exists to be the "named type" for the common unit of a bunch of input units.
//
// To be well-formed, the units must be listed in the same order every time.  End users cannot be
// responsible for this; thus, they should never name this type directly.  Rather, they should name
// the `CommonUnitT` alias, which will handle the canonicalization.
template <typename... Us>
struct CommonUnit {
    static_assert(AreElementsInOrder<CommonUnit, CommonUnit<Us...>>::value,
                  "Elements must be listed in ascending order");
    static_assert(HasSameDimension<Us...>::value,
                  "Common unit only meaningful if units have same dimension");

    using Dim = CommonDimensionT<detail::DimT<Us>...>;
    using Mag = CommonMagnitudeT<detail::MagT<Us>...>;
};

template <typename A, typename B>
struct InOrderFor<CommonUnit, A, B> : InOrderFor<UnitProduct, A, B> {};

template <typename... Us>
struct UnitList {};
template <typename A, typename B>
struct InOrderFor<UnitList, A, B> : InOrderFor<UnitProduct, A, B> {};

namespace detail {
// This machinery searches a unit list for one that "matches" a target unit.
//
// If none do, it will produce the target unit.

// Generic template.
template <template <class, class> class Matcher,
          typename TargetUnit,
          typename UnitListT = TargetUnit>
struct FirstMatchingUnit;

// Base case for an empty list: the target unit is the best match.
template <template <class, class> class Matcher,
          typename TargetUnit,
          template <class...>
          class List>
struct FirstMatchingUnit<Matcher, TargetUnit, List<>> : stdx::type_identity<TargetUnit> {};

// Recursive case for a non-empty list: return head if it matches, or else recurse.
template <template <class, class> class Matcher,
          typename TargetUnit,
          template <class...>
          class List,
          typename H,
          typename... Ts>
struct FirstMatchingUnit<Matcher, TargetUnit, List<H, Ts...>>
    : std::conditional_t<Matcher<TargetUnit, H>::value,
                         stdx::type_identity<H>,
                         FirstMatchingUnit<Matcher, TargetUnit, List<Ts...>>> {};

// A "redundant" unit, among a list of units, is one that is an exact integer multiple of another.
//
// If two units are identical, then each is redundant with the other.
//
// If two units are distinct, but quantity-equivalent, then the unit that comes later in the
// standard unit ordering (i.e., `InOrderFor<Pack, ...>`) is the redundant one.
template <typename Pack>
struct EliminateRedundantUnitsImpl;
template <typename Pack>
using EliminateRedundantUnits = typename EliminateRedundantUnitsImpl<Pack>::type;

// Base case: no units to eliminate.
template <template <class...> class Pack>
struct EliminateRedundantUnitsImpl<Pack<>> : stdx::type_identity<Pack<>> {};

// Helper for recursive case.
template <template <class...> class Pack, typename U1, typename U2>
struct IsFirstUnitRedundant
    : std::conditional_t<std::is_same<U1, U2>::value,
                         std::true_type,
                         std::conditional_t<AreUnitsQuantityEquivalent<U1, U2>::value,
                                            InOrderFor<Pack, U2, U1>,
                                            stdx::conjunction<IsInteger<UnitRatioT<U1, U2>>,
                                                              IsPositive<UnitRatioT<U1, U2>>>>> {};

// Recursive case: eliminate first unit if it is redundant; else, keep it and eliminate any later
// units that are redundant with it.
template <template <class...> class Pack, typename H, typename... Ts>
struct EliminateRedundantUnitsImpl<Pack<H, Ts...>>
    : std::conditional<

          // If `H` is redundant with _any later unit_, simply omit it.
          stdx::disjunction<IsFirstUnitRedundant<Pack, H, Ts>...>::value,
          EliminateRedundantUnits<Pack<Ts...>>,

          // Otherwise, we know we'll need to keep `H`, so we prepend it to the remaining result.
          //
          // To get that result, we first replace any units _that `H` makes redundant_ with `void`.
          // Then, we drop all `void`, before finally recursively eliminating any units that are
          // redundant among those that remain.
          PrependT<
              EliminateRedundantUnits<DropAll<
                  void,

                  // `Pack<Ts...>`, but with redundant-with-`H` units replaced by `void`:
                  Pack<std::conditional_t<IsFirstUnitRedundant<Pack, Ts, H>::value, void, Ts>...>>>,

              H>> {};

template <typename U, typename... Us>
struct AllUnitsQuantityEquivalent : stdx::conjunction<AreUnitsQuantityEquivalent<U, Us>...> {};

template <typename... Us>
struct CommonUnitLabelImpl {
    static_assert(sizeof...(Us) > 1u, "Common unit label only makes sense for multiple units");
    static_assert(AllUnitsQuantityEquivalent<Us...>::value,
                  "Must pre-reduce units before constructing common-unit label");

    using LabelT = ExtendedLabel<7u + 2u * (sizeof...(Us) - 1u), Us...>;
    static constexpr LabelT value = concatenate("EQUIV{", join_by(", ", unit_label<Us>()...), "}");
};
template <typename... Us>
constexpr typename CommonUnitLabelImpl<Us...>::LabelT CommonUnitLabelImpl<Us...>::value;

template <typename U>
struct CommonUnitLabelImpl<U> : UnitLabel<U> {};

template <typename U>
struct UnscaledUnitImpl : stdx::type_identity<U> {};
template <typename U, typename M>
struct UnscaledUnitImpl<ScaledUnit<U, M>> : stdx::type_identity<U> {};
template <typename U>
using UnscaledUnit = typename UnscaledUnitImpl<U>::type;

template <typename U>
struct DistinctUnscaledUnitsImpl : stdx::type_identity<UnitList<UnscaledUnit<U>>> {};
template <typename U>
using DistinctUnscaledUnits = typename DistinctUnscaledUnitsImpl<U>::type;
template <typename... Us>
struct DistinctUnscaledUnitsImpl<CommonUnit<Us...>>
    : stdx::type_identity<FlatDedupedTypeListT<UnitList, UnscaledUnit<Us>...>> {};

template <typename U, typename DistinctUnits>
struct SimplifyIfOnlyOneUnscaledUnitImpl;
template <typename U>
using SimplifyIfOnlyOneUnscaledUnit =
    typename SimplifyIfOnlyOneUnscaledUnitImpl<U, DistinctUnscaledUnits<U>>::type;
template <>
struct SimplifyIfOnlyOneUnscaledUnitImpl<Zero, UnitList<Zero>> : stdx::type_identity<Zero> {};
template <typename U, typename SoleUnscaledUnit>
struct SimplifyIfOnlyOneUnscaledUnitImpl<U, UnitList<SoleUnscaledUnit>>
    : stdx::type_identity<decltype(SoleUnscaledUnit{} * UnitRatioT<U, SoleUnscaledUnit>{})> {};
template <typename U, typename... Us>
struct SimplifyIfOnlyOneUnscaledUnitImpl<U, UnitList<Us...>> : stdx::type_identity<U> {};

// Explicit specialization to short-circuit `FirstMatchingUnit` machinery for `Zero`.
template <>
struct FirstMatchingUnit<AreUnitsQuantityEquivalent, Zero, Zero> : stdx::type_identity<Zero> {};

template <typename U>
struct ReplaceCommonPointUnitWithCommonUnitImpl : stdx::type_identity<U> {};
template <typename U>
using ReplaceCommonPointUnitWithCommonUnit =
    typename ReplaceCommonPointUnitWithCommonUnitImpl<U>::type;
}  // namespace detail

template <typename A, typename B>
struct InOrderFor<detail::CommonUnitLabelImpl, A, B> : InOrderFor<UnitProduct, A, B> {};

template <typename... Us>
using CommonUnitLabel = FlatDedupedTypeListT<detail::CommonUnitLabelImpl, Us...>;

template <typename... Us>
struct ComputeCommonUnitImpl
    : stdx::type_identity<detail::EliminateRedundantUnits<
          FlatDedupedTypeListT<CommonUnit, detail::ReplaceCommonPointUnitWithCommonUnit<Us>...>>> {
};
template <>
struct ComputeCommonUnitImpl<> : stdx::type_identity<Zero> {};

template <typename T>
struct IsNonzero : stdx::negation<std::is_same<T, Zero>> {};

template <typename... Us>
struct ComputeCommonUnit
    : stdx::type_identity<detail::SimplifyIfOnlyOneUnscaledUnit<typename detail::FirstMatchingUnit<
          AreUnitsQuantityEquivalent,
          typename detail::IncludeInPackIf<IsNonzero, ComputeCommonUnitImpl, Us...>::type>::type>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonPointUnitT` helper implementation.

namespace detail {

// For equal origins expressed in different units, we can compare the values in their native units
// as a way to decide which unit has the biggest Magnitude.  Bigger Magnitude, smaller value.  (We
// could have tried to assess the Magnitude directly, but this method works better with Zero, and we
// will often encounter Zero when dealing with origins.)
//
// This will be used as a tiebreaker for different origin types.  (For example, the origin of
// Celsius may be represented as Centikelvins or Millikelvins, and we want Centikelvins to "win"
// because it will result in smaller multiplications.)
template <typename T>
constexpr auto get_value_in_native_unit(const T &t) {
    return t.in(T::unit);
}

// If the input is "0", then its value _in any unit_ is 0.
constexpr auto get_value_in_native_unit(const Zero &) { return 0; }

// The common origin of a collection of units is the smallest origin.
//
// We try to keep the result symmetric under reordering of the inputs.
template <typename... Us>
struct CommonOrigin;

template <typename U>
struct CommonOrigin<U> : OriginOf<U> {};

template <typename Head, typename... Tail>
struct CommonOrigin<Head, Tail...> :
    // If the new value is strictly less than the common-so-far, then it wins, so choose it.
    std::conditional_t<
        (OriginOf<Head>::value() < CommonOrigin<Tail...>::value()),
        OriginOf<Head>,

        // If the new value is strictly greater than the common-so-far, it's worse, so skip it.
        std::conditional_t<
            (OriginOf<Head>::value() > CommonOrigin<Tail...>::value()),
            CommonOrigin<Tail...>,

            // If we're here, the origins represent the same _quantity_, but may be expressed in
            // different _units_.  We'd like the biggest unit, since it leads to the smallest
            // multiplications.  For equal quantities, "biggest unit" is equivalent to "smallest
            // value", so we compare the values.
            std::conditional_t<(get_value_in_native_unit(OriginOf<Head>::value()) <
                                get_value_in_native_unit(CommonOrigin<Tail...>::value())),
                               OriginOf<Head>,
                               CommonOrigin<Tail...>>>> {};

// `UnitOfLowestOrigin<Us...>` is any unit among `Us` whose origin equals `CommonOrigin<Us...>`.
template <typename... Us>
struct UnitOfLowestOriginImpl;
template <typename... Us>
using UnitOfLowestOrigin = typename SortAs<UnitProduct, UnitOfLowestOriginImpl<Us...>>::type;
template <typename U>
struct UnitOfLowestOriginImpl<U> : stdx::type_identity<U> {};
template <typename U, typename U1, typename... Us>
struct UnitOfLowestOriginImpl<U, U1, Us...>
    : std::conditional<(OriginOf<U>::value() == CommonOrigin<U, U1, Us...>::value()),
                       U,
                       UnitOfLowestOrigin<U1, Us...>> {};

template <typename U1, typename U2>
struct OriginDisplacementUnit {
    static_assert(OriginOf<U1>::value() != OriginOf<U2>::value(),
                  "OriginDisplacementUnit must be an actual unit, so it must be nonzero.");

    using Dim = CommonDimensionT<DimT<U1>, DimT<U2>>;
    using Mag = ValueDisplacementMagnitude<OriginOf<U1>, OriginOf<U2>>;
};

// `ComputeOriginDisplacementUnit<U1, U2>` produces an ad hoc unit equal to the displacement from
// the origin of `U1` to the origin of `U2`.  If `U1` and `U2` have equal origins, then it is
// `Zero`.  Otherwise, it will be `OriginDisplacementUnit<U1, U2>`.
template <typename U1, typename U2>
using ComputeOriginDisplacementUnit =
    std::conditional_t<(OriginOf<U1>::value() == OriginOf<U2>::value()),
                       Zero,
                       OriginDisplacementUnit<U1, U2>>;

template <typename U1, typename U2>
constexpr auto origin_displacement_unit(U1, U2) {
    return ComputeOriginDisplacementUnit<AssociatedUnitForPointsT<U1>,
                                         AssociatedUnitForPointsT<U2>>{};
}

// MagTypeT<T> gives some measure of the size of the unit for this "quantity-alike" type.
//
// Zero acts like a quantity in this context, and we treat it as if its unit's Magnitude is Zero.
// This is specifically done for the `CommonPointUnit` implementation; there is no guarantee that
template <typename QuantityOrZero>
struct MagType : stdx::type_identity<MagT<typename QuantityOrZero::Unit>> {};
template <typename QuantityOrZero>
using MagTypeT = typename MagType<stdx::remove_cvref_t<QuantityOrZero>>::type;
template <>
struct MagType<Zero> : stdx::type_identity<Zero> {};

}  // namespace detail

template <typename U1, typename U2>
struct UnitLabel<detail::OriginDisplacementUnit<U1, U2>> {
    using LabelT = detail::ExtendedLabel<15u, U1, U2>;
    static constexpr LabelT value =
        detail::concatenate("(@(0 ", UnitLabel<U2>::value, ") - @(0 ", UnitLabel<U1>::value, "))");
};
template <typename U1, typename U2>
constexpr typename UnitLabel<detail::OriginDisplacementUnit<U1, U2>>::LabelT
    UnitLabel<detail::OriginDisplacementUnit<U1, U2>>::value;

// This exists to be the "named type" for the common unit of a bunch of input units.
//
// To be well-formed, the units must be listed in the same order every time.  End users cannot be
// responsible for this; thus, they should never name this type directly.  Rather, they should name
// the `CommonPointUnitT` alias, which will handle the canonicalization.
template <typename... Us>
using CommonAmongUnitsAndOriginDisplacements =
    CommonUnitT<Us...,
                detail::ComputeOriginDisplacementUnit<detail::UnitOfLowestOrigin<Us...>, Us>...>;
template <typename... Us>
struct CommonPointUnit : CommonAmongUnitsAndOriginDisplacements<Us...> {
    static_assert(AreElementsInOrder<CommonPointUnit, CommonPointUnit<Us...>>::value,
                  "Elements must be listed in ascending order");
    static_assert(HasSameDimension<Us...>::value,
                  "Common unit only meaningful if units have same dimension");

    static constexpr auto origin() { return detail::CommonOrigin<Us...>::value(); }
};

namespace detail {
template <typename... Us>
struct ReplaceCommonPointUnitWithCommonUnitImpl<CommonPointUnit<Us...>>
    : stdx::type_identity<CommonAmongUnitsAndOriginDisplacements<Us...>> {};
}  // namespace detail

template <typename A, typename B>
struct InOrderFor<CommonPointUnit, A, B> : InOrderFor<UnitProduct, A, B> {};

template <typename... Us>
using ComputeCommonPointUnitImpl = FlatDedupedTypeListT<CommonPointUnit, Us...>;

template <typename... Us>
struct ComputeCommonPointUnit
    : detail::FirstMatchingUnit<AreUnitsPointEquivalent, ComputeCommonPointUnitImpl<Us...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnitLabel` implementation.

namespace detail {

template <typename Unit>
using HasLabel = decltype(Unit::label);

// Implementation for units that do have a label.
template <typename T>
struct LabelRef {
    static constexpr auto &value = T::label;
};

// Utility for labeling a unit raised to some power.
template <typename ExpLabel, typename Unit>
struct PowerLabeler {
    using LabelT = ExtendedLabel<ExpLabel::value().size() + 1, Unit>;
    static constexpr LabelT value = join_by("^", unit_label<Unit>(), ExpLabel::value());
};
template <typename ExpLabeler, typename Unit>
constexpr typename PowerLabeler<ExpLabeler, Unit>::LabelT PowerLabeler<ExpLabeler, Unit>::value;

// Utility to generate the exponent label for a Pow.
template <std::intmax_t N>
struct ExpLabelForPow {
    static constexpr auto value() { return parens_if<(N < 0)>(IToA<N>::value); }
};

// Utility to generate the exponent label for a RatioPow.
template <std::intmax_t N, std::intmax_t D>
struct ExpLabelForRatioPow {
    static constexpr auto value() {
        return concatenate("(", IToA<N>::value, "/", IToA<D>::value, ")");
    }
};

enum class ParensPolicy {
    OMIT,
    ADD_IF_MULITPLE,
};

template <typename T, ParensPolicy Policy = ParensPolicy::ADD_IF_MULITPLE>
struct CompoundLabel;
template <typename... Us, ParensPolicy Policy>
struct CompoundLabel<UnitProduct<Us...>, Policy> {
    static constexpr auto value() {
        constexpr bool add_parens =
            (Policy == ParensPolicy::ADD_IF_MULITPLE) && (sizeof...(Us) > 1);
        return parens_if<add_parens>(join_by(" * ", unit_label<Us>()...));
    }
};

// Labeler for a quotient of products-of-Units: general case.
//
// The dummy template parameter exists to enable `au` to be a header-only library.
template <typename N, typename D, typename T = void>
struct QuotientLabeler {
    using LabelT =
        StringConstant<CompoundLabel<N>::value().size() + CompoundLabel<D>::value().size() + 3>;
    static constexpr LabelT value =
        join_by(" / ", CompoundLabel<N>::value(), CompoundLabel<D>::value());
};
template <typename N, typename D, typename T>
constexpr typename QuotientLabeler<N, D, T>::LabelT QuotientLabeler<N, D, T>::value;

// Special case for denominator of 1.
template <typename N, typename T>
struct QuotientLabeler<N, UnitProduct<>, T> {
    using LabelT = StringConstant<CompoundLabel<N, ParensPolicy::OMIT>::value().size()>;
    static constexpr LabelT value = CompoundLabel<N, ParensPolicy::OMIT>::value();
};
template <typename N, typename T>
constexpr typename QuotientLabeler<N, UnitProduct<>, T>::LabelT
    QuotientLabeler<N, UnitProduct<>, T>::value;

// Special case for numerator of 1.
template <typename D, typename T>
struct QuotientLabeler<UnitProduct<>, D, T> {
    using LabelT = StringConstant<CompoundLabel<D>::value().size() + 4>;
    static constexpr LabelT value = concatenate("1 / ", CompoundLabel<D>::value());
};
template <typename D, typename T>
constexpr typename QuotientLabeler<UnitProduct<>, D, T>::LabelT
    QuotientLabeler<UnitProduct<>, D, T>::value;

// Special case for numerator _and_ denominator of 1 (null product).
template <typename T>
struct QuotientLabeler<UnitProduct<>, UnitProduct<>, T> {
    static constexpr const char value[] = "";
};
template <typename T>
constexpr const char QuotientLabeler<UnitProduct<>, UnitProduct<>, T>::value[];
}  // namespace detail

// Unified implementation.
template <typename Unit>
struct UnitLabel
    : std::conditional_t<stdx::experimental::is_detected<detail::HasLabel, Unit>::value,
                         detail::LabelRef<Unit>,
                         DefaultUnitLabel<void>> {};

// Implementation for Pow.
template <typename Unit, std::intmax_t N>
struct UnitLabel<Pow<Unit, N>> : detail::PowerLabeler<detail::ExpLabelForPow<N>, Unit> {};

// Implementation for RatioPow.
template <typename Unit, std::intmax_t N, std::intmax_t D>
struct UnitLabel<RatioPow<Unit, N, D>>
    : detail::PowerLabeler<detail::ExpLabelForRatioPow<N, D>, Unit> {};

// Implementation for UnitProduct: split into positive and negative powers.
template <typename... Us>
struct UnitLabel<UnitProduct<Us...>>
    : detail::QuotientLabeler<detail::NumeratorPartT<UnitProduct<Us...>>,
                              detail::DenominatorPartT<UnitProduct<Us...>>,
                              void> {};

// Implementation for ScaledUnit: scaling unit U by M gets label `"[M U]"`.
template <typename U, typename M>
struct UnitLabel<ScaledUnit<U, M>> {
    using MagLab = MagnitudeLabel<M>;
    using LabelT = detail::
        ExtendedLabel<detail::parens_if<MagLab::has_exposed_slash>(MagLab::value).size() + 3u, U>;
    static constexpr LabelT value =
        detail::concatenate("[",
                            detail::parens_if<MagLab::has_exposed_slash>(MagLab::value),
                            " ",
                            UnitLabel<U>::value,
                            "]");
};
template <typename U, typename M>
constexpr typename UnitLabel<ScaledUnit<U, M>>::LabelT UnitLabel<ScaledUnit<U, M>>::value;

// Special case for unit scaled by (-1).
template <typename U>
struct UnitLabel<ScaledUnit<U, Magnitude<Negative>>> {
    using LabelT = detail::ExtendedLabel<3u, U>;
    static constexpr LabelT value = detail::concatenate("[-", UnitLabel<U>::value, "]");
};
template <typename U>
constexpr typename UnitLabel<ScaledUnit<U, Magnitude<Negative>>>::LabelT
    UnitLabel<ScaledUnit<U, Magnitude<Negative>>>::value;

// Implementation for CommonUnit: give size in terms of each constituent unit.
template <typename... Us>
struct UnitLabel<CommonUnit<Us...>>
    : CommonUnitLabel<decltype(Us{} *
                               (detail::MagT<CommonUnit<Us...>>{} / detail::MagT<Us>{}))...> {};

// Implementation for CommonPointUnit: give size in terms of each constituent unit, taking any
// origin displacements into account.
template <typename... Us>
struct UnitLabel<CommonPointUnit<Us...>>
    : UnitLabel<CommonAmongUnitsAndOriginDisplacements<Us...>> {};

template <typename Unit>
constexpr const auto &unit_label(Unit) {
    return detail::as_char_array(UnitLabel<AssociatedUnitT<Unit>>::value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnitProduct` implementation.
//
// It's just a standard pack product, so all we need to do is carefully define the total ordering.

namespace detail {
template <typename A, typename B>
struct OrderByDim : InStandardPackOrder<DimT<A>, DimT<B>> {};

template <typename A, typename B>
struct OrderByMag : InStandardPackOrder<MagT<A>, MagT<B>> {};

// Order by "scaledness" of scaled units.  This is always false unless BOTH are specializations of
// the `ScaledUnit<U, M>` template.  If they are, we *assume* we would never call this unless both
// `OrderByDim` and `OrderByMag` are tied.  Therefore, we go by the _scale factor itself_.
template <typename A, typename B>
struct OrderByScaledness : std::false_type {};
template <typename A, typename B>
struct OrderByScaleFactor : std::false_type {};
template <typename U1, typename M1, typename U2, typename M2>
struct OrderByScaleFactor<ScaledUnit<U1, M1>, ScaledUnit<U2, M2>> : InStandardPackOrder<M1, M2> {};

template <typename U1, typename M1, typename U2, typename M2>
struct OrderByScaledness<ScaledUnit<U1, M1>, ScaledUnit<U2, M2>>
    : LexicographicTotalOrdering<ScaledUnit<U1, M1>, ScaledUnit<U2, M2>, OrderByScaleFactor> {};

// OrderAsUnitProduct<A, B> can only be true if both A and B are unit products, _and_ they are in
// the standard pack order for unit products.  This default case handles the usual case where either
// A or B (or both) is not a UnitProduct<...> in the first place.
template <typename A, typename B>
struct OrderAsUnitProduct : std::false_type {};

// This specialization handles the non-trivial case, where we do have two UnitProduct instances.
template <typename... U1s, typename... U2s>
struct OrderAsUnitProduct<UnitProduct<U1s...>, UnitProduct<U2s...>>
    : InStandardPackOrder<UnitProduct<U1s...>, UnitProduct<U2s...>> {};

// OrderAsOriginDisplacementUnit<A, B> can only be true if both A and B are `OriginDisplacementUnit`
// specializations, _and_ their first units are in order, or their first units are identical and
// their second units are in order.  This default case handles the usual case where either A or B
// (or both) is not a `OriginDisplacementUnit` specialization in the first place.
template <typename A, typename B>
struct OrderAsOriginDisplacementUnit : std::false_type {};

template <typename A, typename B>
struct OrderByFirstInOriginDisplacementUnit;
template <typename A1, typename A2, typename B1, typename B2>
struct OrderByFirstInOriginDisplacementUnit<OriginDisplacementUnit<A1, A2>,
                                            OriginDisplacementUnit<B1, B2>>
    : InOrderFor<UnitProduct, A1, B1> {};

template <typename A, typename B>
struct OrderBySecondInOriginDisplacementUnit;
template <typename A1, typename A2, typename B1, typename B2>
struct OrderBySecondInOriginDisplacementUnit<OriginDisplacementUnit<A1, A2>,
                                             OriginDisplacementUnit<B1, B2>>
    : InOrderFor<UnitProduct, A2, B2> {};

template <typename A1, typename A2, typename B1, typename B2>
struct OrderAsOriginDisplacementUnit<OriginDisplacementUnit<A1, A2>, OriginDisplacementUnit<B1, B2>>
    : LexicographicTotalOrdering<OriginDisplacementUnit<A1, A2>,
                                 OriginDisplacementUnit<B1, B2>,
                                 OrderByFirstInOriginDisplacementUnit,
                                 OrderBySecondInOriginDisplacementUnit> {};

template <typename A, typename B>
struct OrderByOrigin
    : stdx::bool_constant<(detail::OriginOf<A>::value() < detail::OriginOf<B>::value())> {};

// "Unit avoidance" is a tiebreaker for quantity-equivalent units.  Anonymous units, such as
// `UnitImpl<...>`, `ScaledUnit<...>`, and `UnitProduct<...>`, are more "avoidable" than units which
// are none of these, because the latter are likely explicitly named and thus more user-facing.  The
// relative ordering among these built-in template types is probably less important than the fact
// that there _is_ a relative ordering among them (because we need to have a strict total ordering).
template <typename T>
struct CoarseUnitOrdering : std::integral_constant<int, 0> {};

template <typename A, typename B>
struct OrderByCoarseUnitOrdering
    : stdx::bool_constant<(CoarseUnitOrdering<A>::value < CoarseUnitOrdering<B>::value)> {};

template <typename... Ts>
struct CoarseUnitOrdering<UnitProduct<Ts...>> : std::integral_constant<int, 1> {};

template <typename... Ts>
struct CoarseUnitOrdering<UnitImpl<Ts...>> : std::integral_constant<int, 2> {};

template <typename... Ts>
struct CoarseUnitOrdering<ScaledUnit<Ts...>> : std::integral_constant<int, 3> {};

template <typename B, std::intmax_t N>
struct CoarseUnitOrdering<Pow<B, N>> : std::integral_constant<int, 4> {};

template <typename B, std::intmax_t N, std::intmax_t D>
struct CoarseUnitOrdering<RatioPow<B, N, D>> : std::integral_constant<int, 5> {};

template <typename... Us>
struct CoarseUnitOrdering<CommonUnit<Us...>> : std::integral_constant<int, 6> {};

template <typename... Us>
struct CoarseUnitOrdering<CommonPointUnit<Us...>> : std::integral_constant<int, 7> {};

template <typename A, typename B>
struct OrderByUnitOrderTiebreaker
    : stdx::bool_constant<(UnitOrderTiebreaker<A>::value < UnitOrderTiebreaker<B>::value)> {};

template <typename U>
struct UnitAvoidance : std::integral_constant<int, 0> {};

}  // namespace detail

template <typename U>
struct UnitOrderTiebreaker : detail::UnitAvoidance<U> {};

template <typename A, typename B>
struct InOrderFor<UnitProduct, A, B>
    : LexicographicTotalOrdering<A,
                                 B,
                                 detail::OrderByCoarseUnitOrdering,
                                 detail::OrderByDim,
                                 detail::OrderByMag,
                                 detail::OrderByScaleFactor,
                                 detail::OrderByOrigin,
                                 detail::OrderAsUnitProduct,
                                 detail::OrderAsOriginDisplacementUnit,
                                 detail::OrderByUnitOrderTiebreaker> {};

}  // namespace au


namespace au {
namespace detail {

template <typename Op>
struct TruncationRiskForImpl;
template <typename Op>
using TruncationRiskFor = typename TruncationRiskForImpl<Op>::type;

template <int N>
struct TruncationRiskClass {
    static constexpr int truncation_risk_class() { return N; }
};

template <typename T>
struct NoTruncationRisk : TruncationRiskClass<0> {
    static constexpr bool would_value_truncate(const T &) { return false; }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImpl;
template <typename T, typename M>
struct ValueTimesRatioIsNotInteger : ValueTimesRatioIsNotIntegerImpl<T, M>,
                                     TruncationRiskClass<10> {};

template <typename T>
using ValueIsNotInteger = ValueTimesRatioIsNotInteger<T, Magnitude<>>;

template <typename T>
struct ValueIsNotZero : TruncationRiskClass<20> {
    static constexpr bool would_value_truncate(const T &x) { return x != T{0}; }
};

template <typename T>
struct CannotAssessTruncationRiskFor : TruncationRiskClass<1000> {
    static constexpr bool would_value_truncate(const T &) { return true; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS (`truncation_risk.hh`):
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` section:

// (A) -> (A)
template <typename T, typename U>
struct TruncationRiskForStaticCastFromArithmeticToArithmetic
    : std::conditional<stdx::conjunction<std::is_floating_point<T>, std::is_integral<U>>::value,
                       ValueIsNotInteger<T>,
                       NoTruncationRisk<T>> {};

// (A) -> (X)
template <typename T, typename U>
struct TruncationRiskForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         TruncationRiskForStaticCastFromArithmeticToArithmetic<T, U>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T>>> {};

// (X) -> (X)
template <typename T, typename U>
struct TruncationRiskForStaticCastAssumingScalar
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForStaticCastFromArithmetic<T, U>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T>>> {};

template <typename T, typename U>
struct TruncationRiskForImpl<StaticCast<T, U>>
    : TruncationRiskForStaticCastAssumingScalar<RealPart<T>, RealPart<U>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` section:

template <typename T, typename M>
struct TruncationRiskForMultiplyArithmeticByIrrational
    : std::conditional<std::is_integral<T>::value, ValueIsNotZero<T>, NoTruncationRisk<T>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyByIrrational
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForMultiplyArithmeticByIrrational<T, M>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T>>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyArithmeticByRationalNontrivialDenominator
    : std::conditional<(get_value_result<RealPart<T>>(DenominatorT<M>{}).outcome ==
                        MagRepresentationOutcome::ERR_CANNOT_FIT),
                       ValueIsNotZero<T>,
                       ValueTimesRatioIsNotInteger<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyArithmeticByRational
    : std::conditional_t<stdx::disjunction<IsInteger<M>, std::is_floating_point<T>>::value,
                         stdx::type_identity<NoTruncationRisk<T>>,
                         TruncationRiskForMultiplyArithmeticByRationalNontrivialDenominator<T, M>> {
};

template <typename T, typename M>
struct TruncationRiskForMultiplyByRational
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForMultiplyArithmeticByRational<T, M>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T>>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyByAssumingScalar
    : std::conditional_t<IsRational<M>::value,
                         TruncationRiskForMultiplyByRational<T, M>,
                         TruncationRiskForMultiplyByIrrational<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForImpl<MultiplyTypeBy<T, M>>
    : TruncationRiskForMultiplyByAssumingScalar<RealPart<T>, M> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger<T, M>` section:

template <typename T, typename M>
struct TruncationRiskForDivideNonArithmeticByInteger
    : stdx::type_identity<CannotAssessTruncationRiskFor<T>> {};

template <typename T, typename M>
struct TruncationRiskForDivideIntegralByInteger
    : std::conditional<(get_value_result<T>(M{}).outcome ==
                        MagRepresentationOutcome::ERR_CANNOT_FIT),
                       ValueIsNotZero<T>,
                       ValueTimesRatioIsNotInteger<T, MagInverseT<M>>> {};

template <typename T, typename M>
struct TruncationRiskForDivideArithmeticByInteger
    : std::conditional_t<std::is_floating_point<T>::value,
                         stdx::type_identity<NoTruncationRisk<T>>,
                         TruncationRiskForDivideIntegralByInteger<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForDivideByIntAssumingScalar
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForDivideArithmeticByInteger<T, M>,
                         TruncationRiskForDivideNonArithmeticByInteger<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForImpl<DivideTypeByInteger<T, M>>
    : TruncationRiskForDivideByIntAssumingScalar<RealPart<T>, M> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<...>` section:

// A little helper to simplify instances of `ValueTimesRatioIsNotInteger` that turn out to be
// trivial (because their type is integral, so they can never produce truncating values).
template <typename T, typename M>
struct ReduceValueTimesRatioIsNotIntegerImpl
    : std::conditional<stdx::conjunction<IsInteger<M>, std::is_integral<T>>::value,
                       NoTruncationRisk<T>,
                       ValueTimesRatioIsNotInteger<T, M>> {};
template <typename T, typename M>
using ReduceValueTimesRatioIsNotInteger =
    typename ReduceValueTimesRatioIsNotIntegerImpl<T, M>::type;

//
// `UpdateRisk<Op, Risk>` adapts a "downstream" risk to the "upstream" interface.
//
// At minimum, this updates the input type to `OpInput<Op>`.  But it may also tweak the parameters
// (e.g., for `ValuesNotSomeIntegerTimes`), or even change the risk type entirely.
//
template <typename Op, typename Risk>
struct UpdateRiskImpl;
template <typename Op, typename Risk>
using UpdateRisk = typename UpdateRiskImpl<Op, Risk>::type;

template <template <class> class Risk, typename T, typename U>
struct UpdateRiskImpl<StaticCast<T, U>, Risk<RealPart<U>>>
    : stdx::type_identity<Risk<RealPart<T>>> {};

template <typename T, typename U, typename M>
struct UpdateRiskImpl<StaticCast<T, U>, ValueTimesRatioIsNotInteger<RealPart<U>, M>>
    : std::conditional<stdx::conjunction<IsInteger<M>, std::is_integral<T>>::value,
                       NoTruncationRisk<RealPart<T>>,
                       ReduceValueTimesRatioIsNotInteger<RealPart<T>, M>> {};

template <template <class> class Risk, typename T, typename M>
struct UpdateRiskImpl<MultiplyTypeBy<T, M>, Risk<RealPart<T>>>
    : stdx::type_identity<Risk<RealPart<T>>> {};

template <template <class> class Risk, typename T, typename M>
struct UpdateRiskImpl<DivideTypeByInteger<T, M>, Risk<RealPart<T>>>
    : stdx::type_identity<Risk<RealPart<T>>> {};

template <typename T, typename M1, typename M2>
struct UpdateRiskImpl<MultiplyTypeBy<T, M1>, ValueTimesRatioIsNotInteger<RealPart<T>, M2>>
    : std::conditional<IsRational<M1>::value,
                       ReduceValueTimesRatioIsNotInteger<RealPart<T>, MagProductT<M1, M2>>,
                       ValueIsNotZero<RealPart<T>>> {};

template <typename T, typename M1, typename M2>
struct UpdateRiskImpl<DivideTypeByInteger<T, M1>, ValueTimesRatioIsNotInteger<RealPart<T>, M2>>
    : stdx::type_identity<ReduceValueTimesRatioIsNotInteger<RealPart<T>, MagQuotientT<M2, M1>>> {};

//
// `BiggestRiskImpl<Risk1, Risk2>` is a helper that computes the "biggest" risk between two risks.
//

template <typename Risk1, typename Risk2>
struct TruncationRisks {};

template <typename Risk1, typename Risk2>
struct OrderByTruncationRiskClass
    : stdx::bool_constant<(Risk1::truncation_risk_class() < Risk2::truncation_risk_class())> {};

template <typename Risk>
struct DenominatorOfRatioImpl : stdx::type_identity<Magnitude<>> {};
template <typename T, typename M>
struct DenominatorOfRatioImpl<ValueTimesRatioIsNotInteger<T, M>>
    : stdx::type_identity<DenominatorT<M>> {};
template <typename Risk>
using DenominatorOfRatio = typename DenominatorOfRatioImpl<Risk>::type;

template <typename Risk1, typename Risk2>
struct OrderByDenominatorOfRatio
    : stdx::bool_constant<(get_value<uint64_t>(DenominatorOfRatio<Risk1>{}) <
                           get_value<uint64_t>(DenominatorOfRatio<Risk2>{}))> {};

}  // namespace detail

// Must be in `::au` namespace:
template <typename Risk1, typename Risk2>
struct InOrderFor<detail::TruncationRisks, Risk1, Risk2>
    : LexicographicTotalOrdering<Risk1,
                                 Risk2,
                                 detail::OrderByTruncationRiskClass,
                                 detail::OrderByDenominatorOfRatio> {};

namespace detail {

template <typename Risk1, typename Risk2>
struct BiggestRiskImpl
    : std::conditional<InOrderFor<TruncationRisks, Risk1, Risk2>::value, Risk2, Risk1> {};

//
// Full `TruncationRiskFor` implementation for `OpSequence<Op>`:
//

template <typename Op>
struct TruncationRiskForImpl<OpSequenceImpl<Op>> : TruncationRiskForImpl<Op> {};

template <typename Op, typename... Ops>
struct TruncationRiskForImpl<OpSequenceImpl<Op, Ops...>>
    : BiggestRiskImpl<UpdateRisk<Op, TruncationRiskFor<OpSequenceImpl<Ops...>>>,
                      TruncationRiskFor<Op>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ValueTimesRatioIsNotInteger` section:

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForIntWhereDenominatorDoesNotFit {
    static constexpr bool would_value_truncate(const T &value) { return value != T{0}; }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForIntWhereDenominatorFits {
    static constexpr bool would_value_truncate(const T &value) {
        return (value % get_value<RealPart<T>>(DenominatorT<M>{})) != T{0};
    }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForInt
    : std::conditional_t<get_value_result<RealPart<T>>(DenominatorT<M>{}).outcome ==
                             MagRepresentationOutcome::ERR_CANNOT_FIT,
                         ValueTimesRatioIsNotIntegerImplForIntWhereDenominatorDoesNotFit<T, M>,
                         ValueTimesRatioIsNotIntegerImplForIntWhereDenominatorFits<T, M>> {};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForFloatGeneric {
    static constexpr bool would_value_truncate(const T &value) {
        const auto result = value * get_value<RealPart<T>>(M{});
        return std::trunc(result) != result;
    }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForFloatDivideByInteger {
    static constexpr bool would_value_truncate(const T &value) {
        const auto result = value / get_value<RealPart<T>>(MagInverseT<M>{});
        return std::trunc(result) != result;
    }
};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImplForFloat
    : std::conditional_t<IsInteger<MagInverseT<M>>::value,
                         ValueTimesRatioIsNotIntegerImplForFloatDivideByInteger<T, M>,
                         ValueTimesRatioIsNotIntegerImplForFloatGeneric<T, M>> {};

template <typename T, typename M>
struct ValueTimesRatioIsNotIntegerImpl
    : std::conditional_t<std::is_integral<T>::value,
                         ValueTimesRatioIsNotIntegerImplForInt<T, M>,
                         ValueTimesRatioIsNotIntegerImplForFloat<T, M>> {};

}  // namespace detail
}  // namespace au



namespace au {

//
// Conversion risk section.
//
// End users can use the constants `OVERFLOW_RISK` and `TRUNCATION_RISK`.  They can combine them as
// flags with `|`.  And they can pass either of these (or the result of `|`) to either
// `check_for()` or `ignore()`.  The result of these functions is a risk _policy_, which can be
// passed as a second argument to conversion functions to control which checks are performed.
//

namespace detail {
enum class ConversionRisk : uint8_t {
    // We use CamelCase instead of UPPER_CASE because `OVERFLOW` is the name of a macro that exists
    // in the wild in some versions of glibc's `math.h`.
    Overflow = (1u << 0u),
    Truncation = (1u << 1u),
};

template <typename T>
struct CheckTheseRisks;

template <uint8_t RiskFlags>
struct RiskSet {
    static_assert(RiskFlags <= 3u, "Invalid risk flags");

    template <uint8_t OtherFlags>
    constexpr RiskSet<RiskFlags | OtherFlags> operator|(RiskSet<OtherFlags>) const {
        return {};
    }

    constexpr uint8_t flags() const { return RiskFlags; }

    friend constexpr CheckTheseRisks<RiskSet<RiskFlags>> check_for(RiskSet) { return {}; }
    friend constexpr CheckTheseRisks<RiskSet<3u - RiskFlags>> ignore(RiskSet) { return {}; }
};

template <uint8_t RiskFlags>
struct CheckTheseRisks<RiskSet<RiskFlags>> {
    constexpr bool should_check(ConversionRisk risk) const {
        return (RiskFlags & static_cast<uint8_t>(risk)) != 0u;
    }
};

constexpr auto OVERFLOW_RISK = RiskSet<static_cast<uint8_t>(ConversionRisk::Overflow)>{};
constexpr auto TRUNCATION_RISK = RiskSet<static_cast<uint8_t>(ConversionRisk::Truncation)>{};

}  // namespace detail

constexpr auto OVERFLOW_RISK = detail::OVERFLOW_RISK;
constexpr auto TRUNCATION_RISK = detail::TRUNCATION_RISK;
constexpr auto ALL_RISKS = OVERFLOW_RISK | TRUNCATION_RISK;

// `IsConversionRiskPolicy<T>` checks whether `T` is a conversion risk policy type.  For now, this
// boils down to being a specialization of `CheckTheseRisks` on some `RiskSet`.
//
// Although we have no such plans at present, it's conceivable that we could create more general
// conversion risk policy types later.  If we do, this trait will still be authoritatively correct.
template <typename T>
struct IsConversionRiskPolicy : std::false_type {};
template <uint8_t RiskFlags>
struct IsConversionRiskPolicy<detail::CheckTheseRisks<detail::RiskSet<RiskFlags>>>
    : std::true_type {};

//
// "Main" conversion policy section.
//

namespace detail {
// Chosen so as to allow populating a `QuantityI32<Hertz>` with an input in MHz.
constexpr auto OVERFLOW_THRESHOLD = mag<2'147>();

// `SettingPureRealFromMixedReal<A, B>` tests whether `A` is a pure real type, _and_ `B` is a type
// that has a real _part_, but is not purely real (call it a "mixed-real" type).
//
// The point is to guard against situations where we're _implicitly_ converting a "mixed-real" type
// (i.e., typically a complex number) to a pure real type.
template <typename Rep, typename SourceRep>
struct SettingPureRealFromMixedReal
    : stdx::conjunction<stdx::negation<std::is_same<SourceRep, RealPart<SourceRep>>>,
                        std::is_same<Rep, RealPart<Rep>>> {};

template <typename T>
constexpr bool meets_threshold(T x) {
    constexpr auto threshold_result = get_value_result<T>(OVERFLOW_THRESHOLD);
    static_assert(threshold_result.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT ||
                      threshold_result.outcome == MagRepresentationOutcome::OK,
                  "Overflow threshold must be a valid representation");
    const auto threshold = (threshold_result.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT)
                               ? std::numeric_limits<T>::max()
                               : threshold_result.value;
    if (Less{}(x, T{0})) {
        x = T{0} - x;
    }
    return x >= threshold;
}

// Check overflow risk from above.
template <bool CanOverflowAbove, typename Op>
struct OverflowAboveRiskAcceptablyLowImpl
    : stdx::bool_constant<meets_threshold(MaxGood<Op>::value())> {};
template <typename Op>
struct OverflowAboveRiskAcceptablyLowImpl<false, Op> : std::true_type {};

template <typename Op>
struct OverflowAboveRiskAcceptablyLow
    : OverflowAboveRiskAcceptablyLowImpl<CanOverflowAbove<Op>::value, Op> {};

// Check overflow risk, using "overflow above" risk only.
//
// We currently do not check the risk for overflowing _below_, because it is overwhelmingly common
// in practice for people to initialize an unsigned integer variable with a constant of a signed
// type whose value is known to be positive.  While we would love to be able to prevent implicit
// signed to unsigned conversions --- and, while our overflow detection machinery can easily do so
// --- we simply cannot afford to break that many _valid_ use cases to catch those invalid ones.
//
// That said, the _runtime_ overflow checkers _do_ check both above and below.
template <typename Op>
struct OverflowRiskAcceptablyLow : OverflowAboveRiskAcceptablyLow<Op> {};

// Check truncation risk.
template <typename Op>
struct TruncationRiskAcceptablyLow
    : std::is_same<TruncationRiskFor<Op>, NoTruncationRisk<RealPart<OpInput<Op>>>> {};

template <typename Op>
struct ConversionRiskAcceptablyLow
    : stdx::conjunction<OverflowRiskAcceptablyLow<Op>, TruncationRiskAcceptablyLow<Op>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct PermitAsCarveOutForIntegerPromotion
    : stdx::conjunction<std::is_same<Abs<ScaleFactor>, Magnitude<>>,
                        std::is_same<SourceRep, PromotedType<Rep>>,
                        stdx::disjunction<IsPositive<ScaleFactor>, std::is_signed<Rep>>,
                        std::is_integral<Rep>,
                        std::is_integral<SourceRep>,
                        std::is_assignable<Rep &, SourceRep>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
struct PassesConversionRiskCheck
    : stdx::disjunction<
          PermitAsCarveOutForIntegerPromotion<Rep, ScaleFactor, SourceRep>,
          ConversionRiskAcceptablyLow<ConversionForRepsAndFactor<SourceRep, Rep, ScaleFactor>>> {};

template <typename Rep, typename ScaleFactor, typename SourceRep>
using ImplicitConversionPolicy =
    stdx::conjunction<PassesConversionRiskCheck<Rep, ScaleFactor, SourceRep>,
                      stdx::negation<SettingPureRealFromMixedReal<Rep, SourceRep>>>;

}  // namespace detail

template <typename Rep, typename ScaleFactor>
struct ImplicitRepPermitted : detail::ImplicitConversionPolicy<Rep, ScaleFactor, Rep> {};

template <typename Rep, typename SourceUnitSlot, typename TargetUnitSlot>
constexpr bool implicit_rep_permitted_from_source_to_target(SourceUnitSlot, TargetUnitSlot) {
    using SourceUnit = AssociatedUnitT<SourceUnitSlot>;
    using TargetUnit = AssociatedUnitT<TargetUnitSlot>;
    static_assert(HasSameDimension<SourceUnit, TargetUnit>::value,
                  "Can only convert same-dimension units");

    return ImplicitRepPermitted<Rep, UnitRatioT<SourceUnit, TargetUnit>>::value;
}

template <typename Unit, typename Rep>
struct ConstructionPolicy {
    // Note: it's tempting to use the UnitRatioT trait here, but we can't, because it produces a
    // hard error for units with different dimensions.  This is for good reason: magnitude ratios
    // are meaningless unless the dimension is the same.  UnitRatioT is the user-facing tool, so we
    // build in this hard error for safety.  Here, we need a soft error, so we do the dimension
    // check manually below.
    template <typename SourceUnit>
    using ScaleFactor = MagQuotientT<detail::MagT<SourceUnit>, detail::MagT<Unit>>;

    template <typename SourceUnit, typename SourceRep>
    using PermitImplicitFrom = stdx::conjunction<
        HasSameDimension<Unit, SourceUnit>,
        detail::ImplicitConversionPolicy<Rep, ScaleFactor<SourceUnit>, SourceRep>>;
};

}  // namespace au



namespace au {

//
// Make a Quantity of the given Unit, which has this value as measured in the Unit.
//
template <typename UnitT, typename T>
constexpr auto make_quantity(T value) {
    return QuantityMaker<UnitT>{}(value);
}

template <typename Unit, typename T>
constexpr auto make_quantity_unless_unitless(T value) {
    return std::conditional_t<IsUnitlessUnit<Unit>::value, stdx::identity, QuantityMaker<Unit>>{}(
        value);
}

// Trait to check whether two Quantity types are exactly equivalent.
//
// For purposes of our library, "equivalent" means that they have the same Dimension and Magnitude.
template <typename Q1, typename Q2>
struct AreQuantityTypesEquivalent;

// Trait for a type T which corresponds exactly to some Quantity type.
//
// "Correspondence" with a `Quantity<U, R>` means that T stores a value in a numeric datatype R, and
// this value represents a quantity whose unit of measure is quantity-equivalent to U.
//
// The canonical examples are the `duration` types from the `std::chrono::library`.  For example,
// `std::chrono::duration<double, std::nano>` exactly corresponds to `QuantityD<Nano<Seconds>>`, and
// it is always OK to convert back and forth between these types implicitly.
//
// To add support for a type T which is equivalent to Quantity<U, R>, define a specialization of
// `CorrespondingQuantity<T>` with a member alias `Unit` for `U`, and `Rep` for `R`.  You should
// then add static member functions as follows to add support for each direction of conversion.
//   - For T -> Quantity, define `R extract_value(T)`.
//   - For Quantity -> T, define `T construct_from_value(R)`.
template <typename T>
struct CorrespondingQuantity {};
template <typename T>
using CorrespondingQuantityT =
    Quantity<typename CorrespondingQuantity<T>::Unit, typename CorrespondingQuantity<T>::Rep>;

// Redirect various cvref-qualified specializations to the "main" specialization.
//
// We use this slightly counterintuitive approach, rather than a more conventional
// `remove_cvref_t`-based approach, because the latter causes an _internal compiler error_ on the
// ACI QNX build.
template <typename T>
struct CorrespondingQuantity<const T> : CorrespondingQuantity<T> {};
template <typename T>
struct CorrespondingQuantity<T &> : CorrespondingQuantity<T> {};
template <typename T>
struct CorrespondingQuantity<const T &> : CorrespondingQuantity<T> {};

// Request conversion of any type to its corresponding Quantity, if there is one.
//
// This is a way to explicitly and readably "enter the au Quantity domain" when we have some
// non-au-Quantity type which is nevertheless exactly and unambiguously equivalent to some Quantity.
//
// `as_quantity()` is SFINAE-friendly: we can use it to constrain templates to types `T` which are
// exactly equivalent to some Quantity type.
template <typename T>
constexpr auto as_quantity(T &&x) -> CorrespondingQuantityT<T> {
    using Q = CorrespondingQuantity<T>;
    static_assert(IsUnit<typename Q::Unit>{}, "No Quantity corresponding to type");

    auto value = Q::extract_value(std::forward<T>(x));
    static_assert(std::is_same<decltype(value), typename Q::Rep>{},
                  "Inconsistent CorrespondingQuantity implementation");

    return make_quantity<typename Q::Unit>(value);
}

// Callsite-readable way to convert a `Quantity` to a raw number.
//
// Only works for dimensionless `Quantities`; will return a compile-time error otherwise.
//
// Identity for non-`Quantity` types.
template <typename U, typename R, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
constexpr R as_raw_number(Quantity<U, R> q, RiskPolicyT policy = RiskPolicyT{}) {
    return q.in(UnitProductT<>{}, policy);
}
template <typename T>
constexpr T as_raw_number(T x) {
    return x;
}

namespace detail {
// We implement `Quantity` comparisons by converting to a common unit, and comparing the values
// stored in the underlying Rep types.  This means we need to know the _sign_ of that common unit,
// so we can know which order to pass those underlying values (it gets reversed for negative units).
template <typename SignMag, typename Op>
struct SignAwareComparison;
}  // namespace detail

template <typename UnitT, typename RepT>
class Quantity {
    template <bool ImplicitOk, typename OtherUnit, typename OtherRep>
    using EnableIfImplicitOkIs = std::enable_if_t<
        ImplicitOk ==
        ConstructionPolicy<UnitT, RepT>::template PermitImplicitFrom<OtherUnit, OtherRep>::value>;

    // We could consider making this public someday, if we had a use case.
    using Sign = UnitSign<UnitT>;

    // Not strictly necessary, but we want to keep each comparator implementation to one line.
    using Eq = detail::SignAwareComparison<Sign, detail::Equal>;
    using Ne = detail::SignAwareComparison<Sign, detail::NotEqual>;
    using Lt = detail::SignAwareComparison<Sign, detail::Less>;
    using Le = detail::SignAwareComparison<Sign, detail::LessEqual>;
    using Gt = detail::SignAwareComparison<Sign, detail::Greater>;
    using Ge = detail::SignAwareComparison<Sign, detail::GreaterEqual>;

 public:
    using Rep = RepT;
    using Unit = UnitT;
    static constexpr auto unit = Unit{};

    static_assert(IsValidRep<Rep>::value, "Rep must meet our requirements for a rep");

    // IMPLICIT constructor for another Quantity of the same Dimension.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>>
    constexpr Quantity(Quantity<OtherUnit, OtherRep> other)  // NOLINT(runtime/explicit)
        : Quantity{other.template as<Rep>(UnitT{})} {}

    // EXPLICIT constructor for another Quantity of the same Dimension.
    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<false, OtherUnit, OtherRep>,
              typename ThisUnusedTemplateParameterDistinguishesUsFromTheAboveConstructor = void>
    // Deleted: use `.as<NewRep>(new_unit)` to force a cast.
    explicit constexpr Quantity(Quantity<OtherUnit, OtherRep> other) = delete;

    // Constructor for another Quantity with an explicit conversion risk policy.
    template <typename OtherUnit,
              typename OtherRep,
              typename RiskPolicyT,
              std::enable_if_t<IsConversionRiskPolicy<RiskPolicyT>::value, int> = 0>
    constexpr Quantity(Quantity<OtherUnit, OtherRep> other, RiskPolicyT policy)
        : value_{other.template in<Rep>(UnitT{}, policy)} {}

    // Construct this Quantity with a value of exactly Zero.
    constexpr Quantity(Zero) : value_{0} {}

    constexpr Quantity() noexcept = default;

    // Implicit construction from any exactly-equivalent type.
    template <
        typename T,
        std::enable_if_t<std::is_convertible<CorrespondingQuantityT<T>, Quantity>::value, int> = 0>
    constexpr Quantity(T &&x) : Quantity{as_quantity(std::forward<T>(x))} {}

    // `q.as<Rep>(new_unit)`, or `q.as<Rep>(new_unit, risk_policy)`
    template <typename NewRep,
              typename NewUnitSlot,
              typename RiskPolicyT = decltype(ignore(ALL_RISKS))>
    constexpr auto as(NewUnitSlot u, RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity<AssociatedUnitT<NewUnitSlot>>(in_impl<NewRep>(u, policy));
    }

    // `q.as(new_unit)`, or `q.as(new_unit, risk_policy)`
    template <typename NewUnitSlot, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
    constexpr auto as(NewUnitSlot u, RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity<AssociatedUnitT<NewUnitSlot>>(in_impl<Rep>(u, policy));
    }

    // `q.in<Rep>(new_unit)`, or `q.in<Rep>(new_unit, risk_policy)`
    template <typename NewRep,
              typename NewUnitSlot,
              typename RiskPolicyT = decltype(ignore(ALL_RISKS))>
    constexpr auto in(NewUnitSlot u, RiskPolicyT policy = RiskPolicyT{}) const {
        return in_impl<NewRep>(u, policy);
    }

    // `q.in(new_unit)`, or `q.in(new_unit, risk_policy)`
    template <typename NewUnitSlot, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
    constexpr auto in(NewUnitSlot u, RiskPolicyT policy = RiskPolicyT{}) const {
        return in_impl<Rep>(u, policy);
    }

    // "Forcing" conversions, which explicitly ignore safety checks for overflow and truncation.
    template <typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `q.coerce_as(new_units)`.
        return as<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `q.coerce_as<T>(new_units)`.
        return as<NewRep>(NewUnit{});
    }
    template <typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `q.coerce_in(new_units)`.
        return in<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `q.coerce_in<T>(new_units)`.
        return in<NewRep>(NewUnit{});
    }

    // Direct access to the underlying value member, with any Quantity-equivalent Unit.
    //
    // Mutable access:
    template <typename UnitSlot>
    constexpr Rep &data_in(UnitSlot) {
        static_assert(AreUnitsQuantityEquivalent<AssociatedUnitT<UnitSlot>, Unit>::value,
                      "Can only access value via Quantity-equivalent unit");
        return value_;
    }
    // Const access:
    template <typename UnitSlot>
    constexpr const Rep &data_in(UnitSlot) const {
        static_assert(AreUnitsQuantityEquivalent<AssociatedUnitT<UnitSlot>, Unit>::value,
                      "Can only access value via Quantity-equivalent unit");
        return value_;
    }

    // Permit this factory functor to access our private constructor.
    //
    // We allow this because it explicitly names the unit at the callsite, even if people refer to
    // this present Quantity type by an alias that omits the unit.  This preserves Unit Safety and
    // promotes callsite readability.
    friend struct QuantityMaker<UnitT>;

    // Comparison operators.
    friend constexpr bool operator==(Quantity a, Quantity b) { return Eq{}(a.value_, b.value_); }
    friend constexpr bool operator!=(Quantity a, Quantity b) { return Ne{}(a.value_, b.value_); }
    friend constexpr bool operator<(Quantity a, Quantity b) { return Lt{}(a.value_, b.value_); }
    friend constexpr bool operator<=(Quantity a, Quantity b) { return Le{}(a.value_, b.value_); }
    friend constexpr bool operator>(Quantity a, Quantity b) { return Gt{}(a.value_, b.value_); }
    friend constexpr bool operator>=(Quantity a, Quantity b) { return Ge{}(a.value_, b.value_); }

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
    using Twc = detail::SignAwareComparison<Sign, detail::ThreeWayCompare>;
    friend constexpr auto operator<=>(Quantity a, Quantity b) { return Twc{}(a.value_, b.value_); }
#endif

    // Addition and subtraction for like quantities.
    friend constexpr Quantity<UnitT, decltype(std::declval<RepT>() + std::declval<RepT>())>
    operator+(Quantity a, Quantity b) {
        return make_quantity<UnitT>(a.value_ + b.value_);
    }
    friend constexpr Quantity<UnitT, decltype(std::declval<RepT>() - std::declval<RepT>())>
    operator-(Quantity a, Quantity b) {
        return make_quantity<UnitT>(a.value_ - b.value_);
    }

    // Scalar multiplication.
    template <typename T, typename = std::enable_if_t<IsProductValidRep<RepT, T>::value>>
    friend constexpr auto operator*(Quantity a, T s) {
        return make_quantity<UnitT>(a.value_ * s);
    }
    template <typename T, typename = std::enable_if_t<IsProductValidRep<T, RepT>::value>>
    friend constexpr auto operator*(T s, Quantity a) {
        return make_quantity<UnitT>(s * a.value_);
    }

    // Scalar division.
    template <typename T, typename = std::enable_if_t<IsQuotientValidRep<RepT, T>::value>>
    friend constexpr auto operator/(Quantity a, T s) {
        return make_quantity<UnitT>(a.value_ / s);
    }
    template <typename T, typename = std::enable_if_t<IsQuotientValidRep<T, RepT>::value>>
    friend constexpr auto operator/(T s, Quantity a) {
        warn_if_integer_division<UnitProductT<>, T>();
        return make_quantity<decltype(pow<-1>(unit))>(s / a.value_);
    }

    // Multiplication for dimensioned quantities.
    template <typename OtherUnit, typename OtherRep>
    constexpr auto operator*(Quantity<OtherUnit, OtherRep> q) const {
        return make_quantity_unless_unitless<UnitProductT<Unit, OtherUnit>>(value_ *
                                                                            q.in(OtherUnit{}));
    }

    // Division for dimensioned quantities.
    template <typename OtherUnit, typename OtherRep>
    constexpr auto operator/(Quantity<OtherUnit, OtherRep> q) const {
        warn_if_integer_division<OtherUnit, OtherRep>();
        return make_quantity_unless_unitless<UnitQuotientT<Unit, OtherUnit>>(value_ /
                                                                             q.in(OtherUnit{}));
    }

    // Short-hand addition and subtraction assignment.
    constexpr Quantity &operator+=(Quantity other) {
        value_ += other.value_;
        return *this;
    }
    constexpr Quantity &operator-=(Quantity other) {
        value_ -= other.value_;
        return *this;
    }

    template <typename T>
    constexpr void perform_shorthand_checks() {
        static_assert(
            IsValidRep<T>::value,
            "This overload is only for scalar mult/div-assignment with raw numeric types");

        static_assert((!std::is_integral<detail::RealPart<Rep>>::value) ||
                          std::is_integral<detail::RealPart<T>>::value,
                      "We don't support compound mult/div of integral types by floating point");
    }

    // Short-hand multiplication assignment.
    template <typename T>
    constexpr Quantity &operator*=(T s) {
        perform_shorthand_checks<T>();

        value_ *= s;
        return *this;
    }

    // Short-hand division assignment.
    template <typename T>
    constexpr Quantity &operator/=(T s) {
        perform_shorthand_checks<T>();

        value_ /= s;
        return *this;
    }

    // Modulo operator (defined only for integral rep).
    friend constexpr Quantity operator%(Quantity a, Quantity b) { return {a.value_ % b.value_}; }

    // Unary plus and minus.
    constexpr Quantity operator+() const { return {+value_}; }
    constexpr Quantity operator-() const { return {-value_}; }

    // Automatic conversion to Rep for Unitless type.
    template <typename U = UnitT, typename = std::enable_if_t<IsUnitlessUnit<U>::value>>
    constexpr operator Rep() const {
        return value_;
    }

    // Automatic conversion to any equivalent type that supports it.
    template <
        typename T,
        std::enable_if_t<std::is_convertible<Quantity, CorrespondingQuantityT<T>>::value, int> = 0>
    constexpr operator T() const {
        return CorrespondingQuantity<T>::construct_from_value(
            CorrespondingQuantityT<T>{*this}.in(typename CorrespondingQuantity<T>::Unit{}));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Pre-C++20 Non-Type Template Parameter (NTTP) functionality.
    //
    // If `Rep` is a built in integral type, then `Quantity::NTTP` can be used as a template
    // parameter.

    enum class NTTP : std::conditional_t<std::is_integral<Rep>::value, Rep, bool> {
        ENUM_VALUES_ARE_UNUSED
    };

    constexpr Quantity(NTTP val) : value_{static_cast<Rep>(val)} {
        static_assert(std::is_integral<Rep>::value,
                      "NTTP functionality only works when rep is built-in integral type");
    }

    constexpr operator NTTP() const {
        static_assert(std::is_integral<Rep>::value,
                      "NTTP functionality only works when rep is built-in integral type");
        return static_cast<NTTP>(value_);
    }

    template <typename C, C x = C::ENUM_VALUES_ARE_UNUSED>
    constexpr operator C() const = delete;
    // If you got here ^^^, then you need to do your unit conversion **manually**.  Check the type
    // of the template parameter, and convert it to that same unit and rep.

    friend constexpr Quantity from_nttp(NTTP val) { return val; }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Hidden friends for select math functions.
    //
    // Moving the implementation here lets us effortlessly support callsites where any number of
    // arguments are "shapeshifter" types that are compatible with this Quantity (such as `ZERO`, or
    // various physical constant).
    //
    // Note that the min/max implementations return by _value_, for consistency with other Quantity
    // implementations (because in the general case, the return type can differ from the inputs).
    // Note, too, that we use the Walter Brown implementation for min/max, where min prefers `a`,
    // max prefers `b`, and they never return the same input (although this matters less when we're
    // returning by value).
    friend constexpr Quantity min(Quantity a, Quantity b) { return b < a ? b : a; }
    friend constexpr Quantity max(Quantity a, Quantity b) { return b < a ? a : b; }
    friend constexpr Quantity clamp(Quantity v, Quantity lo, Quantity hi) {
        return (v < lo) ? lo : ((hi < v) ? hi : v);
    }

#if defined(__cpp_lib_interpolate) && __cpp_lib_interpolate >= 201902L
    // `std::lerp` requires C++20 support.
    template <typename T>
    friend constexpr auto lerp(Quantity a, Quantity b, T t) {
        return make_quantity<UnitT>(std::lerp(a.in(unit), b.in(unit), as_raw_number(t)));
    }
#endif

 private:
    template <typename OtherUnit, typename OtherRep>
    static constexpr void warn_if_integer_division() {
        constexpr bool uses_integer_division =
            (std::is_integral<Rep>::value && std::is_integral<OtherRep>::value);
        constexpr bool are_units_quantity_equivalent =
            AreUnitsQuantityEquivalent<UnitT, OtherUnit>::value;
        static_assert(are_units_quantity_equivalent || !uses_integer_division,
                      "Integer division forbidden.  See "
                      "<https://aurora-opensource.github.io/au/main/troubleshooting/"
                      "#integer-division-forbidden> for more details about the risks, "
                      "and your options to resolve this error.");
    }

    template <typename OtherRep, typename OtherUnitSlot, typename RiskPolicyT>
    constexpr OtherRep in_impl(OtherUnitSlot, RiskPolicyT) const {
        using OtherUnit = AssociatedUnitT<OtherUnitSlot>;
        static_assert(IsUnit<OtherUnit>::value, "Invalid type passed to unit slot");

        using Op = detail::ConversionForRepsAndFactor<Rep, OtherRep, UnitRatioT<Unit, OtherUnit>>;

        constexpr bool should_check_overflow =
            RiskPolicyT{}.should_check(detail::ConversionRisk::Overflow);
        constexpr bool is_overflow_risk_ok = detail::OverflowRiskAcceptablyLow<Op>::value;

        constexpr bool should_check_truncation =
            RiskPolicyT{}.should_check(detail::ConversionRisk::Truncation);
        constexpr bool is_truncation_risk_ok = detail::TruncationRiskAcceptablyLow<Op>::value;

        constexpr bool is_overflow_only_unacceptable_risk =
            (should_check_overflow && !is_overflow_risk_ok && is_truncation_risk_ok);
        static_assert(!is_overflow_only_unacceptable_risk,
                      "Overflow risk too high.  See "
                      "<https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>"
                      ".  Your \"risk set\" is `OVERFLOW_RISK`.");

        constexpr bool is_truncation_only_unacceptable_risk =
            (should_check_truncation && !is_truncation_risk_ok && is_overflow_risk_ok);
        static_assert(!is_truncation_only_unacceptable_risk,
                      "Truncation risk too high.  See "
                      "<https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>"
                      ".  Your \"risk set\" is `TRUNCATION_RISK`.");

        constexpr bool are_both_overflow_and_truncation_unacceptably_risky =
            (should_check_overflow || should_check_truncation) && !is_overflow_risk_ok &&
            !is_truncation_risk_ok;
        static_assert(!are_both_overflow_and_truncation_unacceptably_risky,
                      "Both truncation and overflow risk too high.  See "
                      "<https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>"
                      ".  Your \"risk set\" is `OVERFLOW_RISK | TRUNCATION_RISK`.");

        return Op::apply_to(value_);
    }

    constexpr Quantity(Rep value) : value_{value} {}

    Rep value_{};
};

// Give more readable error messages when passing `Quantity` to a unit slot.
template <typename U, typename R>
struct AssociatedUnit<Quantity<U, R>> {
    static_assert(
        detail::AlwaysFalse<U, R>::value,
        "Can't pass `Quantity` to a unit slot (see: "
        "https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)");
};
template <typename U, typename R>
struct AssociatedUnitForPoints<Quantity<U, R>> {
    static_assert(
        detail::AlwaysFalse<U, R>::value,
        "Can't pass `Quantity` to a unit slot for points (see: "
        "https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)");
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Machinery to explicitly unblock integer division.
//
// Dividing by `unblock_int_div(x)` will allow integer division for any `x`.  If the division would
// have been allowed anyway, then `unblock_int_div` is a no-op: this enables us to write templated
// code to handle template parameters that may or may not be integral.

template <typename U, typename R>
class AlwaysDivisibleQuantity;

// Unblock integer divisoin for a `Quantity`.
template <typename U, typename R>
constexpr AlwaysDivisibleQuantity<U, R> unblock_int_div(Quantity<U, R> q) {
    return AlwaysDivisibleQuantity<U, R>{q};
}

// Unblock integer division for any non-`Quantity` type.
template <typename R>
constexpr AlwaysDivisibleQuantity<UnitProductT<>, R> unblock_int_div(R x) {
    return AlwaysDivisibleQuantity<UnitProductT<>, R>{make_quantity<UnitProductT<>>(x)};
}

template <typename U, typename R>
class AlwaysDivisibleQuantity {
 public:
    // Divide a `Quantity` by this always-divisible quantity type.
    template <typename U2, typename R2>
    friend constexpr auto operator/(Quantity<U2, R2> q2, AlwaysDivisibleQuantity q) {
        return make_quantity<UnitQuotientT<U2, U>>(q2.in(U2{}) / q.q_.in(U{}));
    }

    // Divide any non-`Quantity` by this always-divisible quantity type.
    template <typename T>
    friend constexpr auto operator/(T x, AlwaysDivisibleQuantity q) {
        return make_quantity<UnitInverseT<U>>(x / q.q_.in(U{}));
    }

    template <typename UU, typename RR>
    friend constexpr AlwaysDivisibleQuantity<UU, RR> unblock_int_div(Quantity<UU, RR> q);

    template <typename RR>
    friend constexpr AlwaysDivisibleQuantity<UnitProductT<>, RR> unblock_int_div(RR x);

 private:
    constexpr AlwaysDivisibleQuantity(Quantity<U, R> q) : q_{q} {}

    Quantity<U, R> q_;
};

// Perform division in the common unit of two inputs.
//
// When two quantities have the same dimension, this is what most people probably expect when
// dividing them.  When they have different dimension, the operation is undefined, and we'll get a
// compiler error.
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto divide_using_common_unit(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
    return q1.as(U{}) / q2.as(U{});
}

// The modulo operator (i.e., the remainder of an integer division).
//
// Only defined whenever (R1{} % R2{}) is defined (i.e., for integral Reps), _and_
// `CommonUnitT<U1, U2>` is also defined.  We convert to that common unit to perform the operation.
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto operator%(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
    return make_quantity<U>(q1.in(U{}) % q2.in(U{}));
}

// Type trait to detect whether two Quantity types are equivalent.
//
// In this library, Quantity types are "equivalent" exactly when they use the same Rep, and are
// based on equivalent units.
template <typename U1, typename U2, typename R1, typename R2>
struct AreQuantityTypesEquivalent<Quantity<U1, R1>, Quantity<U2, R2>>
    : stdx::conjunction<std::is_same<R1, R2>, AreUnitsQuantityEquivalent<U1, U2>> {};

// Cast Quantity to a different underlying type.
template <typename NewRep, typename Unit, typename Rep>
constexpr auto rep_cast(Quantity<Unit, Rep> q) {
    return q.template as<NewRep>(Unit{});
}

// Help Zero act more faithfully like a Quantity.
//
// Casting Zero to any "Rep" is trivial, because it has no Rep, and is already consistent with all.
template <typename NewRep>
constexpr auto rep_cast(Zero z) {
    return z;
}

template <typename UnitT>
struct QuantityMaker {
    using Unit = UnitT;
    static constexpr auto unit = Unit{};

    template <typename T>
    constexpr Quantity<Unit, T> operator()(T value) const {
        return {value};
    }

    template <typename U, typename R>
    constexpr void operator()(Quantity<U, R>) const {
        constexpr bool is_not_already_a_quantity = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_already_a_quantity, "Input to QuantityMaker is already a Quantity");
    }

    template <typename U, typename R>
    constexpr void operator()(QuantityPoint<U, R>) const {
        constexpr bool is_not_a_quantity_point = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_a_quantity_point, "Input to QuantityMaker is a QuantityPoint");
    }

    template <typename... BPs>
    constexpr auto operator*(Magnitude<BPs...> m) const {
        return QuantityMaker<decltype(unit * m)>{};
    }

    template <typename... BPs>
    constexpr auto operator/(Magnitude<BPs...> m) const {
        return QuantityMaker<decltype(unit / m)>{};
    }

    template <typename DivisorUnit>
    constexpr auto operator/(SingularNameFor<DivisorUnit>) const {
        return QuantityMaker<UnitQuotientT<Unit, DivisorUnit>>{};
    }

    template <typename MultiplierUnit>
    friend constexpr auto operator*(SingularNameFor<MultiplierUnit>, QuantityMaker) {
        return QuantityMaker<UnitProductT<MultiplierUnit, Unit>>{};
    }

    template <typename OtherUnit>
    constexpr auto operator*(QuantityMaker<OtherUnit>) const {
        return QuantityMaker<UnitProductT<Unit, OtherUnit>>{};
    }

    template <typename OtherUnit>
    constexpr auto operator/(QuantityMaker<OtherUnit>) const {
        return QuantityMaker<UnitQuotientT<Unit, OtherUnit>>{};
    }
};

template <typename U>
struct AssociatedUnit<QuantityMaker<U>> : stdx::type_identity<U> {};

template <int Exp, typename Unit>
constexpr auto pow(QuantityMaker<Unit>) {
    return QuantityMaker<UnitPowerT<Unit, Exp>>{};
}

template <int N, typename Unit>
constexpr auto root(QuantityMaker<Unit>) {
    return QuantityMaker<UnitPowerT<Unit, 1, N>>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Runtime conversion checkers

// Check conversion for overflow (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool will_conversion_overflow(Quantity<U, R> q, TargetUnitSlot) {
    using Op =
        detail::ConversionForRepsAndFactor<R, R, UnitRatioT<U, AssociatedUnitT<TargetUnitSlot>>>;
    return detail::would_value_overflow<Op>(q.in(U{}));
}

// Check conversion for overflow (new rep).
template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
constexpr bool will_conversion_overflow(Quantity<U, R> q, TargetUnitSlot) {
    using Op = detail::
        ConversionForRepsAndFactor<R, TargetRep, UnitRatioT<U, AssociatedUnitT<TargetUnitSlot>>>;
    return detail::would_value_overflow<Op>(q.in(U{}));
}

// Check conversion for truncation (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool will_conversion_truncate(Quantity<U, R> q, TargetUnitSlot) {
    using Op =
        detail::ConversionForRepsAndFactor<R, R, UnitRatioT<U, AssociatedUnitT<TargetUnitSlot>>>;
    return detail::TruncationRiskFor<Op>::would_value_truncate(q.in(U{}));
}

// Check conversion for truncation (new rep).
template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
constexpr bool will_conversion_truncate(Quantity<U, R> q, TargetUnitSlot) {
    using Op = detail::
        ConversionForRepsAndFactor<R, TargetRep, UnitRatioT<U, AssociatedUnitT<TargetUnitSlot>>>;
    return detail::TruncationRiskFor<Op>::would_value_truncate(q.in(U{}));
}

// Check for any lossiness in conversion (no change of rep).
template <typename U, typename R, typename TargetUnitSlot>
constexpr bool is_conversion_lossy(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return will_conversion_truncate(q, target_unit) || will_conversion_overflow(q, target_unit);
}

// Check for any lossiness in conversion (new rep).
template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
constexpr bool is_conversion_lossy(Quantity<U, R> q, TargetUnitSlot target_unit) {
    return will_conversion_truncate<TargetRep>(q, target_unit) ||
           will_conversion_overflow<TargetRep>(q, target_unit);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Comparing and/or combining Quantities of different types.

namespace detail {
// Helper to cast this Quantity to its common type with some other Quantity (explicitly supplied).
//
// Note that `TargetUnit` is supposed to be the common type of the input Quantity and some other
// Quantity.  This function should never be called directly; it should only be called by
// `using_common_type()`.  The program behaviour is undefined if anyone calls this function
// directly.  (In particular, we explicitly assume that the conversion to the Rep of TargetUnit is
// not narrowing for the input Quantity.)
//
// We would have liked this to just be a simple lambda, but some old compilers sometimes struggle
// with understanding that the lambda implementation of this can be constexpr.
template <typename TargetUnit, typename U, typename R>
constexpr auto cast_to_common_type(Quantity<U, R> q) {
    // When we perform a unit conversion to U, we need to make sure the library permits this
    // conversion *implicitly* for a rep R.  The form `rep_cast<R>(q).as(U{})` achieves
    // this.  First, we cast the Rep to R (which will typically be the wider of the input Reps).
    // Then, we use the *unit-only* form of the conversion operator: `as(U{})`, not
    // `as<R>(U{})`, because only the former actually checks the conversion policy.
    return rep_cast<typename TargetUnit::Rep>(q).as(TargetUnit::unit);
}

template <typename T, typename U, typename Func>
constexpr auto using_common_type(T t, U u, Func f) {
    using C = std::common_type_t<T, U>;
    static_assert(
        std::is_same<typename C::Rep, std::common_type_t<typename T::Rep, typename U::Rep>>::value,
        "Rep of common type is not common type of Reps (this should never occur)");

    return f(cast_to_common_type<C>(t), cast_to_common_type<C>(u));
}

template <typename Op, typename U1, typename U2, typename R1, typename R2>
constexpr auto convert_and_compare(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
    using ComRep1 = detail::CommonTypeButPreserveIntSignedness<R1, R2>;
    using ComRep2 = detail::CommonTypeButPreserveIntSignedness<R2, R1>;
    return detail::SignAwareComparison<UnitSign<U>, Op>{}(
        q1.template in<ComRep1>(U{}, check_for(ALL_RISKS)),
        q2.template in<ComRep2>(U{}, check_for(ALL_RISKS)));
}
}  // namespace detail

// Comparison functions for compatible Quantity types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator==(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::convert_and_compare<detail::Equal>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator!=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::convert_and_compare<detail::NotEqual>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator<(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::convert_and_compare<detail::Less>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator<=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::convert_and_compare<detail::LessEqual>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator>(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::convert_and_compare<detail::Greater>(q1, q2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr bool operator>=(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::convert_and_compare<detail::GreaterEqual>(q1, q2);
}

// Addition and subtraction functions for compatible Quantity types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator+(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::plus);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator-(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::minus);
}

// Mixed-type operations with a left-Quantity, and right-Quantity-equivalent.
template <typename U, typename R, typename QLike>
constexpr auto operator+(Quantity<U, R> q1, QLike q2) -> decltype(q1 + as_quantity(q2)) {
    return q1 + as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator-(Quantity<U, R> q1, QLike q2) -> decltype(q1 - as_quantity(q2)) {
    return q1 - as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator==(Quantity<U, R> q1, QLike q2) -> decltype(q1 == as_quantity(q2)) {
    return q1 == as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator!=(Quantity<U, R> q1, QLike q2) -> decltype(q1 != as_quantity(q2)) {
    return q1 != as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator<(Quantity<U, R> q1, QLike q2) -> decltype(q1 < as_quantity(q2)) {
    return q1 < as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator<=(Quantity<U, R> q1, QLike q2) -> decltype(q1 <= as_quantity(q2)) {
    return q1 <= as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator>(Quantity<U, R> q1, QLike q2) -> decltype(q1 > as_quantity(q2)) {
    return q1 > as_quantity(q2);
}
template <typename U, typename R, typename QLike>
constexpr auto operator>=(Quantity<U, R> q1, QLike q2) -> decltype(q1 >= as_quantity(q2)) {
    return q1 >= as_quantity(q2);
}

// Mixed-type operations with a left-Quantity-equivalent, and right-Quantity.
template <typename U, typename R, typename QLike>
constexpr auto operator+(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) + q2) {
    return as_quantity(q1) + q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator-(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) - q2) {
    return as_quantity(q1) - q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator==(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) == q2) {
    return as_quantity(q1) == q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator!=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) != q2) {
    return as_quantity(q1) != q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator<(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) < q2) {
    return as_quantity(q1) < q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator<=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) <= q2) {
    return as_quantity(q1) <= q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator>(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) > q2) {
    return as_quantity(q1) > q2;
}
template <typename U, typename R, typename QLike>
constexpr auto operator>=(QLike q1, Quantity<U, R> q2) -> decltype(as_quantity(q1) >= q2) {
    return as_quantity(q1) >= q2;
}

namespace detail {
template <typename Op>
struct SignAwareComparison<Magnitude<>, Op> {
    template <typename T1, typename T2>
    constexpr auto operator()(const T1 &lhs, const T2 &rhs) const {
        return Op{}(lhs, rhs);
    }
};

template <typename Op>
struct SignAwareComparison<Magnitude<Negative>, Op> {
    template <typename T1, typename T2>
    constexpr auto operator()(const T1 &lhs, const T2 &rhs) const {
        return Op{}(rhs, lhs);
    }
};
}  // namespace detail

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto operator<=>(const Quantity<U1, R1> &lhs, const Quantity<U2, R2> &rhs) {
    return detail::convert_and_compare<detail::ThreeWayCompare>(lhs, rhs);
}
#endif

// Helper to compute the `std::common_type_t` of two `Quantity` types.
//
// `std::common_type` requires its specializations to be SFINAE-friendly, meaning that the `type`
// member should not exist for specializations with no common type.  Unfortunately, we can't
// directly use SFINAE on `std::common_type`.  What we can do is inherit our specialization's
// implementation from a different structure which we fully control, and which either has or doesn't
// have a `type` member as appropriate.
template <typename Q1, typename Q2, typename Enable = void>
struct CommonQuantity {};
template <typename U1, typename U2, typename R1, typename R2>
struct CommonQuantity<Quantity<U1, R1>,
                      Quantity<U2, R2>,
                      std::enable_if_t<HasSameDimension<U1, U2>::value>>
    : stdx::type_identity<Quantity<CommonUnitT<U1, U2>, std::common_type_t<R1, R2>>> {};

//
// Formatter implementation for fmtlib or `std::format`.
//
// To use with fmtlib, add this template specialization to a file that includes both
// `"au/quantity.hh"`, and `"fmt/format.h"`:
//
//    namespace fmt {
//    template <typename U, typename R>
//    struct formatter<::au::Quantity<U, R>> : ::au::QuantityFormatter<U, R, ::fmt::formatter> {};
//    }  // namespace fmt
//
// Then, include that file any time you want to format a `Quantity`.
//
template <typename U, typename R, template <class...> class Formatter>
struct QuantityFormatter {
    template <typename FormatParseContext>
    constexpr auto parse_unit_label_part(FormatParseContext &ctx) {
        auto it = ctx.begin();

        if (it == ctx.end()) {
            return it;
        }

        if (*it != 'U') {
            return it;
        }
        // Consume the 'U'.
        ++it;

        // Parse the total width.
        while (it != ctx.end() && *it >= '0' && *it <= '9') {
            min_label_width_ = (min_label_width_ * 10) + static_cast<std::size_t>(*it++ - '0');
        }

        if (it == ctx.end() || *it == '}') {
            return it;
        }

        if (*it++ != ';') {
            // Cause an error condition in further parsing.
            it = ctx.end();
        }
        return it;
    }

    template <typename FormatParseContext>
    constexpr auto parse(FormatParseContext &ctx) {
        ctx.advance_to(parse_unit_label_part(ctx));
        return value_format.parse(ctx);
    }

    template <typename FormatContext>
    constexpr auto format(const au::Quantity<U, R> &q, FormatContext &ctx) const {
        value_format.format(q.data_in(U{}), ctx);
        *ctx.out()++ = ' ';
        return write_and_pad(unit_label(U{}), sizeof(unit_label(U{})), ctx);
    }

    template <typename FormatContext>
    constexpr auto write_and_pad(const char *data,
                                 std::size_t data_size,
                                 FormatContext &ctx) const {
        Formatter<const char *> unit_label_formatter{};
        unit_label_formatter.format(data, ctx);
        while (data_size <= min_label_width_) {
            *ctx.out()++ = ' ';
            ++data_size;
        }
        return ctx.out();
    }

    Formatter<R> value_format{};
    std::size_t min_label_width_{0};
};

}  // namespace au

namespace std {
// Note: we would prefer not to reopen `namespace std` [1].  However, some older compilers (which we
// still want to support) incorrectly treat the preferred syntax recommended in [1] as an error.
// This usage does not encounter any of the pitfalls described in that link, so we use it.
//
// [1] https://quuxplusone.github.io/blog/2021/10/27/dont-reopen-namespace-std/
template <typename U1, typename U2, typename R1, typename R2>
struct common_type<au::Quantity<U1, R1>, au::Quantity<U2, R2>>
    : au::CommonQuantity<au::Quantity<U1, R1>, au::Quantity<U2, R2>> {};
}  // namespace std

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct UnosLabel {
    static constexpr const char label[] = "U";
};
template <typename T>
constexpr const char UnosLabel<T>::label[];
struct Unos : UnitProductT<>, UnosLabel<void> {
    using UnosLabel<void>::label;
};
constexpr auto unos = QuantityMaker<Unos>{};

}  // namespace au


// "Mixin" classes to add operations for a "unit wrapper" --- that is, a template with a _single
// template parameter_ that is a unit.
//
// The operations are multiplication and division.  The mixins will specify what types the wrapper
// can combine with in this way, and what the resulting type will be.  They also take care of
// getting the resulting unit correct.  Finally, they handle integer division carefully.
//
// Every mixin has at least two template parameters.
//
//   1. The unit wrapper (a template template parameter).
//   2. The specific unit that it's wrapping (for convenience in the implementation).
//
// For mixins that compose with something that is _not_ a unit wrapper --- e.g., a raw number, or a
// magnitude --- this is all they need.  Other mixins compose with _other unit wrappers_, and these
// take two more template parameters: the wrapper we're composing with, and the resulting wrapper.

namespace au {
namespace detail {

// A SFINAE helper that is the identity, but only if we think a type is a valid rep.
//
// For now, we are restricting this to arithmetic types.  This doesn't mean they're the only reps we
// support; it just means they're the only reps we can _construct via this method_.  Later on, we
// would like to have a well-defined concept that defines what is and is not an acceptable rep for
// our `Quantity`.  Once we have that, we can simply constrain on that concept.  For more on this
// idea, see: https://github.com/aurora-opensource/au/issues/52
struct NoTypeMember {};
template <typename T>
struct TypeIdentityIfLooksLikeValidRep
    : std::conditional_t<std::is_arithmetic<T>::value, stdx::type_identity<T>, NoTypeMember> {};
template <typename T>
using TypeIdentityIfLooksLikeValidRepT = typename TypeIdentityIfLooksLikeValidRep<T>::type;

//
// A mixin that enables turning a raw number into a Quantity by multiplying or dividing.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct MakesQuantityFromNumber {
    // (N * W), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator*(T x, UnitWrapper<Unit>)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRepT<T>> {
        return make_quantity<Unit>(x);
    }

    // (W * N), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator*(UnitWrapper<Unit>, T x)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRepT<T>> {
        return make_quantity<Unit>(x);
    }

    // (N / W), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator/(T x, UnitWrapper<Unit>)
        -> Quantity<UnitInverseT<Unit>, TypeIdentityIfLooksLikeValidRepT<T>> {
        return make_quantity<UnitInverseT<Unit>>(x);
    }

    // (W / N), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator/(UnitWrapper<Unit>, T x)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRepT<T>> {
        static_assert(!std::is_integral<T>::value,
                      "Dividing by an integer value disallowed: would almost always produce 0");
        return make_quantity<Unit>(T{1} / x);
    }
};

//
// A mixin that enables scaling the units of a Quantity by multiplying or dividing.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct ScalesQuantity {
    // (W * Q), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator*(UnitWrapper<Unit>, Quantity<U, R> q) {
        return make_quantity<UnitProductT<Unit, U>>(q.in(U{}));
    }

    // (Q * W), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator*(Quantity<U, R> q, UnitWrapper<Unit>) {
        return make_quantity<UnitProductT<U, Unit>>(q.in(U{}));
    }

    // (Q / W), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator/(Quantity<U, R> q, UnitWrapper<Unit>) {
        return make_quantity<UnitQuotientT<U, Unit>>(q.in(U{}));
    }

    // (W / Q), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator/(UnitWrapper<Unit>, Quantity<U, R> q) {
        static_assert(!std::is_integral<R>::value,
                      "Dividing by an integer value disallowed: would almost always produce 0");
        return make_quantity<UnitQuotientT<Unit, U>>(R{1} / q.in(U{}));
    }
};

// A mixin to compose `op(U, O)` into a new unit wrapper, for "main" wrapper `U` and "other" wrapper
// `O`.  (Implementation detail helper for `ComposesWith`.)
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class OtherWrapper,
          template <typename U>
          class ResultWrapper>
struct PrecomposesWith {
    // (U * O), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitProductT<Unit, U>> operator*(UnitWrapper<Unit>,
                                                                    OtherWrapper<U>) {
        return {};
    }

    // (U / O), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitQuotientT<Unit, U>> operator/(UnitWrapper<Unit>,
                                                                     OtherWrapper<U>) {
        return {};
    }
};

// A mixin to compose `op(O, U)` into a new unit wrapper, for "main" wrapper `U` and "other" wrapper
// `O`.  (Implementation detail helper for `ComposesWith`.)
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class OtherWrapper,
          template <typename U>
          class ResultWrapper>
struct PostcomposesWith {
    // (O * U), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitProductT<U, Unit>> operator*(OtherWrapper<U>,
                                                                    UnitWrapper<Unit>) {
        return {};
    }

    // (O / U), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitQuotientT<U, Unit>> operator/(OtherWrapper<U>,
                                                                     UnitWrapper<Unit>) {
        return {};
    }
};

// An empty version of `PostcomposesWith` for when `UnitWrapper` is the same as `OtherWrapper`.
// In this case, if we left it non-empty, the definitions would be ambiguous/redundant with the ones
// in `PrecoposesWith`.
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class ResultWrapper>
struct PostcomposesWith<UnitWrapper, Unit, UnitWrapper, ResultWrapper> {};

//
// A mixin to compose two unit wrappers into a new unit wrapper.
//
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class OtherWrapper,
          template <typename U>
          class ResultWrapper>
struct ComposesWith : PrecomposesWith<UnitWrapper, Unit, OtherWrapper, ResultWrapper>,
                      PostcomposesWith<UnitWrapper, Unit, OtherWrapper, ResultWrapper> {};

//
// A mixin to enable scaling a unit wrapper by a magnitude.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct CanScaleByMagnitude {
    // (M * W), for magnitude M and wrapper W.
    template <typename... BPs>
    friend constexpr auto operator*(Magnitude<BPs...> m, UnitWrapper<Unit>) {
        return UnitWrapper<decltype(Unit{} * m)>{};
    }

    // (W * M), for magnitude M and wrapper W.
    template <typename... BPs>
    friend constexpr auto operator*(UnitWrapper<Unit>, Magnitude<BPs...> m) {
        return UnitWrapper<decltype(Unit{} * m)>{};
    }

    // (M / W), for magnitude M and wrapper W.
    template <typename... BPs>
    friend constexpr auto operator/(Magnitude<BPs...> m, UnitWrapper<Unit>) {
        return UnitWrapper<decltype(UnitInverseT<Unit>{} * m)>{};
    }

    // (W / M), for magnitude M and wrapper W.
    template <typename... BPs>
    friend constexpr auto operator/(UnitWrapper<Unit>, Magnitude<BPs...> m) {
        return UnitWrapper<decltype(Unit{} / m)>{};
    }

    friend constexpr auto operator-(UnitWrapper<Unit>) {
        return UnitWrapper<decltype(Unit{} * (-mag<1>()))>{};
    }
};

//
// A mixin to enable raising a unit wrapper to a rational power.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct SupportsRationalPowers {
    // (W^N), for wrapper W and integer N.
    template <std::intmax_t N>
    friend constexpr auto pow(UnitWrapper<Unit>) {
        return UnitWrapper<UnitPowerT<Unit, N>>{};
    }

    // (W^(1/N)), for wrapper W and integer N.
    template <std::intmax_t N>
    friend constexpr auto root(UnitWrapper<Unit>) {
        return UnitWrapper<UnitPowerT<Unit, 1, N>>{};
    }
};

}  // namespace detail
}  // namespace au


namespace au {

//
// A monovalue type to represent a constant value, including its units, if any.
//
// Users can multiply or divide `Constant` instances by raw numbers or `Quantity` instances, and it
// will perform symbolic arithmetic at compile time without affecting the stored numeric value.
// `Constant` also composes with other constants, and with `QuantityMaker` and other related types.
//
// Although `Constant` does not have any specific numeric type associated with it (as opposed to
// `Quantity`), it can easily convert to any appropriate `Quantity` type, with any rep.  Unlike
// `Quantity`, these conversions support _exact_ safety checks, so that every conversion producing a
// correctly representable value will succeed, and every unrepresentable conversion will fail.
//
template <typename Unit>
struct Constant : detail::MakesQuantityFromNumber<Constant, Unit>,
                  detail::ScalesQuantity<Constant, Unit>,
                  detail::ComposesWith<Constant, Unit, Constant, Constant>,
                  detail::ComposesWith<Constant, Unit, QuantityMaker, QuantityMaker>,
                  detail::ComposesWith<Constant, Unit, SingularNameFor, SingularNameFor>,
                  detail::SupportsRationalPowers<Constant, Unit>,
                  detail::CanScaleByMagnitude<Constant, Unit> {
    // Convert this constant to a Quantity of the given rep.
    template <typename T>
    constexpr auto as() const {
        return make_quantity<Unit>(static_cast<T>(1));
    }

    // Convert this constant to a Quantity of the given unit and rep, ignoring safety checks.
    template <typename T, typename OtherUnit>
    constexpr auto coerce_as(OtherUnit u) const {
        return as<T>().coerce_as(u);
    }

    // Convert this constant to a Quantity of the given unit and rep.
    template <typename T, typename OtherUnit>
    constexpr auto as(OtherUnit u) const {
        return as<T>(u, check_for(ALL_RISKS));
    }

    // Convert this constant to a Quantity of the given unit and rep, following this risk policy.
    template <typename T, typename OtherUnit, typename RiskPolicyT>
    constexpr auto as(OtherUnit, RiskPolicyT) const {
        constexpr auto this_value = make_quantity<Unit>(static_cast<T>(1));

        constexpr bool has_unacceptable_overflow =
            RiskPolicyT{}.should_check(detail::ConversionRisk::Overflow) &&
            will_conversion_overflow(this_value, OtherUnit{});
        static_assert(!has_unacceptable_overflow, "Constant conversion known to overflow");

        constexpr bool has_unacceptable_truncation =
            RiskPolicyT{}.should_check(detail::ConversionRisk::Truncation) &&
            will_conversion_truncate(this_value, OtherUnit{});
        static_assert(!has_unacceptable_truncation, "Constant conversion known to truncate");

        return this_value.as(OtherUnit{}, ignore(ALL_RISKS));
    }

    // Get the value of this constant in the given unit and rep, ignoring safety checks.
    template <typename T, typename OtherUnit>
    constexpr auto coerce_in(OtherUnit u) const {
        return as<T>().coerce_in(u);
    }

    // Get the value of this constant in the given unit and rep.
    template <typename T, typename OtherUnit>
    constexpr auto in(OtherUnit u) const {
        return in<T>(u, check_for(ALL_RISKS));
    }

    // Get the value of this constant in the given unit and rep, following this risk policy.
    template <typename T, typename OtherUnit, typename RiskPolicyT>
    constexpr auto in(OtherUnit u, RiskPolicyT policy) const {
        return as<T>(u, policy).in(u);
    }

    // Implicitly convert to any quantity type which passes safety checks.
    template <typename U, typename R>
    constexpr operator Quantity<U, R>() const {
        return as<R>(U{});
    }

    // Static function to check whether this constant can be exactly-represented in the given rep
    // `T` and unit `OtherUnit`.
    template <typename T, typename OtherUnit>
    static constexpr bool can_store_value_in(OtherUnit other) {
        return representable_in<T>(unit_ratio(Unit{}, other));
    }

    // Implicitly convert to type with an exactly corresponding quantity that passes safety checks.
    template <
        typename T,
        typename = std::enable_if_t<can_store_value_in<typename CorrespondingQuantity<T>::Rep>(
            typename CorrespondingQuantity<T>::Unit{})>>
    constexpr operator T() const {
        return as<typename CorrespondingQuantity<T>::Rep>(
            typename CorrespondingQuantity<T>::Unit{});
    }
};

// Make a constant from the given unit.
//
// Note that the argument is a _unit slot_, and thus can also accept things like `QuantityMaker` and
// `SymbolFor` in addition to regular units.
template <typename UnitSlot>
constexpr Constant<AssociatedUnitT<UnitSlot>> make_constant(UnitSlot) {
    return {};
}

constexpr Zero make_constant(Zero) { return {}; }

// Support using `Constant` in a unit slot.
template <typename Unit>
struct AssociatedUnit<Constant<Unit>> : stdx::type_identity<Unit> {};

}  // namespace au


namespace au {

//
// A representation of the symbol for a unit.
//
// To use, create an instance variable templated on a unit, and make the instance variable's name
// the symbol to represent.  For example:
//
//     constexpr auto m = SymbolFor<Meters>{};
//
template <typename Unit>
struct SymbolFor : detail::MakesQuantityFromNumber<SymbolFor, Unit>,
                   detail::ScalesQuantity<SymbolFor, Unit>,
                   detail::ComposesWith<SymbolFor, Unit, SymbolFor, SymbolFor>,
                   detail::SupportsRationalPowers<SymbolFor, Unit>,
                   detail::CanScaleByMagnitude<SymbolFor, Unit> {};

//
// Create a unit symbol using the more fluent APIs that unit slots make possible.  For example:
//
//     constexpr auto mps = symbol_for(meters / second);
//
// This is generally easier to work with and makes code that is easier to read, at the cost of being
// (very slightly) slower to compile.
//
template <typename UnitSlot>
constexpr auto symbol_for(UnitSlot) {
    return SymbolFor<AssociatedUnitT<UnitSlot>>{};
}

// Support using symbols in unit slot APIs (e.g., `v.in(m / s)`).
template <typename U>
struct AssociatedUnit<SymbolFor<U>> : stdx::type_identity<U> {};

}  // namespace au


namespace au {

// `QuantityPoint`: an _affine space type_ modeling points on a line.
//
// For a quick primer on affine space types, see: http://videocortex.io/2018/Affine-Space-Types/
//
// By "modeling points", we mean that `QuantityPoint` instances cannot be added to each other, and
// cannot be multiplied.  However, they can be subtracted: the difference between two
// `QuantityPoint` instances (of the same Unit) is a `Quantity` of that unit.  We can also add a
// `Quantity` to a `QuantityPoint`, and vice versa; the result is a new `QuantityPoint`.
//
// Key motivating examples include _mile markers_ (effectively `QuantityPoint<Miles, T>`), and
// _absolute temperature measurements_ (e.g., `QuantityPoint<Celsius, T>`).  This type is also
// analogous to `std::chrono::time_point`, in the same way that `Quantity` is analogous to
// `std::chrono::duration`.

// Make a Quantity of the given Unit, which has this value as measured in the Unit.
template <typename UnitT, typename T>
constexpr auto make_quantity_point(T value) {
    return QuantityPointMaker<UnitT>{}(value);
}

// Trait to check whether two QuantityPoint types are exactly equivalent.
template <typename P1, typename P2>
struct AreQuantityPointTypesEquivalent;

namespace detail {
template <typename FromRep, typename ToRep>
struct IntermediateRep;
}  // namespace detail

// Some units have an "origin".  This is not meaningful by itself, but its difference w.r.t. the
// "origin" of another unit of the same Dimension _is_ meaningful.  This type trait provides access
// to that difference.
template <typename U1, typename U2>
constexpr auto origin_displacement(U1, U2) {
    return make_constant(detail::ComputeOriginDisplacementUnit<AssociatedUnitForPointsT<U1>,
                                                               AssociatedUnitForPointsT<U2>>{});
}

template <typename U1, typename U2>
using OriginDisplacement = decltype(origin_displacement(U1{}, U2{}));

// QuantityPoint implementation and API elaboration.
template <typename UnitT, typename RepT>
class QuantityPoint {
    // Q: When should we enable IMPLICIT construction from another QuantityPoint type?
    // A: EXACTLY WHEN our own Diff type can be IMPLICITLY constructed from BOTH the target's Diff
    //    type AND the offset between our Units' zero points.
    //
    // In other words, there are two ways to fail implicit convertibility.
    //
    //   1. Their Diff type might not work with our Rep.  Examples:
    //      BAD: QuantityPoint<Milli<Meters>, int> -> QuantityPoint<Meters, int>
    //      OK : QuantityPoint<Kilo<Meters> , int> -> QuantityPoint<Meters, int>
    //
    //   2. Their zero point might be offset from ours by a non-representable amount.  Examples:
    //      BAD: QuantityPoint<Celsius, int> -> QuantityPoint<Kelvins, int>
    //      OK : QuantityPoint<Celsius, int> -> QuantityPoint<Kelvins, double>
    //      OK : QuantityPoint<Celsius, int> -> QuantityPoint<Milli<Kelvins>, int>
    template <typename OtherUnit, typename OtherRep>
    static constexpr bool should_enable_implicit_construction_from() {
        using Com = CommonUnitT<OtherUnit, detail::ComputeOriginDisplacementUnit<Unit, OtherUnit>>;
        return std::is_convertible<Quantity<Com, OtherRep>, QuantityPoint::Diff>::value;
    }

    // This machinery exists to give us a conditionally explicit constructor, using SFINAE to select
    // the explicit or implicit version (https://stackoverflow.com/a/26949793/15777264).  If we had
    // C++20, we could use the `explicit(bool)` feature, making this code simpler and faster.
    template <bool ImplicitOk, typename OtherUnit, typename OtherRep>
    using EnableIfImplicitOkIs = std::enable_if_t<
        ImplicitOk ==
        QuantityPoint::should_enable_implicit_construction_from<OtherUnit, OtherRep>()>;

 public:
    using Rep = RepT;
    using Unit = UnitT;
    static constexpr Unit unit{};
    using Diff = Quantity<Unit, Rep>;

    // The default constructor produces a QuantityPoint whose value is default constructed.  It
    // exists to give you an object you can assign to.  The main motivating factor for including
    // this is to support `std::atomic`, which requires its types to be default-constructible.
    constexpr QuantityPoint() noexcept : x_{} {}

    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<true, OtherUnit, OtherRep>>
    constexpr QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other)  // NOLINT(runtime/explicit)
        : QuantityPoint{other.template as<Rep>(unit)} {}

    template <typename OtherUnit,
              typename OtherRep,
              typename Enable = EnableIfImplicitOkIs<false, OtherUnit, OtherRep>,
              typename ThisUnusedTemplateParameterDistinguishesUsFromTheAboveConstructor = void>
    // Deleted: use `.as<NewRep>(new_unit)` to force a cast.
    constexpr explicit QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other) = delete;

    // Construct from another QuantityPoint with an explicit conversion risk policy.
    template <typename OtherUnit,
              typename OtherRep,
              typename RiskPolicyT,
              std::enable_if_t<IsConversionRiskPolicy<RiskPolicyT>::value, int> = 0>
    constexpr QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other, RiskPolicyT policy)
        : QuantityPoint{other.template as<Rep>(Unit{}, policy)} {}

    // The notion of "0" is *not* unambiguous for point types, because different scales can make
    // different decisions about what point is labeled as "0".
    constexpr QuantityPoint(Zero) = delete;

    template <typename NewRep, typename NewUnit, typename RiskPolicyT = decltype(ignore(ALL_RISKS))>
    constexpr auto as(NewUnit u, RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity_point<AssociatedUnitForPointsT<NewUnit>>(in_impl<NewRep>(u, policy));
    }

    template <typename NewUnit, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
    constexpr auto as(NewUnit u, RiskPolicyT policy = RiskPolicyT{}) const {
        return make_quantity_point<AssociatedUnitForPointsT<NewUnit>>(in_impl<Rep>(u, policy));
    }

    template <typename NewRep, typename NewUnit, typename RiskPolicyT = decltype(ignore(ALL_RISKS))>
    constexpr NewRep in(NewUnit u, RiskPolicyT policy = RiskPolicyT{}) const {
        return in_impl<NewRep>(u, policy);
    }

    template <typename NewUnit, typename RiskPolicyT = decltype(check_for(ALL_RISKS))>
    constexpr Rep in(NewUnit u, RiskPolicyT policy = RiskPolicyT{}) const {
        return in_impl<Rep>(u, policy);
    }

    // "Forcing" conversions, which explicitly ignore safety checks for overflow and truncation.
    template <typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `p.coerce_as(new_units)`.
        return as<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_as(NewUnit) const {
        // Usage example: `p.coerce_as<T>(new_units)`.
        return as<NewRep>(NewUnit{});
    }
    template <typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `p.coerce_in(new_units)`.
        return in<Rep>(NewUnit{});
    }
    template <typename NewRep, typename NewUnit>
    constexpr auto coerce_in(NewUnit) const {
        // Usage example: `p.coerce_in<T>(new_units)`.
        return in<NewRep>(NewUnit{});
    }

    // Direct access to the underlying value member, with any Point-equivalent Unit.
    //
    // Mutable access:
    template <typename UnitSlot>
    constexpr Rep &data_in(UnitSlot) {
        static_assert(AreUnitsPointEquivalent<AssociatedUnitForPointsT<UnitSlot>, Unit>::value,
                      "Can only access value via Point-equivalent unit");
        return x_.data_in(AssociatedUnitForPointsT<UnitSlot>{});
    }
    // Const access:
    template <typename UnitSlot>
    constexpr const Rep &data_in(UnitSlot) const {
        static_assert(AreUnitsPointEquivalent<AssociatedUnitForPointsT<UnitSlot>, Unit>::value,
                      "Can only access value via Point-equivalent unit");
        return x_.data_in(AssociatedUnitForPointsT<UnitSlot>{});
    }

    // Comparison operators.
    constexpr friend bool operator==(QuantityPoint a, QuantityPoint b) { return a.x_ == b.x_; }
    constexpr friend bool operator!=(QuantityPoint a, QuantityPoint b) { return a.x_ != b.x_; }
    constexpr friend bool operator>=(QuantityPoint a, QuantityPoint b) { return a.x_ >= b.x_; }
    constexpr friend bool operator>(QuantityPoint a, QuantityPoint b) { return a.x_ > b.x_; }
    constexpr friend bool operator<=(QuantityPoint a, QuantityPoint b) { return a.x_ <= b.x_; }
    constexpr friend bool operator<(QuantityPoint a, QuantityPoint b) { return a.x_ < b.x_; }

    // Subtraction between two QuantityPoint types.
    constexpr friend Diff operator-(QuantityPoint a, QuantityPoint b) { return a.x_ - b.x_; }

    // Left and right addition of a Diff.
    constexpr friend auto operator+(Diff d, QuantityPoint p) { return QuantityPoint{d + p.x_}; }
    constexpr friend auto operator+(QuantityPoint p, Diff d) { return QuantityPoint{p.x_ + d}; }

    // Right subtraction of a Diff.
    constexpr friend auto operator-(QuantityPoint p, Diff d) { return QuantityPoint{p.x_ - d}; }

    // Short-hand addition assignment.
    constexpr QuantityPoint &operator+=(Diff diff) {
        x_ += diff;
        return *this;
    }

    // Short-hand subtraction assignment.
    constexpr QuantityPoint &operator-=(Diff diff) {
        x_ -= diff;
        return *this;
    }

    // Permit this factory functor to access our private constructor.
    //
    // We allow this because it explicitly names the unit at the callsite, even if people refer to
    // this present Quantity type by an alias that omits the unit.  This preserves Unit Safety and
    // promotes callsite readability.
    friend struct QuantityPointMaker<Unit>;

 private:
    template <typename OtherRep, typename OtherPointUnitSlot, typename RiskPolicyT>
    constexpr OtherRep in_impl(OtherPointUnitSlot, RiskPolicyT policy) const {
        using OtherUnit = AssociatedUnitForPointsT<OtherPointUnitSlot>;
        using OriginDisplacementUnit = detail::ComputeOriginDisplacementUnit<Unit, OtherUnit>;
        using Common = CommonUnitT<Unit, OtherUnit, OriginDisplacementUnit>;

        using CalcRep = typename detail::IntermediateRep<Rep, OtherRep>::type;

        Quantity<Common, CalcRep> intermediate_result =
            x_.template as<CalcRep>(Common{}, policy) + origin_displacement(OtherUnit{}, unit);
        return intermediate_result.template in<OtherRep>(OtherUnit{}, policy);
    }

    constexpr explicit QuantityPoint(Diff x) : x_{x} {}

    Diff x_;
};

template <typename Unit>
struct QuantityPointMaker {
    static constexpr auto unit = Unit{};

    template <typename T>
    constexpr auto operator()(T value) const {
        return QuantityPoint<Unit, T>{make_quantity<Unit>(value)};
    }

    template <typename U, typename R>
    constexpr void operator()(Quantity<U, R>) const {
        constexpr bool is_not_a_quantity = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_a_quantity, "Input to QuantityPointMaker is a Quantity");
    }

    template <typename U, typename R>
    constexpr void operator()(QuantityPoint<U, R>) const {
        constexpr bool is_not_already_a_quantity_point = detail::AlwaysFalse<U, R>::value;
        static_assert(is_not_already_a_quantity_point,
                      "Input to QuantityPointMaker is already a QuantityPoint");
    }

    template <typename... BPs>
    constexpr auto operator*(Magnitude<BPs...> m) const {
        return QuantityPointMaker<decltype(unit * m)>{};
    }

    template <typename... BPs>
    constexpr auto operator/(Magnitude<BPs...> m) const {
        return QuantityPointMaker<decltype(unit / m)>{};
    }
};

template <typename U>
struct AssociatedUnitForPoints<QuantityPointMaker<U>> : stdx::type_identity<U> {};

// Provide nicer error messages when users try passing a `QuantityPoint` to a unit slot.
template <typename U, typename R>
struct AssociatedUnit<QuantityPoint<U, R>> {
    static_assert(
        detail::AlwaysFalse<U, R>::value,
        "Cannot pass QuantityPoint to a unit slot (see: "
        "https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)");
};
template <typename U, typename R>
struct AssociatedUnitForPoints<QuantityPoint<U, R>> {
    static_assert(
        detail::AlwaysFalse<U, R>::value,
        "Cannot pass QuantityPoint to a unit slot (see: "
        "https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)");
};

// Type trait to detect whether two QuantityPoint types are equivalent.
//
// In this library, QuantityPoint types are "equivalent" exactly when they use the same Rep, and are
// based on point-equivalent units.
template <typename U1, typename U2, typename R1, typename R2>
struct AreQuantityPointTypesEquivalent<QuantityPoint<U1, R1>, QuantityPoint<U2, R2>>
    : stdx::conjunction<std::is_same<R1, R2>, AreUnitsPointEquivalent<U1, U2>> {};

// Cast QuantityPoint to a different underlying type.
template <typename NewRep, typename Unit, typename Rep>
constexpr auto rep_cast(QuantityPoint<Unit, Rep> q) {
    return q.template as<NewRep>(Unit{});
}

namespace detail {
template <typename X, typename Y, typename Func>
constexpr auto using_common_point_unit(X x, Y y, Func f) {
    using R = std::common_type_t<typename X::Rep, typename Y::Rep>;
    constexpr auto u = CommonPointUnitT<typename X::Unit, typename Y::Unit>{};
    return f(rep_cast<R>(x).as(u), rep_cast<R>(y).as(u));
}

template <typename Op, typename U1, typename U2, typename R1, typename R2>
constexpr auto convert_and_compare(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    using U = CommonPointUnitT<U1, U2>;
    using ComRep1 = detail::CommonTypeButPreserveIntSignedness<R1, R2>;
    using ComRep2 = detail::CommonTypeButPreserveIntSignedness<R2, R1>;
    return detail::SignAwareComparison<UnitSign<U>, Op>{}(
        p1.template in<ComRep1>(U{}, check_for(ALL_RISKS)),
        p2.template in<ComRep2>(U{}, check_for(ALL_RISKS)));
}
}  // namespace detail

// Comparison functions for compatible QuantityPoint types.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator<(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::Less>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator>(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::Greater>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator<=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::LessEqual>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator>=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::GreaterEqual>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator==(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::Equal>(p1, p2);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator!=(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::convert_and_compare<detail::NotEqual>(p1, p2);
}

namespace detail {
// Another subtlety arises when we mix QuantityPoint and Quantity in adding or subtracting.  We
// actually don't want to use `CommonPointUnitT`, because this is too restrictive if the units have
// different origins.  Imagine adding a `Quantity<Kelvins>` to a `QuantityPoint<Celsius>`---we
// wouldn't want this to subdivide the unit of measure to satisfy an additive relative offset which
// we will never actually use!
//
// The solution is to set the (unused!) origin of the `Quantity` unit to the same as the
// `QuantityPoint` unit.  Once we do, everything flows simply from there.
//
// This utility should be used for every overload below which combines a `QuantityPoint` with a
// `Quantity`.
template <typename Target, typename U>
constexpr auto borrow_origin(U u) {
    return Target{} * unit_ratio(u, Target{});
}
}  // namespace detail

// Addition and subtraction functions for compatible QuantityPoint types.
template <typename UnitP, typename UnitQ, typename RepP, typename RepQ>
constexpr auto operator+(QuantityPoint<UnitP, RepP> p, Quantity<UnitQ, RepQ> q) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(p, q.as(new_unit_q), detail::plus);
}
template <typename UnitQ, typename UnitP, typename RepQ, typename RepP>
constexpr auto operator+(Quantity<UnitQ, RepQ> q, QuantityPoint<UnitP, RepP> p) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(q.as(new_unit_q), p, detail::plus);
}
template <typename UnitP, typename UnitQ, typename R1, typename RepQ>
constexpr auto operator-(QuantityPoint<UnitP, R1> p, Quantity<UnitQ, RepQ> q) {
    constexpr auto new_unit_q = detail::borrow_origin<UnitP>(UnitQ{});
    return detail::using_common_point_unit(p, q.as(new_unit_q), detail::minus);
}
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto operator-(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::minus);
}

#if defined(__cpp_impl_three_way_comparison) && __cpp_impl_three_way_comparison >= 201907L
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto operator<=>(const QuantityPoint<U1, R1> &lhs, const QuantityPoint<U2, R2> &rhs) {
    return detail::convert_and_compare<detail::ThreeWayCompare>(lhs, rhs);
}
#endif

namespace detail {

// We simply want a version of `std::make_signed_t` that won't choke on non-integral types.
template <typename T>
struct MakeSigned : std::conditional<std::is_integral<T>::value, std::make_signed_t<T>, T> {};

// If the destination is a signed integer, we want to ensure we do our
// computations in a signed type.  Otherwise, just use the common type for our
// intermediate computations.
template <typename CommonT, bool IsDestinationSigned>
struct IntermediateRepImpl
    : std::conditional_t<stdx::conjunction<std::is_integral<CommonT>,
                                           stdx::bool_constant<IsDestinationSigned>>::value,
                         MakeSigned<CommonT>,
                         stdx::type_identity<CommonT>> {};

template <typename FromRep, typename ToRep>
struct IntermediateRep
    : IntermediateRepImpl<std::common_type_t<FromRep, ToRep>, std::is_signed<ToRep>::value> {};

}  // namespace detail
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct BitsLabel {
    static constexpr const char label[] = "b";
};
template <typename T>
constexpr const char BitsLabel<T>::label[];
struct Bits : UnitImpl<Information>, BitsLabel<void> {
    using BitsLabel<void>::label;
};
constexpr auto bit = SingularNameFor<Bits>{};
constexpr auto bits = QuantityMaker<Bits>{};

namespace symbols {
constexpr auto b = SymbolFor<Bits>{};
}
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct RadiansLabel {
    static constexpr const char label[] = "rad";
};
template <typename T>
constexpr const char RadiansLabel<T>::label[];
struct Radians : UnitImpl<Angle>, RadiansLabel<void> {
    using RadiansLabel<void>::label;
};
constexpr auto radian = SingularNameFor<Radians>{};
constexpr auto radians = QuantityMaker<Radians>{};

namespace symbols {
constexpr auto rad = SymbolFor<Radians>{};
}
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct CandelasLabel {
    static constexpr const char label[] = "cd";
};
template <typename T>
constexpr const char CandelasLabel<T>::label[];
struct Candelas : UnitImpl<LuminousIntensity>, CandelasLabel<void> {
    using CandelasLabel<void>::label;
};
constexpr auto candela = SingularNameFor<Candelas>{};
constexpr auto candelas = QuantityMaker<Candelas>{};

namespace symbols {
constexpr auto cd = SymbolFor<Candelas>{};
}
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct MolesLabel {
    static constexpr const char label[] = "mol";
};
template <typename T>
constexpr const char MolesLabel<T>::label[];
struct Moles : UnitImpl<AmountOfSubstance>, MolesLabel<void> {
    using MolesLabel<void>::label;
};
constexpr auto mole = SingularNameFor<Moles>{};
constexpr auto moles = QuantityMaker<Moles>{};

namespace symbols {
constexpr auto mol = SymbolFor<Moles>{};
}
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct AmperesLabel {
    static constexpr const char label[] = "A";
};
template <typename T>
constexpr const char AmperesLabel<T>::label[];
struct Amperes : UnitImpl<Current>, AmperesLabel<void> {
    using AmperesLabel<void>::label;
};
constexpr auto ampere = SingularNameFor<Amperes>{};
constexpr auto amperes = QuantityMaker<Amperes>{};

namespace symbols {
constexpr auto A = SymbolFor<Amperes>{};
}

}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct KelvinsLabel {
    static constexpr const char label[] = "K";
};
template <typename T>
constexpr const char KelvinsLabel<T>::label[];
struct Kelvins : UnitImpl<Temperature>, KelvinsLabel<void> {
    using KelvinsLabel<void>::label;
};
constexpr auto kelvin = SingularNameFor<Kelvins>{};
constexpr auto kelvins = QuantityMaker<Kelvins>{};
constexpr auto kelvins_pt = QuantityPointMaker<Kelvins>{};

namespace symbols {
constexpr auto K = SymbolFor<Kelvins>{};
}
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct GramsLabel {
    static constexpr const char label[] = "g";
};
template <typename T>
constexpr const char GramsLabel<T>::label[];
struct Grams : UnitImpl<Mass>, GramsLabel<void> {
    using GramsLabel<void>::label;
};
constexpr auto gram = SingularNameFor<Grams>{};
constexpr auto grams = QuantityMaker<Grams>{};

namespace symbols {
constexpr auto g = SymbolFor<Grams>{};
}
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct SecondsLabel {
    static constexpr const char label[] = "s";
};
template <typename T>
constexpr const char SecondsLabel<T>::label[];
struct Seconds : UnitImpl<Time>, SecondsLabel<void> {
    using SecondsLabel<void>::label;
};
constexpr auto second = SingularNameFor<Seconds>{};
constexpr auto seconds = QuantityMaker<Seconds>{};

namespace symbols {
constexpr auto s = SymbolFor<Seconds>{};
}
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct MetersLabel {
    static constexpr const char label[] = "m";
};
template <typename T>
constexpr const char MetersLabel<T>::label[];
struct Meters : UnitImpl<Length>, MetersLabel<void> {
    using MetersLabel<void>::label;
};
constexpr auto meter = SingularNameFor<Meters>{};
constexpr auto meters = QuantityMaker<Meters>{};
constexpr auto meters_pt = QuantityPointMaker<Meters>{};

namespace symbols {
constexpr auto m = SymbolFor<Meters>{};
}
}  // namespace au


namespace au {

template <template <class U> class Prefix>
struct PrefixApplier {
    // Applying a Prefix to a Unit instance, creates an instance of the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(U) const {
        return Prefix<U>{};
    }

    // Applying a Prefix to a QuantityMaker instance, creates a maker for the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(QuantityMaker<U>) const {
        return QuantityMaker<Prefix<U>>{};
    }

    // Applying a Prefix to a QuantityPointMaker instance, changes it to make the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(QuantityPointMaker<U>) const {
        return QuantityPointMaker<Prefix<U>>{};
    }

    // Applying a Prefix to a SingularNameFor instance, creates a singularly-named instance of the
    // Prefixed Unit.
    template <typename U>
    constexpr auto operator()(SingularNameFor<U>) const {
        return SingularNameFor<Prefix<U>>{};
    }

    // Applying a Prefix to a SymbolFor instance, creates a symbolically-named instance of the
    // Prefixed unit.
    template <typename U>
    constexpr auto operator()(SymbolFor<U>) const {
        return SymbolFor<Prefix<U>>{};
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SI Prefixes.

template <typename U>
struct Quetta : decltype(U{} * pow<30>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Q", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Quetta<U>::label;
constexpr auto quetta = PrefixApplier<Quetta>{};

template <typename U>
struct Ronna : decltype(U{} * pow<27>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("R", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Ronna<U>::label;
constexpr auto ronna = PrefixApplier<Ronna>{};

template <typename U>
struct Yotta : decltype(U{} * pow<24>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Y", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Yotta<U>::label;
constexpr auto yotta = PrefixApplier<Yotta>{};

template <typename U>
struct Zetta : decltype(U{} * pow<21>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Z", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Zetta<U>::label;
constexpr auto zetta = PrefixApplier<Zetta>{};

template <typename U>
struct Exa : decltype(U{} * pow<18>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("E", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Exa<U>::label;
constexpr auto exa = PrefixApplier<Exa>{};

template <typename U>
struct Peta : decltype(U{} * pow<15>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("P", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Peta<U>::label;
constexpr auto peta = PrefixApplier<Peta>{};

template <typename U>
struct Tera : decltype(U{} * pow<12>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("T", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Tera<U>::label;
constexpr auto tera = PrefixApplier<Tera>{};

template <typename U>
struct Giga : decltype(U{} * pow<9>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("G", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Giga<U>::label;
constexpr auto giga = PrefixApplier<Giga>{};

template <typename U>
struct Mega : decltype(U{} * pow<6>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("M", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Mega<U>::label;
constexpr auto mega = PrefixApplier<Mega>{};

template <typename U>
struct Kilo : decltype(U{} * pow<3>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("k", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Kilo<U>::label;
constexpr auto kilo = PrefixApplier<Kilo>{};

template <typename U>
struct Hecto : decltype(U{} * pow<2>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("h", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Hecto<U>::label;
constexpr auto hecto = PrefixApplier<Hecto>{};

template <typename U>
struct Deka : decltype(U{} * pow<1>(mag<10>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("da", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Deka<U>::label;
constexpr auto deka = PrefixApplier<Deka>{};

template <typename U>
struct Deci : decltype(U{} * pow<-1>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("d", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Deci<U>::label;
constexpr auto deci = PrefixApplier<Deci>{};

template <typename U>
struct Centi : decltype(U{} * pow<-2>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("c", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Centi<U>::label;
constexpr auto centi = PrefixApplier<Centi>{};

template <typename U>
struct Milli : decltype(U{} * pow<-3>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("m", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Milli<U>::label;
constexpr auto milli = PrefixApplier<Milli>{};

template <typename U>
struct Micro : decltype(U{} * pow<-6>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("u", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Micro<U>::label;
constexpr auto micro = PrefixApplier<Micro>{};

template <typename U>
struct Nano : decltype(U{} * pow<-9>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("n", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Nano<U>::label;
constexpr auto nano = PrefixApplier<Nano>{};

template <typename U>
struct Pico : decltype(U{} * pow<-12>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("p", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Pico<U>::label;
constexpr auto pico = PrefixApplier<Pico>{};

template <typename U>
struct Femto : decltype(U{} * pow<-15>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("f", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Femto<U>::label;
constexpr auto femto = PrefixApplier<Femto>{};

template <typename U>
struct Atto : decltype(U{} * pow<-18>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("a", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Atto<U>::label;
constexpr auto atto = PrefixApplier<Atto>{};

template <typename U>
struct Zepto : decltype(U{} * pow<-21>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("z", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Zepto<U>::label;
constexpr auto zepto = PrefixApplier<Zepto>{};

template <typename U>
struct Yocto : decltype(U{} * pow<-24>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("y", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Yocto<U>::label;
constexpr auto yocto = PrefixApplier<Yocto>{};

template <typename U>
struct Ronto : decltype(U{} * pow<-27>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("r", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Ronto<U>::label;
constexpr auto ronto = PrefixApplier<Ronto>{};

template <typename U>
struct Quecto : decltype(U{} * pow<-30>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("q", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Quecto<U>::label;
constexpr auto quecto = PrefixApplier<Quecto>{};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary Prefixes.

template <typename U>
struct Yobi : decltype(U{} * pow<80>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Yi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Yobi<U>::label;
constexpr auto yobi = PrefixApplier<Yobi>{};

template <typename U>
struct Zebi : decltype(U{} * pow<70>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Zi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Zebi<U>::label;
constexpr auto zebi = PrefixApplier<Zebi>{};

template <typename U>
struct Exbi : decltype(U{} * pow<60>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ei", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Exbi<U>::label;
constexpr auto exbi = PrefixApplier<Exbi>{};

template <typename U>
struct Pebi : decltype(U{} * pow<50>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Pi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Pebi<U>::label;
constexpr auto pebi = PrefixApplier<Pebi>{};

template <typename U>
struct Tebi : decltype(U{} * pow<40>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ti", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Tebi<U>::label;
constexpr auto tebi = PrefixApplier<Tebi>{};

template <typename U>
struct Gibi : decltype(U{} * pow<30>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Gi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Gibi<U>::label;
constexpr auto gibi = PrefixApplier<Gibi>{};

template <typename U>
struct Mebi : decltype(U{} * pow<20>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Mi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Mebi<U>::label;
constexpr auto mebi = PrefixApplier<Mebi>{};

template <typename U>
struct Kibi : decltype(U{} * pow<10>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ki", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Kibi<U>::label;
constexpr auto kibi = PrefixApplier<Kibi>{};

}  // namespace au



namespace au {

// If we don't provide these, then unqualified uses of `sin()`, etc. from <cmath> will break.  Name
// Lookup will stop once it hits `::au::sin()`, hiding the `::sin()` overload in the global
// namespace.  To learn more about Name Lookup, see this article (https://abseil.io/tips/49).
using std::abs;
using std::cbrt;
using std::copysign;
using std::cos;
using std::fmod;
using std::hypot;
using std::isinf;
using std::isnan;
using std::max;
using std::min;
using std::remainder;
using std::sin;
using std::sqrt;
using std::tan;

namespace detail {

// This utility handles converting Quantity to Radians in a uniform way, while also giving a more
// direct error message via the static_assert if users make a coding error and pass the wrong type.
template <typename U, typename R>
auto in_radians(Quantity<U, R> q) {
    static_assert(HasSameDimension<U, Radians>{},
                  "Can only use trig functions with Angle-dimensioned Quantity instances");

    // The standard library trig functions handle types as follows:
    // - For floating point inputs, return the same type as the input.
    // - For integral inputs, cast to a `double` and return a `double`.
    // See, for instance: https://en.cppreference.com/w/cpp/numeric/math/sin
    using PromotedT = std::conditional_t<std::is_floating_point<R>::value, R, double>;

    return q.template in<PromotedT>(radians);
}

template <typename T>
constexpr T int_pow_impl(T x, int exp) {
    if (exp < 0) {
        return T{1} / int_pow_impl(x, -exp);
    }

    if (exp == 0) {
        return T{1};
    }

    if (exp % 2 == 1) {
        return x * int_pow_impl(x, exp - 1);
    }

    const auto root = int_pow_impl(x, exp / 2);
    return root * root;
}

// Rounding a Quantity by a function `f()` (where `f` could be `std::round`, `std::ceil`, or
// `std::floor`) can require _two_ steps: unit conversion, and type conversion.  The unit conversion
// risks truncating the value if R is an integral type!  To prevent this, when we do the unit
// conversion, we use "whatever Rep `f()` would produce," because that is always a floating point
// type.
//
// This risks breaking the correspondence between the Rep of our Quantity, and the output type of
// `f()`.  For that correspondence to be _preserved_, we would need to make sure that
// `f(RoundingRepT{})` returns the same type as `f(R{})`.  We believe this is always the case based
// on the documentation:
//
// https://en.cppreference.com/w/cpp/numeric/math/round
// https://en.cppreference.com/w/cpp/numeric/math/floor
// https://en.cppreference.com/w/cpp/numeric/math/ceil
//
// Both of these assumptions---that our RoundingRep is floating point, and that it doesn't change
// the output Rep type---we verify via `static_assert`.
template <typename Q, typename RoundingUnits>
struct RoundingRep;
template <typename Q, typename RoundingUnits>
using RoundingRepT = typename RoundingRep<Q, RoundingUnits>::type;
template <typename U, typename R, typename RoundingUnits>
struct RoundingRep<Quantity<U, R>, RoundingUnits> {
    using type = decltype(std::round(R{}));

    // Test our floating point assumption.
    static_assert(std::is_floating_point<type>::value, "");

    // Test our type identity assumption, for every function which is a client of this utility.
    static_assert(std::is_same<decltype(std::round(type{})), decltype(std::round(R{}))>::value, "");
    static_assert(std::is_same<decltype(std::floor(type{})), decltype(std::floor(R{}))>::value, "");
    static_assert(std::is_same<decltype(std::ceil(type{})), decltype(std::ceil(R{}))>::value, "");
};
template <typename U, typename R, typename RoundingUnits>
struct RoundingRep<QuantityPoint<U, R>, RoundingUnits>
    : RoundingRep<Quantity<U, R>, RoundingUnits> {};
}  // namespace detail

// The absolute value of a Quantity.
template <typename U, typename R>
auto abs(Quantity<U, R> q) {
    return make_quantity<U>(std::abs(q.in(U{})));
}

// Wrapper for std::acos() which returns strongly typed angle quantity.
template <typename T>
auto arccos(T x) {
    return radians(std::acos(x));
}

// Wrapper for std::asin() which returns strongly typed angle quantity.
template <typename T>
auto arcsin(T x) {
    return radians(std::asin(x));
}

// Wrapper for std::atan() which returns strongly typed angle quantity.
template <typename T>
auto arctan(T x) {
    return radians(std::atan(x));
}

// Wrapper for std::atan2() which returns strongly typed angle quantity.
template <typename T, typename U>
auto arctan2(T y, U x) {
    return radians(std::atan2(y, x));
}

// arctan2() overload which supports same-dimensioned Quantity types.
template <typename U1, typename R1, typename U2, typename R2>
auto arctan2(Quantity<U1, R1> y, Quantity<U2, R2> x) {
    constexpr auto common_unit = CommonUnitT<U1, U2>{};
    return arctan2(y.in(common_unit), x.in(common_unit));
}

// Wrapper for std::cbrt() which handles Quantity types.
template <typename U, typename R>
auto cbrt(Quantity<U, R> q) {
    return make_quantity<UnitPowerT<U, 1, 3>>(std::cbrt(q.in(U{})));
}

// Clamp the first quantity to within the range of the second two.
template <typename UV, typename ULo, typename UHi, typename RV, typename RLo, typename RHi>
constexpr auto clamp(Quantity<UV, RV> v, Quantity<ULo, RLo> lo, Quantity<UHi, RHi> hi) {
    using U = CommonUnitT<UV, ULo, UHi>;
    using R = std::common_type_t<RV, RLo, RHi>;
    using ResultT = Quantity<U, R>;
    return (v < lo) ? ResultT{lo} : (hi < v) ? ResultT{hi} : ResultT{v};
}

// Clamp the first point to within the range of the second two.
template <typename UV, typename ULo, typename UHi, typename RV, typename RLo, typename RHi>
constexpr auto clamp(QuantityPoint<UV, RV> v,
                     QuantityPoint<ULo, RLo> lo,
                     QuantityPoint<UHi, RHi> hi) {
    using U = CommonPointUnitT<UV, ULo, UHi>;
    using R = std::common_type_t<RV, RLo, RHi>;
    using ResultT = QuantityPoint<U, R>;
    return (v < lo) ? ResultT{lo} : (hi < v) ? ResultT{hi} : ResultT{v};
}

template <typename U1, typename R1, typename U2, typename R2>
auto hypot(Quantity<U1, R1> x, Quantity<U2, R2> y) {
    using U = CommonUnitT<U1, U2>;
    return make_quantity<U>(std::hypot(x.in(U{}), y.in(U{})));
}

// Copysign where the magnitude has units.
template <typename U, typename R, typename T>
constexpr auto copysign(Quantity<U, R> mag, T sgn) {
    return make_quantity<U>(std::copysign(mag.in(U{}), sgn));
}

// Copysign where the sign has units.
template <typename T, typename U, typename R>
constexpr auto copysign(T mag, Quantity<U, R> sgn) {
    return std::copysign(mag, sgn.in(U{}));
}

// Copysign where both the magnitude and sign have units (disambiguates between the above).
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto copysign(Quantity<U1, R1> mag, Quantity<U2, R2> sgn) {
    return make_quantity<U1>(std::copysign(mag.in(U1{}), sgn.in(U2{})));
}

// Wrapper for std::cos() which accepts a strongly typed angle quantity.
template <typename U, typename R>
auto cos(Quantity<U, R> q) {
    return std::cos(detail::in_radians(q));
}

// The floating point remainder of two values of the same dimension.
template <typename U1, typename R1, typename U2, typename R2>
auto fmod(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
    using R = decltype(std::fmod(R1{}, R2{}));
    return make_quantity<U>(std::fmod(q1.template in<R>(U{}), q2.template in<R>(U{})));
}

// Raise a Quantity to an integer power.
template <int Exp, typename U, typename R>
constexpr auto int_pow(Quantity<U, R> q) {
    static_assert((!std::is_integral<R>::value) || (Exp >= 0),
                  "Negative exponent on integral represented units are not supported.");

    return make_quantity<UnitPowerT<U, Exp>>(detail::int_pow_impl(q.in(U{}), Exp));
}

//
// The value of the "smart" inverse of a Quantity, in a given destination Unit and Rep.
//
// This is the "explicit Rep" format, which is semantically equivalent to a `static_cast`.
//
template <typename TargetRep, typename TargetUnits, typename U, typename R>
constexpr auto inverse_in(TargetUnits target_units, Quantity<U, R> q) {
    using Rep = std::common_type_t<TargetRep, R>;
    constexpr auto UNITY = make_constant(UnitProductT<>{});
    return static_cast<TargetRep>(UNITY.in<Rep>(associated_unit(target_units) * U{}) / q.in(U{}));
}

//
// The value of the "smart" inverse of a Quantity, in a given destination unit.
//
// By "smart", we mean that, e.g., you can convert an integral Quantity of Kilo<Hertz> to an
// integral Quantity of Nano<Seconds>, without ever leaving the integral domain.  (Under the hood,
// in this case, the library will know to divide into 1'000'000 instead of dividing into 1.)
//
template <typename TargetUnits, typename U, typename R>
constexpr auto inverse_in(TargetUnits target_units, Quantity<U, R> q) {
    // The policy here is similar to our overflow policy, in that we try to avoid "bad outcomes"
    // when users store values less than 1000.  (The thinking, here as there, is that values _more_
    // than 1000 would tend to be stored in the next SI-prefixed unit up, e.g., 1 km instead of 1000
    // m.)
    //
    // The "bad outcome" here is a lossy conversion.  Since we're mainly worried about the integral
    // domain (because floating point numbers are already pretty well behaved), this means that:
    //
    //    inverse_in(a, inverse_as(b, a(n)))
    //
    // should be the identity for all n <= 1000.  For this to be true, we need a threshold of
    // (1'000 ^ 2) = 1'000'000.
    //
    // (An extreme instance of this kind of lossiness would be the inverse of a nonzero value
    // getting represented as 0, which would happen for values over the threshold.)

    // This will fail at compile time for types that can't hold 1'000'000.
    constexpr R threshold = 1'000'000;

    constexpr auto UNITY = make_constant(UnitProductT<>{});

    static_assert(
        UNITY.in<R>(associated_unit(TargetUnits{}) * U{}) >= threshold ||
            std::is_floating_point<R>::value,
        "Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired");

    // Having passed safety checks (at compile time!), we can delegate to the explicit-Rep version.
    return inverse_in<R>(target_units, q);
}

//
// The "smart" inverse of a Quantity, in a given destination unit.
//
// (See `inverse_in()` comment above for how this inverse is "smart".)
//
template <typename TargetUnits, typename U, typename R>
constexpr auto inverse_as(TargetUnits target_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in(target_units, q));
}

//
// The "smart" inverse of a Quantity, in a given destination Unit and Rep.
//
// This is the "explicit Rep" format, which is semantically equivalent to a `static_cast`.
//
template <typename TargetRep, typename TargetUnits, typename U, typename R>
constexpr auto inverse_as(TargetUnits target_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in<TargetRep>(target_units, q));
}

//
// Check whether the value stored is (positive or negative) infinity.
//
template <typename U, typename R>
constexpr bool isinf(Quantity<U, R> q) {
    return std::isinf(q.in(U{}));
}

// Overload of `isinf` for `QuantityPoint`.
template <typename U, typename R>
constexpr bool isinf(QuantityPoint<U, R> p) {
    return std::isinf(p.in(U{}));
}

//
// Check whether the value stored is "not a number" (NaN).
//
template <typename U, typename R>
constexpr bool isnan(Quantity<U, R> q) {
    return std::isnan(q.in(U{}));
}

// Overload of `isnan` for `QuantityPoint`.
template <typename U, typename R>
constexpr bool isnan(QuantityPoint<U, R> p) {
    return std::isnan(p.in(U{}));
}

//
// Linear interpolation between two values of the same dimension, as per `std::lerp`.
//
// Note that `std::lerp` is not defined until C++20, so neither is `au::lerp`.
//
// Note, too, that the implementation for same-type `Quantity` instances lives inside of the
// `Quantity` class implementation as a hidden friend, so that we can support shapeshifter types
// such as `Zero` or `Constant<U>`.
//
#if defined(__cpp_lib_interpolate) && __cpp_lib_interpolate >= 201902L
template <typename U1, typename R1, typename U2, typename R2, typename T>
constexpr auto lerp(Quantity<U1, R1> q1, Quantity<U2, R2> q2, T t) {
    using U = CommonUnitT<U1, U2>;
    return make_quantity<U>(std::lerp(q1.in(U{}), q2.in(U{}), as_raw_number(t)));
}

template <typename U1, typename R1, typename U2, typename R2, typename T>
constexpr auto lerp(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2, T t) {
    using U = CommonPointUnitT<U1, U2>;
    return make_quantity_point<U>(std::lerp(p1.in(U{}), p2.in(U{}), as_raw_number(t)));
}
#endif

namespace detail {
// We can't use lambdas in `constexpr` contexts until C++17, so we make a manual function object.
struct StdMaxByValue {
    template <typename T>
    constexpr auto operator()(T a, T b) const {
        return std::max(a, b);
    }
};
}  // namespace detail

// The maximum of two values of the same dimension.
//
// Unlike std::max, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto max(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::StdMaxByValue{});
}

// The maximum of two point values of the same dimension.
//
// Unlike std::max, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto max(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::StdMaxByValue{});
}

// Overload to resolve ambiguity with `std::max` for identical `QuantityPoint` types.
template <typename U, typename R>
constexpr auto max(QuantityPoint<U, R> a, QuantityPoint<U, R> b) {
    return std::max(a, b);
}

namespace detail {
// We can't use lambdas in `constexpr` contexts until C++17, so we make a manual function object.
struct StdMinByValue {
    template <typename T>
    constexpr auto operator()(T a, T b) const {
        return std::min(a, b);
    }
};
}  // namespace detail

// The minimum of two values of the same dimension.
//
// Unlike std::min, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto min(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, detail::StdMinByValue{});
}

// The minimum of two point values of the same dimension.
//
// Unlike std::min, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
constexpr auto min(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, detail::StdMinByValue{});
}

// Overload to resolve ambiguity with `std::min` for identical `QuantityPoint` types.
template <typename U, typename R>
constexpr auto min(QuantityPoint<U, R> a, QuantityPoint<U, R> b) {
    return std::min(a, b);
}

template <typename U0, typename R0, typename... Us, typename... Rs>
constexpr auto mean(Quantity<U0, R0> q0, Quantity<Us, Rs>... qs) {
    static_assert(sizeof...(qs) > 0, "mean() requires at least two inputs");
    using R = std::common_type_t<R0, Rs...>;
    using Common = Quantity<CommonUnitT<U0, Us...>, R>;
    const auto base = Common{q0};
    Common diffs[] = {(Common{qs} - base)...};
    Common sum_diffs = diffs[0];
    for (auto i = 1u; i < sizeof...(qs); ++i) {
        sum_diffs += diffs[i];
    }
    return base + (sum_diffs / static_cast<R>(1u + sizeof...(qs)));
}

template <typename U0, typename R0, typename... Us, typename... Rs>
constexpr auto mean(QuantityPoint<U0, R0> p0, QuantityPoint<Us, Rs>... ps) {
    static_assert(sizeof...(ps) > 0, "mean() requires at least two inputs");
    using U = CommonPointUnitT<U0, Us...>;
    using R = std::common_type_t<R0, Rs...>;
    const auto base = QuantityPoint<U, R>{p0};
    Quantity<U, R> diffs[] = {(QuantityPoint<U, R>{ps} - base)...};
    Quantity<U, R> sum_diffs = diffs[0];
    for (auto i = 1u; i < sizeof...(ps); ++i) {
        sum_diffs += diffs[i];
    }
    return base + (sum_diffs / static_cast<R>(1u + sizeof...(ps)));
}

// The (zero-centered) floating point remainder of two values of the same dimension.
template <typename U1, typename R1, typename U2, typename R2>
auto remainder(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    using U = CommonUnitT<U1, U2>;
    using R = decltype(std::remainder(R1{}, R2{}));
    return make_quantity<U>(std::remainder(q1.template in<R>(U{}), q2.template in<R>(U{})));
}

//
// Round the value of this Quantity or QuantityPoint to the nearest integer in the given units.
//
// This is the "Unit-only" format (i.e., `round_in(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::round(q.template in<OurRoundingRep>(rounding_units));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    using OurRoundingRep = detail::RoundingRepT<QuantityPoint<U, R>, RoundingUnits>;
    return std::round(p.template in<OurRoundingRep>(rounding_units));
}

//
// Round the value of this Quantity or QuantityPoint to the nearest integer in the given units,
// returning OutputRep.
//
// This is the "Explicit-Rep" format (e.g., `round_in<int>(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(round_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return static_cast<OutputRep>(round_in(rounding_units, p));
}

//
// The integral-valued Quantity or QuantityPoint, in this unit, nearest to the input.
//
// This is the "Unit-only" format (i.e., `round_as(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPointsT<RoundingUnits>>(
        round_in(rounding_units, p));
}

//
// The integral-valued Quantity or QuantityPoint, in this unit, nearest to the input, using the
// specified OutputRep.
//
// This is the "Explicit-Rep" format (e.g., `round_as<float>(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in<OutputRep>(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPointsT<RoundingUnits>>(
        round_in<OutputRep>(rounding_units, p));
}

//
// Return the largest integral value in `rounding_units` which is not greater than `q`.
//
// This is the "Unit-only" format (i.e., `floor_in(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::floor(q.template in<OurRoundingRep>(rounding_units));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    using OurRoundingRep = detail::RoundingRepT<QuantityPoint<U, R>, RoundingUnits>;
    return std::floor(p.template in<OurRoundingRep>(rounding_units));
}

//
// Return `OutputRep` with largest integral value in `rounding_units` which is not greater than `q`.
//
// This is the "Explicit-Rep" format (e.g., `floor_in<int>(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(floor_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return static_cast<OutputRep>(floor_in(rounding_units, p));
}

//
// The largest integral-valued Quantity or QuantityPoint, in this unit, not greater than the input.
//
// This is the "Unit-only" format (i.e., `floor_as(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(floor_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPointsT<RoundingUnits>>(
        floor_in(rounding_units, p));
}

//
// The largest integral-valued Quantity or QuantityPoint, in this unit, not greater than the input,
// using the specified `OutputRep`.
//
// This is the "Explicit-Rep" format (e.g., `floor_as<float>(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(floor_in<OutputRep>(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPointsT<RoundingUnits>>(
        floor_in<OutputRep>(rounding_units, p));
}

//
// Return the smallest integral value in `rounding_units` which is not less than `q`.
//
// This is the "Unit-only" format (i.e., `ceil_in(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::ceil(q.template in<OurRoundingRep>(rounding_units));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    using OurRoundingRep = detail::RoundingRepT<QuantityPoint<U, R>, RoundingUnits>;
    return std::ceil(p.template in<OurRoundingRep>(rounding_units));
}

//
// Return the smallest integral value in `rounding_units` which is not less than `q`.
//
// This is the "Explicit-Rep" format (e.g., `ceil_in<int>(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(ceil_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return static_cast<OutputRep>(ceil_in(rounding_units, p));
}

//
// The smallest integral-valued Quantity or QuantityPoint, in this unit, not less than the input.
//
// This is the "Unit-only" format (i.e., `ceil_as(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(ceil_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPointsT<RoundingUnits>>(ceil_in(rounding_units, p));
}

//
// The smallest integral-valued Quantity or QuantityPoint, in this unit, not less than the input,
// using the specified `OutputRep`.
//
// This is the "Explicit-Rep" format (e.g., `ceil_as<float>(rounding_units, q)`).
//
// a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(ceil_in<OutputRep>(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPointsT<RoundingUnits>>(
        ceil_in<OutputRep>(rounding_units, p));
}

// Wrapper for std::sin() which accepts a strongly typed angle quantity.
template <typename U, typename R>
auto sin(Quantity<U, R> q) {
    return std::sin(detail::in_radians(q));
}

// Wrapper for std::sqrt() which handles Quantity types.
template <typename U, typename R>
auto sqrt(Quantity<U, R> q) {
    return make_quantity<UnitPowerT<U, 1, 2>>(std::sqrt(q.in(U{})));
}

// Wrapper for std::tan() which accepts a strongly typed angle quantity.
template <typename U, typename R>
auto tan(Quantity<U, R> q) {
    return std::tan(detail::in_radians(q));
}

}  // namespace au

namespace std {
/// `numeric_limits` specialization.  The default implementation default constructs the scalar,
/// which would return the obviously-wrong value of 0 for max().
///
/// Per the standard, we are allowed to specialize this for our own types, and are also not required
/// to define every possible field. This is nice because it means that we will get compile errors
/// for unsupported operations (instead of having them silently fail, which is the default)
///
/// Source: https://stackoverflow.com/a/16519653
template <typename U, typename R>
struct numeric_limits<au::Quantity<U, R>> {
    // To validily extent std::numeric_limits<T>, we must define all members declared static
    // constexpr in the primary template, in such a way that they are usable as integral constant
    // expressions.
    //
    // Source for rule: https://en.cppreference.com/w/cpp/language/extending_std
    // List of members: https://en.cppreference.com/w/cpp/types/numeric_limits
    static constexpr bool is_specialized = true;
    static constexpr bool is_integer = numeric_limits<R>::is_integer;
    static constexpr bool is_signed = numeric_limits<R>::is_signed;
    static constexpr bool is_exact = numeric_limits<R>::is_exact;
    static constexpr bool has_infinity = numeric_limits<R>::has_infinity;
    static constexpr bool has_quiet_NaN = numeric_limits<R>::has_quiet_NaN;
    static constexpr bool has_signaling_NaN = numeric_limits<R>::has_signaling_NaN;
    static constexpr bool has_denorm = numeric_limits<R>::has_denorm;
    static constexpr bool has_denorm_loss = numeric_limits<R>::has_denorm_loss;
    static constexpr float_round_style round_style = numeric_limits<R>::round_style;
    static constexpr bool is_iec559 = numeric_limits<R>::is_iec559;
    static constexpr bool is_bounded = numeric_limits<R>::is_bounded;
    static constexpr bool is_modulo = numeric_limits<R>::is_modulo;
    static constexpr int digits = numeric_limits<R>::digits;
    static constexpr int digits10 = numeric_limits<R>::digits10;
    static constexpr int max_digits10 = numeric_limits<R>::max_digits10;
    static constexpr int radix = numeric_limits<R>::radix;
    static constexpr int min_exponent = numeric_limits<R>::min_exponent;
    static constexpr int min_exponent10 = numeric_limits<R>::min_exponent10;
    static constexpr int max_exponent = numeric_limits<R>::max_exponent;
    static constexpr int max_exponent10 = numeric_limits<R>::max_exponent10;
    static constexpr bool traps = numeric_limits<R>::traps;
    static constexpr bool tinyness_before = numeric_limits<R>::tinyness_before;

    static constexpr au::Quantity<U, R> max() {
        return au::make_quantity<U>(std::numeric_limits<R>::max());
    }

    static constexpr au::Quantity<U, R> lowest() {
        return au::make_quantity<U>(std::numeric_limits<R>::lowest());
    }

    static constexpr au::Quantity<U, R> min() {
        return au::make_quantity<U>(std::numeric_limits<R>::min());
    }

    static constexpr au::Quantity<U, R> epsilon() {
        return au::make_quantity<U>(std::numeric_limits<R>::epsilon());
    }

    static constexpr au::Quantity<U, R> round_error() {
        return au::make_quantity<U>(std::numeric_limits<R>::round_error());
    }

    static constexpr au::Quantity<U, R> infinity() {
        return au::make_quantity<U>(std::numeric_limits<R>::infinity());
    }

    static constexpr au::Quantity<U, R> quiet_NaN() {
        return au::make_quantity<U>(std::numeric_limits<R>::quiet_NaN());
    }

    static constexpr au::Quantity<U, R> signaling_NaN() {
        return au::make_quantity<U>(std::numeric_limits<R>::signaling_NaN());
    }

    static constexpr au::Quantity<U, R> denorm_min() {
        return au::make_quantity<U>(std::numeric_limits<R>::denorm_min());
    }
};

// Specialize for cv-qualified Quantity types by inheriting from bare Quantity implementation.
template <typename U, typename R>
struct numeric_limits<const au::Quantity<U, R>> : numeric_limits<au::Quantity<U, R>> {};
template <typename U, typename R>
struct numeric_limits<volatile au::Quantity<U, R>> : numeric_limits<au::Quantity<U, R>> {};
template <typename U, typename R>
struct numeric_limits<const volatile au::Quantity<U, R>> : numeric_limits<au::Quantity<U, R>> {};

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_specialized;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_integer;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_signed;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_exact;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_infinity;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_quiet_NaN;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_signaling_NaN;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_denorm;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::has_denorm_loss;

template <typename U, typename R>
constexpr float_round_style numeric_limits<au::Quantity<U, R>>::round_style;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_iec559;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_bounded;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::is_modulo;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::digits;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::digits10;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::max_digits10;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::radix;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::min_exponent;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::min_exponent10;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::max_exponent;

template <typename U, typename R>
constexpr int numeric_limits<au::Quantity<U, R>>::max_exponent10;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::traps;

template <typename U, typename R>
constexpr bool numeric_limits<au::Quantity<U, R>>::tinyness_before;

}  // namespace std

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct MinutesLabel {
    static constexpr const char label[] = "min";
};
template <typename T>
constexpr const char MinutesLabel<T>::label[];
struct Minutes : decltype(Seconds{} * mag<60>()), MinutesLabel<void> {
    using MinutesLabel<void>::label;
};
constexpr auto minute = SingularNameFor<Minutes>{};
constexpr auto minutes = QuantityMaker<Minutes>{};

namespace symbols {
constexpr auto min = SymbolFor<Minutes>{};
}
}  // namespace au

// Keep corresponding `_fwd.hh` file on top.


namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct HoursLabel {
    static constexpr const char label[] = "h";
};
template <typename T>
constexpr const char HoursLabel<T>::label[];
struct Hours : decltype(Minutes{} * mag<60>()), HoursLabel<void> {
    using HoursLabel<void>::label;
};
constexpr auto hour = SingularNameFor<Hours>{};
constexpr auto hours = QuantityMaker<Hours>{};

namespace symbols {
constexpr auto h = SymbolFor<Hours>{};
}
}  // namespace au



namespace au {

// Streaming output support for Quantity types.
template <typename U, typename R>
std::ostream &operator<<(std::ostream &out, const Quantity<U, R> &q) {
    // In the case that the Rep is a type that resolves to 'char' (e.g. int8_t),
    // the << operator will match the implementation that takes a character
    // literal.  Using the unary + operator will trigger an integer promotion on
    // the operand, which will then match an appropriate << operator that will
    // output the integer representation.
    out << +q.in(U{}) << " " << unit_label(U{});
    return out;
}

// Streaming output support for QuantityPoint types.
template <typename U, typename R>
std::ostream &operator<<(std::ostream &out, const QuantityPoint<U, R> &p) {
    out << "@(" << (p - rep_cast<R>(make_quantity_point<U>(0))) << ")";
    return out;
}

// Streaming output support for Zero.  (Useful for printing in unit test failures.)
inline std::ostream &operator<<(std::ostream &out, Zero) {
    out << "0";
    return out;
}

// Streaming support for Magnitude: print the magnitude label.
template <typename... BPs>
std::ostream &operator<<(std::ostream &out, Magnitude<BPs...> m) {
    return (out << mag_label(m));
}

// Streaming support for Constant: print the unit label.
template <typename U>
std::ostream &operator<<(std::ostream &out, Constant<U>) {
    return (out << unit_label(U{}));
}

// Streaming support for unit symbols: print the unit label.
template <typename U>
std::ostream &operator<<(std::ostream &out, SymbolFor<U>) {
    return (out << unit_label(U{}));
}

}  // namespace au



namespace au {

// Define 1:1 mapping between duration types of chrono library and our library.
template <typename RepT, typename Period>
struct CorrespondingQuantity<std::chrono::duration<RepT, Period>> {
    using Unit = decltype(Seconds{} * (mag<Period::num>() / mag<Period::den>()));
    using Rep = RepT;

    using ChronoDuration = std::chrono::duration<Rep, Period>;

    static constexpr Rep extract_value(ChronoDuration d) { return d.count(); }
    static constexpr ChronoDuration construct_from_value(Rep x) { return ChronoDuration{x}; }
};

// Define special mappings for widely used chrono types.
template <typename ChronoType, typename AuUnit>
struct SpecialCorrespondingQuantity {
    using Unit = AuUnit;
    using Rep = decltype(ChronoType{}.count());

    static constexpr Rep extract_value(ChronoType d) { return d.count(); }
    static constexpr ChronoType construct_from_value(Rep x) { return ChronoType{x}; }
};

template <>
struct CorrespondingQuantity<std::chrono::nanoseconds>
    : SpecialCorrespondingQuantity<std::chrono::nanoseconds, Nano<Seconds>> {};

template <>
struct CorrespondingQuantity<std::chrono::microseconds>
    : SpecialCorrespondingQuantity<std::chrono::microseconds, Micro<Seconds>> {};

template <>
struct CorrespondingQuantity<std::chrono::milliseconds>
    : SpecialCorrespondingQuantity<std::chrono::milliseconds, Milli<Seconds>> {};

template <>
struct CorrespondingQuantity<std::chrono::seconds>
    : SpecialCorrespondingQuantity<std::chrono::seconds, Seconds> {};

template <>
struct CorrespondingQuantity<std::chrono::minutes>
    : SpecialCorrespondingQuantity<std::chrono::minutes, Minutes> {};

template <>
struct CorrespondingQuantity<std::chrono::hours>
    : SpecialCorrespondingQuantity<std::chrono::hours, Hours> {};

// Convert any Au duration quantity to an equivalent `std::chrono::duration`.
template <typename U, typename R>
constexpr auto as_chrono_duration(Quantity<U, R> dt) {
    constexpr auto ratio = unit_ratio(U{}, seconds);
    static_assert(is_rational(ratio), "Cannot convert to chrono::duration with non-rational ratio");
    static_assert(is_positive(ratio), "Chrono library does not support negative duration units");
    return std::chrono::duration<R,
                                 std::ratio<get_value<std::intmax_t>(numerator(ratio)),
                                            get_value<std::intmax_t>(denominator(ratio))>>{dt};
}

}  // namespace au

