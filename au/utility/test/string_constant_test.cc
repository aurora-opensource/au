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

#include "gtest/gtest.h"

namespace au {
namespace detail {

TEST(StringConstant, CanCreateFromStringLiteral) {
    constexpr StringConstant<5> x{"hello"};
    EXPECT_STREQ(x.c_str(), "hello");
}

TEST(StringConstant, HasLengthMember) {
    EXPECT_EQ(StringConstant<2>::length, 2);
    EXPECT_EQ(StringConstant<13>::length, 13);
}

TEST(AsStringConstant, CanCreateFromStringLiteral) {
    constexpr auto x = as_string_constant("hello");
    EXPECT_STREQ(x.c_str(), "hello");
}

TEST(AsStringConstant, PassingStringConstantIsIdentity) {
    constexpr auto x = as_string_constant(as_string_constant("goodbye"));
    EXPECT_STREQ(x.c_str(), "goodbye");
}

TEST(IToA, ValueHoldsStringVersionOfTemplateParameter) {
    EXPECT_STREQ(IToA<0>::value.c_str(), "0");

    EXPECT_STREQ(IToA<1>::value.c_str(), "1");
    EXPECT_STREQ(IToA<9>::value.c_str(), "9");
    EXPECT_STREQ(IToA<10>::value.c_str(), "10");
    EXPECT_STREQ(IToA<91>::value.c_str(), "91");
    EXPECT_STREQ(IToA<312839>::value.c_str(), "312839");

    EXPECT_STREQ(IToA<-1>::value.c_str(), "-1");
    EXPECT_STREQ(IToA<-83294>::value.c_str(), "-83294");
}

TEST(IToA, HasLengthMember) {
    EXPECT_EQ(IToA<0>::length, 1);

    EXPECT_EQ(IToA<2>::length, 1);
    EXPECT_EQ(IToA<9>::length, 1);
    EXPECT_EQ(IToA<10>::length, 2);
    EXPECT_EQ(IToA<12345>::length, 5);

    EXPECT_EQ(IToA<-2>::length, 2);
    EXPECT_EQ(IToA<-9>::length, 2);
    EXPECT_EQ(IToA<-10>::length, 3);
    EXPECT_EQ(IToA<-12345>::length, 6);
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
