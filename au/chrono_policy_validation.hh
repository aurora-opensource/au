// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#pragma once

#include <chrono>
#include <utility>

#include "au/dimension.hh"
#include "au/quantity.hh"
#include "au/stdx/experimental/is_detected.hh"
#include "au/stdx/type_traits.hh"
#include "au/unit_of_measure.hh"
#include "gtest/gtest.h"

// The utilities in this file exist to make it easy to see whether we follow the same policy as
// `std::chrono::duration` for certain binary operations, whenever our types are "analogous" to
// `duration` types.  By "analogous", we mean that by _default_, we should handle an operation with
// `Quantity<U1, R1>` and `Quantity<U2, R2>` the same way we would handle
// `duration<R1, unit_ratio(U1{}, base)>` and `duration<R2, unit_ratio(U2{}, base)>`, for some unit
// instance `base` of the same dimension as `U1` and `U2, whenever these unit ratios are purely
// rational numbers.  (`std::chrono::duration` does not support irrational scale factors.)
//
// There are some cases where we intentionally diverge from the policy which the chrono library
// follows.  These cases are typically refinements of the policy to prevent pitfalls that stem from
// different usage patterns (e.g., our freer embrace of 32-bit and smaller integer types).  The
// point of these tests is that when we _do_ deviate from the chrono policy, we should do so
// _intentionally_.
//
// The operations we designed these tools to test are construction/assignment, addition,
// subtraction, and comparison operators.

namespace au {

struct SomeUnit : UnitImpl<Length> {};  // Arbitrary.
static constexpr auto some_units = QuantityMaker<SomeUnit>{};

// ChronoToAuMapper: map types from a `chrono`-based computation to their equivalents in an
// `au`-based computation, where some specific but arbitrary base unit takes the place of "seconds"
// in the `chrono` library.
//
// This is the default implementation, which is the identity mapping for non-`duration` types.
template <typename T>
struct ChronoToAuMapper {
    static constexpr auto convert(T x) { return x; }
};

// ChronoToAuMapper specialization for `duration`: create the corresponding Quantity of SomeUnit.
template <typename Rep, typename Period>
struct ChronoToAuMapper<std::chrono::duration<Rep, Period>> {
    static constexpr auto convert(std::chrono::duration<Rep, Period> t) {
        return (some_units * mag<Period::num>() / mag<Period::den>())(t.count());
    }
};

template <typename T>
constexpr auto map_to_au(T x) {
    return ChronoToAuMapper<T>::convert(x);
}

template <typename T>
using MappedToAuT = decltype(map_to_au(std::declval<T>()));

////////////////////////////////////////////////////////////////////////////////////////////////////
// Testing for equivalence of result types.
//
// Result types are "equivalent" for Quantity types if the Units are equivalent (i.e., same
// Dimension and Magnitude).  We don't want to fail for equivalent Units which happen to be
// different C++ types, such as `Milli<SomeUnit>{}` and `SomeUnit{} / mag<1000>()`!
//
// For any other types, "equivalent" means "same type".

template <typename T, typename U>
struct EquivalentResultTypesImpl : std::is_same<T, U> {};

template <typename U1, typename U2, typename R1, typename R2>
struct EquivalentResultTypesImpl<Quantity<U1, R1>, Quantity<U2, R2>>
    : AreQuantityTypesEquivalent<Quantity<U1, R1>, Quantity<U2, R2>> {};

template <typename T, typename U>
struct EquivalentResultTypes
    : EquivalentResultTypesImpl<stdx::remove_cvref_t<T>, stdx::remove_cvref_t<U>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic policy-matching utilities.

// The return type of a binary operator Op, acting on inputs of types T and U.
//
// (In these policy tests, operators act via a static member function template `op(T, U)`.)
template <typename Op, typename T, typename U>
using OpReturn = decltype(Op{}(std::declval<T>(), std::declval<U>()));

// Use the Detector Pattern to check whether Op{}(T, U) is valid.
template <typename Op, typename T, typename U>
using HasOp = stdx::experimental::is_detected<OpReturn, Op, T, U>;

// Specialization of policy matching for when the operation IS permitted by the `chrono` library.
template <typename Op, typename DurationA, typename DurationB>
::testing::AssertionResult both_permit(DurationA a, DurationB b) {
    static_assert(HasOp<Op, DurationA, DurationB>::value, "Expected chrono lib to permit this op");
    static_assert(HasOp<Op, MappedToAuT<DurationA>, MappedToAuT<DurationB>>::value,
                  "AU library FORBIDS an operation which the CHRONO library PERMITS");

    constexpr Op op{};
    const auto expected = map_to_au(op(a, b));
    const auto actual = op(map_to_au(a), map_to_au(b));

    if (!EquivalentResultTypes<decltype(expected), decltype(actual)>::value) {
        return ::testing::AssertionFailure() << "Result did not have expected type";
    }

    return ::testing::AssertionResult{expected == actual};
}

// Overload for `both_permit<...>(...)` which lets you supply the expected type/value.
template <typename Op, typename DurationA, typename DurationB, typename T>
::testing::AssertionResult both_permit(DurationA a, DurationB b, T expected) {
    if (!EquivalentResultTypes<T, OpReturn<Op, DurationA, DurationB>>::value) {
        return ::testing::AssertionFailure()
               << "Expected value had different type than actual result";
    }

    if (Op{}(a, b) != expected) {
        return ::testing::AssertionFailure() << "Did not obtain expected value";
    }

    return both_permit<Op>(a, b);
}

// Specialization of policy matching for when the operation ISN'T permitted by the `chrono` library.
template <typename Op, typename DurationA, typename DurationB>
bool both_forbid(DurationA, DurationB) {
    static_assert(!HasOp<Op, DurationA, DurationB>::value, "Expected chrono lib to forbid this op");
    static_assert(!HasOp<Op, MappedToAuT<DurationA>, MappedToAuT<DurationB>>::value,
                  "AU library PERMITS an operation which the CHRONO library FORBIDS");

    return true;
}

// Specialization of policy matching for when the operation IS permitted by the `chrono` library,
// but is NOT permitted by the `au` library.
template <typename Op, typename DurationA, typename DurationB>
bool chrono_permits_but_au_forbids(DurationA, DurationB) {
    static_assert(HasOp<Op, DurationA, DurationB>::value, "Expected chrono lib to permit this op");
    static_assert(
        !HasOp<Op, MappedToAuT<DurationA>, MappedToAuT<DurationB>>::value,
        "AU library PERMITS an operation which the CHRONO library PERMITS, but AU should FORBID");

    return true;
}

}  // namespace au
