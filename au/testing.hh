// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#pragma once

#include "au/io.hh"
#include "au/stdx/type_traits.hh"
#include "au/unit_of_measure.hh"
#include "gmock/gmock.h"

namespace au {

MATCHER_P(SameType, target, "") {
    return std::is_same<stdx::remove_cvref_t<decltype(arg)>,
                        stdx::remove_cvref_t<decltype(target)>>::value;
}

template <typename T>
auto SameTypeAndValue(T target) {
    return ::testing::AllOf(SameType(target), ::testing::Eq(target));
}

template <typename Q1, typename Q2>
struct AreQuantityTypesEquivalent;

// Matcher to test "Quantity equivalence": i.e., same Rep and Value, and _quantity-equivalent_ Unit.
// Usage example:
//
//   EXPECT_THAT(q, QuantityEquivalent(8.3_m));
MATCHER_P(QuantityEquivalent, target, "") {
    return AreQuantityTypesEquivalent<stdx::remove_cvref_t<decltype(arg)>,
                                      stdx::remove_cvref_t<decltype(target)>>::value &&
           (arg == target);
}

// Matcher to test "Point equivalence": i.e., same Rep and Value, and _point-equivalent_ Unit.
// Usage example:
//
//   EXPECT_THAT(p, PointEquivalent(meters_pt(8.3)));
MATCHER_P(PointEquivalent, target, "") {
    return AreQuantityPointTypesEquivalent<stdx::remove_cvref_t<decltype(arg)>,
                                           stdx::remove_cvref_t<decltype(target)>>::value &&
           (arg == target);
}

template <typename Unit, std::size_t N>
void expect_label(const char (&label)[N]) {
    EXPECT_STREQ(unit_label<Unit>(), label);
    EXPECT_EQ(sizeof(unit_label<Unit>()), N);
}

namespace detail {
// Compute the absolute difference in a specified Unit, with a floating point Rep.
template <typename ResultUnit, typename Q1, typename Q2>
auto absolute_diff(Q1 q1, Q2 q2) {
    auto diff = q2.template as<double>(ResultUnit{}) - q1.template as<double>(ResultUnit{});
    return (diff < ZERO) ? -diff : diff;
}

// Compare the arg to the target, within the tolerance.
//
// This is a separate function to make it easier to write unit tests on the contents of the message.
template <typename ArgT, typename TargetT, typename ToleranceUnit, typename ToleranceRep>
::testing::AssertionResult arg_matches_target_within_tolerance(
    ArgT arg, TargetT target, Quantity<ToleranceUnit, ToleranceRep> tolerance) {
    const auto diff = absolute_diff<ToleranceUnit>(arg, target);
    const bool within_tolerance = (diff <= tolerance);
    return ::testing::AssertionResult{within_tolerance}
           << "whose difference from target " << target << " is " << diff << ", which "
           << (within_tolerance ? "does not exceed" : "exceeds") << " tolerance " << tolerance
           << ".";
}
}  // namespace detail

//
// Custom GMock matcher to match a Quantity to a target within a given tolerance.
//
// Use any combination of units you like, as long as they're all the same dimension!  Absolute
// differences will be printed in the same units as the tolerance, for easy visual comparison.
//
MATCHER_P2(IsNear, target, tolerance, "") {
    const auto assertion_result =
        detail::arg_matches_target_within_tolerance(arg, target, tolerance);
    *result_listener << assertion_result.message();
    return assertion_result;
}

// GMock matcher for values consistently greater than this argument.
//
// By "consistently greater", we mean that we test all six comparisons, and make sure they have the
// results we would expect if the value is greater than this argument.
template <typename T>
auto ConsistentlyGreaterThan(T x) {
    using ::testing::AllOf;
    using ::testing::Eq;
    using ::testing::Ge;
    using ::testing::Gt;
    using ::testing::Le;
    using ::testing::Lt;
    using ::testing::Ne;
    using ::testing::Not;

    return AllOf(Not(Eq(x)), Ge(x), Gt(x), Not(Le(x)), Not(Lt(x)), Ne(x));
}

// GMock matcher for values consistently equal to this argument.
//
// By "consistently equal", we mean that we test all six comparisons, and make sure they have the
// results we would expect if the value were equal to this argument.
template <typename T>
auto ConsistentlyEqualTo(T x) {
    using ::testing::AllOf;
    using ::testing::Eq;
    using ::testing::Ge;
    using ::testing::Gt;
    using ::testing::Le;
    using ::testing::Lt;
    using ::testing::Ne;
    using ::testing::Not;

    return AllOf(Eq(x), Ge(x), Not(Gt(x)), Le(x), Not(Lt(x)), Not(Ne(x)));
}

// GMock matcher for values consistently less than this argument.
//
// By "consistently less", we mean that we test all six comparisons, and make sure they have the
// results we would expect if the value is less than this argument.
template <typename T>
auto ConsistentlyLessThan(T x) {
    using ::testing::AllOf;
    using ::testing::Eq;
    using ::testing::Ge;
    using ::testing::Gt;
    using ::testing::Le;
    using ::testing::Lt;
    using ::testing::Ne;
    using ::testing::Not;

    return AllOf(Not(Eq(x)), Not(Ge(x)), Not(Gt(x)), Le(x), Lt(x), Ne(x));
}

}  // namespace au
