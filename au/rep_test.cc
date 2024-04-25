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
#include "au/units/miles.hh"
#include "au/units/webers.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::StaticAssertTypeEq;

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

TEST(IsValidRep, FalseForVoid) { EXPECT_FALSE(IsValidRep<void>::value); }

TEST(IsValidRep, TrueForArithmeticTypes) {
    EXPECT_TRUE(IsValidRep<int>::value);
    EXPECT_TRUE(IsValidRep<float>::value);
    EXPECT_TRUE(IsValidRep<double>::value);
    EXPECT_TRUE(IsValidRep<uint8_t>::value);
    EXPECT_TRUE(IsValidRep<int64_t>::value);
}

TEST(IsValidRep, TrueForStdComplex) {
    EXPECT_TRUE(IsValidRep<std::complex<float>>::value);
    EXPECT_TRUE(IsValidRep<std::complex<uint16_t>>::value);
}

TEST(IsValidRep, FalseForMagnitude) {
    EXPECT_FALSE(IsValidRep<decltype(mag<84>())>::value);
    EXPECT_FALSE(IsValidRep<decltype(sqrt(PI))>::value);
}

TEST(IsValidRep, FalseForUnits) {
    EXPECT_FALSE(IsValidRep<Liters>::value);
    EXPECT_FALSE(IsValidRep<Nano<Webers>>::value);
}

TEST(IsValidRep, FalseForQuantity) {
    EXPECT_FALSE((IsValidRep<Quantity<Milli<Liters>, int>>::value));
}

TEST(IsValidRep, FalseForQuantityPoint) {
    EXPECT_FALSE((IsValidRep<QuantityPoint<Miles, double>>::value));
}

TEST(IsValidRep, FalseForConstant) {
    EXPECT_FALSE(IsValidRep<decltype(make_constant(liters / mile))>::value);
}

TEST(IsValidRep, FalseForSymbol) { EXPECT_FALSE(IsValidRep<SymbolFor<Webers>>::value); }

TEST(IsValidRep, FalseForTypeWithCorrespondingQuantity) {
    EXPECT_FALSE(IsValidRep<MyMeters>::value);
    EXPECT_FALSE(IsValidRep<std::chrono::nanoseconds>::value);
}

TEST(IsProductValidRep, FalseIfProductDoesNotExist) {
    EXPECT_FALSE((IsProductValidRep<IntWithNoOps, int>::value));
    EXPECT_FALSE((IsProductValidRep<int, IntWithNoOps>::value));
}

TEST(IsProductValidRep, TrueOnlyForSideWhereProductExists) {
    ASSERT_EQ(LeftMultiplyDoubleByThree{} * 4.5, 13.5);

    EXPECT_TRUE((IsProductValidRep<LeftMultiplyDoubleByThree, double>::value));
    EXPECT_FALSE((IsProductValidRep<double, LeftMultiplyDoubleByThree>::value));
}

TEST(IsQuotientValidRep, FalseIfQuotientDoesNotExist) {
    EXPECT_FALSE((IsQuotientValidRep<IntWithNoOps, int>::value));
    EXPECT_FALSE((IsQuotientValidRep<int, IntWithNoOps>::value));
}

TEST(IsQuotientValidRep, FalseIfQuotientIsQuantity) {
    // Dividing by a Quantity can complicate matters because it involves hard compiler errors when
    // that quantity has an integral rep.  Make sure we handle this gracefully.
    EXPECT_FALSE((IsQuotientValidRep<int, Quantity<Miles, int>>::value));
}

TEST(IsQuotientValidRep, TrueOnlyForSideWhereQuotientExists) {
    ASSERT_EQ(DivideTenByFloat{} / 2.0f, 5.0f);

    EXPECT_FALSE((IsQuotientValidRep<float, DivideTenByFloat>::value));
    EXPECT_TRUE((IsQuotientValidRep<DivideTenByFloat, float>::value));
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
