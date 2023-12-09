// Copyright 2023 Aurora Operations, Inc.
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

#include "au/magnitude.hh"

namespace au {
namespace detail {

// The various categories by which a magnitude can be applied to a numeric quantity.
enum class ApplyAs {
    INTEGER_MULTIPLY,
    INTEGER_DIVIDE,
    RATIONAL_MULTIPLY,
    IRRATIONAL_MULTIPLY,
};

template <typename... BPs>
constexpr ApplyAs categorize_magnitude(Magnitude<BPs...>) {
    if (IsInteger<Magnitude<BPs...>>::value) {
        return ApplyAs::INTEGER_MULTIPLY;
    }

    if (IsInteger<MagInverseT<Magnitude<BPs...>>>::value) {
        return ApplyAs::INTEGER_DIVIDE;
    }

    return IsRational<Magnitude<BPs...>>::value ? ApplyAs::RATIONAL_MULTIPLY
                                                : ApplyAs::IRRATIONAL_MULTIPLY;
}

template <typename Mag, ApplyAs Category, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl;

template <typename T, bool IsMagnitudeValid>
struct OverflowChecker {
    // Default case: `IsMagnitudeValid` is true.
    static constexpr bool would_product_overflow(T x, T mag_value) {
        return (x > (std::numeric_limits<T>::max() / mag_value)) ||
               (x < (std::numeric_limits<T>::lowest() / mag_value));
    }
};

template <typename T>
struct OverflowChecker<T, false> {
    // Specialization for when `IsMagnitudeValid` is false.
    //
    // This means that the magnitude itself could not fit inside of the type; therefore, the only
    // possible value that would not overflow is zero.
    static constexpr bool would_product_overflow(T x, T) { return (x != T{0}); }
};

// Multiplying by an integer, for any type T.
template <typename Mag, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl<Mag, ApplyAs::INTEGER_MULTIPLY, T, is_T_integral> {
    static_assert(categorize_magnitude(Mag{}) == ApplyAs::INTEGER_MULTIPLY,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(is_T_integral == std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) { return x * get_value<T>(Mag{}); }

    static constexpr bool would_overflow(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(Mag{});
        return OverflowChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_product_overflow(x, mag_value_result.value);
    }
};

// Dividing by an integer, for any type T.
template <typename Mag, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl<Mag, ApplyAs::INTEGER_DIVIDE, T, is_T_integral> {
    static_assert(categorize_magnitude(Mag{}) == ApplyAs::INTEGER_DIVIDE,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(is_T_integral == std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) { return x / get_value<T>(MagInverseT<Mag>{}); }

    static constexpr bool would_overflow(const T &) { return false; }
};

// Applying a (non-integer, non-inverse-integer) rational, for any integral type T.
template <typename Mag, typename T>
struct ApplyMagnitudeImpl<Mag, ApplyAs::RATIONAL_MULTIPLY, T, true> {
    static_assert(categorize_magnitude(Mag{}) == ApplyAs::RATIONAL_MULTIPLY,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) {
        return x * get_value<T>(numerator(Mag{})) / get_value<T>(denominator(Mag{}));
    }

    static constexpr bool would_overflow(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(numerator(Mag{}));
        return OverflowChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_product_overflow(x, mag_value_result.value);
    }
};

// Applying a (non-integer, non-inverse-integer) rational, for any non-integral type T.
template <typename Mag, typename T>
struct ApplyMagnitudeImpl<Mag, ApplyAs::RATIONAL_MULTIPLY, T, false> {
    static_assert(categorize_magnitude(Mag{}) == ApplyAs::RATIONAL_MULTIPLY,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(!std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) { return x * get_value<T>(Mag{}); }

    static constexpr bool would_overflow(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(Mag{});
        return OverflowChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_product_overflow(x, mag_value_result.value);
    }
};

// Applying an irrational for any type T (although only non-integral T makes sense).
template <typename Mag, typename T, bool is_T_integral>
struct ApplyMagnitudeImpl<Mag, ApplyAs::IRRATIONAL_MULTIPLY, T, is_T_integral> {
    static_assert(!std::is_integral<T>::value, "Cannot apply irrational magnitude to integer type");

    static_assert(categorize_magnitude(Mag{}) == ApplyAs::IRRATIONAL_MULTIPLY,
                  "Mismatched instantiation (should never be done manually)");
    static_assert(is_T_integral == std::is_integral<T>::value,
                  "Mismatched instantiation (should never be done manually)");

    constexpr T operator()(const T &x) { return x * get_value<T>(Mag{}); }

    static constexpr bool would_overflow(const T &x) {
        constexpr auto mag_value_result = get_value_result<T>(Mag{});
        return OverflowChecker<T, mag_value_result.outcome == MagRepresentationOutcome::OK>::
            would_product_overflow(x, mag_value_result.value);
    }
};

template <typename T, typename MagT>
struct ApplyMagnitudeType;
template <typename T, typename MagT>
using ApplyMagnitudeT = typename ApplyMagnitudeType<T, MagT>::type;
template <typename T, typename... BPs>
struct ApplyMagnitudeType<T, Magnitude<BPs...>>
    : stdx::type_identity<ApplyMagnitudeImpl<Magnitude<BPs...>,
                                             categorize_magnitude(Magnitude<BPs...>{}),
                                             T,
                                             std::is_integral<T>::value>> {};

template <typename T, typename... BPs>
constexpr T apply_magnitude(const T &x, Magnitude<BPs...>) {
    return ApplyMagnitudeT<T, Magnitude<BPs...>>{}(x);
}

}  // namespace detail
}  // namespace au
