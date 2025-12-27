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

}  // namespace

// Set up the correspondence between `MyMeters` and `QuantityI<Meters>`.
template <>
struct CorrespondingQuantity<MyMeters> {
    using Unit = Meters;
    using Rep = int;
};

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

namespace detail {

TEST(ResultIfNoneAreQuantity, GivesResultWhenNoneAreQuantity) {
    StaticAssertTypeEq<int, ResultIfNoneAreQuantityT<std::common_type_t, int, int>>();
    StaticAssertTypeEq<std::tuple<int, double, float>,
                       ResultIfNoneAreQuantityT<std::tuple, int, double, float>>();
}

TEST(ResultIfNoneAreQuantity, GivesVoidWhenAnyIsQuantity) {
    StaticAssertTypeEq<void,
                       ResultIfNoneAreQuantityT<std::common_type_t, int, Quantity<Miles, int>>>();
    StaticAssertTypeEq<void,
                       ResultIfNoneAreQuantityT<std::tuple, int, Quantity<Miles, int>, float>>();
}

TEST(ResultIfNoneAreQuantity, GivesVoidWhenAnyIsCorrespondingQuantity) {
    StaticAssertTypeEq<void, ResultIfNoneAreQuantityT<std::common_type_t, int, MyMeters>>();
    StaticAssertTypeEq<void, ResultIfNoneAreQuantityT<std::tuple, int, std::chrono::nanoseconds>>();
}

TEST(ProductTypeOrVoid, GivesProductTypeForArithmeticInputs) {
    StaticAssertTypeEq<int, ProductTypeOrVoid<int, int>>();
}

TEST(ProductTypeOrVoid, GivesVoidForInputsWithNoProductType) {
    StaticAssertTypeEq<void, ProductTypeOrVoid<IntWithNoOps, int>>();
    StaticAssertTypeEq<void, ProductTypeOrVoid<int, IntWithNoOps>>();
}

}  // namespace detail
}  // namespace au
