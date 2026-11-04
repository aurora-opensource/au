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
//
// The mixins themselves are empty "tag" base classes.  The operations they enable are function
// templates at namespace scope, each constrained to apply only to wrappers that inherit the
// corresponding tag.  Argument dependent lookup still finds these operations, because the namespace
// of a base class is an associated namespace of its derived class.
//
// We deliberately avoid the more traditional "hidden friend" approach.  Every time the compiler
// instantiates a class template with friend functions, it checks each friend declaration against
// _all_ same-named functions already declared in the enclosing namespace.  This makes the cost of
// each new wrapper specialization grow with the number of specializations already instantiated: a
// quadratic compile time cost, which can become severe for translation units that use many distinct
// units (for example, many distinct unit literals).
//
// The operations live in their own namespace, so that they only participate in overload resolution
// for types that inherit one of these mixins.  We re-export the mixin names into `au::detail`.

namespace au {
namespace detail {
namespace wrapper_ops {

// (Note: `TypeIdentityIfValidRep`, which these operations use, lives in "au/quantity.hh".)

//
// A mixin that enables turning a raw number into a Quantity by multiplying or dividing.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct MakesQuantityFromNumber {};

//
// A mixin that enables scaling the units of a Quantity by multiplying or dividing.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct ScalesQuantity {};

// A mixin to compose `op(U, O)` into a new unit wrapper, for "main" wrapper `U` and "other" wrapper
// `O`.  (Implementation detail helper for `ComposesWith`.)
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class OtherWrapper,
          template <typename U>
          class ResultWrapper>
struct PrecomposesWith {};

// A mixin to compose `op(O, U)` into a new unit wrapper, for "main" wrapper `U` and "other" wrapper
// `O`.  (Implementation detail helper for `ComposesWith`.)
template <template <typename U> class UnitWrapper,
          typename Unit,
          template <typename U>
          class OtherWrapper,
          template <typename U>
          class ResultWrapper>
struct PostcomposesWith {};

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
struct CanScaleByMagnitude {};

//
// A mixin to enable raising a unit wrapper to a rational power.
//
template <template <typename U> class UnitWrapper, typename Unit>
struct SupportsRationalPowers {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Constraint helpers.

template <template <typename U> class W, typename Unit>
using EnableIfMakesQuantityFromNumber =
    std::enable_if_t<std::is_base_of<MakesQuantityFromNumber<W, Unit>, W<Unit>>::value>;

template <template <typename U> class W, typename Unit>
using EnableIfScalesQuantity =
    std::enable_if_t<std::is_base_of<ScalesQuantity<W, Unit>, W<Unit>>::value>;

template <template <typename U> class W, typename Unit>
using EnableIfCanScaleByMagnitude =
    std::enable_if_t<std::is_base_of<CanScaleByMagnitude<W, Unit>, W<Unit>>::value>;

template <template <typename U> class W, typename Unit>
using EnableIfSupportsRationalPowers =
    std::enable_if_t<std::is_base_of<SupportsRationalPowers<W, Unit>, W<Unit>>::value>;

// A type to hold a single-parameter template, so we can compute it as the result of a metafunction.
template <template <typename U> class W>
struct WrapperTemplate {
    template <typename U>
    using Apply = W<U>;
};

// Find the `ResultWrapper` for composing `W<Unit>` with `O<U>`, by deducing it from the (unique)
// matching base class.  If there is no such base, this is a substitution failure.
template <template <typename U> class W,
          typename Unit,
          template <typename U>
          class O,
          template <typename U>
          class R>
WrapperTemplate<R> precompose_result(const volatile PrecomposesWith<W, Unit, O, R> *);
template <template <typename U> class W,
          typename Unit,
          template <typename U>
          class O,
          template <typename U>
          class R>
WrapperTemplate<R> postcompose_result(const volatile PostcomposesWith<W, Unit, O, R> *);

template <template <typename U> class W, typename Unit, template <typename U> class O>
using PrecomposeResult = decltype(precompose_result<W, Unit, O>(std::declval<W<Unit> *>()));

// When `W` and `O` are the same template, the "pre" composition already handles it, so we disable
// the "post" composition to avoid ambiguity.
template <template <typename U> class W, typename Unit, template <typename U> class O>
using PostcomposeResult =
    std::enable_if_t<!std::is_same<WrapperTemplate<W>, WrapperTemplate<O>>::value,
                     decltype(postcompose_result<W, Unit, O>(std::declval<W<Unit> *>()))>;

// The result wrapper for `op(L<UL>, R<UR>)`, where either `L` precomposes with `R`, or `R`
// postcomposes with `L`.  The former takes precedence, if both are present.
struct ComposeFallback {};
struct ComposePreferred : ComposeFallback {};

template <template <typename U> class L, typename UL, template <typename U> class R, typename UR>
auto compose_result(ComposePreferred) -> PrecomposeResult<L, UL, R>;
template <template <typename U> class L, typename UL, template <typename U> class R, typename UR>
auto compose_result(ComposeFallback) -> PostcomposeResult<R, UR, L>;

template <template <typename U> class L, typename UL, template <typename U> class R, typename UR>
using ComposeResult = decltype(compose_result<L, UL, R, UR>(ComposePreferred{}));

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MakesQuantityFromNumber` operations.

// (N * W), for number N and wrapper W.
template <typename T,
          template <typename U>
          class W,
          typename Unit,
          typename = EnableIfMakesQuantityFromNumber<W, Unit>>
constexpr auto operator*(T x, W<Unit>) -> Quantity<Unit, TypeIdentityIfValidRep<T>> {
    return make_quantity<Unit>(x);
}

// (W * N), for number N and wrapper W.
template <typename T,
          template <typename U>
          class W,
          typename Unit,
          typename = EnableIfMakesQuantityFromNumber<W, Unit>>
constexpr auto operator*(W<Unit>, T x) -> Quantity<Unit, TypeIdentityIfValidRep<T>> {
    return make_quantity<Unit>(x);
}

// (N / W), for number N and wrapper W.
template <typename T,
          template <typename U>
          class W,
          typename Unit,
          typename = EnableIfMakesQuantityFromNumber<W, Unit>>
constexpr auto operator/(T x, W<Unit>) -> Quantity<UnitInverse<Unit>, TypeIdentityIfValidRep<T>> {
    return make_quantity<UnitInverse<Unit>>(x);
}

// (W / N), for number N and wrapper W.
template <typename T,
          template <typename U>
          class W,
          typename Unit,
          typename = EnableIfMakesQuantityFromNumber<W, Unit>>
constexpr auto operator/(W<Unit>, T x) -> Quantity<Unit, TypeIdentityIfValidRep<T>> {
    static_assert(!std::is_integral<T>::value,
                  "Dividing by an integer value disallowed: would almost always produce 0");
    return make_quantity<Unit>(T{1} / x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ScalesQuantity` operations.

// (W * Q), for wrapper W and quantity Q.
template <template <typename U> class W,
          typename Unit,
          typename U,
          typename R,
          typename = EnableIfScalesQuantity<W, Unit>>
constexpr auto operator*(W<Unit>, Quantity<U, R> q) {
    return make_quantity<UnitProduct<Unit, U>>(q.in(U{}));
}

// (Q * W), for wrapper W and quantity Q.
template <template <typename U> class W,
          typename Unit,
          typename U,
          typename R,
          typename = EnableIfScalesQuantity<W, Unit>>
constexpr auto operator*(Quantity<U, R> q, W<Unit>) {
    return make_quantity<UnitProduct<U, Unit>>(q.in(U{}));
}

// (Q / W), for wrapper W and quantity Q.
template <template <typename U> class W,
          typename Unit,
          typename U,
          typename R,
          typename = EnableIfScalesQuantity<W, Unit>>
constexpr auto operator/(Quantity<U, R> q, W<Unit>) {
    return make_quantity<UnitQuotient<U, Unit>>(q.in(U{}));
}

// (W / Q), for wrapper W and quantity Q.
template <template <typename U> class W,
          typename Unit,
          typename U,
          typename R,
          typename = EnableIfScalesQuantity<W, Unit>>
constexpr auto operator/(W<Unit>, Quantity<U, R> q) {
    static_assert(!std::is_integral<R>::value,
                  "Dividing by an integer value disallowed: would almost always produce 0");
    return make_quantity<UnitQuotient<Unit, U>>(R{1} / q.in(U{}));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ComposesWith` operations.
//
// A single operator handles both "pre" composition (the left operand's wrapper declares that it
// composes with the right operand's wrapper) and "post" composition (vice versa).  Separate
// operators for each would be ambiguous with each other.

// (L * R), for wrappers L and R, where one composes with the other.
template <template <typename U> class L,
          typename UL,
          template <typename U>
          class R,
          typename UR,
          typename Result = ComposeResult<L, UL, R, UR>>
constexpr typename Result::template Apply<UnitProduct<UL, UR>> operator*(L<UL>, R<UR>) {
    return {};
}

// (L / R), for wrappers L and R, where one composes with the other.
template <template <typename U> class L,
          typename UL,
          template <typename U>
          class R,
          typename UR,
          typename Result = ComposeResult<L, UL, R, UR>>
constexpr typename Result::template Apply<UnitQuotient<UL, UR>> operator/(L<UL>, R<UR>) {
    return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanScaleByMagnitude` operations.

// (M * W), for magnitude M and wrapper W.
template <template <typename U> class W,
          typename Unit,
          typename... BPs,
          typename = EnableIfCanScaleByMagnitude<W, Unit>>
constexpr auto operator*(Magnitude<BPs...> m, W<Unit>) {
    return W<decltype(Unit{} * m)>{};
}

// (W * M), for magnitude M and wrapper W.
template <template <typename U> class W,
          typename Unit,
          typename... BPs,
          typename = EnableIfCanScaleByMagnitude<W, Unit>>
constexpr auto operator*(W<Unit>, Magnitude<BPs...> m) {
    return W<decltype(Unit{} * m)>{};
}

// (M / W), for magnitude M and wrapper W.
template <template <typename U> class W,
          typename Unit,
          typename... BPs,
          typename = EnableIfCanScaleByMagnitude<W, Unit>>
constexpr auto operator/(Magnitude<BPs...> m, W<Unit>) {
    return W<decltype(UnitInverse<Unit>{} * m)>{};
}

// (W / M), for magnitude M and wrapper W.
template <template <typename U> class W,
          typename Unit,
          typename... BPs,
          typename = EnableIfCanScaleByMagnitude<W, Unit>>
constexpr auto operator/(W<Unit>, Magnitude<BPs...> m) {
    return W<decltype(Unit{} / m)>{};
}

// (0 * W) and (W * 0), for wrapper W.
template <template <typename U> class W,
          typename Unit,
          typename = EnableIfCanScaleByMagnitude<W, Unit>>
constexpr Zero operator*(Zero, W<Unit>) {
    return {};
}
template <template <typename U> class W,
          typename Unit,
          typename = EnableIfCanScaleByMagnitude<W, Unit>>
constexpr Zero operator*(W<Unit>, Zero) {
    return {};
}

// Unary Plus and Negation.
template <template <typename U> class W,
          typename Unit,
          typename = EnableIfCanScaleByMagnitude<W, Unit>>
constexpr auto operator+(W<Unit> u) {
    return u;
}
template <template <typename U> class W,
          typename Unit,
          typename = EnableIfCanScaleByMagnitude<W, Unit>>
constexpr auto operator-(W<Unit>) {
    return W<decltype(Unit{} * (-mag<1>()))>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `SupportsRationalPowers` operations.

// (W^N), for wrapper W and integer N.
template <std::intmax_t N,
          template <typename U>
          class W,
          typename Unit,
          typename = EnableIfSupportsRationalPowers<W, Unit>>
constexpr auto pow(W<Unit>) {
    return W<UnitPower<Unit, N>>{};
}

// (W^(1/N)), for wrapper W and integer N.
template <std::intmax_t N,
          template <typename U>
          class W,
          typename Unit,
          typename = EnableIfSupportsRationalPowers<W, Unit>>
constexpr auto root(W<Unit>) {
    return W<UnitPower<Unit, 1, N>>{};
}

}  // namespace wrapper_ops

using wrapper_ops::CanScaleByMagnitude;
using wrapper_ops::ComposesWith;
using wrapper_ops::MakesQuantityFromNumber;
using wrapper_ops::PostcomposesWith;
using wrapper_ops::PrecomposesWith;
using wrapper_ops::ScalesQuantity;
using wrapper_ops::SupportsRationalPowers;

}  // namespace detail
}  // namespace au
