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

#include "au/prefix.hh"

#include "au/testing.hh"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {

struct Bytes : UnitImpl<Information> {};

struct Inches : UnitImpl<Length> {};

struct XeroxedBytes : Bytes {
    static constexpr const char label[] = "X";
};

TEST(PrefixApplier, ConvertsUnitToCorrespondingPrefixedType) {
    // Give it a different name to avoid any question of shadowing the globally defined `yotta`.
    constexpr auto make_yotta = PrefixApplier<Yotta>{};
    constexpr auto bytes = Bytes{};

    ::testing::StaticAssertTypeEq<decltype(make_yotta(bytes)), Yotta<Bytes>>();
}

TEST(PrefixApplier, ConvertsQuantityMakerToMakerOfCorrespondingPrefixedType) {
    // Give it a different name to avoid any question of shadowing the globally defined `milli`.
    constexpr auto make_milli = PrefixApplier<Milli>{};

    constexpr auto inches = QuantityMaker<Inches>{};

    constexpr auto d = inches(2);
    EXPECT_THAT(d.in(make_milli(inches)), SameTypeAndValue(2'000));

    EXPECT_THAT(make_milli(inches)(5'777).in<int>(inches), SameTypeAndValue(5));
}

TEST(PrefixApplier, ConvertsSingularNameForToCorrespondingPrefixedType) {
    // Give it a different name to avoid any question of shadowing the globally defined `milli`.
    constexpr auto make_milli = PrefixApplier<Milli>{};

    constexpr auto inch = SingularNameFor<Inches>{};

    ::testing::StaticAssertTypeEq<decltype(make_milli(inch)), SingularNameFor<Milli<Inches>>>();
}

TEST(SiPrefixes, HaveCorrectAbsoluteValues) {
    EXPECT_EQ(unit_ratio(Yotta<Bytes>{}, Bytes{}), pow<24>(mag<10>()));
    EXPECT_EQ(unit_ratio(Zetta<Bytes>{}, Bytes{}), pow<21>(mag<10>()));
    EXPECT_EQ(unit_ratio(Exa<Bytes>{}, Bytes{}), pow<18>(mag<10>()));
    EXPECT_EQ(unit_ratio(Peta<Bytes>{}, Bytes{}), pow<15>(mag<10>()));
    EXPECT_EQ(unit_ratio(Tera<Bytes>{}, Bytes{}), pow<12>(mag<10>()));
    EXPECT_EQ(unit_ratio(Giga<Bytes>{}, Bytes{}), pow<9>(mag<10>()));
    EXPECT_EQ(unit_ratio(Mega<Bytes>{}, Bytes{}), pow<6>(mag<10>()));
    EXPECT_EQ(unit_ratio(Kilo<Bytes>{}, Bytes{}), pow<3>(mag<10>()));

    EXPECT_EQ(unit_ratio(Hecto<Bytes>{}, Bytes{}), pow<2>(mag<10>()));
    EXPECT_EQ(unit_ratio(Deka<Bytes>{}, Bytes{}), pow<1>(mag<10>()));
    EXPECT_EQ(unit_ratio(Deci<Bytes>{}, Bytes{}), pow<-1>(mag<10>()));
    EXPECT_EQ(unit_ratio(Centi<Bytes>{}, Bytes{}), pow<-2>(mag<10>()));

    EXPECT_EQ(unit_ratio(Milli<Bytes>{}, Bytes{}), pow<-3>(mag<10>()));
    EXPECT_EQ(unit_ratio(Micro<Bytes>{}, Bytes{}), pow<-6>(mag<10>()));
    EXPECT_EQ(unit_ratio(Nano<Bytes>{}, Bytes{}), pow<-9>(mag<10>()));
    EXPECT_EQ(unit_ratio(Pico<Bytes>{}, Bytes{}), pow<-12>(mag<10>()));
    EXPECT_EQ(unit_ratio(Femto<Bytes>{}, Bytes{}), pow<-15>(mag<10>()));
    EXPECT_EQ(unit_ratio(Atto<Bytes>{}, Bytes{}), pow<-18>(mag<10>()));
    EXPECT_EQ(unit_ratio(Zepto<Bytes>{}, Bytes{}), pow<-21>(mag<10>()));
    EXPECT_EQ(unit_ratio(Yocto<Bytes>{}, Bytes{}), pow<-24>(mag<10>()));
}

TEST(SiPrefixes, PrefixAppliersPredefined) {
    constexpr QuantityMaker<Inches> inches{};

    EXPECT_EQ(yotta(inches)(1), zetta(inches)(1000));
    EXPECT_EQ(zetta(inches)(1), exa(inches)(1000));
    EXPECT_EQ(exa(inches)(1), peta(inches)(1000));
    EXPECT_EQ(peta(inches)(1), tera(inches)(1000));
    EXPECT_EQ(tera(inches)(1), giga(inches)(1000));
    EXPECT_EQ(giga(inches)(1), mega(inches)(1000));
    EXPECT_EQ(mega(inches)(1), kilo(inches)(1000));

    EXPECT_EQ(kilo(inches)(1), hecto(inches)(10));
    EXPECT_EQ(hecto(inches)(1), deka(inches)(10));
    EXPECT_EQ(deka(inches)(1), deci(inches)(100));
    EXPECT_EQ(deci(inches)(1), centi(inches)(10));
    EXPECT_EQ(centi(inches)(1), milli(inches)(10));

    EXPECT_EQ(milli(inches)(1), micro(inches)(1000));
    EXPECT_EQ(micro(inches)(1), nano(inches)(1000));
    EXPECT_EQ(nano(inches)(1), pico(inches)(1000));
    EXPECT_EQ(pico(inches)(1), femto(inches)(1000));
    EXPECT_EQ(femto(inches)(1), atto(inches)(1000));
    EXPECT_EQ(atto(inches)(1), zepto(inches)(1000));
    EXPECT_EQ(zepto(inches)(1), yocto(inches)(1000));
}

TEST(SiPrefixes, CorrectlyLabelUnits) {
    // Reference: https://physics.nist.gov/cuu/Units/prefixes.html
    expect_label<Kilo<XeroxedBytes>>("kX");
    expect_label<Mega<XeroxedBytes>>("MX");
    expect_label<Giga<XeroxedBytes>>("GX");
    expect_label<Tera<XeroxedBytes>>("TX");
    expect_label<Peta<XeroxedBytes>>("PX");
    expect_label<Exa<XeroxedBytes>>("EX");
    expect_label<Zetta<XeroxedBytes>>("ZX");
    expect_label<Yotta<XeroxedBytes>>("YX");
    expect_label<Milli<XeroxedBytes>>("mX");
    expect_label<Micro<XeroxedBytes>>("uX");
    expect_label<Nano<XeroxedBytes>>("nX");
    expect_label<Pico<XeroxedBytes>>("pX");
    expect_label<Femto<XeroxedBytes>>("fX");
    expect_label<Atto<XeroxedBytes>>("aX");
    expect_label<Zepto<XeroxedBytes>>("zX");
    expect_label<Yocto<XeroxedBytes>>("yX");
    expect_label<Hecto<XeroxedBytes>>("hX");
    expect_label<Deka<XeroxedBytes>>("daX");
    expect_label<Deci<XeroxedBytes>>("dX");
    expect_label<Centi<XeroxedBytes>>("cX");
}

TEST(BinaryPrefixes, HaveCorrectAbsoluteValues) {
    EXPECT_EQ(unit_ratio(Yobi<Bytes>{}, Bytes{}), pow<8>(mag<1024>()));
    EXPECT_EQ(unit_ratio(Zebi<Bytes>{}, Bytes{}), pow<7>(mag<1024>()));
    EXPECT_EQ(unit_ratio(Exbi<Bytes>{}, Bytes{}), pow<6>(mag<1024>()));
    EXPECT_EQ(unit_ratio(Pebi<Bytes>{}, Bytes{}), pow<5>(mag<1024>()));
    EXPECT_EQ(unit_ratio(Tebi<Bytes>{}, Bytes{}), pow<4>(mag<1024>()));
    EXPECT_EQ(unit_ratio(Gibi<Bytes>{}, Bytes{}), pow<3>(mag<1024>()));
    EXPECT_EQ(unit_ratio(Mebi<Bytes>{}, Bytes{}), pow<2>(mag<1024>()));
    EXPECT_EQ(unit_ratio(Kibi<Bytes>{}, Bytes{}), pow<1>(mag<1024>()));
}

TEST(BinaryPrefixes, PrefixAppliersPredefined) {
    constexpr QuantityMaker<Bytes> bytes{};

    EXPECT_EQ(yobi(bytes)(1), zebi(bytes)(1024));
    EXPECT_EQ(zebi(bytes)(1), exbi(bytes)(1024));
    EXPECT_EQ(exbi(bytes)(1), pebi(bytes)(1024));
    EXPECT_EQ(pebi(bytes)(1), tebi(bytes)(1024));
    EXPECT_EQ(tebi(bytes)(1), gibi(bytes)(1024));
    EXPECT_EQ(gibi(bytes)(1), mebi(bytes)(1024));
    EXPECT_EQ(mebi(bytes)(1), kibi(bytes)(1024));
}

TEST(BinaryPrefixes, CorrectlyLabelUnits) {
    // Reference: https://physics.nist.gov/cuu/Units/binary.html
    expect_label<Kibi<XeroxedBytes>>("KiX");
    expect_label<Mebi<XeroxedBytes>>("MiX");
    expect_label<Gibi<XeroxedBytes>>("GiX");
    expect_label<Tebi<XeroxedBytes>>("TiX");
    expect_label<Pebi<XeroxedBytes>>("PiX");
    expect_label<Exbi<XeroxedBytes>>("EiX");
    expect_label<Zebi<XeroxedBytes>>("ZiX");  // https://en.wikipedia.org/wiki/Zebibit
    expect_label<Yobi<XeroxedBytes>>("YiX");  // https://en.wikipedia.org/wiki/Yobibit
}
}  // namespace au
