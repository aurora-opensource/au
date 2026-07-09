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

#include "au/compatibility/eigen.hh"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/LU>

#include "au/au.hh"
#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {

struct Meters : UnitImpl<Length> {};
constexpr auto meters = QuantityMaker<Meters>{};

struct Feet : decltype(Meters{} * mag<381>() / mag<1250>()) {};
constexpr auto feet = QuantityMaker<Feet>{};

struct Secs : UnitImpl<Time> {};
constexpr auto secs = QuantityMaker<Secs>{};
constexpr auto sec = SingularNameFor<Secs>{};

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

TEST(EigenCompatibility, CanCreateQuantityOfVector3d) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);

    auto q = meters(v);

    StaticAssertTypeEq<decltype(q), Quantity<Meters, Eigen::Vector3d>>();
}

TEST(EigenCompatibility, OperatorParenReturnsQuantityOfScalar) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);
    auto q = meters(v);

    EXPECT_THAT(q(0), SameTypeAndValue(meters(1.0)));
    EXPECT_THAT(q(1), SameTypeAndValue(meters(2.0)));
    EXPECT_THAT(q(2), SameTypeAndValue(meters(3.0)));
}

TEST(EigenCompatibility, OperatorBracketReturnsQuantityOfScalar) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);
    auto q = meters(v);

    auto elem = q[0];

    StaticAssertTypeEq<decltype(elem), Quantity<Meters, double>>();
    EXPECT_THAT(elem, SameTypeAndValue(meters(1.0)));
}

TEST(EigenCompatibility, MutableViewAllowsElementAssignment) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);
    auto q = meters(v);

    q.mutable_view()(0) = meters(10.0);
    EXPECT_THAT(q(0), SameTypeAndValue(meters(10.0)));

    q.mutable_view()[1] = meters(20.0);
    EXPECT_THAT(q(1), SameTypeAndValue(meters(20.0)));
}

TEST(EigenCompatibility, SameUnitAdditionWorks) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);
    auto q = meters(v);

    auto sum = q + q;

    Eigen::Vector3d expected(2.0, 4.0, 6.0);
    EXPECT_THAT(sum.data_in(meters), Eq(expected));
}

TEST(EigenCompatibility, UnitConversionWorks) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);
    auto q = meters(v);

    auto result = q.in<Eigen::Vector3d>(centi(meters));

    Eigen::Vector3d expected(100.0, 200.0, 300.0);
    EXPECT_THAT(result, Eq(expected));
}

TEST(EigenCompatibility, ExplicitRepConversionForcesEvaluation) {
    Eigen::Vector3d v_m(1.0, 2.0, 3.0);
    auto q_m = meters(v_m);

    // With explicit rep, the conversion forces evaluation to Vector3d
    auto converted = q_m.in<Eigen::Vector3d>(centi(meters));

    Eigen::Vector3d expected(100.0, 200.0, 300.0);
    EXPECT_THAT(converted, Eq(expected));
}

TEST(EigenCompatibility, EvalForcesExpressionEvaluation) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);
    auto q = meters(v);

    // .as() returns a Quantity with expression template rep
    auto q_cm = q.as(centi(meters));

    // eval() forces materialization to concrete type
    auto q_cm_materialized = eval(q_cm);

    StaticAssertTypeEq<decltype(q_cm_materialized), Quantity<Centi<Meters>, Eigen::Vector3d>>();
    EXPECT_THAT(q_cm_materialized.data_in(centi(meters)), Eq(Eigen::Vector3d(100.0, 200.0, 300.0)));
}

TEST(EigenCompatibility, MixedUnitAdditionWithExplicitConversion) {
    Eigen::Vector3d v_m(1.0, 2.0, 3.0);
    Eigen::Vector3d v_cm(100.0, 200.0, 300.0);

    auto q_m = meters(v_m);
    auto q_cm = centi(meters)(v_cm);

    auto sum = q_m.as(centi(meters)) + q_cm;

    Eigen::Vector3d result = sum.data_in(centi(meters));
    Eigen::Vector3d expected(200.0, 400.0, 600.0);
    EXPECT_THAT(result, Eq(expected));
}

TEST(EigenCompatibility, MixedUnitAdditionWithImplicitConversion) {
    auto q_m = meters(Eigen::Vector3d(1.0, 2.0, 3.0));
    auto q_cm = centi(meters)(Eigen::Vector3d(100.0, 200.0, 300.0));

    // `auto` is a danger sign.  This is safe _only_ because `q_m` and `q_cm` are still alive when
    // we evaluate `sum` below.
    auto sum = q_m + q_cm;

    Eigen::Vector3d expected(200.0, 400.0, 600.0);
    EXPECT_THAT(sum.in(centi(meters)), Eq(expected));
}

TEST(EigenCompatibility, SameUnitSubtractionWorks) {
    auto q1 = meters(Eigen::Vector3d{4.0, 5.0, 6.0});
    auto q2 = meters(Eigen::Vector3d{1.0, 2.0, 3.0});

    // `auto` is a danger sign.  This is safe _only_ because `q1` and `q2` are still alive when we
    // evaluate `diff` below.
    auto diff = q1 - q2;

    Eigen::Vector3d expected(3.0, 3.0, 3.0);
    EXPECT_THAT(diff.data_in(meters), Eq(expected));
}

TEST(EigenCompatibility, SameUnitEqualityWorks) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);
    auto q1 = meters(v);
    auto q2 = meters(v);

    EXPECT_THAT(q1 == q2, IsTrue());
}

TEST(EigenCompatibility, DifferentUnitEqualityWorks) {
    auto q_m = meters(Eigen::Vector3d{1.0, 2.0, 3.0});
    auto q_cm = eval(q_m.as(centi(meters)));

    EXPECT_THAT(q_m == q_cm, IsTrue());
}

TEST(EigenCompatibility, EqualityOfDistinctExpressionRepsWorks) {
    // `==` / `!=` between two same-unit quantities whose reps are *different* lazy Eigen expression
    // templates.  Like heterogeneous addition, this used to fail to compile inside
    // `std::common_type` (undefined for two distinct Eigen expressions).  Unlike order comparisons,
    // equality is well-defined for vectors --- Eigen's `operator==` compares all coefficients ---
    // so it's something we should expect to work.  This regression test makes sure that it does.
    const Eigen::Vector3d values{4.0, 5.0, 6.0};
    const auto base = meters(values);

    auto twice = base * 2.0;        // (8, 10, 12) m; rep: one scalar-product node
    auto also_twice = base + base;  // (8, 10, 12) m; rep: one sum node (different type)
    auto thrice = base + twice;     // (12, 15, 18) m; different value *and* rep type
    static_assert(!std::is_same<decltype(twice), decltype(also_twice)>::value, "");
    EXPECT_THAT(twice == also_twice, IsTrue());
    EXPECT_THAT(twice != also_twice, IsFalse());
    EXPECT_THAT(twice == thrice, IsFalse());
    EXPECT_THAT(twice != thrice, IsTrue());
}

TEST(EigenCompatibility, CrossUnitEqualityOfExpressionRepsWorks) {
    // Operands in *different* units, so one is scaled to the common unit (staying a lazy
    // expression) before the coefficient-wise comparison.  Note that both operands here have the
    // *same* expression rep type: this used to route through the common-rep path and try to
    // `static_cast` one Eigen expression into another (a hard error).  Non-arithmetic reps now
    // always take the `ref_or_scaled_copy` path instead.
    const Eigen::Vector3d m_values{8.0, 10.0, 12.0};
    const Eigen::Vector3d cm_values{800.0, 1000.0, 1200.0};
    const auto in_m = meters(m_values);
    const auto in_cm = centi(meters)(cm_values);

    auto lhs = in_m * 2.0;   // (16, 20, 24) m
    auto rhs = in_cm * 2.0;  // (1600, 2000, 2400) cm == (16, 20, 24) m; identical rep type
    StaticAssertTypeEq<decltype(lhs)::Rep, decltype(rhs)::Rep>();

    EXPECT_THAT(lhs == rhs, IsTrue());
    EXPECT_THAT(lhs != rhs, IsFalse());
}

TEST(EigenCompatibility, ScalarMultiplicationWorks) {
    auto q = meters(Eigen::Vector3d{1.0, 2.0, 3.0});

    // `auto` is a danger sign.  This is safe _only_ because `q` is still alive when we evaluate
    // `scaled` below.
    auto scaled = q * 2.0;

    Eigen::Vector3d expected(2.0, 4.0, 6.0);
    EXPECT_THAT(scaled.data_in(meters), Eq(expected));
}

TEST(EigenCompatibility, ScalarDivisionWorks) {
    auto q = meters(Eigen::Vector3d{2.0, 4.0, 6.0});

    // `auto` is a danger sign.  This is safe _only_ because `q` is still alive when we evaluate
    // `scaled` below.
    auto scaled = q / 2.0;

    Eigen::Vector3d expected(1.0, 2.0, 3.0);
    EXPECT_THAT(scaled.data_in(meters), Eq(expected));
}

TEST(EigenCompatibility, UnaryMinusWorks) {
    auto q = meters(Eigen::Vector3d{1.0, 2.0, 3.0});

    // `auto` is a danger sign.  This is safe _only_ because `q` is still alive when we evaluate
    // `negated` below.
    auto negated = -q;

    Eigen::Vector3d expected(-1.0, -2.0, -3.0);
    EXPECT_THAT(negated.data_in(meters), Eq(expected));
}

TEST(EigenCompatibility, CompoundAdditionWorks) {
    Eigen::Vector3d v(1.0, 2.0, 3.0);
    auto q = meters(v);
    auto delta = meters(Eigen::Vector3d(10.0, 20.0, 30.0));

    q += delta;

    Eigen::Vector3d expected(11.0, 22.0, 33.0);
    EXPECT_THAT(q.data_in(meters), Eq(expected));
}

TEST(EigenCompatibility, CompoundScalarMultiplicationWorks) {
    auto q = meters(Eigen::Vector3d{1.0, 2.0, 3.0});

    q *= 2.0;

    Eigen::Vector3d expected(2.0, 4.0, 6.0);
    EXPECT_THAT(q.data_in(meters), Eq(expected));
}

TEST(EigenCompatibility, DifferentUnitAdditionWorks) {
    auto q1 = meters(Eigen::Vector3d{1.0, 2.0, 3.0});
    auto q2 = centi(meters)(Eigen::Vector3d{100.0, 200.0, 300.0});

    auto sum = q1 + q2;

    Eigen::Vector3d expected(200.0, 400.0, 600.0);
    EXPECT_THAT(sum.data_in(centi(meters)), Eq(expected));
}

TEST(EigenCompatibility, SameUnitAdditionOfDistinctExpressionRepsWorks) {
    // Regression test: adding two same-unit quantities whose reps are *different* lazy Eigen
    // expression templates used to fail to compile.  The hidden-friend `operator+(const Quantity&,
    // const Quantity&)` is a candidate, and probing it asks whether one operand converts to the
    // other's type via the implicit constructor -- whose SFINAE guard hard-errored inside
    // `std::common_type`, which is undefined for two distinct Eigen expressions.
    const Eigen::Vector3d v{4.0, 5.0, 6.0};

    // `base` must outlive `term_a`/`term_b`: their lazy reps bind its `value_` by reference. (Using
    // the `meters(v)` temporaries inline would leave the reps dangling after each statement's `;`.)
    const auto base = meters(v);

    // Two `Quantity<Meters, ...>` values with distinct expression-template rep types.
    auto term_a = base * 2.0;        // rep: one scalar-product node
    auto term_b = 0.5 * base * 2.0;  // rep: nested scalar-product nodes (different type)
    ASSERT_THAT((std::is_same<decltype(term_a), decltype(term_b)>::value), IsFalse());

    auto sum = term_a + term_b;

    // The result must still be a lazy expression (not a materialized Matrix), preserving fusion.
    ASSERT_THAT((std::is_same<std::decay_t<decltype(sum.data_in(meters))>, Eigen::Vector3d>::value),
                IsFalse());
    EXPECT_THAT(eval(sum).data_in(meters), Eq(Eigen::Vector3d(12.0, 15.0, 18.0)));
}

TEST(EigenCompatibility, ConstantAccelerationKinematicsExpressionWorks) {
    // The canonical `p0 + v*t + 0.5*a*t*t` expression, with an Eigen-vector rep.  Each product term
    // is a distinct lazy Eigen expression that simplifies to the same unit (Meters), so summing
    // them exercises the same-unit / distinct-expression-rep addition path.
    const auto p0 = meters(Eigen::Vector3d{1.0, 2.0, 3.0});
    const auto v = (meters / sec)(Eigen::Vector3d{4.0, 5.0, 6.0});
    const auto a = (meters / squared(sec))(Eigen::Vector3d{1.0, 1.0, 1.0});
    const auto t = secs(2.0);

    auto trajectory = p0 + v * t + 0.5 * a * t * t;

    EXPECT_THAT(eval(trajectory).data_in(meters), Eq(Eigen::Vector3d(11.0, 14.0, 17.0)));
}

TEST(EigenCompatibility, MixedInputAdditionWithConvertedQuantityWorks) {
    auto q_m = meters(Eigen::Vector3d{1.0, 2.0, 3.0});
    auto q_cm = centi(meters)(Eigen::Vector3d{100.0, 200.0, 300.0});

    // Pass a converted Quantity (with expression template rep) to mixed-input operator.
    //
    // `auto` is a danger sign.  This is safe _only_ because `q_m` and `q_cm` are still alive when
    // we evaluate `sum` below.
    auto sum = q_m.as(centi(meters)) + q_cm;

    Eigen::Vector3d expected(200.0, 400.0, 600.0);
    EXPECT_THAT(sum.data_in(centi(meters)), Eq(expected));
}

TEST(EigenCompatibility, IntegralMultiStepConversionWorks) {
    // Meters to feet requires multiply then divide (irrational ratio).
    // This tests whether intermediate expression templates in OpSequence cause dangling.
    auto q_m = meters(Eigen::Vector3i{1000, 2000, 3000});

    auto result = q_m.in(feet, ignore(TRUNCATION_RISK)).eval();

    // 1000 m * 1250 / 381 = 3280 ft (truncated)
    // 2000 m * 1250 / 381 = 6561 ft (truncated)
    // 3000 m * 1250 / 381 = 9842 ft (truncated)
    Eigen::Vector3i expected(3280, 6561, 9842);
    EXPECT_THAT(result, Eq(expected));
}

TEST(EigenCompatibility, IntegralMultiStepConversionWithImplicitRepWorks) {
    // Same as above but using implicit rep .in() to see if expression templates dangle
    auto q_m = meters(Eigen::Vector3i{1000, 2000, 3000});

    // Use implicit rep with risk acknowledgement - returns expression template if not materialized
    auto result = q_m.in(feet, ignore(TRUNCATION_RISK));

    Eigen::Vector3i expected(3280, 6561, 9842);
    EXPECT_THAT(result, Eq(expected));
}

__attribute__((noinline)) auto do_conversion_return_expr(
    const Quantity<Meters, Eigen::Vector3i> &q) {
    // Return expression template from function - intermediate temps should die
    return q.in(feet, ignore(TRUNCATION_RISK));
}

TEST(EigenCompatibility, IntegralMultiStepConversionAcrossFunctionBoundary) {
    Eigen::Vector3i v_m(1000, 2000, 3000);
    auto q_m = meters(v_m);

    // Get the expression template, then evaluate it after function returns
    auto expr = do_conversion_return_expr(q_m);
    // At this point, any intermediates created inside in_impl are dead

    Eigen::Vector3i result = expr;

    Eigen::Vector3i expected(3280, 6561, 9842);
    EXPECT_THAT(result, Eq(expected));
}

//
// Free-function forms of Eigen member functions.
//

TEST(EigenFreeFunctions, NormIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(3.0, 4.0, 0.0));

    EXPECT_THAT(norm(q), SameTypeAndValue(meters(5.0)));
}

TEST(EigenFreeFunctions, SquaredNormSquaresTheUnit) {
    auto q = meters(Eigen::Vector3d(3.0, 4.0, 0.0));

    auto sq = squaredNorm(q);

    StaticAssertTypeEq<decltype(sq), Quantity<UnitPowerT<Meters, 2>, double>>();
    EXPECT_THAT(sq.in(meters * meters), Eq(25.0));
}

TEST(EigenFreeFunctions, SumIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    EXPECT_THAT(sum(q), SameTypeAndValue(meters(6.0)));
}

TEST(EigenFreeFunctions, MeanIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    EXPECT_THAT(mean(q), SameTypeAndValue(meters(2.0)));
}

TEST(EigenFreeFunctions, MinCoeffIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(3.0, 1.0, 2.0));

    EXPECT_THAT(minCoeff(q), SameTypeAndValue(meters(1.0)));
}

TEST(EigenFreeFunctions, MaxCoeffIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(3.0, 1.0, 2.0));

    EXPECT_THAT(maxCoeff(q), SameTypeAndValue(meters(3.0)));
}

TEST(EigenFreeFunctions, TransposeIsUnitPreserving) {
    Eigen::Matrix<double, 2, 3> m;
    m << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0;
    auto q = meters(m);

    // `transpose` is lazy; `q` stays alive through the evaluation below.
    auto qt = transpose(q);

    Eigen::Matrix<double, 3, 2> expected = m.transpose();
    EXPECT_THAT(eval(qt).data_in(meters), Eq(expected));
}

TEST(EigenFreeFunctions, DotProductMultipliesUnits) {
    auto a = meters(Eigen::Vector3d(1.0, 2.0, 3.0));
    auto b = feet(Eigen::Vector3d(1.0, 1.0, 1.0));

    auto d = dot(a, b);

    StaticAssertTypeEq<decltype(d), Quantity<UnitProductT<Meters, Feet>, double>>();
    EXPECT_THAT(d.in(meters * feet), Eq(6.0));
}

TEST(EigenFreeFunctions, DotProductWithRawEigenVectorKeepsQuantityUnit) {
    auto a = meters(Eigen::Vector3d(1.0, 2.0, 3.0));
    Eigen::Vector3d raw(4.0, 5.0, 6.0);

    auto d = dot(a, raw);

    StaticAssertTypeEq<decltype(d), Quantity<Meters, double>>();
    EXPECT_THAT(d, SameTypeAndValue(meters(32.0)));
}

TEST(EigenFreeFunctions, DotProductWithRawEigenVectorFirstKeepsQuantityUnit) {
    Eigen::Vector3d raw(4.0, 5.0, 6.0);
    auto b = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    auto d = dot(raw, b);

    StaticAssertTypeEq<decltype(d), Quantity<Meters, double>>();
    EXPECT_THAT(d, SameTypeAndValue(meters(32.0)));
}

TEST(EigenFreeFunctions, CrossProductMultipliesUnits) {
    auto a = meters(Eigen::Vector3d(1.0, 0.0, 0.0));
    auto b = feet(Eigen::Vector3d(0.0, 1.0, 0.0));

    // `cross` is lazy; `a` and `b` stay alive through the evaluation below.
    auto c = cross(a, b);

    Eigen::Vector3d expected(0.0, 0.0, 1.0);
    EXPECT_THAT(eval(c).data_in(meters * feet), Eq(expected));
}

TEST(EigenFreeFunctions, CrossProductWithRawEigenVectorKeepsQuantityUnit) {
    auto a = meters(Eigen::Vector3d(1.0, 0.0, 0.0));
    Eigen::Vector3d raw(0.0, 1.0, 0.0);

    // `cross` is lazy; `a` and `raw` stay alive through the evaluation below.
    auto c = cross(a, raw);

    StaticAssertTypeEq<decltype(eval(c)), Quantity<Meters, Eigen::Vector3d>>();
    Eigen::Vector3d expected(0.0, 0.0, 1.0);
    EXPECT_THAT(eval(c).data_in(meters), Eq(expected));
}

TEST(EigenFreeFunctions, CrossProductWithRawEigenVectorFirstKeepsQuantityUnit) {
    Eigen::Vector3d raw(1.0, 0.0, 0.0);
    auto b = meters(Eigen::Vector3d(0.0, 1.0, 0.0));

    // `cross` is lazy; `raw` and `b` stay alive through the evaluation below.
    auto c = cross(raw, b);

    StaticAssertTypeEq<decltype(eval(c)), Quantity<Meters, Eigen::Vector3d>>();
    Eigen::Vector3d expected(0.0, 0.0, 1.0);
    EXPECT_THAT(eval(c).data_in(meters), Eq(expected));
}

TEST(EigenFreeFunctions, NormalizedReturnsRawVector) {
    auto q = meters(Eigen::Vector3d(3.0, 4.0, 0.0));

    auto n = normalized(q);

    // The result is unitless by definition, so we return the raw Eigen vector, not a Quantity.
    StaticAssertTypeEq<decltype(n), Eigen::Vector3d>();

    Eigen::Vector3d expected(0.6, 0.8, 0.0);
    EXPECT_THAT(n, Eq(expected));
}

//
// Additional scalar reductions.
//

TEST(EigenFreeFunctions, StableNormIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(3.0, 4.0, 0.0));

    EXPECT_THAT(stableNorm(q), SameTypeAndValue(meters(5.0)));
}

TEST(EigenFreeFunctions, BlueNormIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(3.0, 4.0, 0.0));

    EXPECT_THAT(blueNorm(q), SameTypeAndValue(meters(5.0)));
}

TEST(EigenFreeFunctions, HypotNormIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(3.0, 4.0, 0.0));

    EXPECT_THAT(hypotNorm(q), SameTypeAndValue(meters(5.0)));
}

TEST(EigenFreeFunctions, LpNormSupportsVariousOrders) {
    auto q = meters(Eigen::Vector3d(3.0, -4.0, 0.0));

    EXPECT_THAT(lpNorm<1>(q), SameTypeAndValue(meters(7.0)));
    EXPECT_THAT(lpNorm<2>(q), SameTypeAndValue(meters(5.0)));
    EXPECT_THAT(lpNorm<Eigen::Infinity>(q), SameTypeAndValue(meters(4.0)));
}

TEST(EigenFreeFunctions, TraceIsUnitPreserving) {
    Eigen::Matrix3d m = Eigen::Vector3d(1.0, 2.0, 3.0).asDiagonal();
    auto q = meters(m);

    EXPECT_THAT(trace(q), SameTypeAndValue(meters(6.0)));
}

//
// Coefficient-wise ops.
//

TEST(EigenFreeFunctions, CwiseProductMultipliesUnits) {
    auto a = meters(Eigen::Vector3d(1.0, 2.0, 3.0));
    auto b = feet(Eigen::Vector3d(2.0, 2.0, 2.0));

    auto c = cwiseProduct(a, b);

    StaticAssertTypeEq<decltype(eval(c)), Quantity<UnitProductT<Meters, Feet>, Eigen::Vector3d>>();
    EXPECT_THAT(eval(c).data_in(UnitProductT<Meters, Feet>{}), Eq(Eigen::Vector3d(2.0, 4.0, 6.0)));
}

TEST(EigenFreeFunctions, CwiseProductWithRawEigenObjectKeepsQuantityUnit) {
    auto a = meters(Eigen::Vector3d(1.0, 2.0, 3.0));
    Eigen::Vector3d raw(2.0, 2.0, 2.0);

    EXPECT_THAT(eval(cwiseProduct(a, raw)).data_in(meters), Eq(Eigen::Vector3d(2.0, 4.0, 6.0)));
    EXPECT_THAT(eval(cwiseProduct(raw, a)).data_in(meters), Eq(Eigen::Vector3d(2.0, 4.0, 6.0)));
}

TEST(EigenFreeFunctions, CwiseQuotientDividesUnits) {
    auto a = meters(Eigen::Vector3d(2.0, 4.0, 6.0));
    auto b = feet(Eigen::Vector3d(1.0, 2.0, 3.0));

    auto c = cwiseQuotient(a, b);

    StaticAssertTypeEq<decltype(eval(c)), Quantity<UnitQuotientT<Meters, Feet>, Eigen::Vector3d>>();
    EXPECT_THAT(eval(c).data_in(UnitQuotientT<Meters, Feet>{}), Eq(Eigen::Vector3d(2.0, 2.0, 2.0)));
}

TEST(EigenFreeFunctions, CwiseQuotientWithRawEigenObject) {
    auto a = meters(Eigen::Vector3d(2.0, 4.0, 6.0));
    Eigen::Vector3d raw(1.0, 2.0, 3.0);

    // Quantity / raw -> keeps the unit.
    auto num = cwiseQuotient(a, raw);
    StaticAssertTypeEq<decltype(eval(num)), Quantity<Meters, Eigen::Vector3d>>();
    EXPECT_THAT(eval(num).data_in(meters), Eq(Eigen::Vector3d(2.0, 2.0, 2.0)));

    // raw / Quantity -> inverts the unit.
    auto den = cwiseQuotient(raw, a);
    StaticAssertTypeEq<decltype(eval(den)), Quantity<UnitInverseT<Meters>, Eigen::Vector3d>>();
    EXPECT_THAT(eval(den).data_in(UnitInverseT<Meters>{}), Eq(Eigen::Vector3d(0.5, 0.5, 0.5)));
}

TEST(EigenFreeFunctions, CwiseAbsIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(-1.0, 2.0, -3.0));

    EXPECT_THAT(eval(cwiseAbs(q)).data_in(meters), Eq(Eigen::Vector3d(1.0, 2.0, 3.0)));
}

//
// View / accessor ops.
//

TEST(EigenFreeFunctions, DiagonalIsUnitPreserving) {
    Eigen::Matrix3d m = Eigen::Vector3d(1.0, 2.0, 3.0).asDiagonal();
    auto q = meters(m);

    EXPECT_THAT(eval(diagonal(q)).data_in(meters), Eq(Eigen::Vector3d(1.0, 2.0, 3.0)));
}

TEST(EigenFreeFunctions, RowAndColAreUnitPreserving) {
    Eigen::Matrix3d m = Eigen::Vector3d(1.0, 2.0, 3.0).asDiagonal();
    auto q = meters(m);

    EXPECT_THAT(eval(row(q, 0)).data_in(meters), Eq(Eigen::RowVector3d(1.0, 0.0, 0.0)));
    EXPECT_THAT(eval(col(q, 1)).data_in(meters), Eq(Eigen::Vector3d(0.0, 2.0, 0.0)));
}

TEST(EigenFreeFunctions, ReverseIsUnitPreserving) {
    auto q = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    EXPECT_THAT(eval(reverse(q)).data_in(meters), Eq(Eigen::Vector3d(3.0, 2.0, 1.0)));
}

TEST(EigenFreeFunctions, ConjugateIsUnitPreservingForRealReps) {
    auto q = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    EXPECT_THAT(eval(conjugate(q)).data_in(meters), Eq(Eigen::Vector3d(1.0, 2.0, 3.0)));
}

TEST(EigenFreeFunctions, HeadFixedAndDynamic) {
    auto q = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    EXPECT_THAT(eval(head<2>(q)).data_in(meters), Eq(Eigen::Vector2d(1.0, 2.0)));
    EXPECT_THAT(eval(head(q, 2)).data_in(meters), Eq(Eigen::Vector2d(1.0, 2.0)));
}

TEST(EigenFreeFunctions, TailFixedAndDynamic) {
    auto q = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    EXPECT_THAT(eval(tail<2>(q)).data_in(meters), Eq(Eigen::Vector2d(2.0, 3.0)));
    EXPECT_THAT(eval(tail(q, 2)).data_in(meters), Eq(Eigen::Vector2d(2.0, 3.0)));
}

TEST(EigenFreeFunctions, SegmentFixedAndDynamic) {
    auto q = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    EXPECT_THAT(eval(segment<2>(q, 1)).data_in(meters), Eq(Eigen::Vector2d(2.0, 3.0)));
    EXPECT_THAT(eval(segment(q, 1, 2)).data_in(meters), Eq(Eigen::Vector2d(2.0, 3.0)));
}

TEST(EigenFreeFunctions, BlockFixedAndDynamic) {
    Eigen::Matrix3d m = Eigen::Vector3d(1.0, 2.0, 3.0).asDiagonal();
    auto q = meters(m);

    Eigen::Matrix2d expected;
    expected << 1.0, 0.0, 0.0, 2.0;

    EXPECT_THAT((eval(block<2, 2>(q, 0, 0)).data_in(meters)), Eq(expected));
    EXPECT_THAT(eval(block(q, 0, 0, 2, 2)).data_in(meters), Eq(expected));
}

TEST(EigenFreeFunctions, ReplicateFixedAndDynamic) {
    auto q = meters(Eigen::Vector3d(1.0, 2.0, 3.0));

    Eigen::Matrix<double, 6, 1> expected;
    expected << 1.0, 2.0, 3.0, 1.0, 2.0, 3.0;

    EXPECT_THAT((eval(replicate<2, 1>(q)).data_in(meters)), Eq(expected));
    EXPECT_THAT(eval(replicate(q, 2, 1)).data_in(meters), Eq(expected));
}

//
// Ops whose result unit is a non-trivial power of the operand unit.
//

TEST(EigenFreeFunctions, CwiseSqrtTakesSquareRootOfUnit) {
    auto q = meters(Eigen::Vector3d(4.0, 9.0, 16.0));

    auto r = cwiseSqrt(q);

    StaticAssertTypeEq<decltype(eval(r)), Quantity<UnitPowerT<Meters, 1, 2>, Eigen::Vector3d>>();
    EXPECT_THAT(eval(r).data_in(UnitPowerT<Meters, 1, 2>{}), Eq(Eigen::Vector3d(2.0, 3.0, 4.0)));
}

TEST(EigenFreeFunctions, InverseInvertsTheUnit) {
    Eigen::Matrix2d m = Eigen::Vector2d(2.0, 4.0).asDiagonal();
    auto q = meters(m);

    auto r = inverse(q);

    StaticAssertTypeEq<decltype(eval(r)), Quantity<UnitInverseT<Meters>, Eigen::Matrix2d>>();

    Eigen::Matrix2d expected = Eigen::Vector2d(0.5, 0.25).asDiagonal();
    EXPECT_THAT(eval(r).data_in(UnitInverseT<Meters>{}), Eq(expected));
}

TEST(EigenFreeFunctions, ProdRaisesUnitToCoefficientCount) {
    auto q = meters(Eigen::Vector3d(2.0, 3.0, 4.0));

    auto p = prod(q);

    StaticAssertTypeEq<decltype(p), Quantity<UnitPowerT<Meters, 3>, double>>();
    EXPECT_THAT(p.data_in(UnitPowerT<Meters, 3>{}), Eq(24.0));
}

TEST(EigenFreeFunctions, DeterminantRaisesUnitToMatrixDimension) {
    Eigen::Matrix3d m = Eigen::Vector3d(2.0, 3.0, 4.0).asDiagonal();
    auto q = meters(m);

    auto d = determinant(q);

    StaticAssertTypeEq<decltype(d), Quantity<UnitPowerT<Meters, 3>, double>>();
    EXPECT_THAT(d.data_in(UnitPowerT<Meters, 3>{}), Eq(24.0));
}

}  // namespace au
