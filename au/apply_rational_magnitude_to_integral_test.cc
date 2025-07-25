// Copyright 2023 Aurora Operations, Inc.
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
#include "au/testing.hh"
#include "gtest/gtest.h"

using ::testing::AllOf;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::Lt;
using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {
namespace {

//
// This file held the unit tests for an older library, `:apply_rational_magnitude_to_integral`,
// which we no longer need. We have retained the test file so that we can still get value out of all
// of the pre-existing test cases.  The first part of this file simply re-implements the
// functionality of the old library very concisely, in a few lines of code that use the replacement
// libraries.
//

template <typename... BPs>
constexpr void ensure_relevant_kind_of_magnitude(Magnitude<BPs...> m) {
    static_assert(is_rational(m), "Magnitude must be rational");
    static_assert(!is_integer(m), "Magnitude must not be purely integer");
    static_assert(!is_integer(ONE / m), "Magnitude must not be purely inverse-integer");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test cases for maximum non-overflowing value.
//
// What are the canonical representative situations for the overflow boundary of `x * N / D`, where
// `x` has type `T`, `TM` is the maximum value of type `T`, and `PM` is the maximum value of type
// `PromotedType<T>`?  We list them here.  Note that they're listed in a kind of "if/else-if" order:
// each number's condition assumes that all of the previous conditions are false.  (So for a test
// case to fall into category "3", it can't be in category "1" or "2".)
//
// 1. `N` cannot fit inside `PromotedType<T>`.  This means any nonzero value always overflows: the
//    max is 0.
//
// 2. `N < D` (whether or not `D` can fit inside `PromotedType<T>`).  This means that the final
//    result will be smaller than the input, and therefore will always fit inside `T` --- as long as
//    the intermediate calculation `(x * N)` doesn't overflow.  So the maximum non-overflowing value
//    is `min(PM / N, TM)`.
//
// 3. `N > D` (all other cases).  The only step that can overflow is the multiplication with `N`.
//    There are two conditions we must meet to avoid overflow.  First, we must avoid going over the
//    limit of the promoted type.  Second, we must ensure that the product is within the limits of
//    the original type after dividing by `D`.  So the maximum non-overflowing value is the smaller
//    of `PM` and `TM * D`, divided by `N`.  (We'll have to be careful that the `TM * D` itself
//    doesn't overflow!  We'll cap it at `PM` if it does.)

//
// `MaxNonOverflowingValue<T, MagT>` is the maximum value of type `T` that can have `MagT` applied
// as numerator-and-denominator without overflowing.  We require that `T` is some integral
// arithmetic type, and that `MagT` is a rational magnitude that is neither purely integral nor
// purely inverse-integral.
//
// This implementation has been migrated from the target (which no longer exists) that this test was
// for.  We hollowed it out and replaced it with a simple implementation that delegates to the
// replacement library.  This lets us get coverage from all of our old test cases.
//
template <typename T, typename MagT>
struct MaxNonOverflowingValue : MaxGood<ConversionForRepsAndFactor<T, T, MagT>> {};

enum class IsPromotable { NO, YES };
enum class NumFitsInPromotedType { NO, YES };
enum class DenFitsInPromotedType { NO, YES };
struct TestSpec {
    IsPromotable is_promotable;
    NumFitsInPromotedType num_fits;
    DenFitsInPromotedType den_fits;
};

//
// `MinNonOverflowingValue<T, MagT>` is the minimum (i.e., most-negative) value of type `T` that can
// have `MagT` applied as numerator-and-denominator without overflowing (i.e., becoming too-negative
// to represent).  We require that `T` is some integral arithmetic type, and that `MagT` is a
// rational magnitude that is neither purely integral nor purely inverse-integral.
//
// This implementation has been migrated from the target (which no longer exists) that this test was
// for.  We hollowed it out and replaced it with a simple implementation that delegates to the
// replacement library.  This lets us get coverage from all of our old test cases.
//
template <typename T, typename MagT>
struct MinNonOverflowingValue : MinGood<ConversionForRepsAndFactor<T, T, MagT>> {};

template <typename T, typename MagT>
void validate_spec(TestSpec spec) {
    using PromotedT = PromotedType<T>;
    const bool is_promotable = !std::is_same<T, PromotedT>::value;
    const bool is_expected_to_be_promotable = (spec.is_promotable == IsPromotable::YES);
    ASSERT_THAT(is_promotable, Eq(is_expected_to_be_promotable))
        << "Expected a type that " << (is_expected_to_be_promotable ? "is" : "is not")
        << " promotable; got a type that " << (is_promotable ? "is" : "is not");

    using Num = decltype(numerator(MagT{}));
    constexpr auto num_value_result = get_value_result<PromotedT>(Num{});
    const bool is_num_representable = (num_value_result.outcome == MagRepresentationOutcome::OK);
    const bool is_num_expected_to_be_representable = (spec.num_fits == NumFitsInPromotedType::YES);
    ASSERT_THAT(is_num_representable, Eq(is_num_expected_to_be_representable))
        << "Expected numerator " << (is_num_expected_to_be_representable ? "to be" : "not to be")
        << " representable in promoted type; it " << (is_num_representable ? "is" : "is not");

    using Den = decltype(denominator(MagT{}));
    constexpr auto den_value_result = get_value_result<PromotedT>(Den{});
    const bool is_den_representable = (den_value_result.outcome == MagRepresentationOutcome::OK);
    const bool is_den_expected_to_be_representable = (spec.den_fits == DenFitsInPromotedType::YES);
    ASSERT_THAT(is_den_representable, Eq(is_den_expected_to_be_representable))
        << "Expected denominator " << (is_den_expected_to_be_representable ? "to be" : "not to be")
        << " representable in promoted type; it " << (is_den_representable ? "is" : "is not");
}

template <typename T, typename MagT>
void populate_max_non_overflowing_value(TestSpec spec, MagT, T &result_out) {
    validate_spec<T, MagT>(spec);
    result_out = MaxNonOverflowingValue<T, MagT>::value();
}

TEST(MaxNonOverflowingValue, AlwaysZeroIfNumCannotFitInPromotedType) {
    // Case "1" above.
    constexpr auto huge = pow<400>(mag<10>()) / mag<3>();

    {
        int max_int = 123;
        populate_max_non_overflowing_value(
            {IsPromotable::NO, NumFitsInPromotedType::NO, DenFitsInPromotedType::YES},
            huge,
            max_int);
        EXPECT_THAT(max_int, Eq(0));
    }
    {
        uint16_t max_u16 = 123u;
        populate_max_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::NO, DenFitsInPromotedType::YES},
            huge,
            max_u16);
        EXPECT_THAT(max_u16, Eq(0));
    }
}

TEST(MaxNonOverflowingValue, ZeroIfFactorIsNegativeAndTypeIsUnsigned) {
    // Alternative region of Case "1" above.

    constexpr auto ratio = -mag<2>() / mag<3>();

    {
        uint64_t max_u64 = 123u;
        populate_max_non_overflowing_value(
            {IsPromotable::NO, NumFitsInPromotedType::NO, DenFitsInPromotedType::YES},
            ratio,
            max_u64);
        EXPECT_THAT(max_u64, Eq(0u));
    }
    {
        uint32_t max_u32 = 123u;
        populate_max_non_overflowing_value(
            {IsPromotable::NO, NumFitsInPromotedType::NO, DenFitsInPromotedType::YES},
            ratio,
            max_u32);
        EXPECT_THAT(max_u32, Eq(0u));
    }
    {
        uint16_t max_u16 = 123u;
        populate_max_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
            ratio,
            max_u16);
        EXPECT_THAT(max_u16, Eq(0u));
    }
    {
        uint8_t max_u8 = 123u;
        populate_max_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
            ratio,
            max_u8);
        EXPECT_THAT(max_u8, Eq(0u));
    }
}

TEST(MaxNonOverflowingValue, IsMaxTDividedByNWhenTIsNotPromotableAndDenomOverflows) {
    // Case "2" above.  a) Overflowing denominator, non-promotable types only.
    constexpr auto huge_denom = mag<3>() / pow<400>(mag<10>());

    {
        int max_int = 0;
        populate_max_non_overflowing_value(
            {IsPromotable::NO, NumFitsInPromotedType::YES, DenFitsInPromotedType::NO},
            huge_denom,
            max_int);
        EXPECT_THAT(max_int, Eq(std::numeric_limits<int>::max() / 3));
    }

    {
        uint64_t max_u64 = 0;
        populate_max_non_overflowing_value(
            {IsPromotable::NO, NumFitsInPromotedType::YES, DenFitsInPromotedType::NO},
            huge_denom,
            max_u64);
        EXPECT_THAT(max_u64, Eq(std::numeric_limits<uint64_t>::max() / 3));
    }
}

TEST(MaxNonOverflowingValue,
     IsSmallerOfMaxPromotedTDividedByNAndMaxTWhenTIsPromotableAndDenomOverflows) {
    // Case "2" above.  b) Overflowing denominator, promotable types.
    {
        int8_t max_i8 = 0;
        populate_max_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::NO},
            mag<3>() / pow<400>(mag<10>()),
            max_i8);
        EXPECT_THAT(max_i8, Eq(127));
    }

    {
        uint16_t max_u16 = 0;
        populate_max_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::NO},
            mag<1'000'000>() / pow<400>(mag<11>()),
            max_u16);

        ASSERT_THAT((std::is_same<PromotedType<uint16_t>, int32_t>::value), IsTrue())
            << "This test will fail on architectures where uint16_t is not promoted to `int32_t`";
        EXPECT_THAT(max_u16, Eq(2'147));
    }
}

TEST(MaxNonOverflowingValue,
     IsSmallerOfMaxPromotedTDividedByNAndMaxTWhenTIsPromotableAndNLessThanD) {
    // Case "2" above.  c) Non-overflowing denominator.

    {
        uint8_t max_u8 = 0;
        populate_max_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
            mag<3>() / mag<10>(),
            max_u8);
        EXPECT_THAT(max_u8, Eq(255));
    }

    {
        uint16_t max_u16 = 0;
        populate_max_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
            mag<1'000'000>() / pow<6>(mag<11>()),
            max_u16);
        EXPECT_THAT(max_u16, Eq(2'147));
    }
}

TEST(MaxNonOverflowingValue, IsPromotedMaxOverNWhenNIsLargeAndDIsSlightlySmaller) {
    // Case 3 above (partial).
    //
    // When `N / D` is very slightly larger than 1, then (PM / N) can be the most constraining.
    // We'll use a promoted type just to make this interesting by making PM different from TM.
    uint16_t max_u16 = 0;
    populate_max_non_overflowing_value(
        {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
        mag<1'000'000>() / mag<999'999>(),
        max_u16);
    ASSERT_THAT((std::is_same<PromotedType<uint16_t>, int32_t>::value), IsTrue())
        << "This test will fail on architectures where `uint16_t` is not promoted to `int32_t`";
    EXPECT_THAT(max_u16, Eq(2'147));
}

TEST(MaxNonOverflowingValue, IsTMaxOverNTimesDWhenMoreConstrainingThanPMaxOverN) {
    // Case 3 above (partial).
    //
    // The goal is to engineer a test case where `(TM * D) / N` is more constraining than `PM / N`.
    // Obviously, if `TM = PM`, then this can never be; therefore, we need to use a promotable type.
    int16_t max_int16 = 0;
    populate_max_non_overflowing_value(
        {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
        mag<1'000>() / mag<3>(),
        max_int16);
    ASSERT_THAT((std::is_same<PromotedType<int16_t>, int32_t>::value), IsTrue())
        << "This test will fail on architectures where `int16_t` is not promoted to `int32_t`";
    ASSERT_THAT(98, Eq(32'768 * 3 / 1'000));
    EXPECT_THAT(max_int16, Eq(98));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Test cases for minimum (i.e. most-negative) non-overflowing value.
//
// The test cases are the mirror image of the above, except that we only check signed types.

template <typename T, typename MagT>
void populate_min_non_overflowing_value(TestSpec spec, MagT, T &result_out) {
    validate_spec<T, MagT>(spec);
    result_out = MinNonOverflowingValue<T, MagT>::value();
}

TEST(MinNonOverflowingValue, AlwaysZeroIfNumCannotFitInPromotedType) {
    constexpr auto huge = pow<400>(mag<10>()) / mag<3>();

    {
        int min_int = 123;
        populate_min_non_overflowing_value(
            {IsPromotable::NO, NumFitsInPromotedType::NO, DenFitsInPromotedType::YES},
            huge,
            min_int);
        EXPECT_THAT(min_int, Eq(0));
    }

    {
        int16_t min_i16 = 123;
        populate_min_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::NO, DenFitsInPromotedType::YES},
            huge,
            min_i16);
        EXPECT_THAT(min_i16, Eq(0));
    }
}

TEST(MinNonOverflowingValue, IsMinTDividedByNWhenTIsNotPromotableAndDenomOverflows) {
    constexpr auto huge_denom = mag<3>() / pow<400>(mag<10>());

    {
        int min_int = 0;
        populate_min_non_overflowing_value(
            {IsPromotable::NO, NumFitsInPromotedType::YES, DenFitsInPromotedType::NO},
            huge_denom,
            min_int);
        EXPECT_THAT(min_int, Eq(std::numeric_limits<int>::lowest() / 3));
    }

    {
        int64_t min_i64 = 0;
        populate_min_non_overflowing_value(
            {IsPromotable::NO, NumFitsInPromotedType::YES, DenFitsInPromotedType::NO},
            huge_denom,
            min_i64);
        EXPECT_THAT(min_i64, Eq(std::numeric_limits<int64_t>::lowest() / 3));
    }
}

TEST(MinNonOverflowingValue,
     IsSmallerOfMinPromotedTDividedByNAndMinTWhenTIsPromotableAndDenomOverflows) {
    {
        int8_t min_i8 = 0;
        populate_min_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::NO},
            mag<3>() / pow<400>(mag<10>()),
            min_i8);
        EXPECT_THAT(min_i8, Eq(-128));
    }

    {
        int16_t min_i16 = 0;
        populate_min_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::NO},
            mag<1'000'000>() / pow<400>(mag<11>()),
            min_i16);

        ASSERT_THAT((std::is_same<PromotedType<int16_t>, int32_t>::value), IsTrue())
            << "This test will fail on architectures where `int16_t` is not promoted to `int32_t`";
        EXPECT_THAT(min_i16, Eq(-2'147));
    }
}

TEST(MinNonOverflowingValue,
     IsSmallerOfMinPromotedTDividedByNAndMinTWhenTIsPromotableAndNLessThanD) {

    {
        int8_t min_i8 = 0;
        populate_min_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
            mag<3>() / mag<10>(),
            min_i8);
        EXPECT_THAT(min_i8, Eq(-128));
    }

    {
        int16_t min_i16 = 0;
        populate_min_non_overflowing_value(
            {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
            mag<1'000'000>() / pow<6>(mag<11>()),
            min_i16);
        EXPECT_THAT(min_i16, Eq(-2'147));
    }
}

TEST(MinNonOverflowingValue, IsPromotedMinOverNWhenNIsLargeAndDIsSlightlySmaller) {
    // When `N / D` is very slightly larger than 1, then (PM / N) can be the most constraining.
    // We'll use a promoted type just to make this interesting by making PM different from TM.
    int16_t min_i16 = 0;
    populate_min_non_overflowing_value(
        {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
        mag<1'000'000>() / mag<999'999>(),
        min_i16);

    ASSERT_THAT((std::is_same<PromotedType<int16_t>, int32_t>::value), IsTrue())
        << "This test will fail on architectures where `int16_t` is not promoted to `int32_t`";
    EXPECT_THAT(min_i16, Eq(-2'147));
}

TEST(MinNonOverflowingValue, IsTMinOverNTimesDWhenMoreConstrainingThanPMinOverN) {
    // The goal is to engineer a test case where `(TM * D) / N` is more constraining than `PM / N`.
    // Obviously, if `TM = PM`, then this can never be; therefore, we need to use a promotable type.
    int16_t min_int16 = 0;
    populate_min_non_overflowing_value(
        {IsPromotable::YES, NumFitsInPromotedType::YES, DenFitsInPromotedType::YES},
        mag<1'000>() / mag<3>(),
        min_int16);
    ASSERT_THAT(-98, Eq(-32'768 * 3 / 1'000));
    EXPECT_THAT(min_int16, Eq(-98));
}

}  // namespace
}  // namespace detail
}  // namespace au
