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

#include "au/utility/string_constant.hh"

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;
using ::testing::StrEq;

namespace detail {

TEST(StringConstant, CanCreateFromStringLiteral) {
    constexpr StringConstant<5> x{"hello"};
    EXPECT_STREQ(x.c_str(), "hello");
}

TEST(StringConstant, HasLengthMember) {
    EXPECT_THAT(StringConstant<2>::length, Eq(2));
    EXPECT_THAT(StringConstant<13>::length, Eq(13));
}

TEST(AsStringConstant, CanCreateFromStringLiteral) {
    constexpr auto x = as_string_constant("hello");
    EXPECT_STREQ(x.c_str(), "hello");
}

TEST(AsStringConstant, PassingStringConstantIsIdentity) {
    constexpr auto x = as_string_constant(as_string_constant("goodbye"));
    EXPECT_STREQ(x.c_str(), "goodbye");
}

TEST(AbsAsUnsigned, IdentityForPositiveNumbers) {
    EXPECT_THAT(abs_as_unsigned(int8_t{0}), SameTypeAndValue(uint8_t{0}));
    EXPECT_THAT(abs_as_unsigned(int8_t{1}), SameTypeAndValue(uint8_t{1}));
    EXPECT_THAT(abs_as_unsigned(int8_t{127}), SameTypeAndValue(uint8_t{127}));
}

TEST(AbsAsUnsigned, NegatesNegativeNumbers) {
    EXPECT_THAT(abs_as_unsigned(int8_t{-1}), SameTypeAndValue(uint8_t{1}));
    EXPECT_THAT(abs_as_unsigned(int8_t{-127}), SameTypeAndValue(uint8_t{127}));
    EXPECT_THAT(abs_as_unsigned(int8_t{-128}), SameTypeAndValue(uint8_t{128}));
}

TEST(IToA, ValueHoldsStringVersionOfTemplateParameter) {
    EXPECT_THAT(IToA<0>::value.c_str(), StrEq("0"));

    EXPECT_THAT(IToA<1>::value.c_str(), StrEq("1"));
    EXPECT_THAT(IToA<9>::value.c_str(), StrEq("9"));
    EXPECT_THAT(IToA<10>::value.c_str(), StrEq("10"));
    EXPECT_THAT(IToA<91>::value.c_str(), StrEq("91"));
    EXPECT_THAT(IToA<312839>::value.c_str(), StrEq("312839"));

    EXPECT_THAT(IToA<-1>::value.c_str(), StrEq("-1"));
    EXPECT_THAT(IToA<-83294>::value.c_str(), StrEq("-83294"));

    // This funny way of writing it is because `-9'223'372'036'854'775'808` isn't a literal.  The
    // actual literal is `9'223'372'036'854'775'808`, which is too big (by one) to fit into
    // `int64_t` --- even though `-9'223'372'036'854'775'808` can!  So instead, we negate the
    // largest literal, and then subtract one to get the lowest literal.
    constexpr int64_t min = -9'223'372'036'854'775'807 - 1;
    EXPECT_THAT(IToA<min>::value, StrEq("-9223372036854775808"));
}

TEST(IToA, HasLengthMember) {
    EXPECT_THAT(IToA<0>::length, Eq(1));

    EXPECT_THAT(IToA<2>::length, Eq(1));
    EXPECT_THAT(IToA<9>::length, Eq(1));
    EXPECT_THAT(IToA<10>::length, Eq(2));
    EXPECT_THAT(IToA<12345>::length, Eq(5));

    EXPECT_THAT(IToA<-2>::length, Eq(2));
    EXPECT_THAT(IToA<-9>::length, Eq(2));
    EXPECT_THAT(IToA<-10>::length, Eq(3));
    EXPECT_THAT(IToA<-12345>::length, Eq(6));
}

TEST(UIToA, CanHandleNumbersBiggerThanIntmaxButWithinUintmax) {
    EXPECT_STREQ(UIToA<10000000000000000000u>::value.c_str(), "10000000000000000000");
}

TEST(join, EmptyStringForNoArguments) {
    constexpr auto x = as_string_constant("sep").join();
    EXPECT_STREQ(x.c_str(), "");
}

TEST(join, InputStringForOneArgument) {
    constexpr auto fish = as_string_constant("sep").join(as_string_constant("fish"));
    EXPECT_STREQ(fish.c_str(), "fish");
}

TEST(join, JoinsMultipleArgumentsWithSep) {
    constexpr auto letter_groups = as_string_constant(" | ").join(
        as_string_constant("a"), as_string_constant("b"), as_string_constant("cde"));
    EXPECT_STREQ(letter_groups.c_str(), "a | b | cde");
}

TEST(JoinBy, SupportsStringConstants) {
    constexpr auto b = as_string_constant("b");

    constexpr auto letter_groups = join_by(" # ", "a", b, "cde");

    EXPECT_STREQ(letter_groups.c_str(), "a # b # cde");
}

TEST(concatenate, EmptyStringForNoArguments) {
    constexpr auto x = concatenate();
    EXPECT_STREQ(x.c_str(), "");
}

TEST(concatenate, ReturnsInputStringForOneArgument) {
    constexpr auto x = concatenate("foo");
    EXPECT_STREQ(x.c_str(), "foo");
}

TEST(concatenate, ConcatenatesMultipleArguments) {
    constexpr auto x = concatenate("a", "b", "cde");
    EXPECT_STREQ(x.c_str(), "abcde");
}

TEST(concatenate, SupportsStringConstants) {
    constexpr auto a = as_string_constant("a");
    constexpr auto cde = as_string_constant("cde");

    constexpr auto x = concatenate(a, "b", cde);

    EXPECT_STREQ(x.c_str(), "abcde");
}

TEST(ParensIf, WrapsInParensIfTrue) {
    EXPECT_STREQ(parens_if<true>("a"), "(a)");
    EXPECT_STREQ(parens_if<false>("123"), "123");
}

}  // namespace detail
}  // namespace au
