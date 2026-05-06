// Copyright 2026 Aurora Operations, Inc.
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

// Comprehensive CUDA compilation test for Au library.
// This file exercises all public APIs to ensure AU_DEVICE_FUNC is applied everywhere needed.
//
// NOTE: This test is designed to FAIL TO COMPILE if AU_DEVICE_FUNC is missing from any
// function that users would call from device code. Each section tests a specific header.

#include "au/au.hh"
#include "au/constants/speed_of_light.hh"
#include "au/units/hertz.hh"
#include "au/units/meters.hh"
#include "au/units/radians.hh"
#include "au/units/seconds.hh"

using namespace au;

// =============================================================================
// SECTION 1: au/zero.hh - Zero type
// =============================================================================
// =============================================================================

__device__ bool test_zero_arithmetic() {
    auto sum = ZERO + ZERO;
    auto diff = ZERO - ZERO;
    return (sum == ZERO) && (diff == ZERO);
}

__device__ bool test_zero_comparisons() {
    return (ZERO == ZERO) && !(ZERO != ZERO) && !(ZERO < ZERO) && (ZERO <= ZERO) &&
           !(ZERO > ZERO) && (ZERO >= ZERO);
}

__device__ int test_zero_conversions() {
    int i = ZERO;
    double d = ZERO;
    float f = ZERO;
    return static_cast<int>(i + d + f);
}

// =============================================================================
// SECTION 2: au/quantity.hh - Quantity type
// =============================================================================

__device__ double test_quantity_make_quantity() {
    auto q = make_quantity<Meters>(5.0);
    return q.in(meters);
}

__device__ double test_quantity_construction_from_zero() {
    Quantity<Meters, double> q = ZERO;
    return q.in(meters);
}

__device__ double test_quantity_construction_implicit() {
    QuantityD<Meters> q1 = meters(5.0);
    QuantityD<Centi<Meters>> q2 = q1;  // Implicit conversion (widening)
    return q2.in(centi(meters));
}

__device__ double test_quantity_as_same_unit() {
    auto q = meters(5.0);
    return q.as<float>(meters).in(meters);
}

__device__ double test_quantity_as_different_unit() {
    auto q = meters(5.0);
    return q.as(centi(meters)).in(centi(meters));
}

__device__ double test_quantity_as_rep_and_unit() {
    auto q = meters(5.0);
    return q.as<float>(centi(meters)).in(centi(meters));
}

__device__ double test_quantity_in_same_unit() {
    auto q = meters(5.0);
    return q.in(meters);
}

__device__ double test_quantity_in_different_unit() {
    auto q = meters(5.0);
    return q.in(centi(meters));
}

__device__ double test_quantity_in_rep_and_unit() {
    auto q = meters(5.0);
    return q.in<float>(centi(meters));
}

// NOTE: coerce_as and coerce_in are deliberately not supported on device code because they are
// deprecated and will be removed in a future release.

__device__ double test_quantity_data_in() {
    auto q = meters(5.0);
    return q.data_in(meters);
}

__device__ void test_quantity_data_in_mutable() {
    auto q = meters(5.0);
    q.data_in(meters) = 10.0;
}

__device__ bool test_quantity_comparison_same_type() {
    auto a = meters(5.0);
    auto b = meters(3.0);
    return (a == a) && (a != b) && (b < a) && (b <= a) && (a > b) && (a >= b);
}

__device__ bool test_quantity_comparison_different_types() {
    auto a = meters(1.0);
    auto b = centi(meters)(100.0);
    return (a == b);
}

__device__ bool test_quantity_comparison_with_zero() {
    auto q = meters(0.0);
    return (q == ZERO) && !(q != ZERO) && !(q < ZERO) && (q <= ZERO) && !(q > ZERO) && (q >= ZERO);
}

__device__ double test_quantity_addition_same_type() {
    auto a = meters(5.0);
    auto b = meters(3.0);
    return (a + b).in(meters);
}

__device__ double test_quantity_addition_different_types() {
    auto a = meters(1.0);
    auto b = centi(meters)(50.0);
    return (a + b).in(centi(meters));
}

__device__ double test_quantity_subtraction_same_type() {
    auto a = meters(5.0);
    auto b = meters(3.0);
    return (a - b).in(meters);
}

__device__ double test_quantity_subtraction_different_types() {
    auto a = meters(1.0);
    auto b = centi(meters)(50.0);
    return (a - b).in(centi(meters));
}

__device__ double test_quantity_scalar_multiplication() {
    auto q = meters(5.0);
    return (q * 2.0).in(meters);
}

__device__ double test_quantity_scalar_multiplication_reverse() {
    auto q = meters(5.0);
    return (2.0 * q).in(meters);
}

__device__ double test_quantity_scalar_division() {
    auto q = meters(10.0);
    return (q / 2.0).in(meters);
}

__device__ double test_quantity_quantity_multiplication() {
    auto d = meters(5.0);
    auto t = seconds(2.0);
    auto result = d * t;
    return result.in(meters * seconds);
}

__device__ double test_quantity_quantity_division() {
    auto d = meters(10.0);
    auto t = seconds(2.0);
    auto v = d / t;
    return v.in(meters / second);
}

__device__ double test_quantity_compound_assignment_add() {
    auto q = meters(5.0);
    q += meters(3.0);
    return q.in(meters);
}

__device__ double test_quantity_compound_assignment_sub() {
    auto q = meters(5.0);
    q -= meters(3.0);
    return q.in(meters);
}

__device__ double test_quantity_compound_assignment_mul() {
    auto q = meters(5.0);
    q *= 2.0;
    return q.in(meters);
}

__device__ double test_quantity_compound_assignment_div() {
    auto q = meters(10.0);
    q /= 2.0;
    return q.in(meters);
}

__device__ double test_quantity_unary_plus() {
    auto q = meters(5.0);
    return (+q).in(meters);
}

__device__ double test_quantity_unary_minus() {
    auto q = meters(5.0);
    return (-q).in(meters);
}

__device__ int test_quantity_modulo() {
    auto a = meters(10);
    auto b = meters(3);
    return (a % b).in(meters);
}

__device__ double test_quantity_min() {
    auto a = meters(5.0);
    auto b = meters(3.0);
    return min(a, b).in(meters);
}

__device__ double test_quantity_max() {
    auto a = meters(5.0);
    auto b = meters(3.0);
    return max(a, b).in(meters);
}

__device__ double test_quantity_clamp() {
    auto v = meters(10.0);
    auto lo = meters(2.0);
    auto hi = meters(8.0);
    return clamp(v, lo, hi).in(meters);
}

__device__ double test_quantity_maker_operations() {
    auto velocity_maker = meters / second;
    auto v = velocity_maker(5.0);
    return v.in(meters / second);
}

__device__ double test_quantity_maker_product() {
    auto area_maker = meters * meters;
    auto a = area_maker(25.0);
    return a.in(meters * meters);
}

__device__ double test_as_raw_number() {
    auto q = make_quantity<UnitProduct<>>(5.0);  // dimensionless
    return as_raw_number(q);
}

__device__ double test_rep_cast() {
    auto q = meters(5.5);
    auto q_int = rep_cast<int>(q);
    return static_cast<double>(q_int.in(meters));
}

__device__ double test_unblock_int_div() {
    auto d = meters(10);
    auto t = seconds(3);
    return (d / unblock_int_div(t)).in(meters / second);
}

// =============================================================================
// SECTION 3: au/quantity_point.hh - QuantityPoint type
// =============================================================================

__device__ double test_quantity_point_construction() {
    auto p = meters_pt(5.0);
    return p.in(meters_pt);
}

__device__ double test_quantity_point_from_zero() {
    QuantityPointD<Meters> p = meters_pt(0.0);
    return p.in(meters_pt);
}

__device__ double test_quantity_point_as() {
    auto p = meters_pt(5.0);
    return p.as(centi(meters_pt)).in(centi(meters_pt));
}

__device__ double test_quantity_point_as_rep() {
    auto p = meters_pt(5.0);
    return p.as<float>(centi(meters_pt)).in(centi(meters_pt));
}

__device__ double test_quantity_point_in() {
    auto p = meters_pt(5.0);
    return p.in(centi(meters_pt));
}

__device__ double test_quantity_point_in_rep() {
    auto p = meters_pt(5.0);
    return p.in<float>(centi(meters_pt));
}

__device__ double test_quantity_point_data_in() {
    auto p = meters_pt(5.0);
    return p.data_in(meters_pt);
}

__device__ bool test_quantity_point_comparison() {
    auto a = meters_pt(5.0);
    auto b = meters_pt(3.0);
    return (a == a) && (a != b) && (b < a) && (b <= a) && (a > b) && (a >= b);
}

__device__ double test_quantity_point_difference() {
    auto a = meters_pt(5.0);
    auto b = meters_pt(3.0);
    return (a - b).in(meters);
}

__device__ double test_quantity_point_add_quantity() {
    auto p = meters_pt(5.0);
    auto q = meters(3.0);
    return (p + q).in(meters_pt);
}

__device__ double test_quantity_point_sub_quantity() {
    auto p = meters_pt(5.0);
    auto q = meters(3.0);
    return (p - q).in(meters_pt);
}

__device__ double test_quantity_point_compound_add() {
    auto p = meters_pt(5.0);
    p += meters(3.0);
    return p.in(meters_pt);
}

__device__ double test_quantity_point_compound_sub() {
    auto p = meters_pt(5.0);
    p -= meters(3.0);
    return p.in(meters_pt);
}

__device__ double test_quantity_point_min() {
    auto a = meters_pt(5.0);
    auto b = meters_pt(3.0);
    return min(a, b).in(meters_pt);
}

__device__ double test_quantity_point_max() {
    auto a = meters_pt(5.0);
    auto b = meters_pt(3.0);
    return max(a, b).in(meters_pt);
}

__device__ double test_quantity_point_clamp() {
    auto v = meters_pt(10.0);
    auto lo = meters_pt(2.0);
    auto hi = meters_pt(8.0);
    return clamp(v, lo, hi).in(meters_pt);
}

// =============================================================================
// SECTION 4: au/magnitude.hh - Magnitude operations
// =============================================================================
// NOTE: Magnitude operations are primarily compile-time utilities and require
// extensive AU_DEVICE_FUNC decoration throughout magnitude.hh. These are not
// typically called from device code, so we skip testing them for now.
// To enable these tests, magnitude.hh needs systematic AU_DEVICE_FUNC decoration.

// =============================================================================
// SECTION 5: au/unit_of_measure.hh - Unit operations
// =============================================================================
// NOTE: Unit operations are primarily compile-time utilities and require
// extensive AU_DEVICE_FUNC decoration throughout unit_of_measure.hh. These are
// not typically called from device code, so we skip testing them for now.
// To enable these tests, unit_of_measure.hh needs systematic AU_DEVICE_FUNC decoration.

// =============================================================================
// SECTION 6: au/constant.hh - Constants
// =============================================================================

__device__ double test_constant_as() { return SPEED_OF_LIGHT.as<double>().in(meters / second); }

__device__ double test_constant_in() { return SPEED_OF_LIGHT.in<double>(meters / second); }

__device__ bool test_constant_comparison_with_zero() {
    return (SPEED_OF_LIGHT != ZERO) && (SPEED_OF_LIGHT > ZERO);
}

__device__ double test_make_constant() {
    auto c = make_constant(meters / second);
    return c.in<double>(meters / second);
}

// =============================================================================
// SECTION 7: au/math.hh - Math functions
// =============================================================================
// NOTE: Most math.hh functions call <cmath> functions (sin, cos, sqrt, etc.)
// which are NOT constexpr and require special handling for CUDA device code.
// These functions would need to call CUDA's device math intrinsics instead.
// For now, we test only the functions that don't depend on <cmath>.

__device__ double test_math_int_pow() {
    auto q = meters(2.0);
    return int_pow<3>(q).in(meters * meters * meters);
}

// TODO: Add tests for these functions once math.hh has CUDA-compatible implementations:
// - abs, sqrt, cbrt (call std::abs, std::sqrt, std::cbrt)
// - sin, cos, tan (call std::sin, std::cos, std::tan)
// - arcsin, arccos, arctan, arctan2 (call std::asin, std::acos, std::atan, std::atan2)
// - hypot (calls std::hypot)
// - fmod, remainder (call std::fmod, std::remainder)
// - copysign (calls std::copysign)
// - round_in/as, floor_in/as, ceil_in/as (call std::round, std::floor, std::ceil)
// - isnan, isinf (call std::isnan, std::isinf)
// - mean (uses division which works, but need to test)

// =============================================================================
// SECTION 8: au/operators.hh - Comparison operator functors
// =============================================================================

__device__ bool test_operators_equal() { return detail::Equal{}(5, 5); }

__device__ bool test_operators_not_equal() { return detail::NotEqual{}(5, 3); }

__device__ bool test_operators_less() { return detail::Less{}(3, 5); }

__device__ bool test_operators_less_equal() { return detail::LessEqual{}(3, 5); }

__device__ bool test_operators_greater() { return detail::Greater{}(5, 3); }

__device__ bool test_operators_greater_equal() { return detail::GreaterEqual{}(5, 3); }

__device__ double test_operators_plus() {
    return detail::plus(meters(3.0), meters(2.0)).in(meters);
}

__device__ double test_operators_minus() {
    return detail::minus(meters(5.0), meters(2.0)).in(meters);
}

// =============================================================================
// SECTION 9: au/stdx/functional.hh - Identity
// =============================================================================

__device__ double test_stdx_identity() { return stdx::identity{}(5.0); }

// =============================================================================
// SECTION 10: au/stdx/utility.hh - Safe integer comparisons
// =============================================================================

__device__ bool test_stdx_cmp_equal() { return stdx::cmp_equal(5, 5); }

__device__ bool test_stdx_cmp_not_equal() { return stdx::cmp_not_equal(5, 3); }

__device__ bool test_stdx_cmp_less() { return stdx::cmp_less(3, 5); }

__device__ bool test_stdx_cmp_greater() { return stdx::cmp_greater(5, 3); }

__device__ bool test_stdx_cmp_less_equal() { return stdx::cmp_less_equal(3, 5); }

__device__ bool test_stdx_cmp_greater_equal() { return stdx::cmp_greater_equal(5, 3); }

__device__ bool test_stdx_in_range() { return stdx::in_range<unsigned char>(100); }

// =============================================================================
// SECTION 11: au/prefix.hh - SI prefixes
// =============================================================================

__device__ double test_prefix_kilo() {
    auto q = kilo(meters)(5.0);
    return q.in(meters);
}

__device__ double test_prefix_milli() {
    auto q = milli(meters)(5000.0);
    return q.in(meters);
}

__device__ double test_prefix_centi() {
    auto q = centi(meters)(500.0);
    return q.in(meters);
}

__device__ double test_prefix_micro() {
    auto q = micro(meters)(5000000.0);
    return q.in(meters);
}

// =============================================================================
// SECTION 12: Conversion risk checking
// =============================================================================

__device__ bool test_will_conversion_overflow() {
    auto q = meters(1e308);
    return will_conversion_overflow(q, centi(meters));
}

__device__ bool test_will_conversion_truncate() {
    auto q = meters(5.5);
    return will_conversion_truncate<int>(q, meters);
}

__device__ bool test_is_conversion_lossy() {
    auto q = meters(5.5);
    return is_conversion_lossy<int>(q, meters);
}

// =============================================================================
// SECTION 13: Risk policy constants
// =============================================================================

__device__ int test_risk_policy_overflow_risk() {
    auto q = giga(hertz)(1);
    return q.in(hertz, ignore(OVERFLOW_RISK));
}

__device__ int test_risk_policy_truncation_risk() {
    auto q = meters(500);
    return q.in(kilo(meters), ignore(TRUNCATION_RISK));
}

__device__ int test_risk_policy_all_risks() {
    auto q = giga(hertz)(1);
    return q.in(hertz * mag<3>() / mag<2>(), ignore(ALL_RISKS));
}

// =============================================================================
// Host-side tests
// =============================================================================
// These tests verify that AU_DEVICE_VAR-annotated variables are accessible from host code.
// This is critical because __device__ alone would make variables device-only.
// By gating AU_DEVICE_VAR on __CUDA_ARCH__, we ensure variables work in both passes.

bool host_test_quantity_operations() {
    auto q = meters(5.0);
    auto t = seconds(2.0);
    auto v = q / t;
    return v.in(meters / second) == 2.5;
}

bool host_test_prefixes() {
    auto q1 = kilo(meters)(5.0);
    auto q2 = milli(meters)(5000.0);
    auto q3 = centi(meters)(500.0);
    auto q4 = micro(meters)(5000000.0);
    return q1.in(meters) == 5000.0 && q2.in(meters) == 5.0 && q3.in(meters) == 5.0 &&
           q4.in(meters) == 5.0;
}

bool host_test_constants() { return SPEED_OF_LIGHT.in<double>(meters / second) > 0.0; }

bool host_test_risk_policies() {
    auto q1 = giga(hertz)(1);
    auto result1 = q1.in(hertz, ignore(OVERFLOW_RISK));

    auto q2 = meters(500);
    auto result2 = q2.in(kilo(meters), ignore(TRUNCATION_RISK));

    auto q3 = giga(hertz)(1);
    auto result3 = q3.in(hertz * mag<3>() / mag<2>(), ignore(ALL_RISKS));

    return result1 > 0 && result2 >= 0 && result3 > 0;
}

bool host_test_zero() {
    auto sum = ZERO + ZERO;
    return sum == ZERO;
}
