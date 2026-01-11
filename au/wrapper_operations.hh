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

#include "au/quantity.hh"
#include "au/stdx/type_traits.hh"

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
struct TypeIdentityIfLooksLikeValidRepImpl
    : std::conditional_t<std::is_arithmetic<T>::value, stdx::type_identity<T>, NoTypeMember> {};
template <typename T>
using TypeIdentityIfLooksLikeValidRep = typename TypeIdentityIfLooksLikeValidRepImpl<T>::type;

//
// A mixin that enables turning a raw number into a Quantity by multiplying or dividing.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct MakesQuantityFromNumber {
    // (N * W), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator*(T x, UnitWrapper<Unit>)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRep<T>> {
        return make_quantity<Unit>(x);
    }

    // (W * N), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator*(UnitWrapper<Unit>, T x)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRep<T>> {
        return make_quantity<Unit>(x);
    }

    // (N / W), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator/(T x, UnitWrapper<Unit>)
        -> Quantity<UnitInverse<Unit>, TypeIdentityIfLooksLikeValidRep<T>> {
        return make_quantity<UnitInverse<Unit>>(x);
    }

    // (W / N), for number N and wrapper W.
    template <typename T>
    friend constexpr auto operator/(UnitWrapper<Unit>, T x)
        -> Quantity<Unit, TypeIdentityIfLooksLikeValidRep<T>> {
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
        return make_quantity<UnitProduct<Unit, U>>(q.in(U{}));
    }

    // (Q * W), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator*(Quantity<U, R> q, UnitWrapper<Unit>) {
        return make_quantity<UnitProduct<U, Unit>>(q.in(U{}));
    }

    // (Q / W), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator/(Quantity<U, R> q, UnitWrapper<Unit>) {
        return make_quantity<UnitQuotient<U, Unit>>(q.in(U{}));
    }

    // (W / Q), for wrapper W and quantity Q.
    template <typename U, typename R>
    friend constexpr auto operator/(UnitWrapper<Unit>, Quantity<U, R> q) {
        static_assert(!std::is_integral<R>::value,
                      "Dividing by an integer value disallowed: would almost always produce 0");
        return make_quantity<UnitQuotient<Unit, U>>(R{1} / q.in(U{}));
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
    friend constexpr ResultWrapper<UnitProduct<Unit, U>> operator*(UnitWrapper<Unit>,
                                                                   OtherWrapper<U>) {
        return {};
    }

    // (U / O), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitQuotient<Unit, U>> operator/(UnitWrapper<Unit>,
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
    friend constexpr ResultWrapper<UnitProduct<U, Unit>> operator*(OtherWrapper<U>,
                                                                   UnitWrapper<Unit>) {
        return {};
    }

    // (O / U), for "main" wrapper U and "other" wrapper O.
    template <typename U>
    friend constexpr ResultWrapper<UnitQuotient<U, Unit>> operator/(OtherWrapper<U>,
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
        return UnitWrapper<decltype(UnitInverse<Unit>{} * m)>{};
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
        return UnitWrapper<UnitPower<Unit, N>>{};
    }

    // (W^(1/N)), for wrapper W and integer N.
    template <std::intmax_t N>
    friend constexpr auto root(UnitWrapper<Unit>) {
        return UnitWrapper<UnitPower<Unit, 1, N>>{};
    }
};

}  // namespace detail
}  // namespace au
