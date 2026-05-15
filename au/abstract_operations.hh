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

//
// `ScaleByRational<T, Num, Den>` represents an operation that scales a value of type `T` by the
// rational number `Num/Den`, taking care to avoid overflow if possible.
//
template <typename T, typename Num, typename Den>
struct ScaleByRational;

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
    static AU_DEVICE_FUNC constexpr U apply_to(T value) { return static_cast<U>(value); }
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
    static AU_DEVICE_FUNC constexpr U apply_to(T value) { return value; }
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
    static AU_DEVICE_FUNC constexpr T apply_to(T value) {
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
    static AU_DEVICE_FUNC constexpr T apply_to(T value) {
        static_assert(MagOutcome == MagRepresentationOutcome::OK, "Internal library error");
        return static_cast<T>(value / get_value<RealPart<T>>(M{}));
    }
};

template <typename T, typename M>
struct DivideTypeByIntegerImpl<T, M, MagRepresentationOutcome::ERR_CANNOT_FIT> {
    // If a number is too big to fit in the type, then dividing by it should produce 0.
    static AU_DEVICE_FUNC constexpr T apply_to(T) { return T{0}; }
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
    static AU_DEVICE_FUNC constexpr auto apply_to(OpInput<OpSequenceImpl> value) {
        return Op::apply_to(value);
    }
};

template <typename Op, typename... Ops>
struct OpSequenceImpl<Op, Ops...> {
    static AU_DEVICE_FUNC constexpr auto apply_to(OpInput<OpSequenceImpl> value) {
        return OpSequenceImpl<Ops...>::apply_to(Op::apply_to(value));
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ScaleByRational<T, Num, Den>` implementation.
//
// Modular decomposition avoids the intermediate `value * Num` overflow that occurs with
// (Num*T)/Den. GCD reduction of Num and Den maximises the safe input range.

template <typename T>
AU_DEVICE_FUNC constexpr T au_gcd(T a, T b) {
    return b == T{0} ? a : au_gcd(b, a % b);
}

template <typename T, typename Num, typename Den, bool IsIntegral>
struct ScaleByRationalImpl;

template <typename T, typename Num, typename Den>
struct OpInputImpl<ScaleByRational<T, Num, Den>> : stdx::type_identity<T> {};
template <typename T, typename Num, typename Den>
struct OpOutputImpl<ScaleByRational<T, Num, Den>> : stdx::type_identity<T> {};

template <typename T, typename Num, typename Den>
struct ScaleByRationalImpl<T, Num, Den, true> {
    static AU_DEVICE_FUNC constexpr T apply(T value) {
        using RealT = RealPart<T>;
        constexpr auto p_raw = get_value<RealT>(Num{});
        constexpr auto q_raw = get_value<RealT>(Den{});
        constexpr auto g = au_gcd(p_raw, q_raw);

        constexpr auto p = p_raw / g;
        constexpr auto q = q_raw / g;

        return static_cast<T>(p * (value / q) + (p * (value % q)) / q);
    }
};

template <typename T, typename Num, typename Den>
struct ScaleByRationalImpl<T, Num, Den, false> {
    static AU_DEVICE_FUNC constexpr T apply(T value) {
        return static_cast<T>(
            value * get_value<RealPart<T>>(MagProductT<Num, MagInverseT<Den>>{})
        );
    }
};

template <typename T, typename Num, typename Den>
struct ScaleByRational {
    static AU_DEVICE_FUNC constexpr T apply_to(T value) {
        return ScaleByRationalImpl<
            T, Num, Den,
            std::is_integral<RealPart<T>>::value
        >::apply(value);
    }
};

}  // namespace detail
}  // namespace au
