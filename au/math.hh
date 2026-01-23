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

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

#include "au/constant.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/units/radians.hh"

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
// `f(RoundingRep{})` returns the same type as `f(R{})`.  We believe this is always the case based
// on the documentation:
//
// https://en.cppreference.com/w/cpp/numeric/math/round
// https://en.cppreference.com/w/cpp/numeric/math/floor
// https://en.cppreference.com/w/cpp/numeric/math/ceil
//
// Both of these assumptions---that our RoundingRep is floating point, and that it doesn't change
// the output Rep type---we verify via `static_assert`.
template <typename Q, typename RoundingUnits>
struct RoundingRepImpl;
template <typename Q, typename RoundingUnits>
using RoundingRep = typename RoundingRepImpl<Q, RoundingUnits>::type;
template <typename U, typename R, typename RoundingUnits>
struct RoundingRepImpl<Quantity<U, R>, RoundingUnits> {
    using type = decltype(std::round(R{}));

    // Test our floating point assumption.
    static_assert(std::is_floating_point<type>::value, "");

    // Test our type identity assumption, for every function which is a client of this utility.
    static_assert(std::is_same<decltype(std::round(type{})), decltype(std::round(R{}))>::value, "");
    static_assert(std::is_same<decltype(std::floor(type{})), decltype(std::floor(R{}))>::value, "");
    static_assert(std::is_same<decltype(std::ceil(type{})), decltype(std::ceil(R{}))>::value, "");
};
template <typename U, typename R, typename RoundingUnits>
struct RoundingRepImpl<QuantityPoint<U, R>, RoundingUnits>
    : RoundingRepImpl<Quantity<U, R>, RoundingUnits> {};
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
    constexpr auto common_unit = CommonUnit<U1, U2>{};
    return arctan2(y.in(common_unit), x.in(common_unit));
}

// Wrapper for std::cbrt() which handles Quantity types.
template <typename U, typename R>
auto cbrt(Quantity<U, R> q) {
    return make_quantity<UnitPower<U, 1, 3>>(std::cbrt(q.in(U{})));
}

// Clamp the first quantity to within the range of the second two.
template <typename UV, typename ULo, typename UHi, typename RV, typename RLo, typename RHi>
constexpr auto clamp(Quantity<UV, RV> v, Quantity<ULo, RLo> lo, Quantity<UHi, RHi> hi) {
    using U = CommonUnit<UV, ULo, UHi>;
    using R = std::common_type_t<RV, RLo, RHi>;
    using ResultT = Quantity<U, R>;
    return (v < lo) ? ResultT{lo} : (hi < v) ? ResultT{hi} : ResultT{v};
}

// Clamp the first point to within the range of the second two.
template <typename UV, typename ULo, typename UHi, typename RV, typename RLo, typename RHi>
constexpr auto clamp(QuantityPoint<UV, RV> v,
                     QuantityPoint<ULo, RLo> lo,
                     QuantityPoint<UHi, RHi> hi) {
    using U = CommonPointUnit<UV, ULo, UHi>;
    using R = std::common_type_t<RV, RLo, RHi>;
    using ResultT = QuantityPoint<U, R>;
    return (v < lo) ? ResultT{lo} : (hi < v) ? ResultT{hi} : ResultT{v};
}

template <typename U1, typename R1, typename U2, typename R2>
auto hypot(Quantity<U1, R1> x, Quantity<U2, R2> y) {
    using U = CommonUnit<U1, U2>;
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
    using U = CommonUnit<U1, U2>;
    using R = decltype(std::fmod(R1{}, R2{}));
    return make_quantity<U>(std::fmod(q1.template in<R>(U{}), q2.template in<R>(U{})));
}

// Raise a Quantity to an integer power.
template <int Exp, typename U, typename R>
constexpr auto int_pow(Quantity<U, R> q) {
    static_assert((!std::is_integral<R>::value) || (Exp >= 0),
                  "Negative exponent on integral represented units are not supported.");

    return make_quantity<UnitPower<U, Exp>>(detail::int_pow_impl(q.in(U{}), Exp));
}

//
// The value of the "smart" inverse of a Quantity, in a given destination Unit and Rep.
//
// This is the "explicit Rep" format, which is semantically equivalent to a `static_cast`.
//
template <typename TargetRep, typename TargetUnits, typename U, typename R>
constexpr auto inverse_in(TargetUnits target_units, Quantity<U, R> q) {
    using Rep = std::common_type_t<TargetRep, R>;
    constexpr auto UNITY = make_constant(UnitProduct<>{});
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

    constexpr auto UNITY = make_constant(UnitProduct<>{});

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
    return make_quantity<AssociatedUnit<TargetUnits>>(inverse_in(target_units, q));
}

//
// The "smart" inverse of a Quantity, in a given destination Unit and Rep.
//
// This is the "explicit Rep" format, which is semantically equivalent to a `static_cast`.
//
template <typename TargetRep, typename TargetUnits, typename U, typename R>
constexpr auto inverse_as(TargetUnits target_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnit<TargetUnits>>(inverse_in<TargetRep>(target_units, q));
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
    using U = CommonUnit<U1, U2>;
    return make_quantity<U>(std::lerp(q1.in(U{}), q2.in(U{}), as_raw_number(t)));
}

template <typename U1, typename R1, typename U2, typename R2, typename T>
constexpr auto lerp(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2, T t) {
    using U = CommonPointUnit<U1, U2>;
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
    using Common = Quantity<CommonUnit<U0, Us...>, R>;
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
    using U = CommonPointUnit<U0, Us...>;
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
    using U = CommonUnit<U1, U2>;
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
    using OurRoundingRep = detail::RoundingRep<Quantity<U, R>, RoundingUnits>;
    return std::round(q.template in<OurRoundingRep>(rounding_units));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    using OurRoundingRep = detail::RoundingRep<QuantityPoint<U, R>, RoundingUnits>;
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
    return make_quantity<AssociatedUnit<RoundingUnits>>(round_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPoints<RoundingUnits>>(round_in(rounding_units, p));
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
    return make_quantity<AssociatedUnit<RoundingUnits>>(round_in<OutputRep>(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPoints<RoundingUnits>>(
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
    using OurRoundingRep = detail::RoundingRep<Quantity<U, R>, RoundingUnits>;
    return std::floor(q.template in<OurRoundingRep>(rounding_units));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    using OurRoundingRep = detail::RoundingRep<QuantityPoint<U, R>, RoundingUnits>;
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
    return make_quantity<AssociatedUnit<RoundingUnits>>(floor_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPoints<RoundingUnits>>(floor_in(rounding_units, p));
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
    return make_quantity<AssociatedUnit<RoundingUnits>>(floor_in<OutputRep>(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPoints<RoundingUnits>>(
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
    using OurRoundingRep = detail::RoundingRep<Quantity<U, R>, RoundingUnits>;
    return std::ceil(q.template in<OurRoundingRep>(rounding_units));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    using OurRoundingRep = detail::RoundingRep<QuantityPoint<U, R>, RoundingUnits>;
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
    return make_quantity<AssociatedUnit<RoundingUnits>>(ceil_in(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPoints<RoundingUnits>>(ceil_in(rounding_units, p));
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
    return make_quantity<AssociatedUnit<RoundingUnits>>(ceil_in<OutputRep>(rounding_units, q));
}
// b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return make_quantity_point<AssociatedUnitForPoints<RoundingUnits>>(
        ceil_in<OutputRep>(rounding_units, p));
}

//
// Rounding function that does not leave the integral domain.  Does not use `std::round`.
//
// This is the "Unit-only" format (i.e., `int_round_in(rounding_units, q)`).
//
// Common implementation helper:
template <typename RoundingUnits, template <class, class> class QType, typename U, typename R>
constexpr auto int_round_as_impl(RoundingUnits, QType<U, R> val) {
    static_assert(std::is_integral<R>::value, "int_round_as requires integral Rep type");

    constexpr auto target = AppropriateAssociatedUnit<QType, RoundingUnits>{};
    auto trunced = val.as(target, ignore(TRUNCATION_RISK));
    trunced.data_in(target) += (val - trunced).in(target / mag<2>(), ignore(TRUNCATION_RISK));
    return trunced;
}
// (a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_round_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_round_as_impl(rounding_units, q);
}
// (b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_round_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_round_as_impl(rounding_units, p);
}

//
// Rounding function that does not leave the integral domain.  Does not use `std::round`.
//
// This is the "Explicit-Rep" format (e.g., `int_round_as<int>(rounding_units, q)`).
//
// Common implementation helper:
template <typename OutputRep,
          typename RoundingUnits,
          template <class, class>
          class QType,
          typename U,
          typename R>
constexpr auto int_round_as_explicit_rep_impl(RoundingUnits, QType<U, R> val) {
    static_assert(std::is_integral<OutputRep>::value, "int_round_as output must be integral");

    constexpr auto target = AppropriateAssociatedUnit<QType, RoundingUnits>{};
    auto trunced = val.template as<OutputRep>(target, ignore(TRUNCATION_RISK));
    trunced.data_in(target) +=
        (val - trunced).template in<OutputRep>(target / mag<2>(), ignore(TRUNCATION_RISK));
    return trunced;
}
// (a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_round_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_round_as_explicit_rep_impl<OutputRep>(rounding_units, q);
}
// (b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_round_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_round_as_explicit_rep_impl<OutputRep>(rounding_units, p);
}

//
// Version of `int_round_as` with raw number outputs.
//
// This is the "Units-only" format (i.e., `int_round_in(rounding_units, q)`).
//
// (a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_round_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_round_as(rounding_units, q).in(rounding_units);
}
// (b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_round_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_round_as(rounding_units, p).in(associated_unit_for_points(rounding_units));
}

//
// Version of `int_round_as` with raw number outputs.
//
// This is the "Explicit-Rep" format (e.g., `int_round_in<int>(rounding_units, q)`).
//
// (a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_round_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_round_as<OutputRep>(rounding_units, q).in(rounding_units);
}
// (b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_round_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_round_as<OutputRep>(rounding_units, p).in(rounding_units);
}

//
// Floor function that does not leave the integral domain.  Does not use `std::floor`.
//
// This is the "Unit-only" format (i.e., `int_floor_in(rounding_units, q)`).
//
// Common implementation helper:
template <typename RoundingUnits, template <class, class> class QType, typename U, typename R>
constexpr auto int_floor_as_impl(RoundingUnits, QType<U, R> val) {
    static_assert(std::is_integral<R>::value, "int_floor_as requires integral Rep type");

    constexpr auto target = AppropriateAssociatedUnit<QType, RoundingUnits>{};
    auto trunced = val.as(target, ignore(TRUNCATION_RISK));
    trunced.data_in(target) -= R{trunced > val};
    return trunced;
}
// (a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_floor_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_floor_as_impl(rounding_units, q);
}
// (b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_floor_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_floor_as_impl(rounding_units, p);
}

//
// Floor function that does not leave the integral domain.  Does not use `std::floor`.
//
// This is the "Explicit-Rep" format (e.g., `int_floor_as<int>(rounding_units, q)`).
//
// Common implementation helper:
template <typename OutputRep,
          typename RoundingUnits,
          template <class, class>
          class QType,
          typename U,
          typename R>
constexpr auto int_floor_as_explicit_rep_impl(RoundingUnits, QType<U, R> val) {
    static_assert(std::is_integral<OutputRep>::value, "int_floor_as output must be integral");

    constexpr auto target = AppropriateAssociatedUnit<QType, RoundingUnits>{};
    auto trunced = val.template as<OutputRep>(target, ignore(TRUNCATION_RISK));
    trunced.data_in(target) -= OutputRep{trunced > val};
    return trunced;
}
// (a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_floor_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_floor_as_explicit_rep_impl<OutputRep>(rounding_units, q);
}
// (b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_floor_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_floor_as_explicit_rep_impl<OutputRep>(rounding_units, p);
}

//
// Version of `int_floor_as` with raw number outputs.
//
// This is the "Units-only" format (i.e., `int_floor_in(rounding_units, q)`).
//
// (a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_floor_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_floor_as(rounding_units, q).in(rounding_units);
}
// (b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_floor_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_floor_as(rounding_units, p).in(associated_unit_for_points(rounding_units));
}

//
// Version of `int_floor_as` with raw number outputs.
//
// This is the "Explicit-Rep" format (e.g., `int_floor_in<int>(rounding_units, q)`).
//
// (a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_floor_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_floor_as<OutputRep>(rounding_units, q).in(rounding_units);
}
// (b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_floor_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_floor_as<OutputRep>(rounding_units, p).in(rounding_units);
}

//
// Ceil function that does not leave the integral domain.  Does not use `std::ceil`.
//
// This is the "Unit-only" format (i.e., `int_ceil_in(rounding_units, q)`).
//
// Common implementation helper:
template <typename RoundingUnits, template <class, class> class QType, typename U, typename R>
constexpr auto int_ceil_as_impl(RoundingUnits, QType<U, R> val) {
    static_assert(std::is_integral<R>::value, "int_ceil_as requires integral Rep type");

    constexpr auto target = AppropriateAssociatedUnit<QType, RoundingUnits>{};
    auto trunced = val.as(target, ignore(TRUNCATION_RISK));
    trunced.data_in(target) += R{trunced < val};
    return trunced;
}
// (a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_ceil_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_ceil_as_impl(rounding_units, q);
}
// (b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_ceil_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_ceil_as_impl(rounding_units, p);
}

//
// Ceil function that does not leave the integral domain.  Does not use `std::ceil`.
//
// This is the "Explicit-Rep" format (e.g., `int_ceil_as<int>(rounding_units, q)`).
//
// Common implementation helper:
template <typename OutputRep,
          typename RoundingUnits,
          template <class, class>
          class QType,
          typename U,
          typename R>
constexpr auto int_ceil_as_explicit_rep_impl(RoundingUnits, QType<U, R> val) {
    static_assert(std::is_integral<OutputRep>::value, "int_ceil_as output must be integral");

    constexpr auto target = AppropriateAssociatedUnit<QType, RoundingUnits>{};
    auto trunced = val.template as<OutputRep>(target, ignore(TRUNCATION_RISK));
    trunced.data_in(target) += OutputRep{trunced < val};
    return trunced;
}
// (a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_ceil_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_ceil_as_explicit_rep_impl<OutputRep>(rounding_units, q);
}
// (b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_ceil_as(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_ceil_as_explicit_rep_impl<OutputRep>(rounding_units, p);
}

//
// Version of `int_ceil_as` with raw number outputs.
//
// This is the "Units-only" format (i.e., `int_ceil_in(rounding_units, q)`).
//
// (a) Version for Quantity.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_ceil_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_ceil_as(rounding_units, q).in(rounding_units);
}
// (b) Version for QuantityPoint.
template <typename RoundingUnits, typename U, typename R>
constexpr auto int_ceil_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_ceil_as(rounding_units, p).in(associated_unit_for_points(rounding_units));
}

//
// Version of `int_ceil_as` with raw number outputs.
//
// This is the "Explicit-Rep" format (e.g., `int_ceil_in<int>(rounding_units, q)`).
//
// (a) Version for Quantity.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_ceil_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return int_ceil_as<OutputRep>(rounding_units, q).in(rounding_units);
}
// (b) Version for QuantityPoint.
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
constexpr auto int_ceil_in(RoundingUnits rounding_units, QuantityPoint<U, R> p) {
    return int_ceil_as<OutputRep>(rounding_units, p).in(rounding_units);
}

// Wrapper for std::sin() which accepts a strongly typed angle quantity.
template <typename U, typename R>
auto sin(Quantity<U, R> q) {
    return std::sin(detail::in_radians(q));
}

// Wrapper for std::sqrt() which handles Quantity types.
template <typename U, typename R>
auto sqrt(Quantity<U, R> q) {
    return make_quantity<UnitPower<U, 1, 2>>(std::sqrt(q.in(U{})));
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
