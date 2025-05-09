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
using ::testing::IsEmpty;
using ::testing::StrEq;

namespace auimpl {

TEST(StringConstant, CanCreateFromStringLiteral) {
    constexpr StringConstant<5> x{"hello"};
    EXPECT_THAT(x, StrEq("hello"));
}

TEST(StringConstant, HasLengthMember) {
    EXPECT_THAT(StringConstant<2>::length, Eq(2));
    EXPECT_THAT(StringConstant<13>::length, Eq(13));
}

TEST(AsStringConstant, CanCreateFromStringLiteral) {
    constexpr auto x = as_string_constant("hello");
    EXPECT_THAT(x, StrEq("hello"));
}

TEST(AsStringConstant, PassingStringConstantIsIdentity) {
    constexpr auto x = as_string_constant(as_string_constant("goodbye"));
    EXPECT_THAT(x, StrEq("goodbye"));
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
    EXPECT_THAT(IToA<0>::value, StrEq("0"));

    EXPECT_THAT(IToA<1>::value, StrEq("1"));
    EXPECT_THAT(IToA<9>::value, StrEq("9"));
    EXPECT_THAT(IToA<10>::value, StrEq("10"));
    EXPECT_THAT(IToA<91>::value, StrEq("91"));
    EXPECT_THAT(IToA<312839>::value, StrEq("312839"));

    EXPECT_THAT(IToA<-1>::value, StrEq("-1"));
    EXPECT_THAT(IToA<-83294>::value, StrEq("-83294"));

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
    EXPECT_THAT(UIToA<10'000'000'000'000'000'000u>::value, StrEq("10000000000000000000"));
}

TEST(join, EmptyStringForNoArguments) {
    constexpr auto x = as_string_constant("sep").join();
    EXPECT_THAT(x, IsEmpty());
}

TEST(join, InputStringForOneArgument) {
    constexpr auto fish = as_string_constant("sep").join(as_string_constant("fish"));
    EXPECT_THAT(fish, StrEq("fish"));
}

TEST(join, JoinsMultipleArgumentsWithSep) {
    constexpr auto letter_groups = as_string_constant(" | ").join(
        as_string_constant("a"), as_string_constant("b"), as_string_constant("cde"));
    EXPECT_THAT(letter_groups, StrEq("a | b | cde"));
}

TEST(JoinBy, SupportsStringConstants) {
    constexpr auto b = as_string_constant("b");

    constexpr auto letter_groups = join_by(" # ", "a", b, "cde");

    EXPECT_THAT(letter_groups, StrEq("a # b # cde"));
}

TEST(concatenate, EmptyStringForNoArguments) {
    constexpr auto x = concatenate();
    EXPECT_THAT(x, IsEmpty());
}

TEST(concatenate, ReturnsInputStringForOneArgument) {
    constexpr auto x = concatenate("foo");
    EXPECT_THAT(x, StrEq("foo"));
}

TEST(concatenate, ConcatenatesMultipleArguments) {
    constexpr auto x = concatenate("a", "b", "cde");
    EXPECT_THAT(x, StrEq("abcde"));
}

TEST(concatenate, SupportsStringConstants) {
    constexpr auto a = as_string_constant("a");
    constexpr auto cde = as_string_constant("cde");

    constexpr auto x = concatenate(a, "b", cde);

    EXPECT_THAT(x, StrEq("abcde"));
}

TEST(ParensIf, WrapsInParensIfTrue) {
    EXPECT_THAT(parens_if<true>("a"), StrEq("(a)"));
    EXPECT_THAT(parens_if<false>("123"), StrEq("123"));
}

}  // namespace auimpl
}  // namespace au
