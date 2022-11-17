// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#pragma once

#include <cmath>
#include <limits>

#include "au/units.hh"

namespace au {

// If we don't provide these, then unqualified uses of `sin()`, etc. from <cmath> will break.  Name
// Lookup will stop once it hits `::au::sin()`, hiding the `::sin()` overload in the global
// namespace.  To learn more about Name Lookup, see this article (https://abseil.io/tips/49).
using std::abs;
using std::copysign;
using std::cos;
using std::fmod;
using std::isnan;
using std::max;
using std::min;
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
    return make_quantity<UnitProductT<>>(Rep{1}).in(associated_unit(target_units) * U{}) /
           q.in(U{});
}

//
// The value of the "smart" inverse of a Quantity, in a given destination unit.
//
// By "smart", we mean that, e.g., you can convert an integral Quantity of Mega<Hertz> to an
// integral Quantity of Nano<Seconds>, without ever leaving the integral domain.  (Under the hood,
// in this case, the library will know to divide into 1000 instead of dividing into 1.)
//
template <typename TargetUnits, typename U, typename R>
constexpr auto inverse_in(TargetUnits target_units, Quantity<U, R> q) {
    // The policy here is similar to our overflow policy, in that we try to avoid "bad outcomes"
    // when users store values less than 1000.  (The thinking, here as there, is that values _more_
    // than 1000 would tend to be stored in the next SI-prefixed unit up, e.g., 1 km instead of 1000
    // m.)
    //
    // The "bad outcome" here is the inverse of a nonzero value getting represented as 0.  To avoid
    // this for all integral numbers 1000 and less, the dividend must be at least 1000.
    static_assert(
        make_quantity<UnitProductT<>>(R{1}).in(associated_unit(target_units) * U{}) >= 1000 ||
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
// Check whether the value stored is "not a number" (NaN).
//
template <typename U, typename R>
constexpr bool isnan(Quantity<U, R> q) {
    return std::isnan(q.in(U{}));
}

// The maximum of two values of the same dimension.
//
// Unlike std::max, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
auto max(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, [](auto a, auto b) { return std::max(a, b); });
}

// Overload to resolve ambiguity with `std::max` for identical `Quantity` types.
template <typename U, typename R>
auto max(Quantity<U, R> a, Quantity<U, R> b) {
    return std::max(a, b);
}

// The maximum of two point values of the same dimension.
//
// Unlike std::max, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
auto max(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, [](auto a, auto b) { return std::max(a, b); });
}

// Overload to resolve ambiguity with `std::max` for identical `QuantityPoint` types.
template <typename U, typename R>
auto max(QuantityPoint<U, R> a, QuantityPoint<U, R> b) {
    return std::max(a, b);
}

// The minimum of two values of the same dimension.
//
// Unlike std::min, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
auto min(Quantity<U1, R1> q1, Quantity<U2, R2> q2) {
    return detail::using_common_type(q1, q2, [](auto a, auto b) { return std::min(a, b); });
}

// Overload to resolve ambiguity with `std::min` for identical `Quantity` types.
template <typename U, typename R>
auto min(Quantity<U, R> a, Quantity<U, R> b) {
    return std::min(a, b);
}

// The minimum of two point values of the same dimension.
//
// Unlike std::min, returns by value rather than by reference, because the types might differ.
template <typename U1, typename U2, typename R1, typename R2>
auto min(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2) {
    return detail::using_common_point_unit(p1, p2, [](auto a, auto b) { return std::min(a, b); });
}

// Overload to resolve ambiguity with `std::min` for identical `QuantityPoint` types.
template <typename U, typename R>
auto min(QuantityPoint<U, R> a, QuantityPoint<U, R> b) {
    return std::min(a, b);
}

//
// Round the value of this Quantity to the nearest integer in the given units.
//
// This is the "Unit-only" format (i.e., `round_in(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::round(q.template in<OurRoundingRep>(rounding_units));
}

//
// Round the value of this Quantity to the nearest integer in the given units, returning OutputRep.
//
// This is the "Explicit-Rep" format (e.g., `round_in<int>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(round_in(rounding_units, q));
}

//
// The integral-valued Quantity, in this unit, nearest to the input.
//
// This is the "Unit-only" format (i.e., `round_as(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in(rounding_units, q));
}

//
// The integral-valued Quantity, in this unit, nearest to the input, using the specified OutputRep.
//
// This is the "Explicit-Rep" format (e.g., `round_as<float>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in<OutputRep>(rounding_units, q));
}

//
// Return the largest integral value in `rounding_units` which is not greater than `q`.
//
// This is the "Unit-only" format (i.e., `floor_in(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::floor(q.template in<OurRoundingRep>(rounding_units));
}

//
// Return `OutputRep` with largest integral value in `rounding_units` which is not greater than `q`.
//
// This is the "Explicit-Rep" format (e.g., `floor_in<int>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(floor_in(rounding_units, q));
}

//
// The largest integral-valued Quantity, in this unit, not greater than the input.
//
// This is the "Unit-only" format (i.e., `floor_as(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(floor_in(rounding_units, q));
}

//
// The largest integral-valued Quantity, in this unit, not greater than the input, using the
// specified `OutputRep`.
//
// This is the "Explicit-Rep" format (e.g., `floor_as<float>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(floor_in<OutputRep>(rounding_units, q));
}

//
// Return the smallest integral value in `rounding_units` which is not less than `q`.
//
// This is the "Unit-only" format (i.e., `ceil_in(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    using OurRoundingRep = detail::RoundingRepT<Quantity<U, R>, RoundingUnits>;
    return std::ceil(q.template in<OurRoundingRep>(rounding_units));
}

//
// Return the smallest integral value in `rounding_units` which is not less than `q`.
//
// This is the "Explicit-Rep" format (e.g., `ceil_in<int>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q) {
    return static_cast<OutputRep>(ceil_in(rounding_units, q));
}

//
// The smallest integral-valued Quantity, in this unit, not less than the input.
//
// This is the "Unit-only" format (i.e., `ceil_as(rounding_units, q)`).
//
template <typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(ceil_in(rounding_units, q));
}

//
// The smallest integral-valued Quantity, in this unit, not less than the input, using the specified
// `OutputRep`.
//
// This is the "Explicit-Rep" format (e.g., `ceil_as<float>(rounding_units, q)`).
//
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q) {
    return make_quantity<AssociatedUnitT<RoundingUnits>>(ceil_in<OutputRep>(rounding_units, q));
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
