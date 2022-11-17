// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#include "au/utility/factoring.hh"

#include "gtest/gtest.h"

namespace au {
namespace detail {
namespace {
std::uintmax_t cube(std::uintmax_t n) { return n * n * n; }
}  // namespace

TEST(FindFirstFactor, ReturnsInputForPrimes) {
    EXPECT_EQ(find_first_factor(2u), 2u);
    EXPECT_EQ(find_first_factor(3u), 3u);
    EXPECT_EQ(find_first_factor(5u), 5u);
    EXPECT_EQ(find_first_factor(7u), 7u);
    EXPECT_EQ(find_first_factor(11u), 11u);

    EXPECT_EQ(find_first_factor(196961u), 196961u);
}

TEST(FindFirstFactor, FindsFirstFactor) {
    EXPECT_EQ(find_first_factor(7u * 11u * 13u), 7u);
    EXPECT_EQ(find_first_factor(cube(196961u)), 196961u);
}

TEST(IsPrime, FalseForLessThan2) {
    EXPECT_FALSE(is_prime(0u));
    EXPECT_FALSE(is_prime(1u));
}

TEST(IsPrime, FindsPrimes) {
    EXPECT_TRUE(is_prime(2u));
    EXPECT_TRUE(is_prime(3u));
    EXPECT_FALSE(is_prime(4u));
    EXPECT_TRUE(is_prime(5u));
    EXPECT_FALSE(is_prime(6u));
    EXPECT_TRUE(is_prime(7u));
    EXPECT_FALSE(is_prime(8u));
    EXPECT_FALSE(is_prime(9u));
    EXPECT_FALSE(is_prime(10u));
    EXPECT_TRUE(is_prime(11u));

    EXPECT_FALSE(is_prime(196959u));
    EXPECT_FALSE(is_prime(196960u));
    EXPECT_TRUE(is_prime(196961u));
    EXPECT_FALSE(is_prime(196962u));
}

TEST(Multiplicity, CountsFactors) {
    constexpr std::uintmax_t n = (2u * 2u * 2u) * (3u) * (5u * 5u);
    EXPECT_EQ(multiplicity(2u, n), 3u);
    EXPECT_EQ(multiplicity(3u, n), 1u);
    EXPECT_EQ(multiplicity(5u, n), 2u);
    EXPECT_EQ(multiplicity(7u, n), 0u);
}

}  // namespace detail
}  // namespace au
