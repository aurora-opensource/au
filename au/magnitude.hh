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
#include <limits>
#include <utility>

#include "au/fwd.hh"
#include "au/packs.hh"
#include "au/power_aliases.hh"
#include "au/stdx/utility.hh"
#include "au/utility/factoring.hh"
#include "au/utility/string_constant.hh"
#include "au/zero.hh"

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
using MagProduct = PackProduct<Magnitude, BPs...>;
template <typename... BPs>
using MagProductT = MagProduct<BPs...>;

template <typename T, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using MagPower = PackPower<Magnitude, T, ExpNum, ExpDen>;
template <typename T, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using MagPowerT = MagPower<T, ExpNum, ExpDen>;

template <typename T, typename U>
using MagQuotient = PackQuotient<Magnitude, T, U>;
template <typename T, typename U>
using MagQuotientT = MagQuotient<T, U>;

template <typename T>
using MagInverse = PackInverse<Magnitude, T>;
template <typename T>
using MagInverseT = MagInverse<T>;

// Enable negative magnitudes with a type representing (-1) that appears/disappears under powers.
struct Negative {};
template <typename... BPs, std::intmax_t ExpNum, std::intmax_t ExpDen>
struct PackPowerImpl<Magnitude, Magnitude<Negative, BPs...>, std::ratio<ExpNum, ExpDen>>
    : std::conditional<(std::ratio<ExpNum, ExpDen>::num % 2 == 0),

                       // Even powers of (-1) are 1 for any root.
                       MagPower<Magnitude<BPs...>, ExpNum, ExpDen>,

                       // At this point, we know we're taking the D'th root of (-1), which is (-1)
                       // if D is odd, and a hard compiler error if D is even.
                       MagProduct<Magnitude<Negative>, MagPower<Magnitude<BPs...>, ExpNum, ExpDen>>>
// Implement the hard error for raising to (odd / even) power:
{
    static_assert(std::ratio<ExpNum, ExpDen>::den % 2 == 1,
                  "Cannot take even root of negative magnitude");
};
template <typename... LeftBPs, typename... RightBPs>
struct PackProductImpl<Magnitude, Magnitude<Negative, LeftBPs...>, Magnitude<Negative, RightBPs...>>
    : stdx::type_identity<MagProduct<Magnitude<LeftBPs...>, Magnitude<RightBPs...>>> {};

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

// Check whether a Magnitude is representable in type T.
template <typename T, typename... BPs>
constexpr bool representable_in(Magnitude<BPs...> m);

// Get the value of this Magnitude in a "traditional" numeric type T.
template <typename T, typename... BPs>
constexpr T get_value(Magnitude<BPs...>);

// Let `Zero` "act like" a `Magnitude` for purposes of `get_value`.
template <typename T>
constexpr T get_value(Zero) {
    return T{0};
}

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
using IntegerPart = typename IntegerPartImpl<MagT>::type;
template <typename MagT>
using IntegerPartT = IntegerPart<MagT>;

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
using Numerator = typename NumeratorImpl<MagT>::type;
template <typename MagT>
using NumeratorT = Numerator<MagT>;

template <typename MagT>
using Denominator = Numerator<MagInverse<Abs<MagT>>>;
template <typename MagT>
using DenominatorT = Denominator<MagT>;

template <typename MagT>
struct IsPositive : std::true_type {};
template <typename... BPs>
struct IsPositive<Magnitude<Negative, BPs...>> : std::false_type {};

template <typename MagT>
struct IsRational
    : std::is_same<MagT,
                   MagQuotient<IntegerPart<Numerator<MagT>>, IntegerPart<Denominator<MagT>>>> {};

template <typename MagT>
struct IsInteger : std::is_same<MagT, IntegerPart<MagT>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Validation utilities for rational Magnitude arithmetic operations.
//
// Many common mathematical operations (comparison, addition, etc.) are not feasible in _general_
// for magnitudes.  However, we _can_ support them for _specific subsets_ of magnitudes, the
// simplest being purely rational magnitudes, whose absolute numerator and denominator fit in a
// 64-bit integer.  We have decided to provide these operations for _this subset only_, as it
// satisfies many practical use cases.

namespace detail {
template <typename MagT>
struct IsMagnitudeU64RationalCompatibleHelper {
    static constexpr bool is_rational() { return IsRational<MagT>::value; }
    static constexpr bool numerator_fits() {
        return representable_in<std::uint64_t>(Abs<Numerator<MagT>>{});
    }
    static constexpr bool denominator_fits() {
        return representable_in<std::uint64_t>(Denominator<MagT>{});
    }
};
template <>
struct IsMagnitudeU64RationalCompatibleHelper<Zero> {
    static constexpr bool is_rational() { return true; }
    static constexpr bool numerator_fits() { return true; }
    static constexpr bool denominator_fits() { return true; }
};

template <typename MagT>
struct IsMagnitudeU64RationalCompatible : IsMagnitudeU64RationalCompatibleHelper<MagT> {
    using IsMagnitudeU64RationalCompatibleHelper<MagT>::is_rational;
    using IsMagnitudeU64RationalCompatibleHelper<MagT>::numerator_fits;
    using IsMagnitudeU64RationalCompatibleHelper<MagT>::denominator_fits;
};

// Instantiating this struct will produce clear compiler errors if the Magnitude doesn't meet the
// requirements for arithmetic operations.
template <typename MagT>
struct AssertMagnitudeU64RationalCompatible {
    using Check = IsMagnitudeU64RationalCompatible<MagT>;
    static_assert(Check::is_rational(), "Mag must be purely rational");
    static_assert(Check::numerator_fits(), "Mag numerator too large to fit in uint64_t");
    static_assert(Check::denominator_fits(), "Mag denominator too large to fit in uint64_t");
};
}  // namespace detail

// The "common magnitude" of two Magnitudes is the largest Magnitude that evenly divides both.
//
// This is possible only if the quotient of the inputs is rational.  If it's not, then the "common
// magnitude" is one that is related to both inputs, and symmetrical under a change in order (to
// fulfill the requirements of a `std::common_type` specialization).
template <typename... Ms>
struct CommonMagnitudeImpl;
template <typename... Ms>
using CommonMagnitude = typename CommonMagnitudeImpl<Ms...>::type;
template <typename... Ms>
using CommonMagnitudeT = CommonMagnitude<Ms...>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Value based interface for Magnitude.

static constexpr auto ONE = Magnitude<>{};

template <typename... BP1s, typename... BP2s>
constexpr auto operator*(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return MagProduct<Magnitude<BP1s...>, Magnitude<BP2s...>>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator/(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return MagQuotient<Magnitude<BP1s...>, Magnitude<BP2s...>>{};
}

template <int E, typename... BPs>
constexpr auto pow(Magnitude<BPs...>) {
    return MagPower<Magnitude<BPs...>, E>{};
}

template <int N, typename... BPs>
constexpr auto root(Magnitude<BPs...>) {
    return MagPower<Magnitude<BPs...>, 1, N>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator==(Magnitude<BP1s...>, Magnitude<BP2s...>) {
    return std::is_same<Magnitude<BP1s...>, Magnitude<BP2s...>>::value;
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator!=(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    return !(m1 == m2);
}

namespace detail {

// Compare absolute values of two magnitudes.
//
// Returns:
//   -1 if |m1| < |m2|
//    0 if |m1| == |m2|
//   +1 if |m1| > |m2|
template <typename M1, typename M2>
constexpr int compare_absolute_magnitudes(M1, M2) {
    using AbsM1OverM2 = Abs<MagQuotient<M1, M2>>;
    (void)AssertMagnitudeU64RationalCompatible<AbsM1OverM2>{};

    constexpr auto lhs = get_value<std::uint64_t>(Numerator<AbsM1OverM2>{});
    constexpr auto rhs = get_value<std::uint64_t>(Denominator<AbsM1OverM2>{});
    return (lhs > rhs) - (lhs < rhs);
}

}  // namespace detail

// Comparison operators for Magnitude types.
//
// These will only be defined for the subset of Magnitudes where this is easy to compute.
template <typename... BP1s, typename... BP2s>
constexpr bool operator<(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    constexpr bool m1_positive = is_positive(m1);
    constexpr bool m2_positive = is_positive(m2);

    if (!m1_positive && m2_positive) {
        return true;
    }
    if (m1_positive && !m2_positive) {
        return false;
    }

    constexpr int abs_cmp = detail::compare_absolute_magnitudes(m1, m2);
    return m1_positive ? (abs_cmp < 0) : (abs_cmp > 0);
}

template <typename... BP1s, typename... BP2s>
constexpr bool operator>(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    return m2 < m1;
}

template <typename... BP1s, typename... BP2s>
constexpr bool operator<=(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    return !(m2 < m1);
}

template <typename... BP1s, typename... BP2s>
constexpr bool operator>=(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    return !(m1 < m2);
}

// Zero/Magnitude comparisons: Zero is less than any positive magnitude, greater than any negative.
template <typename... BPs>
constexpr bool operator<(Zero, Magnitude<BPs...>) {
    return IsPositive<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool operator>(Zero, Magnitude<BPs...>) {
    return !IsPositive<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool operator<=(Zero, Magnitude<BPs...>) {
    return IsPositive<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool operator>=(Zero, Magnitude<BPs...>) {
    return !IsPositive<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool operator<(Magnitude<BPs...>, Zero) {
    return !IsPositive<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool operator>(Magnitude<BPs...>, Zero) {
    return IsPositive<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool operator<=(Magnitude<BPs...>, Zero) {
    return !IsPositive<Magnitude<BPs...>>::value;
}

template <typename... BPs>
constexpr bool operator>=(Magnitude<BPs...>, Zero) {
    return IsPositive<Magnitude<BPs...>>::value;
}

//
// Addition and subtraction of Magnitudes (and Zero).
//
// Again, these are only defined for the subset of Magnitudes where this is easy to compute.
//

// Addition:
template <typename... BP1s, typename... BP2s>
constexpr auto operator+(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    constexpr auto sgn1 = sign(m1);
    constexpr auto sgn2 = sign(m2);

    constexpr auto abs_common = Abs<CommonMagnitude<Magnitude<BP1s...>, Magnitude<BP2s...>>>{};
    constexpr auto abs_num1 = abs(m1) / abs_common;
    constexpr auto abs_num2 = abs(m2) / abs_common;

    // These `get_value` calls automatically check that individual _inputs_ fit in `uint64_t`.
    constexpr auto abs_num1_u64 = get_value<std::uint64_t>(abs_num1);
    constexpr auto abs_num2_u64 = get_value<std::uint64_t>(abs_num2);

    // Biggest absolute input determines overall sign.
    //
    // Note that when the magnitudes are equal, either the choice doesn't matter (when the inputs
    // have the same sign), or the outcome should just be `Zero`.  In the latter case, we rely on
    // the explicit `Negative` overloads below being a better match.
    constexpr auto sgn =
        std::conditional_t<(abs_num1_u64 > abs_num2_u64), decltype(sgn1), decltype(sgn2)>{};

    // Here, we are taking advantage of modular arithmetic on unsigned integers.  This actually does
    // handle all the signs correctly, although it may not be obvious at first glance.
    constexpr auto num1_u64 = is_positive(sgn1) ? abs_num1_u64 : -abs_num1_u64;
    constexpr auto num2_u64 = is_positive(sgn2) ? abs_num2_u64 : -abs_num2_u64;
    constexpr auto abs_sum_u64 = is_positive(sgn) ? (num1_u64 + num2_u64) : -(num1_u64 + num2_u64);

    // Here is where we guard against overflow in the _output_.
    static_assert((sgn1 != sgn2) || abs_sum_u64 >= abs_num1_u64,
                  "Magnitude addition overflowed uint64_t");

    return sgn * mag<abs_sum_u64>() * abs_common;
}
template <typename... BPs>
constexpr Zero operator+(Magnitude<Negative, BPs...>, Magnitude<BPs...>) {
    return {};
}
template <typename... BPs>
constexpr Zero operator+(Magnitude<BPs...>, Magnitude<Negative, BPs...>) {
    return {};
}
template <typename... BPs>
constexpr auto operator+(Zero, Magnitude<BPs...> m) {
    return m;
}
template <typename... BPs>
constexpr auto operator+(Magnitude<BPs...> m, Zero) {
    return m;
}

// Subtraction:
template <typename... BP1s, typename... BP2s>
constexpr auto operator-(Magnitude<BP1s...> m1, Magnitude<BP2s...> m2) {
    return m1 + (-m2);
}
template <typename... BPs>
constexpr auto operator-(Zero, Magnitude<BPs...> m) {
    return -m;
}
template <typename... BPs>
constexpr auto operator-(Magnitude<BPs...> m, Zero) {
    return m;
}

template <typename... BPs>
constexpr auto integer_part(Magnitude<BPs...>) {
    return IntegerPart<Magnitude<BPs...>>{};
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
    return Numerator<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr auto denominator(Magnitude<BPs...>) {
    return Denominator<Magnitude<BPs...>>{};
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
    return CommonMagnitude<Ms...>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `mag<N>()` implementation.

namespace detail {

// Helper to perform prime factorization.
template <std::uintmax_t N>
struct PrimeFactorizationImpl;
template <std::uintmax_t N>
using PrimeFactorization = typename PrimeFactorizationImpl<N>::type;

// Base case: factorization of 1.
template <>
struct PrimeFactorizationImpl<1u> : stdx::type_identity<Magnitude<>> {};

template <std::uintmax_t N>
struct PrimeFactorizationImpl {
    static_assert(N > 0, "Can only factor positive integers");

    static constexpr std::uintmax_t base = find_prime_factor(N);
    static constexpr std::uintmax_t power = multiplicity(base, N);
    static constexpr std::uintmax_t remainder = N / int_pow(base, power);

    using type = MagProduct<Magnitude<Pow<Prime<base>, static_cast<std::intmax_t>(power)>>,
                            PrimeFactorization<remainder>>;
};

}  // namespace detail

template <std::uintmax_t N>
constexpr auto mag() {
    return detail::PrimeFactorization<N>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `integer_part()` implementation.

template <typename B, typename P>
struct IntegerPartOfBasePower : stdx::type_identity<Magnitude<>> {};

// Raise B to the largest natural number power which won't exceed (N/D), or 0 if there isn't one.
template <std::uintmax_t B, std::intmax_t N, std::intmax_t D>
struct IntegerPartOfBasePower<Prime<B>, std::ratio<N, D>>
    : stdx::type_identity<MagPower<Magnitude<Prime<B>>, ((N >= D) ? (N / D) : 0)>> {};

template <typename... BPs>
struct IntegerPartImpl<Magnitude<BPs...>>
    : stdx::type_identity<
          MagProduct<typename IntegerPartOfBasePower<Base<BPs>, Exp<BPs>>::type...>> {};

template <typename... BPs>
struct IntegerPartImpl<Magnitude<Negative, BPs...>>
    : stdx::type_identity<MagProduct<Magnitude<Negative>, IntegerPart<Magnitude<BPs...>>>> {};

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
          MagProduct<std::conditional_t<(Exp<BPs>::num > 0), Magnitude<BPs>, Magnitude<>>...>> {};

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
        constexpr auto widened_result = product(
            {base_power_value<RealPart<T>,
                              Exp<BPs>::num,
                              static_cast<std::uintmax_t>(Exp<BPs>::den)>(Base<BPs>::value())...});

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
    static constexpr const char value[25] = "(UNLABELED SCALE FACTOR)";

    static constexpr const bool has_exposed_slash = false;
};
template <typename MagT, MagLabelCategory Category>
constexpr const char MagnitudeLabelImplementation<MagT, Category>::value[25];
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
    using LabelT = ExtendedMagLabel<3u, Numerator<MagT>, Denominator<MagT>>;
    static constexpr LabelT value = join_by(
        " / ", MagnitudeLabel<Numerator<MagT>>::value, MagnitudeLabel<Denominator<MagT>>::value);

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
struct PrependIfExpNegativeImpl;
template <typename BP, typename MagT>
using PrependIfExpNegative = typename PrependIfExpNegativeImpl<BP, MagT>::type;
template <typename BP, typename... Ts>
struct PrependIfExpNegativeImpl<BP, Magnitude<Ts...>>
    : std::conditional<(Exp<BP>::num < 0), Magnitude<BP, Ts...>, Magnitude<Ts...>> {};

// Remove all positive powers from M.
template <typename M>
using NegativePowers = MagQuotient<M, NumeratorPart<M>>;
}  // namespace detail

// 1-ary case: identity.
template <typename M>
struct CommonMagnitudeImpl<M> : stdx::type_identity<M> {};

// 2-ary base case: both Magnitudes null.
template <>
struct CommonMagnitudeImpl<Magnitude<>, Magnitude<>> : stdx::type_identity<Magnitude<>> {};

// 2-ary base case: only left Magnitude is null.
template <typename Head, typename... Tail>
struct CommonMagnitudeImpl<Magnitude<>, Magnitude<Head, Tail...>>
    : stdx::type_identity<detail::NegativePowers<Magnitude<Head, Tail...>>> {};

// 2-ary base case: only right Magnitude is null.
template <typename Head, typename... Tail>
struct CommonMagnitudeImpl<Magnitude<Head, Tail...>, Magnitude<>>
    : stdx::type_identity<detail::NegativePowers<Magnitude<Head, Tail...>>> {};

// 2-ary recursive case: two non-null Magnitudes.
template <typename H1, typename... T1, typename H2, typename... T2>
struct CommonMagnitudeImpl<Magnitude<H1, T1...>, Magnitude<H2, T2...>> :

    // If the bases for H1 and H2 are in-order, prepend H1-if-negative to the remainder.
    std::conditional<
        (InOrderFor<Magnitude, Base<H1>, Base<H2>>::value),
        detail::PrependIfExpNegative<H1, CommonMagnitude<Magnitude<T1...>, Magnitude<H2, T2...>>>,

        // If the bases for H2 and H1 are in-order, prepend H2-if-negative to the remainder.
        std::conditional_t<
            (InOrderFor<Magnitude, Base<H2>, Base<H1>>::value),
            detail::PrependIfExpNegative<H2,
                                         CommonMagnitude<Magnitude<T2...>, Magnitude<H1, T1...>>>,

            // If we got here, the bases must be the same.  (We can assume that `InOrderFor` does
            // proper checking to guard against equivalent-but-not-identical bases, which would
            // violate total ordering.)
            std::conditional_t<
                (std::ratio_subtract<Exp<H1>, Exp<H2>>::num < 0),
                detail::Prepend<CommonMagnitude<Magnitude<T1...>, Magnitude<T2...>>, H1>,
                detail::Prepend<CommonMagnitude<Magnitude<T1...>, Magnitude<T2...>>, H2>>>> {};

// N-ary case: recurse.
template <typename M1, typename M2, typename... Tail>
struct CommonMagnitudeImpl<M1, M2, Tail...>
    : CommonMagnitudeImpl<M1, CommonMagnitude<M2, Tail...>> {};

// Zero is always ignored.
template <typename M>
struct CommonMagnitudeImpl<M, Zero> : stdx::type_identity<M> {};
template <typename M>
struct CommonMagnitudeImpl<Zero, M> : stdx::type_identity<M> {};
template <>
struct CommonMagnitudeImpl<Zero, Zero> : stdx::type_identity<Zero> {};

}  // namespace  au
