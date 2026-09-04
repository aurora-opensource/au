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

#include "au/rep.hh"

#include <complex>
#include <cstdint>
#include <utility>

#include "au/chrono_interop.hh"
#include "au/constant.hh"
#include "au/magnitude.hh"
#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/unit_symbol.hh"
#include "au/units/liters.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/webers.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

namespace {

// A custom quantity that corresponds to `QuantityI<Meters>`.
struct MyMeters {
    int value;
};

// A custom rep with no operations defined on it.
struct IntWithNoOps {
    int value;
};

// A custom type that can left-multiply a `double`.
struct LeftMultiplyDoubleByThree {
    friend double operator*(const LeftMultiplyDoubleByThree &, double x) { return 3.0 * x; }
};

// A custom type that divides a `float` into `10.0f`.
struct DivideTenByFloat {
    friend float operator/(const DivideTenByFloat &, float x) { return 10.0f / x; }
};

// A class rep with an implicit conversion to a standard integer.  Under a naive conversion-based
// normalization it would be silently rewritten to `int`.  This helps us get coverage for the
// `!is_class` guard.
struct ConvertsToInt {
    operator int() const { return 0; }  // NOLINT(google-explicit-constructor)
};

// An unscoped enum promotes to its underlying integer under unary `+`, so integer detection alone
// would incorrectly scoop it up too.  This helps us get coverage for the `!is_enum` guard.
enum UnscopedEnum : int { kUnscopedEnumValue = 0 };

// A vector whose scalar is advertised via the STL `value_type` convention, with a templated scalar
// division operator, as in #666.
template <typename T>
struct VectorWithValueType {
    using value_type = T;

    T elements[3];

    template <typename Scalar>
    friend constexpr auto operator/(const VectorWithValueType &v, Scalar s)
        -> VectorWithValueType<decltype(std::declval<T>() / std::declval<Scalar>())> {
        return {{v.elements[0] / s, v.elements[1] / s, v.elements[2] / s}};
    }
};

// A vector whose scalar is advertised via the Eigen `Scalar` convention.
template <typename T>
struct VectorWithScalar {
    using Scalar = T;

    T elements[3];
};

// A vector with no automatically detectable scalar; we specialize `ScalarOfTrait` below.
template <typename T>
struct OpaqueVector {
    T elements[3];
};

}  // namespace

// Set up the correspondence between `MyMeters` and `QuantityI<Meters>`.
template <>
struct CorrespondingQuantity<MyMeters> {
    using Unit = Meters;
    using Rep = int;
};

// Advertise the scalar of `OpaqueVector` via a `ScalarOfTrait` specialization.
template <typename T>
struct ScalarOfTrait<OpaqueVector<T>> : stdx::type_identity<T> {};

TEST(IsValidRep, FalseForVoid) { EXPECT_THAT(IsValidRep<void>::value, IsFalse()); }

TEST(IsValidRep, TrueForArithmeticTypes) {
    EXPECT_THAT(IsValidRep<int>::value, IsTrue());
    EXPECT_THAT(IsValidRep<float>::value, IsTrue());
    EXPECT_THAT(IsValidRep<double>::value, IsTrue());
    EXPECT_THAT(IsValidRep<uint8_t>::value, IsTrue());
    EXPECT_THAT(IsValidRep<int64_t>::value, IsTrue());
}

TEST(IsValidRep, TrueForStdComplex) {
    EXPECT_THAT(IsValidRep<std::complex<float>>::value, IsTrue());
    EXPECT_THAT(IsValidRep<std::complex<uint16_t>>::value, IsTrue());
}

TEST(IsValidRep, FalseForMagnitude) {
    EXPECT_THAT(IsValidRep<decltype(mag<84>())>::value, IsFalse());
    EXPECT_THAT(IsValidRep<decltype(sqrt(Magnitude<Pi>{}))>::value, IsFalse());
}

TEST(IsValidRep, FalseForUnits) {
    EXPECT_THAT(IsValidRep<Liters>::value, IsFalse());
    EXPECT_THAT(IsValidRep<Nano<Webers>>::value, IsFalse());
}

TEST(IsValidRep, FalseForQuantity) {
    EXPECT_THAT((IsValidRep<Quantity<Milli<Liters>, int>>::value), IsFalse());
}

TEST(IsValidRep, FalseForQuantityPoint) {
    EXPECT_THAT((IsValidRep<QuantityPoint<Miles, double>>::value), IsFalse());
}

TEST(IsValidRep, FalseForConstant) {
    EXPECT_THAT(IsValidRep<decltype(make_constant(liters / mile))>::value, IsFalse());
}

TEST(IsValidRep, FalseForSymbol) { EXPECT_THAT(IsValidRep<SymbolFor<Webers>>::value, IsFalse()); }

TEST(IsValidRep, FalseForTypeWithCorrespondingQuantity) {
    EXPECT_THAT(IsValidRep<MyMeters>::value, IsFalse());
    EXPECT_THAT(IsValidRep<std::chrono::nanoseconds>::value, IsFalse());
}

TEST(IsValidRep, FalseForTypeWhoseScalarIsQuantityLike) {
    // A vector of quantities as a rep would produce nested units, so it can never be valid.  We
    // catch every scalar type that `ScalarOf` can detect, whichever probe it fires on.
    EXPECT_THAT((IsValidRep<VectorWithValueType<Quantity<Meters, float>>>::value), IsFalse());
    EXPECT_THAT((IsValidRep<VectorWithScalar<Quantity<Meters, float>>>::value), IsFalse());
    EXPECT_THAT((IsValidRep<std::complex<Quantity<Meters, float>>>::value), IsFalse());

    // "Quantity-like" also covers `QuantityPoint`, and types with a `CorrespondingQuantity`.
    EXPECT_THAT((IsValidRep<VectorWithValueType<QuantityPoint<Miles, double>>>::value), IsFalse());
    EXPECT_THAT((IsValidRep<VectorWithValueType<MyMeters>>::value), IsFalse());
}

TEST(IsValidRep, TrueForVectorOfPlainScalars) {
    EXPECT_THAT((IsValidRep<VectorWithValueType<float>>::value), IsTrue());
    EXPECT_THAT((IsValidRep<VectorWithScalar<double>>::value), IsTrue());
}

TEST(IsValidRep, HonorsUserSpecializationsOfScalarOfTrait) {
    // `OpaqueVector` has no automatically detectable scalar, so only its `ScalarOfTrait`
    // specialization tells us that `OpaqueVector<Quantity<...>>` has a quantity-like scalar.
    EXPECT_THAT((IsValidRep<OpaqueVector<Quantity<Meters, float>>>::value), IsFalse());
    EXPECT_THAT((IsValidRep<OpaqueVector<float>>::value), IsTrue());
}

TEST(IsProductValidRep, FalseIfProductDoesNotExist) {
    EXPECT_THAT((IsProductValidRep<IntWithNoOps, int>::value), IsFalse());
    EXPECT_THAT((IsProductValidRep<int, IntWithNoOps>::value), IsFalse());
}

TEST(IsProductValidRep, TrueOnlyForSideWhereProductExists) {
    ASSERT_THAT(LeftMultiplyDoubleByThree{} * 4.5, Eq(13.5));

    EXPECT_THAT((IsProductValidRep<LeftMultiplyDoubleByThree, double>::value), IsTrue());
    EXPECT_THAT((IsProductValidRep<double, LeftMultiplyDoubleByThree>::value), IsFalse());
}

TEST(IsQuotientValidRep, FalseIfQuotientDoesNotExist) {
    EXPECT_THAT((IsQuotientValidRep<IntWithNoOps, int>::value), IsFalse());
    EXPECT_THAT((IsQuotientValidRep<int, IntWithNoOps>::value), IsFalse());
}

TEST(IsQuotientValidRep, FalseIfQuotientIsQuantity) {
    // Dividing by a Quantity can complicate matters because it involves hard compiler errors when
    // that quantity has an integral rep.  Make sure we handle this gracefully.
    EXPECT_THAT((IsQuotientValidRep<int, Quantity<Miles, int>>::value), IsFalse());
}

TEST(IsQuotientValidRep, TrueOnlyForSideWhereQuotientExists) {
    ASSERT_THAT(DivideTenByFloat{} / 2.0f, Eq(5.0f));

    EXPECT_THAT((IsQuotientValidRep<float, DivideTenByFloat>::value), IsFalse());
    EXPECT_THAT((IsQuotientValidRep<DivideTenByFloat, float>::value), IsTrue());
}

TEST(IsQuotientValidRep, FalseWhenQuotientHasQuantityLikeScalar) {
    // The gate that resolves #666: `VectorWithValueType<Quantity<...>> / float` exists, but its
    // result is a vector of quantities, which is not a valid rep.  This is what disables the
    // "divide into a scalar" overload of `Quantity::operator/`, so that it cannot be ambiguous
    // with the vector's own scalar division operator.
    EXPECT_THAT((IsQuotientValidRep<VectorWithValueType<Quantity<Meters, float>>, float>::value),
                IsFalse());

    // The same vector of _plain_ scalars remains a valid quotient rep.
    EXPECT_THAT((IsQuotientValidRep<VectorWithValueType<float>, float>::value), IsTrue());
}

namespace detail {

TEST(ResultIfNoneAreQuantity, GivesResultWhenNoneAreQuantity) {
    StaticAssertTypeEq<int, ResultIfNoneAreQuantity<std::common_type_t, int, int>>();
    StaticAssertTypeEq<std::tuple<int, double, float>,
                       ResultIfNoneAreQuantity<std::tuple, int, double, float>>();
}

TEST(ResultIfNoneAreQuantity, GivesVoidWhenAnyIsQuantity) {
    StaticAssertTypeEq<void,
                       ResultIfNoneAreQuantity<std::common_type_t, int, Quantity<Miles, int>>>();
    StaticAssertTypeEq<void,
                       ResultIfNoneAreQuantity<std::tuple, int, Quantity<Miles, int>, float>>();
}

TEST(ResultIfNoneAreQuantity, GivesVoidWhenAnyIsCorrespondingQuantity) {
    StaticAssertTypeEq<void, ResultIfNoneAreQuantity<std::common_type_t, int, MyMeters>>();
    StaticAssertTypeEq<void, ResultIfNoneAreQuantity<std::tuple, int, std::chrono::nanoseconds>>();
}

TEST(ProductTypeOrVoid, GivesProductTypeForArithmeticInputs) {
    StaticAssertTypeEq<int, ProductTypeOrVoid<int, int>>();
}

TEST(ProductTypeOrVoid, GivesVoidForInputsWithNoProductType) {
    StaticAssertTypeEq<void, ProductTypeOrVoid<IntWithNoOps, int>>();
    StaticAssertTypeEq<void, ProductTypeOrVoid<int, IntWithNoOps>>();
}

TEST(NormalizeRep, IsIdentityOnStandardIntegerTypes) {
    StaticAssertTypeEq<NormalizeRep<bool>, bool>();
    StaticAssertTypeEq<NormalizeRep<char>, char>();
    StaticAssertTypeEq<NormalizeRep<signed char>, signed char>();
    StaticAssertTypeEq<NormalizeRep<unsigned char>, unsigned char>();
    StaticAssertTypeEq<NormalizeRep<short>, short>();
    StaticAssertTypeEq<NormalizeRep<unsigned short>, unsigned short>();
    StaticAssertTypeEq<NormalizeRep<int>, int>();
    StaticAssertTypeEq<NormalizeRep<unsigned int>, unsigned int>();
    StaticAssertTypeEq<NormalizeRep<long>, long>();
    StaticAssertTypeEq<NormalizeRep<unsigned long>, unsigned long>();
    StaticAssertTypeEq<NormalizeRep<long long>, long long>();
    StaticAssertTypeEq<NormalizeRep<unsigned long long>, unsigned long long>();

#if defined(__cpp_char8_t)
    StaticAssertTypeEq<NormalizeRep<char8_t>, char8_t>();
#endif

    StaticAssertTypeEq<NormalizeRep<int8_t>, int8_t>();
    StaticAssertTypeEq<NormalizeRep<uint8_t>, uint8_t>();

    StaticAssertTypeEq<NormalizeRep<int16_t>, int16_t>();
    StaticAssertTypeEq<NormalizeRep<uint16_t>, uint16_t>();

    StaticAssertTypeEq<NormalizeRep<int32_t>, int32_t>();
    StaticAssertTypeEq<NormalizeRep<uint32_t>, uint32_t>();

    StaticAssertTypeEq<NormalizeRep<int64_t>, int64_t>();
    StaticAssertTypeEq<NormalizeRep<uint64_t>, uint64_t>();
}

TEST(NormalizeRep, IsIdentityOnNonIntegralTypes) {
    StaticAssertTypeEq<NormalizeRep<float>, float>();
    StaticAssertTypeEq<NormalizeRep<double>, double>();
    StaticAssertTypeEq<NormalizeRep<long double>, long double>();
    StaticAssertTypeEq<NormalizeRep<std::complex<double>>, std::complex<double>>();
    StaticAssertTypeEq<NormalizeRep<IntWithNoOps>, IntWithNoOps>();
}

TEST(NormalizeRep, IsIdentityOnIntegerAdjacentClassEnumAndPointer) {
    // Class with an implicit conversion to `int`: guarded by `!is_class`.
    StaticAssertTypeEq<NormalizeRep<ConvertsToInt>, ConvertsToInt>();

    // Unscoped enum (promotes to `int` under `+`): guarded by `!is_enum`.
    StaticAssertTypeEq<NormalizeRep<UnscopedEnum>, UnscopedEnum>();

    // Pointer (converts to `bool`; also `+ptr` is a pointer, not integral): left alone.
    StaticAssertTypeEq<NormalizeRep<int *>, int *>();
}

// `rep_is_signed` recovers signedness by value (attribute-immune), not via `std::is_signed`.
template <typename T>
struct RepIsSignedAgreesWithStd
    : stdx::bool_constant<rep_is_signed<T>() == std::is_signed<T>::value> {};

TEST(NormalizeRep, RepIsSignedMatchesStdIsSignedForStandardIntegers) {
    EXPECT_THAT(RepIsSignedAgreesWithStd<signed char>::value, IsTrue());
    EXPECT_THAT(RepIsSignedAgreesWithStd<short>::value, IsTrue());
    EXPECT_THAT(RepIsSignedAgreesWithStd<int>::value, IsTrue());
    EXPECT_THAT(RepIsSignedAgreesWithStd<long long>::value, IsTrue());
    EXPECT_THAT(RepIsSignedAgreesWithStd<unsigned char>::value, IsTrue());
    EXPECT_THAT(RepIsSignedAgreesWithStd<unsigned short>::value, IsTrue());
    EXPECT_THAT(RepIsSignedAgreesWithStd<unsigned int>::value, IsTrue());
    EXPECT_THAT(RepIsSignedAgreesWithStd<unsigned long long>::value, IsTrue());
}

// `ShouldNormalizeRep` is false for everything we can spell portably (standard integers are already
// standard; floats/classes/enums are excluded).  It goes true only for an integer-behaving type
// that names no standard integer --- i.e. an attributed type, which we cannot spell portably.
TEST(NormalizeRep, ShouldNormalizeRepIsFalseForAllPortablyExpressibleTypes) {
    EXPECT_THAT(ShouldNormalizeRep<int>::value, IsFalse());
    EXPECT_THAT(ShouldNormalizeRep<uint16_t>::value, IsFalse());
    EXPECT_THAT(ShouldNormalizeRep<double>::value, IsFalse());
    EXPECT_THAT(ShouldNormalizeRep<ConvertsToInt>::value, IsFalse());
    EXPECT_THAT(ShouldNormalizeRep<UnscopedEnum>::value, IsFalse());
    EXPECT_THAT(ShouldNormalizeRep<IntWithNoOps>::value, IsFalse());
}

// The mechanism that fires on GHS: the size+signedness remap that `NormalizeRep` uses for a
// non-standard integral type (the portable stand-in for the non-portable attributed
// `__packed uint16_t` that motivates the trait).  We exercise `FirstMatchingIntegerOr` directly,
// since there is no portable way to spell an attributed integral type.
TEST(NormalizeRep, RemapPicksStandardIntegerBySizeAndSignedness) {
    StaticAssertTypeEq<FirstMatchingIntegerOr<void,
                                              sizeof(int16_t),
                                              true,
                                              std::int8_t,
                                              std::uint8_t,
                                              std::int16_t,
                                              std::uint16_t,
                                              std::int32_t,
                                              std::uint32_t,
                                              std::int64_t,
                                              std::uint64_t>::type,
                       int16_t>();
    StaticAssertTypeEq<FirstMatchingIntegerOr<void,
                                              sizeof(uint16_t),
                                              false,
                                              std::int8_t,
                                              std::uint8_t,
                                              std::int16_t,
                                              std::uint16_t,
                                              std::int32_t,
                                              std::uint32_t,
                                              std::int64_t,
                                              std::uint64_t>::type,
                       uint16_t>();
}

// When no fixed-width standard integer matches (e.g. a hypothetical oversized integral), the remap
// leaves the type untouched via its fallback, so exotic reps like `__int128` never become a hard
// error.
TEST(NormalizeRep, RemapFallsBackWhenNoStandardIntegerMatches) {
    struct Tag {};  // stand-in for "some type"; only used as the fallback here
    StaticAssertTypeEq<FirstMatchingIntegerOr<Tag,
                                              sizeof(std::int64_t) * 2,
                                              true,
                                              std::int8_t,
                                              std::int16_t,
                                              std::int32_t,
                                              std::int64_t>::type,
                       Tag>();
}

// Identity across every fixed-width standard integer (each is already standard, so it round-trips
// unchanged --- the full-width companion to the spot checks in `IsIdentityOnStandardIntegerTypes`).
TEST(NormalizeRep, IsIdentityForAllFixedWidthStandardIntegers) {
    StaticAssertTypeEq<NormalizeRep<int8_t>, int8_t>();
    StaticAssertTypeEq<NormalizeRep<uint8_t>, uint8_t>();
    StaticAssertTypeEq<NormalizeRep<int16_t>, int16_t>();
    StaticAssertTypeEq<NormalizeRep<uint16_t>, uint16_t>();
    StaticAssertTypeEq<NormalizeRep<int32_t>, int32_t>();
    StaticAssertTypeEq<NormalizeRep<uint32_t>, uint32_t>();
    StaticAssertTypeEq<NormalizeRep<int64_t>, int64_t>();
    StaticAssertTypeEq<NormalizeRep<uint64_t>, uint64_t>();
}

}  // namespace detail
}  // namespace au
