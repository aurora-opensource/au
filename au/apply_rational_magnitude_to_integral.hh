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

#include <algorithm>

#include "au/magnitude.hh"
#include "au/stdx/utility.hh"
#include "au/utility/type_traits.hh"

// This file exists to analyze one single calculation: `x * N / D`, where `x` is
// some integral type, and `N` and `D` are the numerator and denominator of a
// rational magnitude (and hence, are automatically in lowest terms),
// represented in that same type.  We want to answer one single question: will
// this calculation overflow at any stage?
//
// Importantly, we need to produce correct answers even when `N` and/or `D`
// _cannot be represented_ in that type (because they would overflow).  We also
// need to handle subtleties around integer promotion, where the type of `x * x`
// can be different from the type of `x` when those types are small.
//
// The goal for the final solution we produce is to be as fast and efficient as
// the best such function that an expert C++ engineer could produce by hand, for
// every combination of integral type and numerator and denominator magnitudes.

namespace au {
namespace detail {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `clamp_to_range_of<T>(x)` returns `x` if it is in the range of `T`, and otherwise returns the
// maximum value representable in `T` if `x` is too large, or the minimum value representable in `T`
// if `x` is too small.
//

template <typename T, typename U>
constexpr T clamp_to_range_of(U x) {
    return stdx::cmp_greater(x, std::numeric_limits<T>::max())
               ? std::numeric_limits<T>::max()
               : (stdx::cmp_less(x, std::numeric_limits<T>::lowest())
                      ? std::numeric_limits<T>::lowest()
                      : static_cast<T>(x));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `is_abs_known_to_be_less_than_one(MagT)` is true if the absolute value of the magnitude `MagT` is
// purely rational; its numerator is representable in `std::uintmax_t`; and, it is less than 1.
//

enum class IsAbsMagLessThanOne {
    DEFINITELY,
    MAYBE_NOT,
};

template <typename... BPs>
constexpr IsAbsMagLessThanOne is_abs_known_to_be_less_than_one(Magnitude<BPs...>) {
    using MagT = Abs<Magnitude<BPs...>>;
    static_assert(is_rational(MagT{}), "Magnitude must be rational");

    constexpr auto num_result = get_value_result<std::uintmax_t>(numerator(MagT{}));
    static_assert(num_result.outcome == MagRepresentationOutcome::OK,
                  "Numerator must be representable in std::uintmax_t");

    constexpr auto den_result = get_value_result<std::uintmax_t>(denominator(MagT{}));
    static_assert(
        den_result.outcome == MagRepresentationOutcome::OK ||
            den_result.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT,
        "Denominator must either be representable in std::uintmax_t, or fail due to overflow");

    return (den_result.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT ||
            num_result.value < den_result.value)
               ? IsAbsMagLessThanOne::DEFINITELY
               : IsAbsMagLessThanOne::MAYBE_NOT;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `MaxNonOverflowingValue<T, MagT>` is the maximum value of type `T` that can have `MagT` applied
// as numerator-and-denominator without overflowing.  We require that `T` is some integral
// arithmetic type, and that `MagT` is a rational magnitude that is neither purely integral nor
// purely inverse-integral.
//
// If you are trying to understand these helpers, we suggest starting at the bottom with
// `MaxNonOverflowingValue`, and reading upwards.
//

//
// Branch based on whether `MagT` is less than 1.
//
template <typename T, typename MagT, IsAbsMagLessThanOne>
struct MaxNonOverflowingValueImplWhenNumFits;

// Implementation helper for "a value of zero" (which recurs a bunch of times).
template <typename T>
struct ValueOfZero {
    static constexpr T value() { return T{0}; }
};

// If `MagT` is less than 1, then we only need to check for the limiting value where the _numerator
// multiplication step alone_ would overflow.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImplWhenNumFits<T, MagT, IsAbsMagLessThanOne::DEFINITELY> {
    using P = PromotedType<T>;

    static constexpr T value() {
        return clamp_to_range_of<T>(std::numeric_limits<P>::max() /
                                    get_value<P>(numerator(MagT{})));
    }
};

// If `MagT` might be greater than 1, then we have two opportunities for overflow: the numerator
// multiplication step can overflow the promoted type; or, the denominator division step can fail to
// restore it to the original type's range.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImplWhenNumFits<T, MagT, IsAbsMagLessThanOne::MAYBE_NOT> {
    using P = PromotedType<T>;

    static constexpr T value() {
        constexpr auto num = get_value<P>(numerator(MagT{}));
        constexpr auto den = get_value<P>(denominator(MagT{}));
        constexpr auto t_max = std::numeric_limits<T>::max();
        constexpr auto p_max = std::numeric_limits<P>::max();
        constexpr auto limit_to_avoid = (den > p_max / t_max) ? p_max : t_max * den;
        return clamp_to_range_of<T>(limit_to_avoid / num);
    }
};

//
// Branch based on whether the numerator of `MagT` can fit in the promoted type of `T`.
//
template <typename T, typename MagT, MagRepresentationOutcome NumOutcome>
struct MaxNonOverflowingValueImpl;

// For any situation where we're applying a negative factor to an unsigned type, simply short
// circuit to set the max to zero.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImpl<T,
                                  MagT,
                                  MagRepresentationOutcome::ERR_NEGATIVE_NUMBER_IN_UNSIGNED_TYPE>
    : ValueOfZero<T> {};

// If the numerator fits in the promoted type of `T`, delegate further based on whether the
// denominator is bigger.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImpl<T, MagT, MagRepresentationOutcome::OK>
    : MaxNonOverflowingValueImplWhenNumFits<T, MagT, is_abs_known_to_be_less_than_one(MagT{})> {};

// If `MagT` can't be represented in the promoted type of `T`, then the result is 0.
template <typename T, typename MagT>
struct MaxNonOverflowingValueImpl<T, MagT, MagRepresentationOutcome::ERR_CANNOT_FIT>
    : ValueOfZero<T> {};

template <typename T, typename MagT>
struct ValidateTypeAndMagnitude {
    static_assert(std::is_integral<T>::value, "Only designed for integral types");
    static_assert(is_rational(MagT{}), "Magnitude must be rational");
    static_assert(!is_integer(MagT{}), "Magnitude must not be purely integral");
    static_assert(!is_integer(inverse(MagT{})), "Magnitude must not be purely inverse-integral");
};

template <typename T, typename MagT>
struct MaxNonOverflowingValue
    : ValidateTypeAndMagnitude<T, MagT>,
      MaxNonOverflowingValueImpl<T,
                                 MagT,
                                 get_value_result<PromotedType<T>>(numerator(MagT{})).outcome> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// `MinNonOverflowingValue<T, MagT>` is the minimum (i.e., most-negative) value of type `T` that can
// have `MagT` applied as numerator-and-denominator without overflowing (i.e., becoming too-negative
// to represent).  We require that `T` is some integral arithmetic type, and that `MagT` is a
// rational magnitude that is neither purely integral nor purely inverse-integral.
//
// If you are trying to understand these helpers, we suggest starting at the bottom with
// `MinNonOverflowingValue`, and reading upwards.
//

//
// Branch based on whether `MagT` is less than 1.
//
template <typename T, typename MagT, IsAbsMagLessThanOne>
struct MinNonOverflowingValueImplWhenNumFits;

// If `MagT` is less than 1, then we only need to check for the limiting value where the _numerator
// multiplication step alone_ would overflow.
template <typename T, typename MagT>
struct MinNonOverflowingValueImplWhenNumFits<T, MagT, IsAbsMagLessThanOne::DEFINITELY> {
    using P = PromotedType<T>;

    static constexpr T value() {
        return clamp_to_range_of<T>(std::numeric_limits<P>::lowest() /
                                    get_value<P>(numerator(MagT{})));
    }
};

// If `MagT` is greater than 1, then we have two opportunities for overflow: the numerator
// multiplication step can overflow the promoted type; or, the denominator division step can fail to
// restore it to the original type's range.
template <typename T, typename MagT>
struct MinNonOverflowingValueImplWhenNumFits<T, MagT, IsAbsMagLessThanOne::MAYBE_NOT> {
    using P = PromotedType<T>;

    static constexpr T value() {
        constexpr auto num = get_value<P>(numerator(MagT{}));
        constexpr auto den = get_value<P>(denominator(MagT{}));
        constexpr auto t_min = std::numeric_limits<T>::lowest();
        constexpr auto p_min = std::numeric_limits<P>::lowest();
        constexpr auto limit_to_avoid = (den > p_min / t_min) ? p_min : t_min * den;
        return clamp_to_range_of<T>(limit_to_avoid / num);
    }
};

//
// Branch based on whether the denominator of `MagT` can fit in the promoted type of `T`.
//
template <typename T, typename MagT, MagRepresentationOutcome NumOutcome>
struct MinNonOverflowingValueImpl;

// If the numerator fits in the promoted type of `T`, delegate further based on whether the
// denominator is bigger.
template <typename T, typename MagT>
struct MinNonOverflowingValueImpl<T, MagT, MagRepresentationOutcome::OK>
    : MinNonOverflowingValueImplWhenNumFits<T, MagT, is_abs_known_to_be_less_than_one(MagT{})> {};

// If the numerator can't be represented in the promoted type of `T`, then the result is 0.
template <typename T, typename MagT>
struct MinNonOverflowingValueImpl<T, MagT, MagRepresentationOutcome::ERR_CANNOT_FIT>
    : ValueOfZero<T> {};

template <typename T, typename MagT>
struct MinNonOverflowingValue
    : ValidateTypeAndMagnitude<T, MagT>,
      MinNonOverflowingValueImpl<T,
                                 MagT,
                                 get_value_result<PromotedType<T>>(numerator(MagT{})).outcome> {
    static_assert(std::is_signed<T>::value, "Only designed for signed types");
    static_assert(std::is_signed<PromotedType<T>>::value,
                  "We assume the promoted type is also signed");
};

}  // namespace detail
}  // namespace au
