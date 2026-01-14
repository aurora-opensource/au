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

#include "au/overflow_boundary.hh"

#include <complex>
#include <limits>

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::Eq;
using ::testing::FloatEq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {
namespace {

constexpr auto PI = Magnitude<Pi>{};

template <typename T>
struct NoUpperLimit {
    static constexpr T upper() { return std::numeric_limits<T>::max(); }
};

template <typename T>
struct NoLowerLimit {
    static constexpr T lower() { return std::numeric_limits<T>::lowest(); }
};

template <typename T>
struct LowerLimitOfZero : NoUpperLimit<T> {
    static constexpr T lower() { return T{0}; }
};

template <typename T>
struct ImplicitLimits {
    static constexpr T lower() { return std::numeric_limits<T>::lowest(); }
    static constexpr T upper() { return std::numeric_limits<T>::max(); }
};

template <typename T, typename M>
MultiplyTypeBy<T, M> multiply_type_by(M) {
    return MultiplyTypeBy<T, M>{};
}

template <typename T, typename M>
DivideTypeByInteger<T, M> divide_type_by_integer(M) {
    return DivideTypeByInteger<T, M>{};
}

template <typename Op>
auto min_good_value(Op) {
    return MinGood<Op>::value();
}

template <typename Op, typename Limits>
auto min_good_value(Op, Limits) {
    return MinGood<Op, Limits>::value();
}

template <typename Op>
auto max_good_value(Op) {
    return MaxGood<Op>::value();
}

template <typename Op, typename Limits>
auto max_good_value(Op, Limits) {
    return MaxGood<Op, Limits>::value();
}

template <typename... Ops>
auto op_sequence(Ops...) {
    return OpSequence<Ops...>{};
}

template <typename Op>
bool can_overflow_below(Op) {
    return CanOverflowBelow<Op>::value;
}

template <typename Op>
bool can_overflow_above(Op) {
    return CanOverflowAbove<Op>::value;
}

// Type-parameterized test infrastructure for cast-like operations.
//
// Both `StaticCast` and `ImplicitConversion` have identical overflow boundary behavior (in fact,
// `ImplicitConversion` delegates its implementation to `StaticCast`).  This wrapper lets us test
// both with a single set of type-parameterized tests.
template <template <class, class> class Op>
struct CastLikeOp {
    template <typename Src, typename Dst>
    using Convert = Op<Src, Dst>;
};

using CastLikeOps = ::testing::Types<CastLikeOp<StaticCast>, CastLikeOp<ImplicitConversion>>;

template <typename T>
class CastLikeOpTest : public ::testing::Test {
 protected:
    template <typename Src, typename Dst>
    using Op = typename T::template Convert<Src, Dst>;
};

TYPED_TEST_SUITE(CastLikeOpTest, CastLikeOps);

// Convenience alias to make tests more concise.
template <typename Fixture, typename Src, typename Dst>
using Convert = typename Fixture::template Op<Src, Dst>;

template <bool IsPositive>
struct MagSignIfPositiveIs : stdx::type_identity<Magnitude<>> {};
template <>
struct MagSignIfPositiveIs<false> : stdx::type_identity<Magnitude<Negative>> {};
template <bool IsPositive>
constexpr auto mag_sign_if_positive_is() {
    return typename MagSignIfPositiveIs<IsPositive>::type{};
}

// Handy little utility to turn an arbitrary floating point number into a Magnitude.
template <typename T, typename ValConst>
struct MagFromFloatingPointConstantImpl {
    static_assert(std::is_floating_point<T>::value,
                  "Must be floating point (internal library error)");

    struct Breakdown {
        bool is_positive = true;
        uint64_t coeff = 0u;
        int64_t exp = 0;

        constexpr Breakdown() = default;
    };

    static constexpr Breakdown breakdown() {
        T x = ValConst::value();

        Breakdown result;

        result.is_positive = (x >= T{0});
        if (!result.is_positive) {
            x = -x;
        }

        while (x > static_cast<T>(std::numeric_limits<uint64_t>::max())) {
            x /= T{2};
            ++result.exp;
        }
        while (result.exp > 64 && static_cast<T>(static_cast<uint64_t>(x)) != x) {
            x *= T{2};
            --result.exp;
        }

        result.coeff = static_cast<uint64_t>(x);
        return result;
    }

    static constexpr auto value() {
        constexpr auto params = breakdown();
        return mag_sign_if_positive_is<params.is_positive>() * mag<params.coeff>() *
               pow<params.exp>(mag<2>());
    }

    using type = decltype(value());
};

template <typename Float>
constexpr auto lowest_floating_point_as_mag() {
    return MagFromFloatingPointConstantImpl<Float, ValueOfLowestInDestination<Float>>::value();
}

template <typename Float>
constexpr auto highest_floating_point_as_mag() {
    return MagFromFloatingPointConstantImpl<Float, ValueOfHighestInDestination<Float>>::value();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MinPossible` section:

TEST(MinPossible, GivesStdNumericLimitsLowestForSimpleTypes) {
    EXPECT_THAT((MinPossible<StaticCast<uint64_t, int>>::value()),
                SameTypeAndValue(std::numeric_limits<uint64_t>::lowest()));

    EXPECT_THAT((MinPossible<StaticCast<int8_t, int32_t>>::value()),
                SameTypeAndValue(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinPossible<StaticCast<float, double>>::value()),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));
}

TEST(MinPossible, GivesStdNumericLimitsLowestOfScalarTypeForCompoundTypes) {
    EXPECT_THAT((MinPossible<StaticCast<std::complex<float>, std::complex<double>>>::value()),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));

    EXPECT_THAT((MinPossible<StaticCast<std::complex<double>, std::complex<float>>>::value()),
                SameTypeAndValue(std::numeric_limits<double>::lowest()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MaxPossible` section:

TEST(MaxPossible, GivesStdNumericLimitsMaxForSimpleTypes) {
    EXPECT_THAT((MaxPossible<StaticCast<uint64_t, int>>::value()),
                SameTypeAndValue(std::numeric_limits<uint64_t>::max()));

    EXPECT_THAT((MaxPossible<StaticCast<int8_t, int32_t>>::value()),
                SameTypeAndValue(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxPossible<StaticCast<float, double>>::value()),
                SameTypeAndValue(std::numeric_limits<float>::max()));
}

TEST(MaxPossible, GivesStdNumericLimitsMaxOfScalarTypeForCompoundTypes) {
    EXPECT_THAT((MaxPossible<StaticCast<std::complex<float>, std::complex<double>>>::value()),
                SameTypeAndValue(std::numeric_limits<float>::max()));

    EXPECT_THAT((MaxPossible<StaticCast<std::complex<double>, std::complex<float>>>::value()),
                SameTypeAndValue(std::numeric_limits<double>::max()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CastLikeOp` section (covers both `StaticCast` and `ImplicitConversion`):

//
// `MinGood<CastLikeOp>`:
//

TYPED_TEST(CastLikeOpTest, MinGoodIsLowestIfDestinationEqualsSource) {
    EXPECT_THAT((MinGood<Convert<TestFixture, int8_t, int8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<Convert<TestFixture, uint16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint16_t>::lowest()));

    EXPECT_THAT((MinGood<Convert<TestFixture, float, float>>::value()),
                Eq(std::numeric_limits<float>::lowest()));
}

TYPED_TEST(CastLikeOpTest, MinGoodIsLowestIfCastWidens) {
    EXPECT_THAT((MinGood<Convert<TestFixture, int8_t, int16_t>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<Convert<TestFixture, uint8_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::lowest()));

    EXPECT_THAT((MinGood<Convert<TestFixture, float, double>>::value()),
                Eq(std::numeric_limits<float>::lowest()));
}

TYPED_TEST(CastLikeOpTest, MinGoodIsZeroFromAnySignedToAnyUnsigned) {
    EXPECT_THAT((MinGood<Convert<TestFixture, int8_t, uint64_t>>::value()),
                SameTypeAndValue(int8_t{0}));
    EXPECT_THAT((MinGood<Convert<TestFixture, int16_t, uint8_t>>::value()),
                SameTypeAndValue(int16_t{0}));
    EXPECT_THAT((MinGood<Convert<TestFixture, int32_t, uint32_t>>::value()),
                SameTypeAndValue(int32_t{0}));
}

TYPED_TEST(CastLikeOpTest, MinGoodIsZeroFromAnyUnsignedToAnyArithmetic) {
    EXPECT_THAT((MinGood<Convert<TestFixture, uint8_t, int64_t>>::value()), Eq(uint8_t{0}));
    EXPECT_THAT((MinGood<Convert<TestFixture, uint16_t, uint8_t>>::value()), Eq(uint16_t{0}));
    EXPECT_THAT((MinGood<Convert<TestFixture, uint32_t, int16_t>>::value()), Eq(uint32_t{0}));
    EXPECT_THAT((MinGood<Convert<TestFixture, uint64_t, int64_t>>::value()), Eq(uint64_t{0}));
    EXPECT_THAT((MinGood<Convert<TestFixture, uint64_t, float>>::value()), Eq(uint64_t{0}));
    EXPECT_THAT((MinGood<Convert<TestFixture, uint8_t, double>>::value()), Eq(uint8_t{0}));
}

TYPED_TEST(CastLikeOpTest, MinGoodIsLowestInDestinationWhenNarrowingToSameFamily) {
    EXPECT_THAT((MinGood<Convert<TestFixture, int64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<int64_t>(std::numeric_limits<int32_t>::lowest())));
    EXPECT_THAT((MinGood<Convert<TestFixture, double, float>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::lowest())));
}

TYPED_TEST(CastLikeOpTest, MinGoodIsZeroFromAnyFloatingPointToAnyUnsigned) {
    EXPECT_THAT((MinGood<Convert<TestFixture, double, uint8_t>>::value()), SameTypeAndValue(0.0));
    EXPECT_THAT((MinGood<Convert<TestFixture, float, uint64_t>>::value()), SameTypeAndValue(0.0f));
}

TYPED_TEST(CastLikeOpTest, MinGoodIsLowestInDestinationFromAnyFloatingPointToAnySigned) {
    EXPECT_THAT((MinGood<Convert<TestFixture, double, int32_t>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<int32_t>::lowest())));
    EXPECT_THAT((MinGood<Convert<TestFixture, float, int64_t>>::value()),
                SameTypeAndValue(static_cast<float>(std::numeric_limits<int64_t>::lowest())));
}

TYPED_TEST(CastLikeOpTest, MinGoodIsLowestFromAnySignedToAnyFloatingPoint) {
    // We could imagine some hypothetical floating point and integral types for which this is not
    // true.  But floating point is designed to cover a very wide range between its min and max
    // values, and in practice, this is true for all commonly used floating point and integral
    // types.
    EXPECT_THAT((MinGood<Convert<TestFixture, int8_t, double>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<Convert<TestFixture, int64_t, float>>::value()),
                Eq(std::numeric_limits<int64_t>::lowest()));
}

TYPED_TEST(CastLikeOpTest, MinGoodUnchangedWithExplicitLimitOfLowestInTargetType) {
    // What all these test cases have in common is that the destination type is already the most
    // constraining factor.  Therefore, the only way to add an _explicit_ limit, which nevertheless
    // does _not_ constrain the answer, is to make that explicit limit equal to the implicit limit:
    // that is, the lowest value of the destination type.

    EXPECT_THAT((MinGood<Convert<TestFixture, int8_t, int8_t>, ImplicitLimits<int8_t>>::value()),
                Eq((MinGood<Convert<TestFixture, int8_t, int8_t>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, uint16_t, uint16_t>, ImplicitLimits<uint16_t>>::value()),
        Eq((MinGood<Convert<TestFixture, uint16_t, uint16_t>>::value())));

    EXPECT_THAT((MinGood<Convert<TestFixture, float, float>, ImplicitLimits<float>>::value()),
                Eq((MinGood<Convert<TestFixture, float, float>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, uint32_t, int32_t>, ImplicitLimits<int32_t>>::value()),
        Eq((MinGood<Convert<TestFixture, uint32_t, int32_t>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, int64_t, uint64_t>, ImplicitLimits<uint64_t>>::value()),
        Eq((MinGood<Convert<TestFixture, int64_t, uint64_t>>::value())));

    EXPECT_THAT((MinGood<Convert<TestFixture, double, float>, ImplicitLimits<float>>::value()),
                Eq((MinGood<Convert<TestFixture, double, float>>::value())));

    EXPECT_THAT((MinGood<Convert<TestFixture, float, uint64_t>, ImplicitLimits<uint64_t>>::value()),
                Eq((MinGood<Convert<TestFixture, float, uint64_t>>::value())));

    EXPECT_THAT((MinGood<Convert<TestFixture, float, int64_t>, ImplicitLimits<int64_t>>::value()),
                Eq((MinGood<Convert<TestFixture, float, int64_t>>::value())));

    EXPECT_THAT((MinGood<Convert<TestFixture, float, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq((MinGood<Convert<TestFixture, float, int32_t>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, uint32_t, uint16_t>, ImplicitLimits<uint16_t>>::value()),
        Eq((MinGood<Convert<TestFixture, uint32_t, uint16_t>>::value())));

    EXPECT_THAT((MinGood<Convert<TestFixture, uint32_t, int8_t>, ImplicitLimits<int8_t>>::value()),
                Eq((MinGood<Convert<TestFixture, uint32_t, int8_t>>::value())));

    EXPECT_THAT((MinGood<Convert<TestFixture, int64_t, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq((MinGood<Convert<TestFixture, int64_t, int32_t>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, int64_t, uint32_t>, ImplicitLimits<uint32_t>>::value()),
        Eq((MinGood<Convert<TestFixture, int64_t, uint32_t>>::value())));
}

TYPED_TEST(CastLikeOpTest, MinGoodUnchangedWithExplicitLimitLessConstrainingThanExistingResult) {
    // In these cases, we are applying a non-trivial lower limit (i.e., it is higher than the
    // `lowest()` value), but it does not constrain the result enough to change it.

    struct DoubleLimitTwiceFloatLowest : NoUpperLimit<double> {
        static constexpr double lower() {
            return static_cast<double>(std::numeric_limits<float>::lowest()) * 2.0;
        }
    };

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, float, double>, DoubleLimitTwiceFloatLowest>::value()),
        Eq((MinGood<Convert<TestFixture, float, double>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, int32_t, double>, DoubleLimitTwiceFloatLowest>::value()),
        Eq((MinGood<Convert<TestFixture, int32_t, double>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, uint16_t, double>, DoubleLimitTwiceFloatLowest>::value()),
        Eq((MinGood<Convert<TestFixture, uint16_t, double>>::value())));

    struct FloatLimitHalfFloatLowest : NoUpperLimit<float> {
        static constexpr float lower() { return std::numeric_limits<float>::lowest() / 2.0f; }
    };

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, uint64_t, float>, FloatLimitHalfFloatLowest>::value()),
        Eq((MinGood<Convert<TestFixture, uint64_t, float>>::value())));

    EXPECT_THAT((MinGood<Convert<TestFixture, int64_t, float>, FloatLimitHalfFloatLowest>::value()),
                Eq((MinGood<Convert<TestFixture, int64_t, float>>::value())));

    struct SignedLimitHalfInt64Lowest : NoUpperLimit<int64_t> {
        static constexpr int64_t lower() { return std::numeric_limits<int64_t>::lowest() / 2; }
    };

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, uint32_t, int64_t>, SignedLimitHalfInt64Lowest>::value()),
        Eq((MinGood<Convert<TestFixture, uint32_t, int64_t>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, int32_t, int64_t>, SignedLimitHalfInt64Lowest>::value()),
        Eq((MinGood<Convert<TestFixture, int32_t, int64_t>>::value())));
}

TYPED_TEST(CastLikeOpTest, MinGoodUnchangedForUnsignedDestinationAndExplicitLimitOfZero) {
    EXPECT_THAT(
        (MinGood<Convert<TestFixture, uint8_t, uint16_t>, LowerLimitOfZero<uint16_t>>::value()),
        Eq((MinGood<Convert<TestFixture, uint8_t, uint16_t>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, int32_t, uint64_t>, LowerLimitOfZero<uint64_t>>::value()),
        Eq((MinGood<Convert<TestFixture, int32_t, uint64_t>>::value())));

    EXPECT_THAT(
        (MinGood<Convert<TestFixture, double, uint32_t>, LowerLimitOfZero<uint32_t>>::value()),
        Eq((MinGood<Convert<TestFixture, double, uint32_t>>::value())));
}

TYPED_TEST(CastLikeOpTest, MinGoodCappedByExplicitFloatLimit) {
    struct FloatLowerLimitMinusOne : NoUpperLimit<float> {
        static constexpr float lower() { return -1.0f; }
    };

    EXPECT_THAT((MinGood<Convert<TestFixture, int16_t, float>, FloatLowerLimitMinusOne>::value()),
                SameTypeAndValue(int16_t{-1}));

    EXPECT_THAT((MinGood<Convert<TestFixture, int64_t, float>, FloatLowerLimitMinusOne>::value()),
                SameTypeAndValue(int64_t{-1}));

    EXPECT_THAT((MinGood<Convert<TestFixture, float, float>, FloatLowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0f));

    EXPECT_THAT((MinGood<Convert<TestFixture, double, float>, FloatLowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0));
}

TYPED_TEST(CastLikeOpTest, MinGoodCappedByExplicitDoubleLimit) {
    struct DoubleLowerLimitMinusOne : NoUpperLimit<double> {
        static constexpr double lower() { return -1.0; }
    };

    EXPECT_THAT((MinGood<Convert<TestFixture, float, double>, DoubleLowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0f));
}

TYPED_TEST(CastLikeOpTest, MinGoodCappedByExplicitI64Limit) {
    struct I64LowerLimitMinusOne : NoUpperLimit<int64_t> {
        static constexpr int64_t lower() { return -1; }
    };

    EXPECT_THAT((MinGood<Convert<TestFixture, int32_t, int64_t>, I64LowerLimitMinusOne>::value()),
                SameTypeAndValue(int32_t{-1}));

    EXPECT_THAT((MinGood<Convert<TestFixture, int64_t, int64_t>, I64LowerLimitMinusOne>::value()),
                SameTypeAndValue(int64_t{-1}));

    EXPECT_THAT((MinGood<Convert<TestFixture, float, int64_t>, I64LowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0f));
}

TYPED_TEST(CastLikeOpTest, MinGoodCappedByExplicitI16Limit) {
    struct I16LowerLimitMinusOne : NoUpperLimit<int16_t> {
        static constexpr int16_t lower() { return -1; }
    };

    EXPECT_THAT((MinGood<Convert<TestFixture, int32_t, int16_t>, I16LowerLimitMinusOne>::value()),
                SameTypeAndValue(int32_t{-1}));

    EXPECT_THAT((MinGood<Convert<TestFixture, double, int16_t>, I16LowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0));
}

TYPED_TEST(CastLikeOpTest, MinGoodForComplexOfTProvidesAnswerAsT) {
    EXPECT_THAT((MinGood<Convert<TestFixture, std::complex<float>, std::complex<double>>>::value()),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));

    EXPECT_THAT((MinGood<Convert<TestFixture, std::complex<double>, std::complex<float>>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::lowest())));
}

//
// `MaxGood<CastLikeOp>`:
//

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestIfDestinationEqualsSource) {
    EXPECT_THAT((MaxGood<Convert<TestFixture, int8_t, int8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint16_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, float>>::value()),
                Eq(std::numeric_limits<float>::max()));
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestIfCastWidens) {
    EXPECT_THAT((MaxGood<Convert<TestFixture, int8_t, int16_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint8_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, double>>::value()),
                Eq(std::numeric_limits<float>::max()));
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestFromSignedToUnsignedOfSameSize) {
    EXPECT_THAT((MaxGood<Convert<TestFixture, int8_t, uint8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<int16_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, uint32_t>>::value()),
                Eq(std::numeric_limits<int32_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int64_t, uint64_t>>::value()),
                Eq(std::numeric_limits<int64_t>::max()));
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestInDestinationFromUnsignedToSignedOfSameSize) {
    EXPECT_THAT((MaxGood<Convert<TestFixture, uint8_t, int8_t>>::value()),
                SameTypeAndValue(static_cast<uint8_t>(std::numeric_limits<int8_t>::max())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint64_t, int64_t>>::value()),
                SameTypeAndValue(static_cast<uint64_t>(std::numeric_limits<int64_t>::max())));
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestFromAnyIntToAnyLargerInt) {
    EXPECT_THAT((MaxGood<Convert<TestFixture, uint8_t, int16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, uint64_t>>::value()),
                Eq(std::numeric_limits<int32_t>::max()));
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestInDestinationFromAnyIntToAnySmallerInt) {
    EXPECT_THAT((MaxGood<Convert<TestFixture, uint16_t, uint8_t>>::value()),
                SameTypeAndValue(static_cast<uint16_t>(std::numeric_limits<uint8_t>::max())));
    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, uint16_t>>::value()),
                SameTypeAndValue(static_cast<int32_t>(std::numeric_limits<uint16_t>::max())));
    EXPECT_THAT((MaxGood<Convert<TestFixture, uint64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<uint64_t>(std::numeric_limits<int32_t>::max())));
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestInDestinationWhenNarrowingToSameFamily) {
    EXPECT_THAT((MaxGood<Convert<TestFixture, uint16_t, uint8_t>>::value()),
                SameTypeAndValue(static_cast<uint16_t>(std::numeric_limits<uint8_t>::max())));
    EXPECT_THAT((MaxGood<Convert<TestFixture, int64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<int64_t>(std::numeric_limits<int32_t>::max())));
    EXPECT_THAT((MaxGood<Convert<TestFixture, double, float>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::max())));
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestInDestinationFromAnyFloatingPointToAnySmallIntegral) {
    // The precondition for this test is that the max for the (destination) integral type is
    // _exactly_ representable in the (source) floating point type.  This helper will double check
    // this assumption.
    auto expect_max_good_is_exact_representation_of_destination_int_max =
        [](auto float_type_val, auto int_type_val, auto op) {
            using Float = decltype(float_type_val);
            using Int = decltype(int_type_val);
            using Op = typename decltype(op)::template Convert<Float, Int>;

            constexpr auto expected = static_cast<Float>(std::numeric_limits<Int>::max());

            ASSERT_THAT(static_cast<Int>(expected), Eq(std::numeric_limits<Int>::max()));
            EXPECT_THAT((MaxGood<Op>::value()), SameTypeAndValue(expected));
        };

    expect_max_good_is_exact_representation_of_destination_int_max(
        double{}, uint8_t{}, TypeParam{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, int8_t{}, TypeParam{});
    expect_max_good_is_exact_representation_of_destination_int_max(
        double{}, uint16_t{}, TypeParam{});
    expect_max_good_is_exact_representation_of_destination_int_max(
        double{}, int16_t{}, TypeParam{});
    expect_max_good_is_exact_representation_of_destination_int_max(
        double{}, uint32_t{}, TypeParam{});
    expect_max_good_is_exact_representation_of_destination_int_max(
        double{}, int32_t{}, TypeParam{});

    expect_max_good_is_exact_representation_of_destination_int_max(float{}, uint8_t{}, TypeParam{});
    expect_max_good_is_exact_representation_of_destination_int_max(float{}, int8_t{}, TypeParam{});
    expect_max_good_is_exact_representation_of_destination_int_max(
        float{}, uint16_t{}, TypeParam{});
    expect_max_good_is_exact_representation_of_destination_int_max(float{}, int16_t{}, TypeParam{});
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestRepresentableFloatBelowCastedIntMaxForTooBigInt) {
    // `float` to 64-bit integer:
    EXPECT_THAT((MaxGood<Convert<TestFixture, float, int64_t>>::value()),
                SameTypeAndValue(
                    std::nextafter(static_cast<float>(std::numeric_limits<int64_t>::max()), 1.0f)));
    EXPECT_THAT((MaxGood<Convert<TestFixture, float, uint64_t>>::value()),
                SameTypeAndValue(std::nextafter(
                    static_cast<float>(std::numeric_limits<uint64_t>::max()), 1.0f)));

    // `double` to 64-bit integer:
    EXPECT_THAT((MaxGood<Convert<TestFixture, double, int64_t>>::value()),
                SameTypeAndValue(
                    std::nextafter(static_cast<double>(std::numeric_limits<int64_t>::max()), 1.0)));
    EXPECT_THAT((MaxGood<Convert<TestFixture, double, uint64_t>>::value()),
                SameTypeAndValue(std::nextafter(
                    static_cast<double>(std::numeric_limits<uint64_t>::max()), 1.0)));
}

TYPED_TEST(CastLikeOpTest, MaxGoodIsHighestFromAnyIntegralToAnyFloatingPoint) {
    // See comments in `MinGoodIsLowestFromAnySignedToAnyFloatingPoint` for more on the assumptions
    // we're making here.

    EXPECT_THAT((MaxGood<Convert<TestFixture, int8_t, double>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint8_t, double>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int64_t, float>>::value()),
                Eq(std::numeric_limits<int64_t>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint64_t, float>>::value()),
                Eq(std::numeric_limits<uint64_t>::max()));
}

TYPED_TEST(CastLikeOpTest, MaxGoodUnchangedWithExplicitLimitOfHighestInTargetType) {
    // What all these test cases have in common is that the destination type is already the most
    // constraining factor.  Therefore, the only way to add an _explicit_ limit, which nevertheless
    // does _not_ constrain the answer, is to make that explicit limit equal to the implicit limit:
    // that is, the highest value of the destination type.

    EXPECT_THAT((MaxGood<Convert<TestFixture, int8_t, int8_t>, ImplicitLimits<int8_t>>::value()),
                Eq((MaxGood<Convert<TestFixture, int8_t, int8_t>>::value())));

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, uint16_t, uint16_t>, ImplicitLimits<uint16_t>>::value()),
        Eq((MaxGood<Convert<TestFixture, uint16_t, uint16_t>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, float>, ImplicitLimits<float>>::value()),
                Eq((MaxGood<Convert<TestFixture, float, float>>::value())));

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, uint32_t, int32_t>, ImplicitLimits<int32_t>>::value()),
        Eq((MaxGood<Convert<TestFixture, uint32_t, int32_t>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, double, float>, ImplicitLimits<float>>::value()),
                Eq((MaxGood<Convert<TestFixture, double, float>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, uint64_t>, ImplicitLimits<uint64_t>>::value()),
                Eq((MaxGood<Convert<TestFixture, float, uint64_t>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, int64_t>, ImplicitLimits<int64_t>>::value()),
                Eq((MaxGood<Convert<TestFixture, float, int64_t>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, double, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq((MaxGood<Convert<TestFixture, double, int32_t>>::value())));

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, double, uint32_t>, ImplicitLimits<uint32_t>>::value()),
        Eq((MaxGood<Convert<TestFixture, double, uint32_t>>::value())));

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, uint32_t, uint16_t>, ImplicitLimits<uint16_t>>::value()),
        Eq((MaxGood<Convert<TestFixture, uint32_t, uint16_t>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint32_t, int8_t>, ImplicitLimits<int8_t>>::value()),
                Eq((MaxGood<Convert<TestFixture, uint32_t, int8_t>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int64_t, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq((MaxGood<Convert<TestFixture, int64_t, int32_t>>::value())));

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, int64_t, uint32_t>, ImplicitLimits<uint32_t>>::value()),
        Eq((MaxGood<Convert<TestFixture, int64_t, uint32_t>>::value())));
}

TYPED_TEST(CastLikeOpTest, MaxGoodUnchangedWithExplicitLimitLessConstrainingThanExistingResult) {
    // In these cases, we are applying a non-trivial upper limit (i.e., it is lower than the
    // `max()` value), but it does not constrain the result enough to change it.

    struct DoubleLimitTwiceFloatMax : NoLowerLimit<double> {
        static constexpr double upper() {
            return static_cast<double>(std::numeric_limits<float>::max()) * 2.0;
        }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, double>, DoubleLimitTwiceFloatMax>::value()),
                Eq((MaxGood<Convert<TestFixture, float, double>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, double>, DoubleLimitTwiceFloatMax>::value()),
                Eq((MaxGood<Convert<TestFixture, int32_t, double>>::value())));

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, uint16_t, double>, DoubleLimitTwiceFloatMax>::value()),
        Eq((MaxGood<Convert<TestFixture, uint16_t, double>>::value())));

    struct FloatLimitHalfFloatMax : NoLowerLimit<float> {
        static constexpr float upper() { return std::numeric_limits<float>::max() / 2.0f; }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint64_t, float>, FloatLimitHalfFloatMax>::value()),
                Eq((MaxGood<Convert<TestFixture, uint64_t, float>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int64_t, float>, FloatLimitHalfFloatMax>::value()),
                Eq((MaxGood<Convert<TestFixture, int64_t, float>>::value())));

    struct SignedLimitHalfInt64Max : NoLowerLimit<int64_t> {
        static constexpr int64_t upper() { return std::numeric_limits<int64_t>::max() / 2; }
    };

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, uint32_t, int64_t>, SignedLimitHalfInt64Max>::value()),
        Eq((MaxGood<Convert<TestFixture, uint32_t, int64_t>>::value())));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, int64_t>, SignedLimitHalfInt64Max>::value()),
                Eq((MaxGood<Convert<TestFixture, int32_t, int64_t>>::value())));

    struct UnsignedLimitUint64MaxMinusTwo : NoLowerLimit<uint64_t> {
        static constexpr uint64_t upper() { return std::numeric_limits<uint64_t>::max() - 2; }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint32_t, uint64_t>,
                         UnsignedLimitUint64MaxMinusTwo>::value()),
                Eq((MaxGood<Convert<TestFixture, uint32_t, uint64_t>>::value())));

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, int32_t, uint64_t>, UnsignedLimitUint64MaxMinusTwo>::value()),
        Eq((MaxGood<Convert<TestFixture, int32_t, uint64_t>>::value())));

    EXPECT_THAT(
        (MaxGood<Convert<TestFixture, int64_t, uint64_t>, UnsignedLimitUint64MaxMinusTwo>::value()),
        Eq((MaxGood<Convert<TestFixture, int64_t, uint64_t>>::value())));
}

TYPED_TEST(CastLikeOpTest, MaxGoodCappedByExplicitFloatLimit) {
    struct FloatUpperLimitOne : NoLowerLimit<float> {
        static constexpr float upper() { return 1.0f; }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, int16_t, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(int16_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint16_t, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(uint16_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int64_t, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(int64_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint64_t, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(uint64_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(1.0f));

    EXPECT_THAT((MaxGood<Convert<TestFixture, double, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(1.0));
}

TYPED_TEST(CastLikeOpTest, MaxGoodCappedByExplicitDoubleLimit) {
    struct DoubleUpperLimitOne : NoLowerLimit<double> {
        static constexpr double upper() { return 1.0; }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, double>, DoubleUpperLimitOne>::value()),
                SameTypeAndValue(1.0f));
}

TYPED_TEST(CastLikeOpTest, MaxGoodCappedByExplicitU64Limit) {
    struct U64UpperLimitOne : NoLowerLimit<uint64_t> {
        static constexpr uint64_t upper() { return 1; }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint32_t, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(uint32_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint64_t, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(uint64_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int64_t, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(int64_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(1.0f));
}

TYPED_TEST(CastLikeOpTest, MaxGoodCappedByExplicitI64Limit) {
    struct I64UpperLimitOne : NoLowerLimit<int64_t> {
        static constexpr int64_t upper() { return 1; }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint32_t, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(uint32_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint64_t, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(uint64_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, int64_t, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(int64_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, float, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(1.0f));
}

TYPED_TEST(CastLikeOpTest, MaxGoodCappedByExplicitI16Limit) {
    struct I16UpperLimitOne : NoLowerLimit<int16_t> {
        static constexpr int16_t upper() { return 1; }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, int16_t>, I16UpperLimitOne>::value()),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint32_t, int16_t>, I16UpperLimitOne>::value()),
                SameTypeAndValue(uint32_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, double, int16_t>, I16UpperLimitOne>::value()),
                SameTypeAndValue(1.0));
}

TYPED_TEST(CastLikeOpTest, MaxGoodCappedByExplicitU16Limit) {
    struct U16UpperLimitOne : NoLowerLimit<uint16_t> {
        static constexpr uint16_t upper() { return 1; }
    };

    EXPECT_THAT((MaxGood<Convert<TestFixture, int32_t, uint16_t>, U16UpperLimitOne>::value()),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, uint32_t, uint16_t>, U16UpperLimitOne>::value()),
                SameTypeAndValue(uint32_t{1}));

    EXPECT_THAT((MaxGood<Convert<TestFixture, double, uint16_t>, U16UpperLimitOne>::value()),
                SameTypeAndValue(1.0));
}

TYPED_TEST(CastLikeOpTest, MaxGoodForComplexOfTProvidesAnswerAsT) {
    EXPECT_THAT((MaxGood<Convert<TestFixture, std::complex<float>, std::complex<double>>>::value()),
                SameTypeAndValue(std::numeric_limits<float>::max()));

    EXPECT_THAT((MaxGood<Convert<TestFixture, std::complex<double>, std::complex<float>>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::max())));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy` section:

//
// `MinGood<MultiplyTypeBy>`:
//

TEST(MultiplyTypeBy, MinGoodForUnsignedIsAlwaysZero) {
    EXPECT_THAT(min_good_value(multiply_type_by<uint8_t>(mag<1>())), SameTypeAndValue(uint8_t{0}));

    EXPECT_THAT(min_good_value(multiply_type_by<uint16_t>(mag<123>())),
                SameTypeAndValue(uint16_t{0}));

    EXPECT_THAT(min_good_value(multiply_type_by<uint32_t>(mag<1>() / mag<234>())),
                SameTypeAndValue(uint32_t{0}));

    EXPECT_THAT(min_good_value(multiply_type_by<uint64_t>(-mag<1>())),
                SameTypeAndValue(uint64_t{0}));

    EXPECT_THAT(min_good_value(multiply_type_by<uint64_t>(-mag<543>())),
                SameTypeAndValue(uint64_t{0}));

    EXPECT_THAT(min_good_value(multiply_type_by<uint64_t>(-mag<1>() / mag<2>())),
                SameTypeAndValue(uint64_t{0}));
}

TEST(MultiplyTypeBy, MinGoodForUnlimitedSignedTimesPosIntIsLowerLimitDivByMag) {
    EXPECT_THAT(min_good_value(multiply_type_by<int8_t>(mag<1>())), SameTypeAndValue(int8_t{-128}));

    EXPECT_THAT(min_good_value(multiply_type_by<int8_t>(mag<64>())), SameTypeAndValue(int8_t{-2}));

    EXPECT_THAT(min_good_value(multiply_type_by<int8_t>(mag<65>())), SameTypeAndValue(int8_t{-1}));

    EXPECT_THAT(min_good_value(multiply_type_by<int8_t>(mag<127>())), SameTypeAndValue(int8_t{-1}));
}

TEST(MultiplyTypeBy, MinGoodForUnlimitedSignedTimesNegativeIntIsUpperLimitDivByMag) {
    EXPECT_THAT(min_good_value(multiply_type_by<int8_t>(-mag<1>())),
                SameTypeAndValue(int8_t{-127}));

    EXPECT_THAT(min_good_value(multiply_type_by<int8_t>(-mag<63>())), SameTypeAndValue(int8_t{-2}));

    EXPECT_THAT(min_good_value(multiply_type_by<int8_t>(-mag<64>())), SameTypeAndValue(int8_t{-1}));
}

TEST(MultiplyTypeBy, MinGoodForUnlimitedFloatTimesPosIrrationalBiggerThanOneIsLowerLimitDivByMag) {
    EXPECT_THAT(min_good_value(multiply_type_by<float>(PI)),
                FloatEq(std::numeric_limits<float>::lowest() / get_value<float>(PI)));
}

TEST(MultiplyTypeBy, MinGoodForUnlimitedFloatTimesNegIrrationalBiggerThanOneIsUpperLimitDivByMag) {
    EXPECT_THAT(min_good_value(multiply_type_by<float>(-PI)),
                FloatEq(std::numeric_limits<float>::max() / get_value<float>(-PI)));
}

TEST(MultiplyTypeBy, MinGoodForUnlimitedFloatTimesPosIrrationalSmallerThanOneIsLowerLimit) {
    constexpr auto m = mag<1>() / PI;
    EXPECT_THAT(min_good_value(multiply_type_by<float>(m)),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));
}

TEST(MultiplyTypeBy, MinGoodForUnlimitedFloatTimesNegIrrationalSmallerThanOneIsNegUpperLimit) {
    constexpr auto m = -mag<1>() / PI;
    EXPECT_THAT(min_good_value(multiply_type_by<float>(m)),
                SameTypeAndValue(-std::numeric_limits<float>::max()));
}

TEST(MultiplyTypeBy, MinGoodForUnlimitedIntTimesPosIrrationalIsZeroAsAPlaceholder) {
    // We can't even compute the overflow boundary for this kind of operation yet, so just return an
    // extremely conservative result of 0.
    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(PI)), SameTypeAndValue(int32_t{0}));
}

TEST(MultiplyTypeBy, MinGoodForSignedTimesPosIntIsLowerLimitDivByMag) {
    struct I32LowerLimitMinus24 : NoUpperLimit<int32_t> {
        static constexpr int32_t lower() { return -24; }
    };

    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(mag<1>()), I32LowerLimitMinus24{}),
                SameTypeAndValue(int32_t{-24}));

    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(mag<8>()), I32LowerLimitMinus24{}),
                SameTypeAndValue(int32_t{-3}));

    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(mag<24>()), I32LowerLimitMinus24{}),
                SameTypeAndValue(int32_t{-1}));

    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(mag<25>()), I32LowerLimitMinus24{}),
                SameTypeAndValue(int32_t{0}));
}

TEST(MultiplyTypeBy, MinGoodForSignedTimesNegIntIsUpperLimitDivByMag) {
    struct I32UpperLimit24 : NoLowerLimit<int32_t> {
        static constexpr int32_t upper() { return 24; }
    };

    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(-mag<1>()), I32UpperLimit24{}),
                SameTypeAndValue(int32_t{-24}));

    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(-mag<8>()), I32UpperLimit24{}),
                SameTypeAndValue(int32_t{-3}));

    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(-mag<24>()), I32UpperLimit24{}),
                SameTypeAndValue(int32_t{-1}));

    EXPECT_THAT(min_good_value(multiply_type_by<int32_t>(-mag<25>()), I32UpperLimit24{}),
                SameTypeAndValue(int32_t{0}));
}

TEST(MultiplyTypeBy, MinGoodForFloatTimesPosIntIsLowerLimitDivByMag) {
    struct FloatLowerLimitMinus64 : NoUpperLimit<float> {
        static constexpr float lower() { return -64.0f; }
    };

    EXPECT_THAT(min_good_value(multiply_type_by<float>(mag<1>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(-64.0f));

    EXPECT_THAT(min_good_value(multiply_type_by<float>(mag<8>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(-8.0f));

    EXPECT_THAT(min_good_value(multiply_type_by<float>(mag<64>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(-1.0f));

    EXPECT_THAT(min_good_value(multiply_type_by<float>(mag<128>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(-0.5f));
}

TEST(MultiplyTypeBy, MinGoodForFloatTimesNegIntIsUpperLimitDivByMag) {
    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };

    EXPECT_THAT(min_good_value(multiply_type_by<float>(-mag<1>()), FloatUpperLimit64{}),
                SameTypeAndValue(-64.0f));

    EXPECT_THAT(min_good_value(multiply_type_by<float>(-mag<8>()), FloatUpperLimit64{}),
                SameTypeAndValue(-8.0f));

    EXPECT_THAT(min_good_value(multiply_type_by<float>(-mag<64>()), FloatUpperLimit64{}),
                SameTypeAndValue(-1.0f));

    EXPECT_THAT(min_good_value(multiply_type_by<float>(-mag<128>()), FloatUpperLimit64{}),
                SameTypeAndValue(-0.5f));
}

TEST(MultiplyTypeBy, MinGoodForFloatTimesPosIrrationalBiggerThanOneIsLowerLimitDivByMag) {
    struct FloatLowerLimitMinus64 : NoUpperLimit<float> {
        static constexpr float lower() { return -64.0f; }
    };

    EXPECT_THAT(min_good_value(multiply_type_by<float>(PI), FloatLowerLimitMinus64{}),
                FloatEq(-64.0f / get_value<float>(PI)));
}

TEST(MultiplyTypeBy, MinGoodForFloatTimesNegIrrationalBiggerThanOneIsUpperLimitDivByMag) {
    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };

    EXPECT_THAT(min_good_value(multiply_type_by<float>(-PI), FloatUpperLimit64{}),
                FloatEq(64.0f / get_value<float>(-PI)));
}

TEST(MultiplyTypeBy, MinGoodForFloatTimesPosIrrationalSmallerThanOneIsClampedLowerLimit) {
    struct FloatLowerLimitMinus64 : NoUpperLimit<float> {
        static constexpr float lower() { return -64.0f; }
    };

    constexpr auto m_no_clamping = mag<1>() / PI;
    EXPECT_THAT(min_good_value(multiply_type_by<float>(m_no_clamping), FloatLowerLimitMinus64{}),
                FloatEq(-64.0f / get_value<float>(m_no_clamping)));

    constexpr auto m_clamping = mag<16>() * PI / highest_floating_point_as_mag<float>();
    ASSERT_THAT(is_positive(m_clamping), IsTrue());
    EXPECT_THAT(min_good_value(multiply_type_by<float>(m_clamping), FloatLowerLimitMinus64{}),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));
}

TEST(MultiplyTypeBy, MinGoodForFloatTimesNegIrrationalSmallerThanOneIsClampedUpperLimit) {
    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };

    constexpr auto m_no_clamping = -mag<1>() / PI;
    EXPECT_THAT(min_good_value(multiply_type_by<float>(m_no_clamping), FloatUpperLimit64{}),
                FloatEq(64.0f / get_value<float>(m_no_clamping)));

    constexpr auto m_clamping = mag<16>() * PI / lowest_floating_point_as_mag<float>();
    ASSERT_THAT(is_positive(m_clamping), IsFalse());
    EXPECT_THAT(min_good_value(multiply_type_by<float>(m_clamping), FloatUpperLimit64{}),
                SameTypeAndValue(-std::numeric_limits<float>::max()));
}

TEST(MultiplyTypeBy, MinGoodForComplexOfTProvidesAnswerAsT) {
    EXPECT_THAT(min_good_value(multiply_type_by<std::complex<int32_t>>(mag<12>())),
                SameTypeAndValue(min_good_value(multiply_type_by<int32_t>(mag<12>()))));
}

//
// `MaxGood<MultiplyTypeBy>`:
//

TEST(MultiplyTypeBy, MaxGoodForUnsignedIsAlwaysZeroIfMagIsNegative) {
    EXPECT_THAT(max_good_value(multiply_type_by<uint8_t>(-mag<1>())), SameTypeAndValue(uint8_t{0}));

    EXPECT_THAT(max_good_value(multiply_type_by<uint16_t>(-mag<123>())),
                SameTypeAndValue(uint16_t{0}));

    EXPECT_THAT(max_good_value(multiply_type_by<uint32_t>(-mag<1>() / mag<234>())),
                SameTypeAndValue(uint32_t{0}));
}

TEST(MultiplyTypeBy, MaxGoodForUnlimitedUnsignedTimesPosIntIsUpperLimitDivByMag) {
    EXPECT_THAT(max_good_value(multiply_type_by<uint8_t>(mag<1>())),
                SameTypeAndValue(uint8_t{255}));

    EXPECT_THAT(max_good_value(multiply_type_by<uint8_t>(mag<127>())),
                SameTypeAndValue(uint8_t{2}));

    EXPECT_THAT(max_good_value(multiply_type_by<uint8_t>(mag<128>())),
                SameTypeAndValue(uint8_t{1}));

    EXPECT_THAT(max_good_value(multiply_type_by<uint8_t>(mag<255>())),
                SameTypeAndValue(uint8_t{1}));
}

TEST(MultiplyTypeBy, MaxGoodForUnlimitedSignedTimesPosIntIsUpperLimitDivByMag) {
    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(mag<1>())), SameTypeAndValue(int8_t{127}));

    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(mag<63>())), SameTypeAndValue(int8_t{2}));

    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(mag<64>())), SameTypeAndValue(int8_t{1}));

    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(mag<127>())), SameTypeAndValue(int8_t{1}));
}

TEST(MultiplyTypeBy, MaxGoodForUnlimitedSignedTimesNegIntIsLowerLimitDivByMag) {
    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(-mag<1>())), SameTypeAndValue(int8_t{127}));

    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(-mag<2>())), SameTypeAndValue(int8_t{64}));

    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(-mag<64>())), SameTypeAndValue(int8_t{2}));

    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(-mag<65>())), SameTypeAndValue(int8_t{1}));

    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(-mag<127>())), SameTypeAndValue(int8_t{1}));

    EXPECT_THAT(max_good_value(multiply_type_by<int8_t>(-mag<128>())), SameTypeAndValue(int8_t{1}));
}

TEST(MultiplyTypeBy, MaxGoodForUnlimitedFloatTimesPosIrrationalBiggerThanOneIsUpperLimitDivByMag) {
    EXPECT_THAT(max_good_value(multiply_type_by<float>(PI)),
                FloatEq(std::numeric_limits<float>::max() / get_value<float>(PI)));
}

TEST(MultiplyTypeBy, MaxGoodForUnlimitedFloatTimesNegIrrationalBiggerThanOneIsLowerLimitDivByMag) {
    EXPECT_THAT(max_good_value(multiply_type_by<float>(-PI)),
                FloatEq(std::numeric_limits<float>::lowest() / get_value<float>(-PI)));
}

TEST(MultiplyTypeBy, MaxGoodForUnlimitedFloatTimesPosIrrationalSmallerThanOneIsUpperLimit) {
    constexpr auto m = mag<1>() / PI;
    EXPECT_THAT(max_good_value(multiply_type_by<float>(m)),
                SameTypeAndValue(std::numeric_limits<float>::max()));
}

TEST(MultiplyTypeBy, MaxGoodForUnlimitedFloatTimesNegIrrationalSmallerThanOneIsNegLowerLimit) {
    constexpr auto m = -mag<1>() / PI;
    EXPECT_THAT(max_good_value(multiply_type_by<float>(m)),
                SameTypeAndValue(-std::numeric_limits<float>::lowest()));
}

TEST(MultiplyTypeBy, MaxGoodForSignedTimesPosIntIsUpperLimitDivByMag) {
    struct I32UpperLimit24 : NoLowerLimit<int32_t> {
        static constexpr int32_t upper() { return 24; }
    };

    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(mag<1>()), I32UpperLimit24{}),
                SameTypeAndValue(int32_t{24}));

    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(mag<8>()), I32UpperLimit24{}),
                SameTypeAndValue(int32_t{3}));

    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(mag<24>()), I32UpperLimit24{}),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(mag<25>()), I32UpperLimit24{}),
                SameTypeAndValue(int32_t{0}));
}

TEST(MultiplyTypeBy, MaxGoodForSignedTimesNegIntIsLowerLimitDivByMag) {
    struct I32LowerLimitMinus24 : NoUpperLimit<int32_t> {
        static constexpr int32_t lower() { return -24; }
    };

    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(-mag<1>()), I32LowerLimitMinus24{}),
                SameTypeAndValue(int32_t{24}));

    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(-mag<8>()), I32LowerLimitMinus24{}),
                SameTypeAndValue(int32_t{3}));

    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(-mag<24>()), I32LowerLimitMinus24{}),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(-mag<25>()), I32LowerLimitMinus24{}),
                SameTypeAndValue(int32_t{0}));
}

TEST(MultiplyTypeBy, MaxGoodForSignedTimesNumericLimitsLowestIsZeroIfNontrivialLowerLimit) {
    // Use the most liberal nontrivial lower limit imaginable.
    struct I32LowerLimitOfNegativeUpperLimit : NoUpperLimit<int32_t> {
        static constexpr int32_t lower() { return -std::numeric_limits<int32_t>::max(); }
    };

    constexpr auto I32_LOWEST =
        -mag<static_cast<uint64_t>(std::numeric_limits<int32_t>::max()) + 1u>();

    // To ensure test validity, make sure we get a nonzero value if the limits are trivial.
    ASSERT_THAT(max_good_value(multiply_type_by<int32_t>(I32_LOWEST)),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT(
        max_good_value(multiply_type_by<int32_t>(I32_LOWEST), I32LowerLimitOfNegativeUpperLimit{}),
        SameTypeAndValue(int32_t{0}));
}

TEST(MultiplyTypeBy, MaxGoodForUnlimitedIntTimesPosIrrationalIsZeroAsAPlaceholder) {
    // We can't even compute the overflow boundary for this kind of operation yet, so just return an
    // extremely conservative result of 0.
    EXPECT_THAT(max_good_value(multiply_type_by<int32_t>(PI)), SameTypeAndValue(int32_t{0}));
}

TEST(MultiplyTypeBy, MaxGoodForFloatTimesPosIntIsUpperLimitDivByMag) {
    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };

    EXPECT_THAT(max_good_value(multiply_type_by<float>(mag<1>()), FloatUpperLimit64{}),
                SameTypeAndValue(64.0f));

    EXPECT_THAT(max_good_value(multiply_type_by<float>(mag<8>()), FloatUpperLimit64{}),
                SameTypeAndValue(8.0f));

    EXPECT_THAT(max_good_value(multiply_type_by<float>(mag<64>()), FloatUpperLimit64{}),
                SameTypeAndValue(1.0f));

    EXPECT_THAT(max_good_value(multiply_type_by<float>(mag<128>()), FloatUpperLimit64{}),
                SameTypeAndValue(0.5f));
}

TEST(MultiplyTypeBy, MaxGoodForFloatTimesNegIntIsLowerLimitDivByMag) {
    struct FloatLowerLimitMinus64 : NoUpperLimit<float> {
        static constexpr float lower() { return -64.0f; }
    };

    EXPECT_THAT(max_good_value(multiply_type_by<float>(-mag<1>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(64.0f));

    EXPECT_THAT(max_good_value(multiply_type_by<float>(-mag<8>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(8.0f));

    EXPECT_THAT(max_good_value(multiply_type_by<float>(-mag<64>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(1.0f));

    EXPECT_THAT(max_good_value(multiply_type_by<float>(-mag<128>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(0.5f));
}

TEST(MultiplyTypeBy, MaxGoodForFloatTimesPosIrrationalBiggerThanOneIsUpperLimitDivByMag) {
    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };

    EXPECT_THAT(max_good_value(multiply_type_by<float>(PI), FloatUpperLimit64{}),
                FloatEq(64.0f / get_value<float>(PI)));
}

TEST(MultiplyTypeBy, MaxGoodForFloatTimesNegIrrationalBiggerThanOneIsLowerLimitDivByMag) {
    struct FloatLowerLimitMinus64 : NoUpperLimit<float> {
        static constexpr float lower() { return -64.0f; }
    };

    EXPECT_THAT(max_good_value(multiply_type_by<float>(-PI), FloatLowerLimitMinus64{}),
                FloatEq(-64.0f / get_value<float>(-PI)));
}

TEST(MultiplyTypeBy, MaxGoodForFloatTimesPosIrrationalSmallerThanOneIsClampedUpperLimit) {
    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };

    constexpr auto m_no_clamping = mag<1>() / PI;
    EXPECT_THAT(max_good_value(multiply_type_by<float>(m_no_clamping), FloatUpperLimit64{}),
                FloatEq(64.0f / get_value<float>(m_no_clamping)));

    constexpr auto m_clamping = mag<16>() * PI / highest_floating_point_as_mag<float>();
    ASSERT_THAT(is_positive(m_clamping), IsTrue());
    EXPECT_THAT(max_good_value(multiply_type_by<float>(m_clamping), FloatUpperLimit64{}),
                SameTypeAndValue(std::numeric_limits<float>::max()));
}

TEST(MultiplyTypeBy, MaxGoodForFloatTimesNegIrrationalSmallerThanOneIsClampedLowerLimit) {
    struct FloatLowerLimitMinus64 : NoUpperLimit<float> {
        static constexpr float lower() { return -64.0f; }
    };

    constexpr auto m_no_clamping = -mag<1>() / PI;
    EXPECT_THAT(max_good_value(multiply_type_by<float>(m_no_clamping), FloatLowerLimitMinus64{}),
                FloatEq(-64.0f / get_value<float>(m_no_clamping)));

    constexpr auto m_clamping = mag<16>() * PI / lowest_floating_point_as_mag<float>();
    ASSERT_THAT(is_positive(m_clamping), IsFalse());
    EXPECT_THAT(max_good_value(multiply_type_by<float>(m_clamping), FloatLowerLimitMinus64{}),
                SameTypeAndValue(-std::numeric_limits<float>::lowest()));
}

TEST(MultiplyTypeBy, MaxGoodForComplexOfTProvidesAnswerAsT) {
    EXPECT_THAT(max_good_value(multiply_type_by<std::complex<int32_t>>(mag<12>())),
                SameTypeAndValue(max_good_value(multiply_type_by<int32_t>(mag<12>()))));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger` section:

//
// `MinGood<DivideTypeByInteger>`:
//

TEST(DivideTypeByInteger, MinGoodForUnsignedIsAlwaysZero) {
    EXPECT_THAT(min_good_value(divide_type_by_integer<uint8_t>(mag<1>())),
                SameTypeAndValue(uint8_t{0}));

    EXPECT_THAT(min_good_value(divide_type_by_integer<uint16_t>(mag<123>())),
                SameTypeAndValue(uint16_t{0}));
}

TEST(DivideTypeByInteger, MinGoodForSignedDivByPosIntIsCappedLowerLimitTimesMagInv) {
    struct I8LowerLimitMinus16 : NoUpperLimit<int8_t> {
        static constexpr int8_t lower() { return -16; }
    };

    EXPECT_THAT(min_good_value(divide_type_by_integer<int8_t>(mag<2>()), I8LowerLimitMinus16{}),
                SameTypeAndValue(int8_t{-32}));

    EXPECT_THAT(min_good_value(divide_type_by_integer<int8_t>(mag<8>()), I8LowerLimitMinus16{}),
                SameTypeAndValue(int8_t{-128}));

    // Clamped case.
    EXPECT_THAT(min_good_value(divide_type_by_integer<int8_t>(mag<9>()), I8LowerLimitMinus16{}),
                SameTypeAndValue(int8_t{-128}));
}

TEST(DivideTypeByInteger, MinGoodForSignedDivByNegativeIntIsCappedUpperLimitTimesMagInv) {
    struct I8UpperLimit16 : NoLowerLimit<int8_t> {
        static constexpr int8_t upper() { return 16; }
    };

    EXPECT_THAT(min_good_value(divide_type_by_integer<int8_t>(-mag<2>()), I8UpperLimit16{}),
                SameTypeAndValue(int8_t{-32}));

    EXPECT_THAT(min_good_value(divide_type_by_integer<int8_t>(-mag<8>()), I8UpperLimit16{}),
                SameTypeAndValue(int8_t{-128}));

    // Clamped case.
    EXPECT_THAT(min_good_value(divide_type_by_integer<int8_t>(-mag<9>()), I8UpperLimit16{}),
                SameTypeAndValue(int8_t{-128}));
}

TEST(DivideTypeByInteger, MinGoodForFloatDivByPosIntIsCappedLowerLimitTimesMagInv) {
    struct FloatLowerLimitMinus64 : NoUpperLimit<float> {
        static constexpr float lower() { return -64.0f; }
    };

    EXPECT_THAT(min_good_value(divide_type_by_integer<float>(mag<2>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(-128.0f));

    EXPECT_THAT(min_good_value(divide_type_by_integer<float>(mag<8>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(-512.0f));

    // Clamped cases.
    constexpr auto m = highest_floating_point_as_mag<float>() / mag<64>();
    ASSERT_THAT(is_integer(m), IsTrue());
    EXPECT_THAT(
        min_good_value(divide_type_by_integer<float>(m / mag<2>()), FloatLowerLimitMinus64{}),
        SameTypeAndValue(std::numeric_limits<float>::lowest() / 2.0f));
    EXPECT_THAT(min_good_value(divide_type_by_integer<float>(m), FloatLowerLimitMinus64{}),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));
    EXPECT_THAT(
        min_good_value(divide_type_by_integer<float>(m * mag<2>()), FloatLowerLimitMinus64{}),
        SameTypeAndValue(std::numeric_limits<float>::lowest()));
}

TEST(DivideTypeByInteger, MinGoodForFloatDivByNegIntIsCappedUpperLimitTimesMagInv) {
    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };

    EXPECT_THAT(min_good_value(divide_type_by_integer<float>(-mag<2>()), FloatUpperLimit64{}),
                SameTypeAndValue(-128.0f));

    EXPECT_THAT(min_good_value(divide_type_by_integer<float>(-mag<8>()), FloatUpperLimit64{}),
                SameTypeAndValue(-512.0f));

    // Clamped cases.
    constexpr auto m = lowest_floating_point_as_mag<float>() / mag<64>();
    ASSERT_THAT(is_integer(m), IsTrue());
    EXPECT_THAT(min_good_value(divide_type_by_integer<float>(m / mag<2>()), FloatUpperLimit64{}),
                SameTypeAndValue(std::numeric_limits<float>::lowest() / 2.0f));
    EXPECT_THAT(min_good_value(divide_type_by_integer<float>(m), FloatUpperLimit64{}),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));
    EXPECT_THAT(min_good_value(divide_type_by_integer<float>(m * mag<2>()), FloatUpperLimit64{}),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));
}

TEST(DivideTypeByInteger, MinGoodForComplexOfTProvidesAnswerAsT) {
    EXPECT_THAT(min_good_value(divide_type_by_integer<std::complex<int32_t>>(mag<12>())),
                SameTypeAndValue(min_good_value(divide_type_by_integer<int32_t>(mag<12>()))));
}

//
// `MaxGood<DivideTypeByInteger>`:
//

TEST(DivideTypeByInteger, MaxGoodForUnsignedIsAlwaysZeroIfMagIsNegative) {
    EXPECT_THAT(max_good_value(divide_type_by_integer<uint8_t>(-mag<1>())),
                SameTypeAndValue(uint8_t{0}));

    EXPECT_THAT(max_good_value(divide_type_by_integer<uint16_t>(-mag<123>())),
                SameTypeAndValue(uint16_t{0}));
}

TEST(DivideTypeByInteger, MaxGoodForUnlimitedUnsignedDivByPosIntIsUpperLimit) {
    EXPECT_THAT(max_good_value(divide_type_by_integer<uint8_t>(mag<1>())),
                SameTypeAndValue(uint8_t{255}));

    EXPECT_THAT(max_good_value(divide_type_by_integer<uint8_t>(mag<2>())),
                SameTypeAndValue(uint8_t{255}));

    EXPECT_THAT(max_good_value(divide_type_by_integer<uint8_t>(mag<8>())),
                SameTypeAndValue(uint8_t{255}));

    EXPECT_THAT(max_good_value(divide_type_by_integer<uint8_t>(mag<255>())),
                SameTypeAndValue(uint8_t{255}));
}

TEST(DivideTypeByInteger, MaxGoodForIntDivByTooBigNumberIsUpperLimitOfType) {
    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(mag<128>())),
                SameTypeAndValue(int8_t{127}));

    struct Int8UpperLimit50 : NoLowerLimit<int8_t> {
        static constexpr int8_t upper() { return 50; }
    };
    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(mag<128>()), Int8UpperLimit50{}),
                SameTypeAndValue(int8_t{127}));
}

TEST(DivideTypeByInteger, MaxGoodForFloatDivByTooBigNumberIsUpperLimitOfType) {
    constexpr auto m = pow<40>(mag<10>());
    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(m)),
                SameTypeAndValue(std::numeric_limits<float>::max()));

    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };
    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(m), FloatUpperLimit64{}),
                SameTypeAndValue(std::numeric_limits<float>::max()));
}

TEST(DivideTypeByInteger, MaxGoodForUnlimitedSignedDivNegIntIsClampedLowerLimit) {
    EXPECT_THAT(max_good_value(divide_type_by_integer<int>(-mag<12>())),
                SameTypeAndValue(std::numeric_limits<int>::max()));
}

TEST(DivideTypeByInteger, MaxGoodForSignedDivByPosIntIsCappedUpperLimitTimesMagInv) {
    struct I8UpperLimit16 : NoLowerLimit<int8_t> {
        static constexpr int8_t upper() { return 16; }
    };

    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(mag<2>()), I8UpperLimit16{}),
                SameTypeAndValue(int8_t{32}));

    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(mag<7>()), I8UpperLimit16{}),
                SameTypeAndValue(int8_t{112}));

    // Clamped cases.
    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(mag<8>()), I8UpperLimit16{}),
                SameTypeAndValue(int8_t{127}));
    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(mag<9>()), I8UpperLimit16{}),
                SameTypeAndValue(int8_t{127}));
}

TEST(DivideTypeByInteger, MaxGoodForSignedDivByNegativeIntIsCappedLowerLimitTimesMagInv) {
    struct I8LowerLimitMinus16 : NoUpperLimit<int8_t> {
        static constexpr int8_t lower() { return -16; }
    };

    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(-mag<2>()), I8LowerLimitMinus16{}),
                SameTypeAndValue(int8_t{32}));

    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(-mag<7>()), I8LowerLimitMinus16{}),
                SameTypeAndValue(int8_t{112}));

    // Clamped cases.
    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(-mag<8>()), I8LowerLimitMinus16{}),
                SameTypeAndValue(int8_t{127}));
    EXPECT_THAT(max_good_value(divide_type_by_integer<int8_t>(-mag<9>()), I8LowerLimitMinus16{}),
                SameTypeAndValue(int8_t{127}));
}

TEST(DivideTypeByInteger, MaxGoodForFloatDivByPosIntIsCappedUpperLimitTimesMagInv) {
    struct FloatUpperLimit64 : NoLowerLimit<float> {
        static constexpr float upper() { return 64.0f; }
    };

    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(mag<2>()), FloatUpperLimit64{}),
                SameTypeAndValue(128.0f));

    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(mag<8>()), FloatUpperLimit64{}),
                SameTypeAndValue(512.0f));

    // Clamped cases.
    constexpr auto m = highest_floating_point_as_mag<float>() / mag<64>();
    ASSERT_THAT(is_integer(m), IsTrue());
    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(m / mag<2>()), FloatUpperLimit64{}),
                SameTypeAndValue(std::numeric_limits<float>::max() / 2.0f));
    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(m), FloatUpperLimit64{}),
                SameTypeAndValue(std::numeric_limits<float>::max()));
    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(m * mag<2>()), FloatUpperLimit64{}),
                SameTypeAndValue(std::numeric_limits<float>::max()));
}

TEST(DivideTypeByInteger, MaxGoodForFloatDivByNegIntIsCappedLowerLimitTimesMagInv) {
    struct FloatLowerLimitMinus64 : NoUpperLimit<float> {
        static constexpr float lower() { return -64.0f; }
    };

    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(-mag<2>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(128.0f));

    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(-mag<8>()), FloatLowerLimitMinus64{}),
                SameTypeAndValue(512.0f));

    // Clamped cases.
    constexpr auto m = lowest_floating_point_as_mag<float>() / mag<64>();
    ASSERT_THAT(is_integer(m), IsTrue());
    EXPECT_THAT(
        max_good_value(divide_type_by_integer<float>(m / mag<2>()), FloatLowerLimitMinus64{}),
        SameTypeAndValue(std::numeric_limits<float>::max() / 2.0f));
    EXPECT_THAT(max_good_value(divide_type_by_integer<float>(m), FloatLowerLimitMinus64{}),
                SameTypeAndValue(std::numeric_limits<float>::max()));
    EXPECT_THAT(
        max_good_value(divide_type_by_integer<float>(m * mag<2>()), FloatLowerLimitMinus64{}),
        SameTypeAndValue(std::numeric_limits<float>::max()));
}

TEST(DivideTypeByInteger, MaxGoodForComplexOfTProvidesAnswerAsT) {
    EXPECT_THAT(max_good_value(divide_type_by_integer<std::complex<int32_t>>(mag<12>())),
                SameTypeAndValue(max_good_value(divide_type_by_integer<int32_t>(mag<12>()))));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence` section:

//
// `MinGood<OpSequence>`:
//

TEST(OpSequence, MinGoodForSequenceOfSingleOpIsMinGoodForThatOp) {
    auto expect_min_good_for_sequence_of_only_this_is_min_good_for_this = [](auto op) {
        EXPECT_THAT(min_good_value(op_sequence(op)), SameTypeAndValue(min_good_value(op)));
    };

    expect_min_good_for_sequence_of_only_this_is_min_good_for_this(
        multiply_type_by<uint8_t>(mag<1>()));

    expect_min_good_for_sequence_of_only_this_is_min_good_for_this(StaticCast<int16_t, float>{});
}

TEST(OpSequence, MinGoodForDivideThenNarrowIsLimitsOfTypeIfDivisorIsBigEnough) {
    EXPECT_THAT(min_good_value(op_sequence(divide_type_by_integer<int16_t>(mag<1000>()),
                                           StaticCast<int16_t, int8_t>{})),
                SameTypeAndValue(std::numeric_limits<int16_t>::min()));
}

TEST(OpSequence, MinGoodForDivideThenNarrowIsScaledUpDestinationBoundIfDivisorIsSmallEnough) {
    EXPECT_THAT(min_good_value(op_sequence(divide_type_by_integer<int16_t>(mag<10>()),
                                           StaticCast<int16_t, int8_t>{})),
                SameTypeAndValue(int16_t{-1280}));
}

TEST(OpSequence, MinGoodOfStaticCastSequenceIsMostConstrainingType) {
    EXPECT_THAT(min_good_value(op_sequence(StaticCast<int64_t, float>{},
                                           StaticCast<float, int32_t>{},
                                           StaticCast<int32_t, int16_t>{},
                                           StaticCast<int16_t, double>{})),
                SameTypeAndValue(static_cast<int64_t>(std::numeric_limits<int16_t>::min())));
}

TEST(OpSequence, MinGoodIsZeroIfUnsignedTypeFoundOnBothSidesOfNegativeMultiplication) {
    EXPECT_THAT(min_good_value(op_sequence(StaticCast<int64_t, float>{},
                                           StaticCast<float, uint32_t>{},
                                           StaticCast<uint32_t, int16_t>{},
                                           multiply_type_by<int16_t>(-mag<1>() / mag<234>()),
                                           StaticCast<int16_t, double>{},
                                           StaticCast<double, uint8_t>{},
                                           StaticCast<uint8_t, int32_t>{})),
                SameTypeAndValue(int64_t{0}));
}

//
// `MaxGood<OpSequence>`:
//

TEST(OpSequence, MaxGoodForSequenceOfSingleOpIsMaxGoodForThatOp) {
    auto expect_max_good_for_sequence_of_only_this_is_max_good_for_this = [](auto op) {
        EXPECT_THAT(max_good_value(op_sequence(op)), SameTypeAndValue(max_good_value(op)));
    };

    expect_max_good_for_sequence_of_only_this_is_max_good_for_this(
        multiply_type_by<uint8_t>(mag<1>()));

    expect_max_good_for_sequence_of_only_this_is_max_good_for_this(StaticCast<int16_t, float>{});
}

TEST(OpSequence, MaxGoodForDivideThenNarrowIsLimitsOfTypeIfDivisorIsBigEnough) {
    EXPECT_THAT(max_good_value(op_sequence(divide_type_by_integer<uint16_t>(mag<1000>()),
                                           StaticCast<uint16_t, uint8_t>{})),
                SameTypeAndValue(std::numeric_limits<uint16_t>::max()));
}

TEST(OpSequence, MaxGoodForDivideThenNarrowIsScaledDownDestinationBoundIfDivisorIsSmallEnough) {
    EXPECT_THAT(max_good_value(op_sequence(divide_type_by_integer<uint16_t>(mag<10>()),
                                           StaticCast<uint16_t, uint8_t>{})),
                SameTypeAndValue(uint16_t{2550}));
}

TEST(OpSequence, MaxGoodOfStaticCastSequenceIsMostConstrainingType) {
    EXPECT_THAT(max_good_value(op_sequence(StaticCast<int64_t, float>{},
                                           StaticCast<float, uint32_t>{},
                                           StaticCast<uint32_t, int16_t>{},
                                           StaticCast<int16_t, double>{})),
                SameTypeAndValue(static_cast<int64_t>(std::numeric_limits<int16_t>::max())));
}

TEST(OpSequence, MaxGoodIsZeroIfUnsignedTypeFoundOnBothSidesOfNegativeMultiplication) {
    EXPECT_THAT(max_good_value(op_sequence(StaticCast<int64_t, float>{},
                                           StaticCast<float, uint32_t>{},
                                           StaticCast<uint32_t, int16_t>{},
                                           divide_type_by_integer<int16_t>(-mag<234>()),
                                           StaticCast<int16_t, double>{},
                                           StaticCast<double, uint8_t>{},
                                           StaticCast<uint8_t, int32_t>{})),
                SameTypeAndValue(int64_t{0}));
}

TEST(OpSequence, DividingByTooBigNumberResetsTheLimitToTheMax) {
    // We are multiplying a promotable integer type by a rational magnitude, whose denominator is
    // too big to fit even in the promoted type.  Steps are:
    //
    // 1. Static cast to the promoted type.
    // 2. Multiply by numerator.
    // 3. Divide by (huge) denominator.
    // 4. Static cast back to the original type.
    //
    // Step 4 imposes a limit of the max of the (tiny) original type.  But in dividing by the (huge)
    // denominator in step 3, _every_ value will end up in the range of the destination type
    // (because they'll all be trivial: 0), so the limit should expand to be the max of the promoted
    // type.  We can tell the difference because step 2 multiplies by an integer, whose effect on
    // the _limit_ is to _divide_ by that integer.  The key is to make sure we're dividing that
    // expanded limit, and not the tiny limit of the original type.
    EXPECT_THAT(max_good_value(op_sequence(StaticCast<int8_t, int>{},
                                           multiply_type_by<int>(mag<3>()),
                                           divide_type_by_integer<int>(pow<400>(mag<10>())),
                                           StaticCast<int, int8_t>{})),
                SameTypeAndValue(std::numeric_limits<int8_t>::max()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanOverflowBelow` section:

TEST(CanOverflowBelow, TrueIfValueCanBeSmallEnoughToGoOutsideBounds) {
    EXPECT_THAT(can_overflow_below(multiply_type_by<int8_t>(mag<2>())), IsTrue());
}

TEST(CanOverflowBelow, TrueForOverflowableStdComplex) {
    EXPECT_THAT(can_overflow_below(multiply_type_by<std::complex<int8_t>>(mag<2>())), IsTrue());
}

TEST(CanOverflowBelow, FalseIfValueCannotBeSmallEnoughToGoOutsideBounds) {
    EXPECT_THAT(can_overflow_below(multiply_type_by<uint8_t>(mag<8>())), IsFalse());
    EXPECT_THAT(can_overflow_below(multiply_type_by<double>(mag<1>() / PI)), IsFalse());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanOverflowAbove` section:

TEST(CanOverflowAbove, TrueIfValueCanBeBigEnoughToGoOutsideBounds) {
    EXPECT_THAT(can_overflow_above(multiply_type_by<int8_t>(mag<2>())), IsTrue());
}

TEST(CanOverflowAbove, TrueForOverflowableStdComplex) {
    EXPECT_THAT(can_overflow_above(multiply_type_by<std::complex<int8_t>>(mag<2>())), IsTrue());
}

TEST(CanOverflowAbove, FalseIfValueCannotBeBigEnoughToGoOutsideBounds) {
    EXPECT_THAT(can_overflow_above(divide_type_by_integer<uint8_t>(mag<8>())), IsFalse());
    EXPECT_THAT(can_overflow_above(multiply_type_by<double>(-mag<1>() / PI)), IsFalse());
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `clamped_negate()` section

TEST(ClampedNegate, SimplyNegatesWhenLimitsOfTypeAreNotRelevant) {
    EXPECT_THAT(clamped_negate(15), SameTypeAndValue(-15));
    EXPECT_THAT(clamped_negate(-15), SameTypeAndValue(15));
}

TEST(ClampedNegate, ClampsSignedIntMinToIntMax) {
    EXPECT_THAT(clamped_negate(int8_t{-128}), SameTypeAndValue(int8_t{127}));

    EXPECT_THAT(clamped_negate(int16_t{-32768}), SameTypeAndValue(int16_t{32767}));

    EXPECT_THAT(clamped_negate(int32_t{-2147483648}), SameTypeAndValue(int32_t{2147483647}));
}

TEST(ClampedNegate, MapsAnyUnsignedInputToZero) {
    EXPECT_THAT(clamped_negate(123u), SameTypeAndValue(0u));

    EXPECT_THAT(clamped_negate(uint64_t{123'456'789'012'345'678u}), SameTypeAndValue(uint64_t{0}));
}

TEST(ClampedNegate, SupportsFloatingPointBySimplyNegating) {
    EXPECT_THAT(clamped_negate(3.14f), FloatEq(-3.14f));
}

}  // namespace
}  // namespace detail
}  // namespace au
