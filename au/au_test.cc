// Copyright 2022 Aurora Operations, Inc.

#include "au/au.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

using namespace std::chrono_literals;

namespace au {

TEST(DurationQuantity, InterconvertsWithExactlyEquivalentChronoDuration) {
    constexpr QuantityD<Seconds> from_chrono = std::chrono::duration<double>{1.23};
    EXPECT_THAT(from_chrono, SameTypeAndValue(seconds(1.23)));

    constexpr auto val = std::chrono::nanoseconds::rep{456};
    constexpr std::chrono::nanoseconds from_au = nano(seconds)(val);
    EXPECT_THAT(from_au.count(), SameTypeAndValue(val));
}

TEST(DurationQuantity, InterconvertsWithIndirectlyEquivalentChronoDuration) {
    constexpr QuantityD<Seconds> from_chrono = as_quantity(1234ms);
    EXPECT_THAT(from_chrono, SameTypeAndValue(seconds(1.234)));
}

TEST(Conversions, SupportIntMHzToU32Hz) {
    constexpr QuantityU32<Hertz> freq = mega(hertz)(40);
    EXPECT_THAT(freq, SameTypeAndValue(hertz(40'000'000u)));
}

TEST(CommonUnit, HandlesPrefixesReasonably) {
    StaticAssertTypeEq<CommonUnitT<Kilo<Meters>, Meters>, Meters>();
}

}  // namespace au
