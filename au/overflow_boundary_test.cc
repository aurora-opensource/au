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

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast` section:

//
// `MinGood<StaticCast>`:
//

TEST(StaticCast, MinGoodIsLowestIfDestinationEqualsSource) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, int8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<uint16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint16_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<float, float>>::value()),
                Eq(std::numeric_limits<float>::lowest()));
}

TEST(StaticCast, MinGoodIsLowestIfCastWidens) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, int16_t>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<uint8_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<float, double>>::value()),
                Eq(std::numeric_limits<float>::lowest()));
}

TEST(StaticCast, MinGoodIsZeroFromAnySignedToAnyUnsigned) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, uint64_t>>::value()), SameTypeAndValue(int8_t{0}));
    EXPECT_THAT((MinGood<StaticCast<int16_t, uint8_t>>::value()), SameTypeAndValue(int16_t{0}));
    EXPECT_THAT((MinGood<StaticCast<int32_t, uint32_t>>::value()), SameTypeAndValue(int32_t{0}));
}

TEST(StaticCast, MinGoodIsZeroFromAnyUnsignedToAnyArithmetic) {
    EXPECT_THAT((MinGood<StaticCast<uint8_t, int64_t>>::value()), Eq(uint8_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint16_t, uint8_t>>::value()), Eq(uint16_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint32_t, int16_t>>::value()), Eq(uint32_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint64_t, int64_t>>::value()), Eq(uint64_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint64_t, float>>::value()), Eq(uint64_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint8_t, double>>::value()), Eq(uint8_t{0}));
}

TEST(StaticCast, MinGoodIsLowestInDestinationWhenNarrowingToSameFamily) {
    EXPECT_THAT((MinGood<StaticCast<int64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<int64_t>(std::numeric_limits<int32_t>::lowest())));
    EXPECT_THAT((MinGood<StaticCast<double, float>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::lowest())));
}

TEST(StaticCast, MinGoodIsZeroFromAnyFloatingPointToAnyUnsigned) {
    EXPECT_THAT((MinGood<StaticCast<double, uint8_t>>::value()), SameTypeAndValue(0.0));
    EXPECT_THAT((MinGood<StaticCast<float, uint64_t>>::value()), SameTypeAndValue(0.0f));
}

TEST(StaticCast, MinGoodIsLowestInDestinationFromAnyFloatingPointToAnySigned) {
    EXPECT_THAT((MinGood<StaticCast<double, int32_t>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<int32_t>::lowest())));
    EXPECT_THAT((MinGood<StaticCast<float, int64_t>>::value()),
                SameTypeAndValue(static_cast<float>(std::numeric_limits<int64_t>::lowest())));
}

TEST(StaticCast, MinGoodIsLowestFromAnySignedToAnyFloatingPoint) {
    // We could imagine some hypothetical floating point and integral types for which this is not
    // true.  But floating point is designed to cover a very wide range between its min and max
    // values, and in practice, this is true for all commonly used floating point and integral
    // types.
    EXPECT_THAT((MinGood<StaticCast<int8_t, double>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<int64_t, float>>::value()),
                Eq(std::numeric_limits<int64_t>::lowest()));
}

TEST(StaticCast, MinGoodUnchangedWithExplicitLimitOfLowestInTargetType) {
    // What all these test cases have in common is that the destination type is already the most
    // constraining factor.  Therefore, the only way to add an _explicit_ limit, which nevertheless
    // does _not_ constrain the answer, is to make that explicit limit equal to the implicit limit:
    // that is, the lowest value of the destination type.

    EXPECT_THAT((MinGood<StaticCast<int8_t, int8_t>, ImplicitLimits<int8_t>>::value()),
                Eq(MinGood<StaticCast<int8_t, int8_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<uint16_t, uint16_t>, ImplicitLimits<uint16_t>>::value()),
                Eq(MinGood<StaticCast<uint16_t, uint16_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<float, float>, ImplicitLimits<float>>::value()),
                Eq(MinGood<StaticCast<float, float>>::value()));

    EXPECT_THAT((MinGood<StaticCast<uint32_t, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq(MinGood<StaticCast<uint32_t, int32_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<int64_t, uint64_t>, ImplicitLimits<uint64_t>>::value()),
                Eq(MinGood<StaticCast<int64_t, uint64_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<double, float>, ImplicitLimits<float>>::value()),
                Eq(MinGood<StaticCast<double, float>>::value()));

    EXPECT_THAT((MinGood<StaticCast<float, uint64_t>, ImplicitLimits<uint64_t>>::value()),
                Eq(MinGood<StaticCast<float, uint64_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<float, int64_t>, ImplicitLimits<int64_t>>::value()),
                Eq(MinGood<StaticCast<float, int64_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<float, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq(MinGood<StaticCast<float, int32_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<uint32_t, uint16_t>, ImplicitLimits<uint16_t>>::value()),
                Eq(MinGood<StaticCast<uint32_t, uint16_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<uint32_t, int8_t>, ImplicitLimits<int8_t>>::value()),
                Eq(MinGood<StaticCast<uint32_t, int8_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<int64_t, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq(MinGood<StaticCast<int64_t, int32_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<int64_t, uint32_t>, ImplicitLimits<uint32_t>>::value()),
                Eq(MinGood<StaticCast<int64_t, uint32_t>>::value()));
}

TEST(StaticCast, MinGoodUnchangedWithExplicitLimitLessConstrainingThanExistingResult) {
    // In these cases, we are applying a non-trivial lower limit (i.e., it is higher than the
    // `lowest()` value), but it does not constrain the result enough to change it.

    struct DoubleLimitTwiceFloatLowest : NoUpperLimit<double> {
        static constexpr double lower() {
            return static_cast<double>(std::numeric_limits<float>::lowest()) * 2.0;
        }
    };

    EXPECT_THAT((MinGood<StaticCast<float, double>, DoubleLimitTwiceFloatLowest>::value()),
                Eq(MinGood<StaticCast<float, double>>::value()));

    EXPECT_THAT((MinGood<StaticCast<int32_t, double>, DoubleLimitTwiceFloatLowest>::value()),
                Eq(MinGood<StaticCast<int32_t, double>>::value()));

    EXPECT_THAT((MinGood<StaticCast<uint16_t, double>, DoubleLimitTwiceFloatLowest>::value()),
                Eq(MinGood<StaticCast<uint16_t, double>>::value()));

    struct FloatLimitHalfFloatLowest : NoUpperLimit<float> {
        static constexpr float lower() { return std::numeric_limits<float>::lowest() / 2.0f; }
    };

    EXPECT_THAT((MinGood<StaticCast<uint64_t, float>, FloatLimitHalfFloatLowest>::value()),
                Eq(MinGood<StaticCast<uint64_t, float>>::value()));

    EXPECT_THAT((MinGood<StaticCast<int64_t, float>, FloatLimitHalfFloatLowest>::value()),
                Eq(MinGood<StaticCast<int64_t, float>>::value()));

    struct SignedLimitHalfInt64Lowest : NoUpperLimit<int64_t> {
        static constexpr int64_t lower() { return std::numeric_limits<int64_t>::lowest() / 2; }
    };

    EXPECT_THAT((MinGood<StaticCast<uint32_t, int64_t>, SignedLimitHalfInt64Lowest>::value()),
                Eq(MinGood<StaticCast<uint32_t, int64_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<int32_t, int64_t>, SignedLimitHalfInt64Lowest>::value()),
                Eq(MinGood<StaticCast<int32_t, int64_t>>::value()));
}

TEST(StaticCast, MinGoodUnchangedForUnsignedDestinationAndExplicitLimitOfZero) {
    EXPECT_THAT((MinGood<StaticCast<uint8_t, uint16_t>, LowerLimitOfZero<uint16_t>>::value()),
                Eq(MinGood<StaticCast<uint8_t, uint16_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<int32_t, uint64_t>, LowerLimitOfZero<uint64_t>>::value()),
                Eq(MinGood<StaticCast<int32_t, uint64_t>>::value()));

    EXPECT_THAT((MinGood<StaticCast<double, uint32_t>, LowerLimitOfZero<uint32_t>>::value()),
                Eq(MinGood<StaticCast<double, uint32_t>>::value()));
}

TEST(StaticCast, MinGoodCappedByExplicitFloatLimit) {
    struct FloatLowerLimitMinusOne : NoUpperLimit<float> {
        static constexpr float lower() { return -1.0f; }
    };

    EXPECT_THAT((MinGood<StaticCast<int16_t, float>, FloatLowerLimitMinusOne>::value()),
                SameTypeAndValue(int16_t{-1}));

    EXPECT_THAT((MinGood<StaticCast<int64_t, float>, FloatLowerLimitMinusOne>::value()),
                SameTypeAndValue(int64_t{-1}));

    EXPECT_THAT((MinGood<StaticCast<float, float>, FloatLowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0f));

    EXPECT_THAT((MinGood<StaticCast<double, float>, FloatLowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0));
}

TEST(StaticCast, MinGoodCappedByExplicitDoubleLimit) {
    struct DoubleLowerLimitMinusOne : NoUpperLimit<double> {
        static constexpr double lower() { return -1.0; }
    };

    EXPECT_THAT((MinGood<StaticCast<float, double>, DoubleLowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0f));
}

TEST(StaticCast, MinGoodCappedByExplicitI64Limit) {
    struct I64LowerLimitMinusOne : NoUpperLimit<int64_t> {
        static constexpr int64_t lower() { return -1; }
    };

    EXPECT_THAT((MinGood<StaticCast<int32_t, int64_t>, I64LowerLimitMinusOne>::value()),
                SameTypeAndValue(int32_t{-1}));

    EXPECT_THAT((MinGood<StaticCast<int64_t, int64_t>, I64LowerLimitMinusOne>::value()),
                SameTypeAndValue(int64_t{-1}));

    EXPECT_THAT((MinGood<StaticCast<float, int64_t>, I64LowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0f));
}

TEST(StaticCast, MinGoodCappedByExplicitI16Limit) {
    struct I16LowerLimitMinusOne : NoUpperLimit<int16_t> {
        static constexpr int16_t lower() { return -1; }
    };

    EXPECT_THAT((MinGood<StaticCast<int32_t, int16_t>, I16LowerLimitMinusOne>::value()),
                SameTypeAndValue(int32_t{-1}));

    EXPECT_THAT((MinGood<StaticCast<double, int16_t>, I16LowerLimitMinusOne>::value()),
                SameTypeAndValue(-1.0));
}

TEST(StaticCast, MinGoodForComplexOfTProvidesAnswerAsT) {
    EXPECT_THAT((MinGood<StaticCast<std::complex<float>, std::complex<double>>>::value()),
                SameTypeAndValue(std::numeric_limits<float>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<std::complex<double>, std::complex<float>>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::lowest())));
}

//
// `MaxGood<StaticCast>`:
//

TEST(StaticCast, MaxGoodIsHighestIfDestinationEqualsSource) {
    EXPECT_THAT((MaxGood<StaticCast<int8_t, int8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<uint16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint16_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<float, float>>::value()),
                Eq(std::numeric_limits<float>::max()));
}

TEST(StaticCast, MaxGoodIsHighestIfCastWidens) {
    EXPECT_THAT((MaxGood<StaticCast<int8_t, int16_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<uint8_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<float, double>>::value()),
                Eq(std::numeric_limits<float>::max()));
}

TEST(StaticCast, MaxGoodIsHighestFromSignedToUnsignedOfSameSize) {
    EXPECT_THAT((MaxGood<StaticCast<int8_t, uint8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<int16_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint32_t>>::value()),
                Eq(std::numeric_limits<int32_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, uint64_t>>::value()),
                Eq(std::numeric_limits<int64_t>::max()));
}

TEST(StaticCast, MaxGoodIsHighestInDestinationFromUnsignedToSignedOfSameSize) {
    EXPECT_THAT((MaxGood<StaticCast<uint8_t, int8_t>>::value()),
                SameTypeAndValue(static_cast<uint8_t>(std::numeric_limits<int8_t>::max())));

    EXPECT_THAT((MaxGood<StaticCast<uint64_t, int64_t>>::value()),
                SameTypeAndValue(static_cast<uint64_t>(std::numeric_limits<int64_t>::max())));
}

TEST(StaticCast, MaxGoodIsHighestFromAnyIntToAnyLargerInt) {
    EXPECT_THAT((MaxGood<StaticCast<uint8_t, int16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint64_t>>::value()),
                Eq(std::numeric_limits<int32_t>::max()));
}

TEST(StaticCast, MaxGoodIsHighestInDestinationFromAnyIntToAnySmallerInt) {
    EXPECT_THAT((MaxGood<StaticCast<uint16_t, uint8_t>>::value()),
                SameTypeAndValue(static_cast<uint16_t>(std::numeric_limits<uint8_t>::max())));
    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint16_t>>::value()),
                SameTypeAndValue(static_cast<int32_t>(std::numeric_limits<uint16_t>::max())));
    EXPECT_THAT((MaxGood<StaticCast<uint64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<uint64_t>(std::numeric_limits<int32_t>::max())));
}

TEST(StaticCast, MaxGoodIsHighestInDestinationWhenNarrowingToSameFamily) {
    EXPECT_THAT((MaxGood<StaticCast<uint16_t, uint8_t>>::value()),
                SameTypeAndValue(static_cast<uint16_t>(std::numeric_limits<uint8_t>::max())));
    EXPECT_THAT((MaxGood<StaticCast<int64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<int64_t>(std::numeric_limits<int32_t>::max())));
    EXPECT_THAT((MaxGood<StaticCast<double, float>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::max())));
}

TEST(StaticCast, MaxGoodIsHighestInDestinationFromAnyFloatingPointToAnySmallIntegral) {
    // The precondition for this test is that the max for the (destination) integral type is
    // _exactly_ representable in the (source) floating point type.  This helper will double check
    // this assumption.
    auto expect_max_good_is_exact_representation_of_destination_int_max = [](auto float_type_val,
                                                                             auto int_type_val) {
        using Float = decltype(float_type_val);
        using Int = decltype(int_type_val);

        constexpr auto expected = static_cast<Float>(std::numeric_limits<Int>::max());

        ASSERT_THAT(static_cast<Int>(expected), Eq(std::numeric_limits<Int>::max()));
        EXPECT_THAT((MaxGood<StaticCast<Float, Int>>::value()), SameTypeAndValue(expected));
    };

    expect_max_good_is_exact_representation_of_destination_int_max(double{}, uint8_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, int8_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, uint16_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, int16_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, uint32_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, int32_t{});

    expect_max_good_is_exact_representation_of_destination_int_max(float{}, uint8_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(float{}, int8_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(float{}, uint16_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(float{}, int16_t{});
}

TEST(StaticCast, MaxGoodIsHighestRepresentableFloatBelowCastedIntMaxForTooBigInt) {
    // `float` to 64-bit integer:
    EXPECT_THAT((MaxGood<StaticCast<float, int64_t>>::value()),
                SameTypeAndValue(
                    std::nextafter(static_cast<float>(std::numeric_limits<int64_t>::max()), 1.0f)));
    EXPECT_THAT((MaxGood<StaticCast<float, uint64_t>>::value()),
                SameTypeAndValue(std::nextafter(
                    static_cast<float>(std::numeric_limits<uint64_t>::max()), 1.0f)));

    // `double` to 64-bit integer:
    EXPECT_THAT((MaxGood<StaticCast<double, int64_t>>::value()),
                SameTypeAndValue(
                    std::nextafter(static_cast<double>(std::numeric_limits<int64_t>::max()), 1.0)));
    EXPECT_THAT((MaxGood<StaticCast<double, uint64_t>>::value()),
                SameTypeAndValue(std::nextafter(
                    static_cast<double>(std::numeric_limits<uint64_t>::max()), 1.0)));
}

TEST(StaticCast, MaxGoodIsHighestFromAnyIntegralToAnyFloatingPoint) {
    // See comments in `MinGoodIsLowestFromAnySignedToAnyFloatingPoint` for more on the assumptions
    // we're making here.

    EXPECT_THAT((MaxGood<StaticCast<int8_t, double>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<uint8_t, double>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, float>>::value()),
                Eq(std::numeric_limits<int64_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<uint64_t, float>>::value()),
                Eq(std::numeric_limits<uint64_t>::max()));
}

TEST(StaticCast, MaxGoodUnchangedWithExplicitLimitOfHighestInTargetType) {
    // What all these test cases have in common is that the destination type is already the most
    // constraining factor.  Therefore, the only way to add an _explicit_ limit, which nevertheless
    // does _not_ constrain the answer, is to make that explicit limit equal to the implicit limit:
    // that is, the highest value of the destination type.

    EXPECT_THAT((MaxGood<StaticCast<int8_t, int8_t>, ImplicitLimits<int8_t>>::value()),
                Eq(MaxGood<StaticCast<int8_t, int8_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<uint16_t, uint16_t>, ImplicitLimits<uint16_t>>::value()),
                Eq(MaxGood<StaticCast<uint16_t, uint16_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<float, float>, ImplicitLimits<float>>::value()),
                Eq(MaxGood<StaticCast<float, float>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq(MaxGood<StaticCast<uint32_t, int32_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<double, float>, ImplicitLimits<float>>::value()),
                Eq(MaxGood<StaticCast<double, float>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<float, uint64_t>, ImplicitLimits<uint64_t>>::value()),
                Eq(MaxGood<StaticCast<float, uint64_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<float, int64_t>, ImplicitLimits<int64_t>>::value()),
                Eq(MaxGood<StaticCast<float, int64_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<double, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq(MaxGood<StaticCast<double, int32_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<double, uint32_t>, ImplicitLimits<uint32_t>>::value()),
                Eq(MaxGood<StaticCast<double, uint32_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, uint16_t>, ImplicitLimits<uint16_t>>::value()),
                Eq(MaxGood<StaticCast<uint32_t, uint16_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, int8_t>, ImplicitLimits<int8_t>>::value()),
                Eq(MaxGood<StaticCast<uint32_t, int8_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, int32_t>, ImplicitLimits<int32_t>>::value()),
                Eq(MaxGood<StaticCast<int64_t, int32_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, uint32_t>, ImplicitLimits<uint32_t>>::value()),
                Eq(MaxGood<StaticCast<int64_t, uint32_t>>::value()));
}

TEST(StaticCast, MaxGoodUnchangedWithExplicitLimitLessConstrainingThanExistingResult) {
    // In these cases, we are applying a non-trivial upper limit (i.e., it is lower than the
    // `max()` value), but it does not constrain the result enough to change it.

    struct DoubleLimitTwiceFloatMax : NoLowerLimit<double> {
        static constexpr double upper() {
            return static_cast<double>(std::numeric_limits<float>::max()) * 2.0;
        }
    };

    EXPECT_THAT((MaxGood<StaticCast<float, double>, DoubleLimitTwiceFloatMax>::value()),
                Eq(MaxGood<StaticCast<float, double>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, double>, DoubleLimitTwiceFloatMax>::value()),
                Eq(MaxGood<StaticCast<int32_t, double>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<uint16_t, double>, DoubleLimitTwiceFloatMax>::value()),
                Eq(MaxGood<StaticCast<uint16_t, double>>::value()));

    struct FloatLimitHalfFloatMax : NoLowerLimit<float> {
        static constexpr float upper() { return std::numeric_limits<float>::max() / 2.0f; }
    };

    EXPECT_THAT((MaxGood<StaticCast<uint64_t, float>, FloatLimitHalfFloatMax>::value()),
                Eq(MaxGood<StaticCast<uint64_t, float>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, float>, FloatLimitHalfFloatMax>::value()),
                Eq(MaxGood<StaticCast<int64_t, float>>::value()));

    struct SignedLimitHalfInt64Max : NoLowerLimit<int64_t> {
        static constexpr int64_t upper() { return std::numeric_limits<int64_t>::max() / 2; }
    };

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, int64_t>, SignedLimitHalfInt64Max>::value()),
                Eq(MaxGood<StaticCast<uint32_t, int64_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, int64_t>, SignedLimitHalfInt64Max>::value()),
                Eq(MaxGood<StaticCast<int32_t, int64_t>>::value()));

    struct UnsignedLimitUint64MaxMinusTwo : NoLowerLimit<uint64_t> {
        static constexpr uint64_t upper() { return std::numeric_limits<uint64_t>::max() - 2; }
    };

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, uint64_t>, UnsignedLimitUint64MaxMinusTwo>::value()),
                Eq(MaxGood<StaticCast<uint32_t, uint64_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint64_t>, UnsignedLimitUint64MaxMinusTwo>::value()),
                Eq(MaxGood<StaticCast<int32_t, uint64_t>>::value()));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, uint64_t>, UnsignedLimitUint64MaxMinusTwo>::value()),
                Eq(MaxGood<StaticCast<int64_t, uint64_t>>::value()));
}

TEST(StaticCast, MaxGoodCappedByExplicitFloatLimit) {
    struct FloatUpperLimitOne : NoLowerLimit<float> {
        static constexpr float upper() { return 1.0f; }
    };

    EXPECT_THAT((MaxGood<StaticCast<int16_t, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(int16_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<uint16_t, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(uint16_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(int64_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<uint64_t, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(uint64_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<float, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(1.0f));

    EXPECT_THAT((MaxGood<StaticCast<double, float>, FloatUpperLimitOne>::value()),
                SameTypeAndValue(1.0));
}

TEST(StaticCast, MaxGoodCappedByExplicitDoubleLimit) {
    struct DoubleUpperLimitOne : NoLowerLimit<double> {
        static constexpr double upper() { return 1.0; }
    };

    EXPECT_THAT((MaxGood<StaticCast<float, double>, DoubleUpperLimitOne>::value()),
                SameTypeAndValue(1.0f));
}

TEST(StaticCast, MaxGoodCappedByExplicitU64Limit) {
    struct U64UpperLimitOne : NoLowerLimit<uint64_t> {
        static constexpr uint64_t upper() { return 1; }
    };

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(uint32_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<uint64_t, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(uint64_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(int64_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<float, uint64_t>, U64UpperLimitOne>::value()),
                SameTypeAndValue(1.0f));
}

TEST(StaticCast, MaxGoodCappedByExplicitI64Limit) {
    struct I64UpperLimitOne : NoLowerLimit<int64_t> {
        static constexpr int64_t upper() { return 1; }
    };

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(uint32_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<uint64_t, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(uint64_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(int64_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<float, int64_t>, I64UpperLimitOne>::value()),
                SameTypeAndValue(1.0f));
}

TEST(StaticCast, MaxGoodCappedByExplicitI16Limit) {
    struct I16UpperLimitOne : NoLowerLimit<int16_t> {
        static constexpr int16_t upper() { return 1; }
    };

    EXPECT_THAT((MaxGood<StaticCast<int32_t, int16_t>, I16UpperLimitOne>::value()),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, int16_t>, I16UpperLimitOne>::value()),
                SameTypeAndValue(uint32_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<double, int16_t>, I16UpperLimitOne>::value()),
                SameTypeAndValue(1.0));
}

TEST(StaticCast, MaxGoodCappedByExplicitU16Limit) {
    struct U16UpperLimitOne : NoLowerLimit<uint16_t> {
        static constexpr uint16_t upper() { return 1; }
    };

    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint16_t>, U16UpperLimitOne>::value()),
                SameTypeAndValue(int32_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<uint32_t, uint16_t>, U16UpperLimitOne>::value()),
                SameTypeAndValue(uint32_t{1}));

    EXPECT_THAT((MaxGood<StaticCast<double, uint16_t>, U16UpperLimitOne>::value()),
                SameTypeAndValue(1.0));
}

TEST(StaticCast, MaxGoodForComplexOfTProvidesAnswerAsT) {
    EXPECT_THAT((MaxGood<StaticCast<std::complex<float>, std::complex<double>>>::value()),
                SameTypeAndValue(std::numeric_limits<float>::max()));

    EXPECT_THAT((MaxGood<StaticCast<std::complex<double>, std::complex<float>>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::max())));
}

}  // namespace
}  // namespace detail
}  // namespace au
