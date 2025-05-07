// Copyright 2022 Aurora Operations, Inc.
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

#include <type_traits>

#include "au/chrono_policy_validation.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace au {

using ::testing::IsTrue;

namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Machinery to gather the spectrum of all comparison results between two values, and compare them
// for correspondence.

struct AllComparisons {
    bool eq;
    bool ne;
    bool lt;
    bool le;
    bool gt;
    bool ge;
};

bool operator==(const AllComparisons &a, const AllComparisons &b) {
    auto all_fields = [](const AllComparisons &x) {
        return std::tie(x.eq, x.ne, x.lt, x.le, x.gt, x.ge);
    };
    return all_fields(a) == all_fields(b);
}

template <typename T, typename U>
AllComparisons compare(const T &t, const U &u) {
    AllComparisons result;
    result.eq = (t == u);
    result.ne = (t != u);
    result.lt = (t < u);
    result.le = (t <= u);
    result.gt = (t > u);
    result.ge = (t >= u);
    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Individual operations to validate.

// Aggregate results for all comparison operations.
struct Comparison {
    template <typename T, typename U>
    constexpr auto operator()(T t, U u) const
        -> decltype(compare(t, u)) {  // Trailing return enables SFINAE.
        return compare(t, u);
    }
};

// Implicit assignment (no explicit casting).
struct Assignment {
    template <typename T, typename U>
    constexpr auto operator()(T t, U u) const
        -> std::remove_reference_t<decltype(t = u)> {  // Trailing return enables SFINAE.
        return (t = u);
    }
};

// Addition of mixed (but same-dimension) types.
struct Addition {
    template <typename T, typename U>
    constexpr auto operator()(T t, U u) const
        -> decltype(t + u) {  // Trailing return enables SFINAE.
        return (t + u);
    }
};

// Subtraction of mixed (but same-dimension) types.
struct Subtraction {
    template <typename T, typename U>
    constexpr auto operator()(T t, U u) const
        -> decltype(t - u) {  // Trailing return enables SFINAE.
        return (t - u);
    }
};

TEST(Assignment, ReturnsExpectedValue) { EXPECT_EQ(2s, Assignment{}(1s, 2s)); }
TEST(Addition, ReturnsExpectedValue) { EXPECT_EQ(1001ms, Addition{}(1s, 1ms)); }
TEST(Subtraction, ReturnsExpectedValue) { EXPECT_EQ(999ms, Subtraction{}(1s, 1ms)); }

}  // namespace

TEST(Comparison, EnabledForArbitraryFloatingPointReps) {
    EXPECT_THAT(
        both_permit<Comparison>(std::chrono::duration<double>(1), std::chrono::duration<double>(1)),
        IsTrue());

    EXPECT_THAT(
        both_permit<Comparison>(std::chrono::duration<double>(1), std::chrono::duration<double>(2)),
        IsTrue());

    EXPECT_THAT(
        both_permit<Comparison>(std::chrono::duration<float>(1), std::chrono::duration<float>(2)),
        IsTrue());

    EXPECT_THAT(both_permit<Comparison>(std::chrono::duration<float>(1),
                                        std::chrono::duration<float, std::milli>(1'000)),
                IsTrue());
    EXPECT_THAT(both_permit<Comparison>(std::chrono::duration<float>(1.001),
                                        std::chrono::duration<float, std::milli>(1'000)),
                IsTrue());
    EXPECT_THAT(both_permit<Comparison>(std::chrono::duration<float>(0.999),
                                        std::chrono::duration<float, std::milli>(1'000)),
                IsTrue());
}

TEST(Comparison, EnabledForReasonableCombosOfUnitsAndIntegralReps) {
    EXPECT_THAT(both_permit<Comparison>(1s, 1000ms), IsTrue());
    EXPECT_THAT(both_permit<Comparison>(1s, 1001ms), IsTrue());
    EXPECT_THAT(both_permit<Comparison>(1s, 999ms), IsTrue());
    EXPECT_THAT(both_permit<Comparison>(1'000'000'000ns, 1000ms), IsTrue());

    EXPECT_THAT(both_permit<Comparison>(std::chrono::duration<int16_t, std::milli>{50},
                                        std::chrono::duration<int64_t, std::micro>{50'001}),
                IsTrue());
}

TEST(Comparison, EnabledForMixedIntegralFloatingPointReps) {
    EXPECT_THAT(both_permit<Comparison>(1.0s, 1000ms), IsTrue());
    EXPECT_THAT(both_permit<Comparison>(1.0s, std::chrono::duration<uint64_t, std::milli>{1000}),
                IsTrue());
}

TEST(Assignment, EnabledForInt64IffScaleWidening) {
    EXPECT_THAT(both_permit<Assignment>(std::chrono::milliseconds{}, 1s), IsTrue());
    EXPECT_THAT(both_forbid<Assignment>(std::chrono::seconds{}, 1ms), IsTrue());
}

TEST(Assignment, EnabledForSameScaleIntegralTypes) {
    EXPECT_THAT(both_permit<Assignment>(std::chrono::duration<int64_t>{},
                                        std::chrono::duration<int32_t>{1}),
                IsTrue());
    EXPECT_THAT(both_permit<Assignment>(std::chrono::duration<int32_t>{},
                                        std::chrono::duration<int64_t>{1}),
                IsTrue());
}

TEST(Assignment, DisabledForOverflowRiskyIntegralConversions) {
    EXPECT_THAT(
        chrono_permits_but_au_forbids<Assignment>(std::chrono::duration<int16_t, std::milli>{},
                                                  std::chrono::duration<uint64_t>{1}),
        IsTrue());
}

TEST(Assignment, DisabledForIntTypesFromFloatTypes) {
    EXPECT_THAT(both_forbid<Assignment>(std::chrono::seconds{}, std::chrono::duration<double>{1}),
                IsTrue());
    EXPECT_THAT(both_forbid<Assignment>(std::chrono::seconds{}, std::chrono::duration<float>{1}),
                IsTrue());
}

TEST(Assignment, EnabledForFloatTypes) {
    EXPECT_THAT(both_permit<Assignment>(std::chrono::duration<float>{}, 1s), IsTrue());
    EXPECT_THAT(both_permit<Assignment>(std::chrono::duration<float>{}, 1ms), IsTrue());
    EXPECT_THAT(both_permit<Assignment>(std::chrono::duration<float>{}, 1ns), IsTrue());

    EXPECT_THAT(both_permit<Assignment>(std::chrono::duration<double>{}, 1s), IsTrue());
    EXPECT_THAT(both_permit<Assignment>(std::chrono::duration<double>{}, 1ms), IsTrue());
    EXPECT_THAT(both_permit<Assignment>(std::chrono::duration<double>{}, 1ns), IsTrue());
}

TEST(Addition, EnabledForWideVarietyOfTypes) {
    EXPECT_THAT(both_permit<Addition>(1s, 1ms, 1001ms), IsTrue());
    EXPECT_THAT(both_permit<Addition>(1ms, 1s, 1001ms), IsTrue());

    EXPECT_THAT(both_permit<Addition>(std::chrono::duration<double, std::milli>{8},
                                      std::chrono::duration<double, std::nano>{321},
                                      std::chrono::duration<double, std::nano>{8'000'321}),
                IsTrue());
    EXPECT_THAT(both_permit<Addition>(std::chrono::duration<int8_t, std::ratio<3, 5>>{1},
                                      std::chrono::duration<float, std::ratio<13, 17>>{2},
                                      std::chrono::duration<float, std::ratio<1, 5 * 17>>{
                                          (1 * 3 * 17) + (2 * 5 * 13)}),
                IsTrue());
}

TEST(Subtraction, EnabledForWideVarietyOfTypes) {
    EXPECT_THAT(both_permit<Subtraction>(1s, 1ms, 999ms), IsTrue());
    EXPECT_THAT(both_permit<Subtraction>(1ms, 1s, -999ms), IsTrue());

    EXPECT_THAT(both_permit<Subtraction>(std::chrono::duration<double, std::milli>{8},
                                         std::chrono::duration<double, std::nano>{321},
                                         std::chrono::duration<double, std::nano>{7'999'679}),
                IsTrue());
    EXPECT_THAT(both_permit<Subtraction>(std::chrono::duration<int8_t, std::ratio<3, 5>>{1},
                                         std::chrono::duration<float, std::ratio<13, 17>>{2},
                                         std::chrono::duration<float, std::ratio<1, 5 * 17>>{
                                             (1 * 3 * 17) - (2 * 5 * 13)}),
                IsTrue());
}

}  // namespace au
