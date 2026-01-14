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
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;
using ::testing::StaticAssertTypeEq;

struct Bytes : UnitImpl<Information> {};

struct Inches : UnitImpl<Length> {
    static constexpr const char label[] = "in";
};
constexpr const char Inches::label[];

struct XeroxedBytes : Bytes {
    static constexpr const char label[] = "X";
};
constexpr const char XeroxedBytes::label[];

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

    EXPECT_THAT(make_milli(inches)(5'777).coerce_in(inches), SameTypeAndValue(5));
}

TEST(PrefixApplier, ConvertsSingularNameForToCorrespondingPrefixedType) {
    // Give it a different name to avoid any question of shadowing the globally defined `milli`.
    constexpr auto make_milli = PrefixApplier<Milli>{};

    constexpr auto inch = SingularNameFor<Inches>{};

    ::testing::StaticAssertTypeEq<decltype(make_milli(inch)), SingularNameFor<Milli<Inches>>>();
}

TEST(PrefixApplier, ConvertsSymbolForToCorrespondingPrefixedType) {
    constexpr auto X = symbol_for(XeroxedBytes{});
    StaticAssertTypeEq<decltype(kibi(X)), SymbolFor<Kibi<XeroxedBytes>>>();
}

TEST(SiPrefixes, HaveCorrectAbsoluteValues) {
    EXPECT_THAT(unit_ratio(Yotta<Bytes>{}, Bytes{}), Eq(pow<24>(mag<10>())));
    EXPECT_THAT(unit_ratio(Zetta<Bytes>{}, Bytes{}), Eq(pow<21>(mag<10>())));
    EXPECT_THAT(unit_ratio(Exa<Bytes>{}, Bytes{}), Eq(pow<18>(mag<10>())));
    EXPECT_THAT(unit_ratio(Peta<Bytes>{}, Bytes{}), Eq(pow<15>(mag<10>())));
    EXPECT_THAT(unit_ratio(Tera<Bytes>{}, Bytes{}), Eq(pow<12>(mag<10>())));
    EXPECT_THAT(unit_ratio(Giga<Bytes>{}, Bytes{}), Eq(pow<9>(mag<10>())));
    EXPECT_THAT(unit_ratio(Mega<Bytes>{}, Bytes{}), Eq(pow<6>(mag<10>())));
    EXPECT_THAT(unit_ratio(Kilo<Bytes>{}, Bytes{}), Eq(pow<3>(mag<10>())));

    EXPECT_THAT(unit_ratio(Hecto<Bytes>{}, Bytes{}), Eq(pow<2>(mag<10>())));
    EXPECT_THAT(unit_ratio(Deka<Bytes>{}, Bytes{}), Eq(pow<1>(mag<10>())));
    EXPECT_THAT(unit_ratio(Deci<Bytes>{}, Bytes{}), Eq(pow<-1>(mag<10>())));
    EXPECT_THAT(unit_ratio(Centi<Bytes>{}, Bytes{}), Eq(pow<-2>(mag<10>())));

    EXPECT_THAT(unit_ratio(Milli<Bytes>{}, Bytes{}), Eq(pow<-3>(mag<10>())));
    EXPECT_THAT(unit_ratio(Micro<Bytes>{}, Bytes{}), Eq(pow<-6>(mag<10>())));
    EXPECT_THAT(unit_ratio(Nano<Bytes>{}, Bytes{}), Eq(pow<-9>(mag<10>())));
    EXPECT_THAT(unit_ratio(Pico<Bytes>{}, Bytes{}), Eq(pow<-12>(mag<10>())));
    EXPECT_THAT(unit_ratio(Femto<Bytes>{}, Bytes{}), Eq(pow<-15>(mag<10>())));
    EXPECT_THAT(unit_ratio(Atto<Bytes>{}, Bytes{}), Eq(pow<-18>(mag<10>())));
    EXPECT_THAT(unit_ratio(Zepto<Bytes>{}, Bytes{}), Eq(pow<-21>(mag<10>())));
    EXPECT_THAT(unit_ratio(Yocto<Bytes>{}, Bytes{}), Eq(pow<-24>(mag<10>())));
}

TEST(SiPrefixes, PrefixAppliersPredefined) {
    constexpr QuantityMaker<Inches> inches{};

    EXPECT_THAT(quetta(inches)(1), Eq(ronna(inches)(1000)));
    EXPECT_THAT(ronna(inches)(1), Eq(yotta(inches)(1000)));
    EXPECT_THAT(yotta(inches)(1), Eq(zetta(inches)(1000)));
    EXPECT_THAT(zetta(inches)(1), Eq(exa(inches)(1000)));
    EXPECT_THAT(exa(inches)(1), Eq(peta(inches)(1000)));
    EXPECT_THAT(peta(inches)(1), Eq(tera(inches)(1000)));
    EXPECT_THAT(tera(inches)(1), Eq(giga(inches)(1000)));
    EXPECT_THAT(giga(inches)(1), Eq(mega(inches)(1000)));
    EXPECT_THAT(mega(inches)(1), Eq(kilo(inches)(1000)));

    EXPECT_THAT(kilo(inches)(1), Eq(hecto(inches)(10)));
    EXPECT_THAT(hecto(inches)(1), Eq(deka(inches)(10)));
    EXPECT_THAT(deka(inches)(1), Eq(inches(10)));

    EXPECT_THAT(inches(1), Eq(deci(inches)(10)));
    EXPECT_THAT(deci(inches)(1), Eq(centi(inches)(10)));
    EXPECT_THAT(centi(inches)(1), Eq(milli(inches)(10)));

    EXPECT_THAT(milli(inches)(1), Eq(micro(inches)(1000)));
    EXPECT_THAT(micro(inches)(1), Eq(nano(inches)(1000)));
    EXPECT_THAT(nano(inches)(1), Eq(pico(inches)(1000)));
    EXPECT_THAT(pico(inches)(1), Eq(femto(inches)(1000)));
    EXPECT_THAT(femto(inches)(1), Eq(atto(inches)(1000)));
    EXPECT_THAT(atto(inches)(1), Eq(zepto(inches)(1000)));
    EXPECT_THAT(zepto(inches)(1), Eq(yocto(inches)(1000)));
    EXPECT_THAT(yocto(inches)(1), Eq(ronto(inches)(1000)));
    EXPECT_THAT(ronto(inches)(1), Eq(quecto(inches)(1000)));
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
    expect_label<Ronna<XeroxedBytes>>("RX");
    expect_label<Quetta<XeroxedBytes>>("QX");
    expect_label<Milli<XeroxedBytes>>("mX");
    expect_label<Micro<XeroxedBytes>>("uX");
    expect_label<Nano<XeroxedBytes>>("nX");
    expect_label<Pico<XeroxedBytes>>("pX");
    expect_label<Femto<XeroxedBytes>>("fX");
    expect_label<Atto<XeroxedBytes>>("aX");
    expect_label<Zepto<XeroxedBytes>>("zX");
    expect_label<Yocto<XeroxedBytes>>("yX");
    expect_label<Ronto<XeroxedBytes>>("rX");
    expect_label<Quecto<XeroxedBytes>>("qX");
    expect_label<Hecto<XeroxedBytes>>("hX");
    expect_label<Deka<XeroxedBytes>>("daX");
    expect_label<Deci<XeroxedBytes>>("dX");
    expect_label<Centi<XeroxedBytes>>("cX");
}

TEST(BinaryPrefixes, HaveCorrectAbsoluteValues) {
    EXPECT_THAT(unit_ratio(Yobi<Bytes>{}, Bytes{}), Eq(pow<8>(mag<1024>())));
    EXPECT_THAT(unit_ratio(Zebi<Bytes>{}, Bytes{}), Eq(pow<7>(mag<1024>())));
    EXPECT_THAT(unit_ratio(Exbi<Bytes>{}, Bytes{}), Eq(pow<6>(mag<1024>())));
    EXPECT_THAT(unit_ratio(Pebi<Bytes>{}, Bytes{}), Eq(pow<5>(mag<1024>())));
    EXPECT_THAT(unit_ratio(Tebi<Bytes>{}, Bytes{}), Eq(pow<4>(mag<1024>())));
    EXPECT_THAT(unit_ratio(Gibi<Bytes>{}, Bytes{}), Eq(pow<3>(mag<1024>())));
    EXPECT_THAT(unit_ratio(Mebi<Bytes>{}, Bytes{}), Eq(pow<2>(mag<1024>())));
    EXPECT_THAT(unit_ratio(Kibi<Bytes>{}, Bytes{}), Eq(pow<1>(mag<1024>())));
}

TEST(BinaryPrefixes, PrefixAppliersPredefined) {
    constexpr QuantityMaker<Bytes> bytes{};

    EXPECT_THAT(yobi(bytes)(1), Eq(zebi(bytes)(1024)));
    EXPECT_THAT(zebi(bytes)(1), Eq(exbi(bytes)(1024)));
    EXPECT_THAT(exbi(bytes)(1), Eq(pebi(bytes)(1024)));
    EXPECT_THAT(pebi(bytes)(1), Eq(tebi(bytes)(1024)));
    EXPECT_THAT(tebi(bytes)(1), Eq(gibi(bytes)(1024)));
    EXPECT_THAT(gibi(bytes)(1), Eq(mebi(bytes)(1024)));
    EXPECT_THAT(mebi(bytes)(1), Eq(kibi(bytes)(1024)));
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

TEST(PrefixedPoweredUnitLabels, OmitsBracketsIfPrefixAppliesBeforeThePower) {
    expect_label<UnitInverse<Milli<XeroxedBytes>>>("mX^(-1)");
    expect_label<UnitPower<Kilo<XeroxedBytes>, 2>>("kX^2");
    expect_label<UnitPower<Mega<XeroxedBytes>, 3>>("MX^3");
}

TEST(PrefixedPoweredUnitLabels, IncludesBracketsIfPrefixAppliesAfterThePower) {
    expect_label<Milli<UnitInverse<XeroxedBytes>>>("m[X^(-1)]");
    expect_label<Kilo<UnitPower<XeroxedBytes, 2>>>("k[X^2]");
    expect_label<Mega<UnitPower<XeroxedBytes, 3>>>("M[X^3]");
}

TEST(PrefixedPoweredUnitLabels, IncludesBracketsIfFirstNumeratorElementHasPower) {
    using SquareInchesTimesXeroxedBytes = UnitProduct<UnitPower<Inches, 2>, XeroxedBytes>;
    expect_label<Kilo<SquareInchesTimesXeroxedBytes>>("k[in^2 * X]");

    using SquareInchesPerXeroxedByte = UnitQuotient<UnitPower<Inches, 2>, XeroxedBytes>;
    expect_label<Milli<SquareInchesPerXeroxedByte>>("m[in^2 / X]");
}

TEST(PrefixedPoweredUnitLabels, OmitsBracketsIfFirstNumeratorElementHasNoPower) {
    using XeroxedBytesPerSquareInch = UnitQuotient<XeroxedBytes, UnitPower<Inches, 2>>;
    expect_label<Gibi<XeroxedBytesPerSquareInch>>("GiX / in^2");

    using XeroxedBytesTimesInches = UnitProduct<XeroxedBytes, Inches>;
    expect_label<Kilo<XeroxedBytesTimesInches>>("kin * X");
}

}  // namespace au
