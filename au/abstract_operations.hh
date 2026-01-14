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
// Note that this operation does *not* model integer promotion.  It will always force the result to
// be `T`.  To model integer promotion, form a compound operation with `OpSequence` that includes
// appropriate `StaticCast`.
//
template <typename T, typename M>
struct MultiplyTypeBy;

//
// `DivideTypeByInteger<T, M>` represents an operation that divides a value of type `T` by the
// magnitude `M`.
//
// Note that this operation does *not* model integer promotion.  It will always force the result to
// be `T`.  To model integer promotion, form a compound operation with `OpSequence` that includes
// appropriate `StaticCast`.
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
template <typename T, typename U>
struct StaticCast {
    static constexpr U apply_to(T value) { return static_cast<U>(value); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ImplicitConversion<T, U>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename U>
struct OpInputImpl<ImplicitConversion<T, U>> : stdx::type_identity<T> {};
template <typename T, typename U>
struct OpOutputImpl<ImplicitConversion<T, U>> : stdx::type_identity<U> {};

// `ImplicitConversion<T, U>` operation:
template <typename T, typename U>
struct ImplicitConversion {
    static constexpr U apply_to(T value) { return value; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename M>
struct OpInputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};
template <typename T, typename M>
struct OpOutputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};

// `MultiplyTypeBy<T, M>` operation:
template <typename T, typename Mag>
struct MultiplyTypeBy {
    static constexpr T apply_to(T value) {
        return static_cast<T>(value * get_value<RealPart<T>>(Mag{}));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger<T, M>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename M>
struct OpInputImpl<DivideTypeByInteger<T, M>> : stdx::type_identity<T> {};
template <typename T, typename M>
struct OpOutputImpl<DivideTypeByInteger<T, M>> : stdx::type_identity<T> {};

template <typename T, typename M, MagRepresentationOutcome MagOutcome>
struct DivideTypeByIntegerImpl {
    static constexpr T apply_to(T value) {
        static_assert(MagOutcome == MagRepresentationOutcome::OK, "Internal library error");
        return static_cast<T>(value / get_value<RealPart<T>>(M{}));
    }
};

template <typename T, typename M>
struct DivideTypeByIntegerImpl<T, M, MagRepresentationOutcome::ERR_CANNOT_FIT> {
    // If a number is too big to fit in the type, then dividing by it should produce 0.
    static constexpr T apply_to(T) { return T{0}; }
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

template <typename Op>
struct OpSequenceImpl<Op> {
    static constexpr auto apply_to(OpInput<OpSequenceImpl> value) { return Op::apply_to(value); }
};

template <typename Op, typename... Ops>
struct OpSequenceImpl<Op, Ops...> {
    static constexpr auto apply_to(OpInput<OpSequenceImpl> value) {
        return OpSequenceImpl<Ops...>::apply_to(Op::apply_to(value));
    }
};

}  // namespace detail
}  // namespace au
