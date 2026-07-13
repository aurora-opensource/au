// Copyright 2024 Aurora Operations, Inc.
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

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "au/fwd.hh"
#include "au/magnitude.hh"
#include "au/stdx/experimental/is_detected.hh"
#include "au/stdx/type_traits.hh"

namespace au {

//
// A type trait that determines if a type is a valid representation type for `Quantity` or
// `QuantityPoint`.
//
template <typename T>
struct IsValidRep;

//
// A type trait to indicate whether the product of two types is a valid rep.
//
// Will validly return `false` if the product does not exist.
//
template <typename T, typename U>
struct IsProductValidRep;

//
// A type trait to indicate whether the quotient of two types is a valid rep.
//
// Will validly return `false` if the quotient does not exist.
//
template <typename T, typename U>
struct IsQuotientValidRep;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {
template <typename T>
struct IsAuType : std::false_type {};

template <typename U, typename R>
struct IsAuType<::au::Quantity<U, R>> : std::true_type {};

template <typename U, typename R>
struct IsAuType<::au::QuantityPoint<U, R>> : std::true_type {};

//
// `NormalizeRep<T>`: strip vendor attributes (e.g. Green Hills' `__packed`) from an integral rep by
// naming a clean standard type, rather than relying on `std::decay` to drop the attribute (which
// GHS does not do).
//
// This is the _identity_ on every genuine standard type (integral or not), so it is a provable
// no-op for any rep a user would normally write.  It only rewrites a type that behaves like an
// integer, yet names *none* of the standard integer types.  This is the telltale sign of attributed
// types, such as `__packed uint16_t`.  In these cases, we map it to the fixed-width standard
// integer with the same `sizeof` and signedness.
//
// Critically, gating this on `std::is_integral<T>` won't work.  The whole reason `std::decay` fails
// to help on GHS is that GHS keeps vendor attributes on the type --- and it *also* mis-answers
// `std::is_integral` for such a type (it reports `false`).  So a normalization gated on
// `is_integral` not only won't be reliable, but also fails on the motivating example.  Instead we
// detect integer-ness through mechanisms the attribute does not defeat:
//
//   * Integer-ness: `is_integral<decltype(+declval<T>())>`.  Unary `+` triggers integral promotion,
//     which yields a fresh prvalue of a *standard* type --- this reliably strips the vendor
//     attribute.  (Note that Au already depends on exactly this behavior in `io.hh`).  We then ask
//     `is_integral` about that clean, promoted type, which GHS answers correctly.
//   * Width: `sizeof(T)` --- a core operator, unaffected by the attribute.
//   * Signedness: the value test `T(-1) < T(0)` --- core arithmetic, not `std::is_signed`.
//
// We additionally leave every standard type untouched, and never normalize class, union, or enum
// types, to keep this fix as targeted as possible.
//

// Is `T` *exactly* one of the standard integer types?  An attributed integral type compares unequal
// to all of these, so it is not "standard" by this definition.
template <typename T>
struct IsStandardInteger : stdx::disjunction<std::is_same<T, bool>,
                                             std::is_same<T, char>,
                                             std::is_same<T, signed char>,
                                             std::is_same<T, unsigned char>,
#if defined(__cpp_char8_t)
                                             std::is_same<T, char8_t>,
#endif
                                             std::is_same<T, char16_t>,
                                             std::is_same<T, char32_t>,
                                             std::is_same<T, wchar_t>,
                                             std::is_same<T, short>,
                                             std::is_same<T, unsigned short>,
                                             std::is_same<T, int>,
                                             std::is_same<T, unsigned int>,
                                             std::is_same<T, long>,
                                             std::is_same<T, unsigned long>,
                                             std::is_same<T, long long>,
                                             std::is_same<T, unsigned long long>> {
};

// The type `T` promotes to under unary `+`.  Integral promotion produces a fresh standard prvalue,
// which launders any vendor attribute off of `T`.  (Ill-formed --- hence a SFINAE removal below ---
// for types with no unary `+`, which is exactly what we want: they are not integers to normalize.)
template <typename T>
using PromotedRep = decltype(+std::declval<T>());

// Attribute-immune signedness: for an unsigned type `T(-1)` wraps to the maximum value (not `< 0`);
// for a signed type it is `-1`.  Uses arithmetic, not `std::is_signed` (which the attribute may
// defeat on GHS).
template <typename T>
constexpr bool rep_is_signed() {
    return static_cast<T>(-1) < static_cast<T>(0);
}

// Should we normalize `T`?  True exactly for an integer-behaving type that is not already a
// standard integer and is not a class/union/enum.  See the mechanism notes above for why none of
// these predicates route through `is_integral<T>` / `is_signed<T>` on the attributed type itself.
template <typename T, typename Enable = void>
struct ShouldNormalizeRep : std::false_type {};  // no unary `+` (e.g. most class reps): leave alone
template <typename T>
struct ShouldNormalizeRep<T, stdx::void_t<PromotedRep<T>>>
    : stdx::conjunction<std::is_integral<PromotedRep<T>>,
                        stdx::negation<IsStandardInteger<T>>,
                        stdx::negation<std::is_class<T>>,
                        stdx::negation<std::is_union<T>>,
                        stdx::negation<std::is_enum<T>>> {};

// Pick the fixed-width standard integer type (`int8_t` ... `int64_t` and unsigned counterparts)
// with the given `sizeof` and signedness; if none matches, fall back to `Fallback` (so an exotic
// integral such as `__int128`, whose width no fixed-width type covers, is left untouched rather
// than becoming a hard error).  We use the fixed-width candidates deliberately: there is exactly
// one per (size, signedness), so the selection is unambiguous --- no reliance on integer-rank
// tie-breaking.
template <typename Fallback, std::size_t Size, bool Signed, typename... Candidates>
struct FirstMatchingIntegerOr : stdx::type_identity<Fallback> {};

template <typename Fallback, std::size_t Size, bool Signed, typename C, typename... Rest>
struct FirstMatchingIntegerOr<Fallback, Size, Signed, C, Rest...>
    : std::conditional_t<sizeof(C) == Size && (std::is_signed<C>::value == Signed),
                         stdx::type_identity<C>,
                         FirstMatchingIntegerOr<Fallback, Size, Signed, Rest...>> {};

template <typename T, typename Enable = void>
struct NormalizeRepImpl : stdx::type_identity<T> {};  // non-integer or already-standard: identity

template <typename T>
struct NormalizeRepImpl<T, std::enable_if_t<ShouldNormalizeRep<T>::value>>
    : FirstMatchingIntegerOr<T,
                             sizeof(T),
                             rep_is_signed<T>(),
                             std::int8_t,
                             std::uint8_t,
                             std::int16_t,
                             std::uint16_t,
                             std::int32_t,
                             std::uint32_t,
                             std::int64_t,
                             std::uint64_t> {};

template <typename T>
using NormalizeRep = typename NormalizeRepImpl<T>::type;

template <typename T>
using CorrespondingUnit = typename CorrespondingQuantity<T>::Unit;

template <typename T>
using CorrespondingRep = typename CorrespondingQuantity<T>::Rep;

template <typename T>
struct HasCorrespondingQuantity
    : stdx::conjunction<stdx::experimental::is_detected<CorrespondingUnit, T>,
                        stdx::experimental::is_detected<CorrespondingRep, T>> {};

template <typename T>
using LooksLikeAuOrOtherQuantity = stdx::disjunction<IsAuType<T>, HasCorrespondingQuantity<T>>;

// We need a way to form an "operation on non-quantity types only".  That is: it's some operation,
// but _if either input is a quantity_, then we _don't even form the type_.
//
// The reason this very specific machinery lives in `rep.hh` is because when we're dealing with
// operations on "types that might be a rep", we know we can exclude quantity types right away.
// (Note that we're using the term "quantity" in an expansive sense, which includes not just
// `au::Quantity`, but also `au::QuantityPoint`, and "quantity-like" types from other libraries
// (which we consider as "anything that has a `CorrespondingQuantity`".
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityImpl;
template <template <class...> class Op, typename... Ts>
using ResultIfNoneAreQuantity = typename ResultIfNoneAreQuantityImpl<Op, Ts...>::type;

// Default implementation where we know that none are quantities.
template <bool AreAnyQuantity, template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityHelper : stdx::type_identity<Op<Ts...>> {};

// Implementation if any of the types are quantities.
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityHelper<true, Op, Ts...> : stdx::type_identity<void> {};

// The main implementation.
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityImpl
    : ResultIfNoneAreQuantityHelper<stdx::disjunction<LooksLikeAuOrOtherQuantity<Ts>...>::value,
                                    Op,
                                    Ts...> {};

// A type whose _scalar_ is itself quantity-like --- say, a vector whose elements are `Quantity`
// --- can never be a valid rep, because using it as one would produce nested units.
template <typename T>
using ScalarOfOrVoid = stdx::experimental::detected_or_t<void, ::au::ScalarOf, T>;

template <typename T>
struct HasQuantityLikeScalar : LooksLikeAuOrOtherQuantity<ScalarOfOrVoid<T>> {};

// The `std::is_empty` is a good way to catch all of the various unit and other monovalue types in
// our library, which have little else in common.  It's also just intrinsically true that it
// wouldn't make much sense to use an empty type as a rep.
template <typename T>
struct IsKnownInvalidRep : stdx::disjunction<std::is_empty<T>,
                                             LooksLikeAuOrOtherQuantity<T>,
                                             std::is_same<void, T>,
                                             HasQuantityLikeScalar<T>> {};

// The type of the product of two types.
template <typename T, typename U>
using ProductType = decltype(std::declval<T>() * std::declval<U>());

template <typename T, typename U>
using ProductTypeOrVoid = stdx::experimental::detected_or_t<void, ProductType, T, U>;

// The type of the quotient of two types.
template <typename T, typename U>
using QuotientType = decltype(std::declval<T>() / std::declval<U>());

template <typename T, typename U>
using QuotientTypeOrVoid = stdx::experimental::detected_or_t<void, QuotientType, T, U>;
}  // namespace detail

// Implementation for `IsValidRep`.
//
// For now, we'll accept anything that isn't explicitly known to be invalid.  We may tighten this up
// later, but this seems like a reasonable starting point.
template <typename T>
struct IsValidRep : stdx::negation<detail::IsKnownInvalidRep<T>> {};

template <typename T, typename U>
struct IsProductValidRep
    : IsValidRep<detail::ResultIfNoneAreQuantity<detail::ProductTypeOrVoid, T, U>> {};

template <typename T, typename U>
struct IsQuotientValidRep
    : IsValidRep<detail::ResultIfNoneAreQuantity<detail::QuotientTypeOrVoid, T, U>> {};

}  // namespace au
