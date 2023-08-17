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

#include "au/packs.hh"
#include "au/power_aliases.hh"
#include "au/stdx/utility.hh"
#include "au/utility/factoring.hh"
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
struct NumeratorImpl;
template <typename MagT>
using NumeratorT = typename NumeratorImpl<MagT>::type;

template <typename MagT>
using DenominatorT = NumeratorT<MagInverseT<MagT>>;

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
static constexpr auto PI = Magnitude<Pi>{};

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
constexpr auto numerator(Magnitude<BPs...>) {
    return NumeratorT<Magnitude<BPs...>>{};
}

template <typename... BPs>
constexpr auto denominator(Magnitude<BPs...>) {
    return DenominatorT<Magnitude<BPs...>>{};
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

    static constexpr std::uintmax_t first_base = find_first_factor(N);
    static constexpr std::uintmax_t first_power = multiplicity(first_base, N);
    static constexpr std::uintmax_t remainder = N / int_pow(first_base, first_power);

    using type =
        MagProductT<Magnitude<Pow<Prime<first_base>, first_power>>, PrimeFactorizationT<remainder>>;
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// `numerator()` implementation.

template <typename... BPs>
struct NumeratorImpl<Magnitude<BPs...>>
    : stdx::type_identity<
          MagProductT<std::conditional_t<(ExpT<BPs>::num > 0), Magnitude<BPs>, Magnitude<>>...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `get_value<T>(Magnitude)` implementation.

namespace detail {

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

template <typename T, std::intmax_t N, typename B>
constexpr Widen<T> base_power_value(B base) {
    return (N < 0) ? (Widen<T>{1} / base_power_value<T, -N>(base))
                   : int_pow(static_cast<Widen<T>>(base), static_cast<std::uintmax_t>(N));
}

template <typename T, std::size_t N>
constexpr T product(const T (&values)[N]) {
    T result{1};
    for (const auto &x : values) {
        result *= x;
    }
    return result;
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

template <typename Target, typename Enable = void>
struct SafeCastingChecker {
    template <typename T>
    constexpr bool operator()(T x) {
        return stdx::cmp_less_equal(std::numeric_limits<Target>::lowest(), x) &&
               stdx::cmp_greater_equal(std::numeric_limits<Target>::max(), x);
    }
};

template <typename Target>
struct SafeCastingChecker<Target, std::enable_if_t<std::is_integral<Target>::value>> {
    template <typename T>
    constexpr bool operator()(T x) {
        return std::is_integral<T>::value &&
               stdx::cmp_less_equal(std::numeric_limits<Target>::lowest(), x) &&
               stdx::cmp_greater_equal(std::numeric_limits<Target>::max(), x);
    }
};

template <typename T, typename InputT>
constexpr bool safe_to_cast_to(InputT x) {
    return SafeCastingChecker<T>{}(x);
}

}  // namespace detail

template <typename T, typename... BPs>
constexpr T get_value(Magnitude<BPs...>) {
    using namespace detail;

    // Representing non-integer values in integral types is something we never plan to support.
    constexpr bool REPRESENTING_NON_INTEGER_IN_INTEGRAL_TYPE =
        stdx::conjunction<std::is_integral<T>, stdx::negation<IsInteger<Magnitude<BPs...>>>>::value;
    static_assert(!REPRESENTING_NON_INTEGER_IN_INTEGRAL_TYPE,
                  "Cannot represent non-integer in integral destination type");

    // Computing values for rational base powers is something we would _like_ to support, but we
    // need a `constexpr` implementation of `powl()` first.
    static_assert(all({(ExpT<BPs>::den == 1)...}),
                  "Computing values for rational powers not yet supported");

    // Force the expression to be evaluated in a constexpr context.
    constexpr auto widened_result =
        product({base_power_value<T, (ExpT<BPs>::num / ExpT<BPs>::den)>(BaseT<BPs>::value())...});

    static_assert(safe_to_cast_to<T>(widened_result), "Value outside range of destination type");
    return static_cast<T>(widened_result);
}

// This simple overload avoids edge cases with creating and passing zero-sized arrays.
template <typename T>
constexpr T get_value(Magnitude<>) {
    return 1;
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

// If M is (N/D), DenominatorPartT<M> is D; we want 1/D.
template <typename M>
using NegativePowers = MagInverseT<DenominatorPartT<M>>;
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
