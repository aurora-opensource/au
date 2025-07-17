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

#include "fuzz/quantity_runtime_conversion_checkers.hh"

#include <tuple>
#include <type_traits>

#include "au/conversion_strategy.hh"
#include "au/quantity.hh"
#include "au/testing.hh"
#include "au/units/inches.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/yards.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::StaticAssertTypeEq;

using ::au::detail::FloatingPointPrefixPart;
using ::au::detail::MultiplyTypeBy;
using ::au::detail::OpSequence;
using ::au::detail::StaticCast;

TEST(PrependToEach, PrependsElementToEachPack) {
    StaticAssertTypeEq<PrependToEach<int, ::testing::Types<>>, ::testing::Types<>>();
    StaticAssertTypeEq<
        PrependToEach<int, ::testing::Types<std::tuple<char>, std::tuple<double, float>>>,
        ::testing::Types<std::tuple<int, char>, std::tuple<int, double, float>>>();
}

TEST(Flatten, ConcatenatesPacksOfSameType) {
    StaticAssertTypeEq<Flatten<::testing::Types<>>, ::testing::Types<>>();
    StaticAssertTypeEq<Flatten<::testing::Types<>, ::testing::Types<>>, ::testing::Types<>>();
}

TEST(CartesianProduct, AppliesPackToEachElementOfSinglePack) {
    StaticAssertTypeEq<CartesianProduct<std::tuple, ::testing::Types<int, double, float>>,
                       ::testing::Types<std::tuple<int>, std::tuple<double>, std::tuple<float>>>();
}

TEST(CartesianProduct, CombinesMultiplePacksIntoASinglePackWithAllCombinations) {
    StaticAssertTypeEq<
        CartesianProduct<std::tuple, ::testing::Types<int, double>, ::testing::Types<float, char>>,
        ::testing::Types<std::tuple<int, float>,
                         std::tuple<int, char>,
                         std::tuple<double, float>,
                         std::tuple<double, char>>>();
}

TEST(CartesianProduct, CanHandleMultipleLayers) {
    struct A_1;
    struct A_2;

    struct B_1;
    struct B_2;

    struct C_1;
    struct C_2;

    StaticAssertTypeEq<CartesianProduct<std::tuple,

                                        ::testing::Types<A_1, A_2>,
                                        ::testing::Types<B_1, B_2>,
                                        ::testing::Types<C_1, C_2>>,

                       ::testing::Types<std::tuple<A_1, B_1, C_1>,
                                        std::tuple<A_1, B_1, C_2>,
                                        std::tuple<A_1, B_2, C_1>,
                                        std::tuple<A_1, B_2, C_2>,
                                        std::tuple<A_2, B_1, C_1>,
                                        std::tuple<A_2, B_1, C_2>,
                                        std::tuple<A_2, B_2, C_1>,
                                        std::tuple<A_2, B_2, C_2>>>();
}

template <typename T>
class QuantityRuntimeConversionChecker : public ::testing::Test {};

TYPED_TEST_SUITE_P(QuantityRuntimeConversionChecker);

TYPED_TEST_P(QuantityRuntimeConversionChecker, RoundTripIsIdentityIffConversionNotLossy) {
    using Rep = std::tuple_element_t<0, TypeParam>;
    constexpr auto destination_unit = std::tuple_element_t<1, TypeParam>{};
    RandomValueGenerator<Rep> generator{9876543210u};
    for (auto i = 0u; i < 1'000'000u; ++i) {
        const auto value = meters(generator.next_value());

        const bool expect_loss = is_conversion_lossy(value, destination_unit);

        const auto round_trip = value.coerce_as(destination_unit).coerce_as(meters);
        const bool actual_loss = (value != round_trip);

        EXPECT_EQ(expect_loss, actual_loss) << "Value: " << value << " Round trip: " << round_trip;
    }
}

REGISTER_TYPED_TEST_SUITE_P(QuantityRuntimeConversionChecker,
                            RoundTripIsIdentityIffConversionNotLossy);

using IntTypes =
    ::testing::Types<uint64_t, int64_t, uint32_t, int32_t, uint16_t, int16_t, uint8_t, int8_t>;
using DestinationUnits = ::testing::Types<Inches, Yards, Miles>;

using TypesToTest = CartesianProduct<std::tuple, IntTypes, DestinationUnits>;
INSTANTIATE_TYPED_TEST_SUITE_P(X, QuantityRuntimeConversionChecker, TypesToTest, );

TEST(FloatingPointPrefixPart, EmptyForSequenceThatStartsAsIntegral) {
    StaticAssertTypeEq<FloatingPointPrefixPart<OpSequence<StaticCast<int, double>>>,
                       OpSequence<>>();
}

TEST(FloatingPointPrefixPart, DiscardsFromFirstOperationThatExitsFloatingPoint) {
    StaticAssertTypeEq<
        FloatingPointPrefixPart<OpSequence<StaticCast<float, double>,
                                           MultiplyTypeBy<double, decltype(mag<2>())>,
                                           StaticCast<double, int>,
                                           MultiplyTypeBy<int, decltype(mag<3>())>>>,
        OpSequence<StaticCast<float, double>, MultiplyTypeBy<double, decltype(mag<2>())>>>();
}

}  // namespace
}  // namespace au
