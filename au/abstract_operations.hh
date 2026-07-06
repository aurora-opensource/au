// Copyright 2025 Aurora Operations, Inc.
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

#include <type_traits>
#include <utility>

#include "au/config.hh"
#include "au/magnitude.hh"

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
// `ImplicitConversion<T, U>` represents an operation that implicitly converts from `T` to `U`.
//
template <typename T, typename U>
struct ImplicitConversion;

//
// `MultiplyTypeBy<T, M>` represents an operation that multiplies a value of type `T` by the
// magnitude `M`.
//
template <typename T, typename M>
struct MultiplyTypeBy;

//
// `DivideTypeByInteger<T, M>` represents an operation that divides a value of type `T` by the
// magnitude `M`.
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
//
// This is an eager (materializing) operation: it produces a fresh `U`.  We take the input by
// forwarding reference and forward it into the cast, so that when the chain hands us an rvalue
// (e.g. the materialized result of a previous step), we *move* rather than copy a heap-backed rep.
template <typename T, typename U>
struct StaticCast {
    template <typename V>
    static AU_DEVICE_FUNC constexpr U apply_to(V &&value) {
        static_assert(std::is_same<std::decay_t<V>, std::decay_t<T>>::value,
                      "Internal library error: input type does not match operation input");
        return static_cast<U>(std::forward<V>(value));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ImplicitConversion<T, U>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename U>
struct OpInputImpl<ImplicitConversion<T, U>> : stdx::type_identity<T> {};
template <typename T, typename U>
struct OpOutputImpl<ImplicitConversion<T, U>> : stdx::type_identity<U> {};

// `ImplicitConversion<T, U>` operation:
//
// Like `StaticCast`, this is eager: forward the input so an rvalue is moved into the produced `U`.
template <typename T, typename U>
struct ImplicitConversion {
    template <typename V>
    static AU_DEVICE_FUNC constexpr U apply_to(V &&value) {
        static_assert(std::is_same<std::decay_t<V>, std::decay_t<T>>::value,
                      "Internal library error: input type does not match operation input");
        return std::forward<V>(value);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename M>
struct OpInputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};
template <typename T, typename M>
struct OpOutputImpl<MultiplyTypeBy<T, M>>
    : stdx::type_identity<decltype(std::declval<T>() * std::declval<RealPart<T>>())> {};

// Identity magnitude preserves type.
template <typename T>
struct OpOutputImpl<MultiplyTypeBy<T, Magnitude<>>> : stdx::type_identity<T> {};

// `MultiplyTypeBy<T, M>` operation:
template <typename T, typename Mag>
struct MultiplyTypeBy {
    static AU_DEVICE_FUNC constexpr OpOutput<MultiplyTypeBy<T, Mag>> apply_to(const T &value) {
        return value * get_value<RealPart<T>>(Mag{});
    }
};

// Specialization for identity magnitude: just return the value unchanged.
//
// This is eager (it produces a `T` by value), so forward the input: an rvalue coming from a prior
// step in the chain is moved through rather than deep-copied.  (The non-identity `MultiplyTypeBy`
// above stays a `const T &` overload on purpose: it returns a *lazy* expression that refers to its
// input, so it must not take ownership of --- or dangle past --- that input.)
template <typename T>
struct MultiplyTypeBy<T, Magnitude<>> {
    template <typename V>
    static AU_DEVICE_FUNC constexpr T apply_to(V &&value) {
        static_assert(std::is_same<std::decay_t<V>, std::decay_t<T>>::value,
                      "Internal library error: input type does not match operation input");
        return std::forward<V>(value);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger<T, M>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename M>
struct OpInputImpl<DivideTypeByInteger<T, M>> : stdx::type_identity<T> {};
template <typename T, typename M>
struct OpOutputImpl<DivideTypeByInteger<T, M>>
    : stdx::type_identity<decltype(std::declval<T>() / std::declval<RealPart<T>>())> {};

template <typename T, typename M, MagRepresentationOutcome MagOutcome>
struct DivideTypeByIntegerImpl {
    static AU_DEVICE_FUNC constexpr OpOutput<DivideTypeByInteger<T, M>> apply_to(const T &value) {
        static_assert(MagOutcome == MagRepresentationOutcome::OK, "Internal library error");
        return value / get_value<RealPart<T>>(M{});
    }
};

template <typename T, typename M>
struct DivideTypeByIntegerImpl<T, M, MagRepresentationOutcome::ERR_CANNOT_FIT> {
    // If a number is too big to fit in the type, then dividing by it should produce 0.
    static AU_DEVICE_FUNC constexpr OpOutput<DivideTypeByInteger<T, M>> apply_to(const T &) {
        return OpOutput<DivideTypeByInteger<T, M>>{0};
    }
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

// We thread the value through the chain by forwarding reference.  Each step's result is a prvalue
// (for eager steps) or a lazy expression referring to a still-live operand; forwarding lets the
// next step *move* an eager result rather than copy it.  The very first step still receives the
// caller's lvalue (e.g. a `Quantity`'s stored member), so it copies/converts exactly once --- that
// inherent conversion pass is unavoidable --- while every subsequent step moves.
template <typename Op>
struct OpSequenceImpl<Op> {
    template <typename V>
    static AU_DEVICE_FUNC constexpr auto apply_to(V &&value) {
        return Op::apply_to(std::forward<V>(value));
    }
};

template <typename Op, typename... Ops>
struct OpSequenceImpl<Op, Ops...> {
    template <typename V>
    static AU_DEVICE_FUNC constexpr auto apply_to(V &&value) {
        return OpSequenceImpl<Ops...>::apply_to(Op::apply_to(std::forward<V>(value)));
    }
};

}  // namespace detail
}  // namespace au
