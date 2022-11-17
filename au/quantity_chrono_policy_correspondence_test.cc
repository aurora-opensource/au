// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#include <type_traits>

#include "au/chrono_policy_validation.hh"
#include "gtest/gtest.h"

using namespace std::chrono_literals;

namespace au {
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
    return {
        .eq = (t == u),
        .ne = (t != u),
        .lt = (t < u),
        .le = (t <= u),
        .gt = (t > u),
        .ge = (t >= u),
    };
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
    EXPECT_TRUE(both_permit<Comparison>(std::chrono::duration<double>(1),
                                        std::chrono::duration<double>(1)));

    EXPECT_TRUE(both_permit<Comparison>(std::chrono::duration<double>(1),
                                        std::chrono::duration<double>(2)));

    EXPECT_TRUE(
        both_permit<Comparison>(std::chrono::duration<float>(1), std::chrono::duration<float>(2)));

    EXPECT_TRUE(both_permit<Comparison>(std::chrono::duration<float>(1),
                                        std::chrono::duration<float, std::milli>(1'000)));
    EXPECT_TRUE(both_permit<Comparison>(std::chrono::duration<float>(1.001),
                                        std::chrono::duration<float, std::milli>(1'000)));
    EXPECT_TRUE(both_permit<Comparison>(std::chrono::duration<float>(0.999),
                                        std::chrono::duration<float, std::milli>(1'000)));
}

TEST(Comparison, EnabledForReasonableCombosOfUnitsAndIntegralReps) {
    EXPECT_TRUE(both_permit<Comparison>(1s, 1000ms));
    EXPECT_TRUE(both_permit<Comparison>(1s, 1001ms));
    EXPECT_TRUE(both_permit<Comparison>(1s, 999ms));
    EXPECT_TRUE(both_permit<Comparison>(1'000'000'000ns, 1000ms));

    EXPECT_TRUE(both_permit<Comparison>(std::chrono::duration<int16_t, std::milli>{50},
                                        std::chrono::duration<int64_t, std::micro>{50'001}));
}

TEST(Comparison, EnabledForMixedIntegralFloatingPointReps) {
    EXPECT_TRUE(both_permit<Comparison>(1.0s, 1000ms));
    EXPECT_TRUE(both_permit<Comparison>(1.0s, std::chrono::duration<uint64_t, std::milli>{1000}));
}

TEST(Assignment, EnabledForInt64IffScaleWidening) {
    EXPECT_TRUE(both_permit<Assignment>(std::chrono::milliseconds{}, 1s));
    EXPECT_TRUE(both_forbid<Assignment>(std::chrono::seconds{}, 1ms));
}

TEST(Assignment, EnabledForSameScaleIntegralTypes) {
    EXPECT_TRUE(both_permit<Assignment>(std::chrono::duration<int64_t>{},
                                        std::chrono::duration<int32_t>{1}));
    EXPECT_TRUE(both_permit<Assignment>(std::chrono::duration<int32_t>{},
                                        std::chrono::duration<int64_t>{1}));
}

TEST(Assignment, DisabledForOverflowRiskyIntegralConversions) {
    EXPECT_TRUE(chrono_permits_but_au_forbids<Assignment>(
        std::chrono::duration<int16_t, std::milli>{}, std::chrono::duration<uint64_t>{1}));
}

TEST(Assignment, DisabledForIntTypesFromFloatTypes) {
    EXPECT_TRUE(both_forbid<Assignment>(std::chrono::seconds{}, std::chrono::duration<double>{1}));
    EXPECT_TRUE(both_forbid<Assignment>(std::chrono::seconds{}, std::chrono::duration<float>{1}));
}

TEST(Assignment, EnabledForFloatTypes) {
    EXPECT_TRUE(both_permit<Assignment>(std::chrono::duration<float>{}, 1s));
    EXPECT_TRUE(both_permit<Assignment>(std::chrono::duration<float>{}, 1ms));
    EXPECT_TRUE(both_permit<Assignment>(std::chrono::duration<float>{}, 1ns));

    EXPECT_TRUE(both_permit<Assignment>(std::chrono::duration<double>{}, 1s));
    EXPECT_TRUE(both_permit<Assignment>(std::chrono::duration<double>{}, 1ms));
    EXPECT_TRUE(both_permit<Assignment>(std::chrono::duration<double>{}, 1ns));
}

TEST(Addition, EnabledForWideVarietyOfTypes) {
    EXPECT_TRUE(both_permit<Addition>(1s, 1ms, 1001ms));
    EXPECT_TRUE(both_permit<Addition>(1ms, 1s, 1001ms));

    EXPECT_TRUE(both_permit<Addition>(std::chrono::duration<double, std::milli>{8},
                                      std::chrono::duration<double, std::nano>{321},
                                      std::chrono::duration<double, std::nano>{8'000'321}));
    EXPECT_TRUE(both_permit<Addition>(
        std::chrono::duration<int8_t, std::ratio<3, 5>>{1},
        std::chrono::duration<float, std::ratio<13, 17>>{2},
        std::chrono::duration<float, std::ratio<1, 5 * 17>>{(1 * 3 * 17) + (2 * 5 * 13)}));
}

TEST(Subtraction, EnabledForWideVarietyOfTypes) {
    EXPECT_TRUE(both_permit<Subtraction>(1s, 1ms, 999ms));
    EXPECT_TRUE(both_permit<Subtraction>(1ms, 1s, -999ms));

    EXPECT_TRUE(both_permit<Subtraction>(std::chrono::duration<double, std::milli>{8},
                                         std::chrono::duration<double, std::nano>{321},
                                         std::chrono::duration<double, std::nano>{7'999'679}));
    EXPECT_TRUE(both_permit<Subtraction>(
        std::chrono::duration<int8_t, std::ratio<3, 5>>{1},
        std::chrono::duration<float, std::ratio<13, 17>>{2},
        std::chrono::duration<float, std::ratio<1, 5 * 17>>{(1 * 3 * 17) - (2 * 5 * 13)}));
}
}  // namespace au
