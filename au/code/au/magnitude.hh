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
template <std::size_t N>
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

#ifndef PI
// Some users must work with frameworks that define `PI` as a macro.  Having a macro with this
// easily collidable name is exceedingly unwise.  Nevertheless, that's not the users' fault, so we
// accommodate those frameworks by omitting the definition of `PI` in this case.
//
// If you are stuck with such a framework, you can choose a different name that does not collide,
// and reproduce the following line in your own system.
[[deprecated(
    "If you need a magnitude instance for pi, define your own as `constexpr auto PI = "
    "Magnitude<Pi>{};`")]] static constexpr auto PI = Magnitude<Pi>{};
#endif

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

template <std::size_t N>
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
constexpr MagRepresentationOrError<T> root(T x, std::uintmax_t n) {
    // The "zeroth root" would be mathematically undefined.
    if (n == 0) {
        return {MagRepresentationOutcome::ERR_INVALID_ROOT};
    }

    // The "first root" is trivial.
    if (n == 1) {
        return {MagRepresentationOutcome::OK, x};
    }

    // We only support nontrivial roots of floating point types.
    if (!std::is_floating_point<T>::value) {
        return {MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE};
    }

    // Handle negative numbers: only odd roots are allowed.
    if (x < 0) {
        if (n % 2 == 0) {
            return {MagRepresentationOutcome::ERR_INVALID_ROOT};
        } else {
            const auto negative_result = root(-x, n);
            if (negative_result.outcome != MagRepresentationOutcome::OK) {
                return {negative_result.outcome};
            }
            return {MagRepresentationOutcome::OK, static_cast<T>(-negative_result.value)};
        }
    }

    // Handle special cases of zero and one.
    if (x == 0 || x == 1) {
        return {MagRepresentationOutcome::OK, x};
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
    // At this point, error conditions are finished, and we can proceed with the "core" algorithm.
    //

    // Always use `long double` for intermediate computations.  We don't ever expect people to be
    // calling this at runtime, so we want maximum accuracy.
    long double lo = 1.0;
    long double hi = static_cast<long double>(x);

    // Do a binary search to find the closest value such that `checked_int_pow` recovers the input.
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

template <typename T, std::intmax_t N, std::uintmax_t D, typename B>
constexpr MagRepresentationOrError<Widen<T>> base_power_value(B base) {
    if (N < 0) {
        const auto inverse_result = base_power_value<T, -N, D>(base);
        if (inverse_result.outcome != MagRepresentationOutcome::OK) {
            return inverse_result;
        }
        return {
            MagRepresentationOutcome::OK,
            Widen<T>{1} / inverse_result.value,
        };
    }

    const auto power_result =
        checked_int_pow(static_cast<Widen<T>>(base), static_cast<std::uintmax_t>(N));
    if (power_result.outcome != MagRepresentationOutcome::OK) {
        return {power_result.outcome};
    }
    return (D > 1) ? root(power_result.value, D) : power_result;
}

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

template <typename T, typename... BPs>
constexpr MagRepresentationOrError<T> get_value_result(Magnitude<BPs...>) {
    // Representing non-integer values in integral types is something we never plan to support.
    constexpr bool REPRESENTING_NON_INTEGER_IN_INTEGRAL_TYPE =
        stdx::conjunction<std::is_integral<T>, stdx::negation<IsInteger<Magnitude<BPs...>>>>::value;
    if (REPRESENTING_NON_INTEGER_IN_INTEGRAL_TYPE) {
        return {MagRepresentationOutcome::ERR_NON_INTEGER_IN_INTEGER_TYPE};
    }

    // Force the expression to be evaluated in a constexpr context.
    constexpr auto widened_result = product(
        {base_power_value<RealPart<T>, ExpT<BPs>::num, static_cast<std::uintmax_t>(ExpT<BPs>::den)>(
            BaseT<BPs>::value())...});

    if ((widened_result.outcome != MagRepresentationOutcome::OK) ||
        !safe_to_cast_to<T>(widened_result.value)) {
        return {MagRepresentationOutcome::ERR_CANNOT_FIT};
    }

    return {MagRepresentationOutcome::OK, static_cast<T>(widened_result.value)};
}

// This simple overload avoids edge cases with creating and passing zero-sized arrays.
template <typename T>
constexpr MagRepresentationOrError<T> get_value_result(Magnitude<>) {
    return {MagRepresentationOutcome::OK, static_cast<T>(1)};
}

template <typename T, typename... BPs>
constexpr MagRepresentationOrError<T> get_value_result(Magnitude<Negative, BPs...>) {
    if (std::is_unsigned<T>::value) {
        return {MagRepresentationOutcome::ERR_NEGATIVE_NUMBER_IN_UNSIGNED_TYPE};
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
    if (IsInteger<Magnitude<BPs...>>::value) {
        return get_value_result<std::uintmax_t>(m).outcome == MagRepresentationOutcome::OK
                   ? MagLabelCategory::INTEGER
                   : MagLabelCategory::UNSUPPORTED;
    }
    if (IsRational<Magnitude<BPs...>>::value) {
        return MagLabelCategory::RATIONAL;
    }
    return MagLabelCategory::UNSUPPORTED;
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
