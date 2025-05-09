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

#include <type_traits>

#include "au/fwd.hh"
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

namespace auimpl {
template <typename T>
struct IsAuType : std::false_type {};

template <typename U, typename R>
struct IsAuType<::au::Quantity<U, R>> : std::true_type {};

template <typename U, typename R>
struct IsAuType<::au::QuantityPoint<U, R>> : std::true_type {};

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
struct ResultIfNoneAreQuantity;
template <template <class...> class Op, typename... Ts>
using ResultIfNoneAreQuantityT = typename ResultIfNoneAreQuantity<Op, Ts...>::type;

// Default implementation where we know that none are quantities.
template <bool AreAnyQuantity, template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityImpl : stdx::type_identity<Op<Ts...>> {};

// Implementation if any of the types are quantities.
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityImpl<true, Op, Ts...> : stdx::type_identity<void> {};

// The main implementation.
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantity
    : ResultIfNoneAreQuantityImpl<stdx::disjunction<LooksLikeAuOrOtherQuantity<Ts>...>::value,
                                  Op,
                                  Ts...> {};

// The `std::is_empty` is a good way to catch all of the various unit and other monovalue types in
// our library, which have little else in common.  It's also just intrinsically true that it
// wouldn't make much sense to use an empty type as a rep.
template <typename T>
struct IsKnownInvalidRep
    : stdx::disjunction<std::is_empty<T>, LooksLikeAuOrOtherQuantity<T>, std::is_same<void, T>> {};

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
}  // namespace auimpl

// Implementation for `IsValidRep`.
//
// For now, we'll accept anything that isn't explicitly known to be invalid.  We may tighten this up
// later, but this seems like a reasonable starting point.
template <typename T>
struct IsValidRep : stdx::negation<auimpl::IsKnownInvalidRep<T>> {};

template <typename T, typename U>
struct IsProductValidRep
    : IsValidRep<auimpl::ResultIfNoneAreQuantityT<auimpl::ProductTypeOrVoid, T, U>> {};

template <typename T, typename U>
struct IsQuotientValidRep
    : IsValidRep<auimpl::ResultIfNoneAreQuantityT<auimpl::QuotientTypeOrVoid, T, U>> {};

}  // namespace au
