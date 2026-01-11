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

#include "au/packs.hh"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

// Short, readable pack name for testing purposes.
template <typename... Ts>
struct Pack;

// Define a total ordering for the types we accept for Pack.
template <typename A, typename B>
struct OrderByIndex : stdx::bool_constant<(A::index < B::index)> {};
template <typename A, typename B>
struct InOrderFor<Pack, A, B> : LexicographicTotalOrdering<A, B, OrderByIndex> {};

// "B" is for "Base".
template <int N>
struct B {
    static constexpr int index = N;
};
template <int N>
constexpr int B<N>::index;

// A struct which is compatible with `Pack`, even though its `::index` has a different _type_.
struct Pie {
    static constexpr double index = 3.14;
};
constexpr double Pie::index;

// A base type with two indices will let us test more complicated lexicographic sorting.
template <int N1, int N2>
struct TwoIndex {
    static constexpr int i1 = N1;
    static constexpr int i2 = N2;
};
template <int N1, int N2>
constexpr int TwoIndex<N1, N2>::i1;
template <int N1, int N2>
constexpr int TwoIndex<N1, N2>::i2;

// A pack that uses the new lexicographic ordering.
template <typename... Ts>
struct LexiPack;

// The total ordering for LexiPack uses the lexicographic ordering for TwoIndex bases.
template <typename A, typename B>
struct OrderByFirst : stdx::bool_constant<(A::i1 < B::i1)> {};
template <typename A, typename B>
struct OrderBySecond : stdx::bool_constant<(A::i2 < B::i2)> {};
template <typename A, typename B>
struct InOrderFor<LexiPack, A, B> : LexicographicTotalOrdering<A, B, OrderByFirst, OrderBySecond> {
};

TEST(Base, IdentityForArbitraryTypes) {
    StaticAssertTypeEq<Base<int>, int>();
    StaticAssertTypeEq<Base<char>, char>();
}

TEST(Base, FirstArgumentOfPow) {
    StaticAssertTypeEq<Base<Pow<int, 3>>, int>();
    StaticAssertTypeEq<Base<Pow<char, -1>>, char>();
}

TEST(Base, FirstArgumentOfRatioPow) {
    StaticAssertTypeEq<Base<RatioPow<int, 3, 2>>, int>();
    StaticAssertTypeEq<Base<RatioPow<char, -1, 4>>, char>();
}

TEST(Exp, Ratio1ForArbitraryTypes) {
    StaticAssertTypeEq<Exp<int>, std::ratio<1>>();
    StaticAssertTypeEq<Exp<char>, std::ratio<1>>();
}

TEST(Exp, SecondArgumentOfPowAsRatio) {
    StaticAssertTypeEq<Exp<Pow<int, 3>>, std::ratio<3>>();
    StaticAssertTypeEq<Exp<Pow<char, -1>>, std::ratio<-1>>();
}

TEST(Exp, RatioOfFinalTwoArgumentsOfRatioPow) {
    StaticAssertTypeEq<Exp<RatioPow<int, 3, 2>>, std::ratio<3, 2>>();
    StaticAssertTypeEq<Exp<RatioPow<char, -1, 4>>, std::ratio<-1, 4>>();
}

TEST(AsPack, IdentityForPackOfSameType) {
    StaticAssertTypeEq<AsPack<Pack, Pack<>>, Pack<>>();
    StaticAssertTypeEq<AsPack<Pack, Pack<int>>, Pack<int>>();
    StaticAssertTypeEq<AsPack<Pack, Pack<int, char>>, Pack<int, char>>();
}

TEST(AsPack, WrapsOtherTypes) {
    StaticAssertTypeEq<AsPack<Pack, int>, Pack<int>>();
    StaticAssertTypeEq<AsPack<Pack, char>, Pack<char>>();
}

TEST(UnpackIfSolo, ReturnsEnclosedElementIfExactlyOne) {
    StaticAssertTypeEq<UnpackIfSolo<Pack, Pack<>>, Pack<>>();

    StaticAssertTypeEq<UnpackIfSolo<Pack, Pack<int>>, int>();
    StaticAssertTypeEq<UnpackIfSolo<Pack, Pack<char>>, char>();

    StaticAssertTypeEq<UnpackIfSolo<Pack, Pack<int, char>>, Pack<int, char>>();
}

TEST(PackProductT, UnaryProductIsIdentity) {
    StaticAssertTypeEq<PackProductT<Pack, Pack<>>, Pack<>>();
    StaticAssertTypeEq<PackProductT<Pack, Pack<B<3>>>, Pack<B<3>>>();
}

TEST(PackProductT, BinaryProductOfNullPacksIsNullPack) {
    StaticAssertTypeEq<PackProductT<Pack, Pack<>, Pack<>>, Pack<>>();
}

TEST(PackProductT, BinaryProductOfNullPackWithNonNullPackGivesNonNullPack) {
    StaticAssertTypeEq<PackProductT<Pack, Pack<>, Pack<B<2>, B<3>>>, Pack<B<2>, B<3>>>();
    StaticAssertTypeEq<PackProductT<Pack, Pack<B<2>, B<3>>, Pack<>>, Pack<B<2>, B<3>>>();
}

TEST(PackProductT, BinaryProductOfNonNullPackWithEarlierBasePrependsBase) {
    StaticAssertTypeEq<PackProductT<Pack, Pack<B<1>>, Pack<B<2>, B<3>>>, Pack<B<1>, B<2>, B<3>>>();
    StaticAssertTypeEq<PackProductT<Pack, Pack<B<2>>, Pack<B<1>, B<3>>>, Pack<B<1>, B<2>, B<3>>>();
}

TEST(PackProductT, BinaryProductWithSameHeadBaseAddsExponents) {
    StaticAssertTypeEq<PackProductT<Pack, Pack<B<2>>, Pack<B<2>>>, Pack<Pow<B<2>, 2>>>();

    StaticAssertTypeEq<PackProductT<Pack, Pack<Pow<B<2>, -1>>, Pack<B<2>>>, Pack<>>();

    StaticAssertTypeEq<PackProductT<Pack, Pack<Pow<B<2>, 2>>, Pack<RatioPow<B<2>, -3, 4>>>,
                       Pack<RatioPow<B<2>, 5, 4>>>();

    StaticAssertTypeEq<PackProductT<Pack, Pack<RatioPow<B<2>, 7, 4>>, Pack<RatioPow<B<2>, -3, 4>>>,
                       Pack<B<2>>>();
}

TEST(PackProductT, NaryProductRecurses) {
    StaticAssertTypeEq<PackProductT<Pack, Pack<B<7>>, Pack<B<2>>, Pack<B<5>>>,
                       Pack<B<2>, B<5>, B<7>>>();
}

TEST(PackPowerT, MultipliesExponentsAndSimplifies) {
    StaticAssertTypeEq<
        PackPowerT<Pack, Pack<B<2>, Pow<B<3>, -3>, RatioPow<B<5>, -3, 2>, RatioPow<B<7>, 1, 2>>, 2>,
        Pack<Pow<B<2>, 2>, Pow<B<3>, -6>, Pow<B<5>, -3>, B<7>>>();
}

TEST(FlatDedupedTypeListT, OrdersAsExpected) {
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, Pack<B<2>>>, Pack<B<2>>>();

    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<2>, B<3>>, Pack<B<2>, B<3>>>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<3>, B<2>>, Pack<B<2>, B<3>>>();

    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<2>, B<3>, B<5>>, Pack<B<2>, B<3>, B<5>>>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<2>, B<5>, B<3>>, Pack<B<2>, B<3>, B<5>>>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<3>, B<2>, B<5>>, Pack<B<2>, B<3>, B<5>>>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<3>, B<5>, B<2>>, Pack<B<2>, B<3>, B<5>>>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<5>, B<2>, B<3>>, Pack<B<2>, B<3>, B<5>>>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<5>, B<3>, B<2>>, Pack<B<2>, B<3>, B<5>>>();
}

TEST(FlatDedupedTypeListT, DedupesAtAnyPosition) {
    using T = Pack<B<2>, B<3>, B<5>, B<7>, B<11>>;

    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<2>, T>, T>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<3>, T>, T>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<5>, T>, T>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<7>, T>, T>();
    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, B<11>, T>, T>();

    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, T, T>, T>();

    StaticAssertTypeEq<FlatDedupedTypeListT<Pack, T, Pack<B<2>>, T, B<11>, T>, T>();
}

TEST(PackPowerT, SupportsRationalPowers) {
    StaticAssertTypeEq<
        PackPowerT<Pack, Pack<Pow<B<2>, 2>, Pow<B<3>, -6>, Pow<B<5>, -3>, B<7>>, 1, 2>,
        Pack<B<2>, Pow<B<3>, -3>, RatioPow<B<5>, -3, 2>, RatioPow<B<7>, 1, 2>>>();
}

TEST(PackPowerT, SupportsZeroPower) {
    StaticAssertTypeEq<PackPowerT<Pack, Pack<Pow<B<2>, 2>, Pow<B<3>, -6>, Pow<B<5>, -3>, B<7>>, 0>,
                       Pack<>>();
}

TEST(PackQuotientT, PackQuotientWithItselfIsNullPack) {
    StaticAssertTypeEq<PackQuotientT<Pack, Pack<B<2>>, Pack<B<2>>>, Pack<>>();

    using ArbitraryPack = Pack<Pow<B<2>, 2>, RatioPow<Pie, 22, 7>, B<13>>;
    StaticAssertTypeEq<PackQuotientT<Pack, ArbitraryPack, ArbitraryPack>, Pack<>>();
}

TEST(PackQuotientT, InverseOfProduct) {
    using B2_B5 = PackProductT<Pack, Pack<B<2>>, Pack<B<5>>>;
    StaticAssertTypeEq<PackQuotientT<Pack, B2_B5, Pack<B<2>>>, Pack<B<5>>>();
}

TEST(PackInverseT, ProductWithOriginalIsNullPack) {
    StaticAssertTypeEq<PackProductT<Pack, PackInverseT<Pack, Pack<B<2>>>, Pack<B<2>>>, Pack<>>();
}

TEST(IsValidPack, FalseIfNotInstanceOfPack) {
    EXPECT_THAT((IsValidPack<Pack, int>::value), IsFalse());
    EXPECT_THAT((IsValidPack<Pack, std::ratio<3, 2>>::value), IsFalse());
}

TEST(LexicographicTotalOrdering, SecondArgumentIgnoredIfFirstIsUnambiguous) {
    EXPECT_THAT((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<1, 0>>::value), IsTrue());
    EXPECT_THAT((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<1, 8>>::value), IsTrue());
    EXPECT_THAT((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<1, -8>>::value), IsTrue());

    EXPECT_THAT((InOrderFor<LexiPack, TwoIndex<1, 0>, TwoIndex<0, 0>>::value), IsFalse());
    EXPECT_THAT((InOrderFor<LexiPack, TwoIndex<1, 0>, TwoIndex<0, 8>>::value), IsFalse());
    EXPECT_THAT((InOrderFor<LexiPack, TwoIndex<1, 0>, TwoIndex<0, -8>>::value), IsFalse());
}

TEST(LexicographicTotalOrdering, SecondArgumentBreaksTieInFirst) {
    EXPECT_THAT((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<0, 0>>::value), IsFalse());
    EXPECT_THAT((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<0, 1>>::value), IsTrue());
}

TEST(InsertUsingOrderingFor, UsesRequestedOrdering) {
    StaticAssertTypeEq<InsertUsingOrderingFor<LexiPack, TwoIndex<5, 0>, LexiPack<>>,
                       LexiPack<TwoIndex<5, 0>>>();
    StaticAssertTypeEq<InsertUsingOrderingFor<LexiPack, TwoIndex<4, 0>, LexiPack<TwoIndex<5, 0>>>,
                       LexiPack<TwoIndex<4, 0>, TwoIndex<5, 0>>>();
    StaticAssertTypeEq<InsertUsingOrderingFor<LexiPack, TwoIndex<6, 0>, LexiPack<TwoIndex<5, 0>>>,
                       LexiPack<TwoIndex<5, 0>, TwoIndex<6, 0>>>();
    StaticAssertTypeEq<
        InsertUsingOrderingFor<LexiPack, TwoIndex<5, 1>, LexiPack<TwoIndex<5, 0>, TwoIndex<6, 0>>>,
        LexiPack<TwoIndex<5, 0>, TwoIndex<5, 1>, TwoIndex<6, 0>>>();
}

TEST(SortAs, SortsAs) {
    StaticAssertTypeEq<SortAs<LexiPack, Pack<>>, Pack<>>();

    StaticAssertTypeEq<SortAs<LexiPack, Pack<TwoIndex<1, 1>>>, Pack<TwoIndex<1, 1>>>();

    StaticAssertTypeEq<SortAs<LexiPack, Pack<TwoIndex<1, 1>, TwoIndex<2, 2>>>,
                       Pack<TwoIndex<1, 1>, TwoIndex<2, 2>>>();

    StaticAssertTypeEq<SortAs<LexiPack, Pack<TwoIndex<2, 2>, TwoIndex<1, 1>>>,
                       Pack<TwoIndex<1, 1>, TwoIndex<2, 2>>>();

    StaticAssertTypeEq<SortAs<LexiPack, Pack<TwoIndex<2, 2>, TwoIndex<3, 3>, TwoIndex<1, 1>>>,
                       Pack<TwoIndex<1, 1>, TwoIndex<2, 2>, TwoIndex<3, 3>>>();
}

TEST(InStandardPackOrder, NullPackComesBeforeEveryNonNullPack) {
    EXPECT_THAT((InStandardPackOrder<Pack<>, Pack<>>::value), IsFalse());

    EXPECT_THAT((InStandardPackOrder<Pack<>, Pack<B<2>>>::value), IsTrue());
    EXPECT_THAT(
        (InStandardPackOrder<Pack<>, Pack<B<2>, Pow<B<3>, 8>, RatioPow<B<7>, 1, 4>>>::value),
        IsTrue());
}

TEST(InStandardPackOrder, IfLeadingBasesUnequalPicksWhicheverComesFirst) {
    EXPECT_THAT((InStandardPackOrder<Pack<B<2>>, Pack<B<3>>>::value), IsTrue());
    EXPECT_THAT((InStandardPackOrder<Pack<Pow<B<2>, 99>>, Pack<B<3>>>::value), IsTrue());
    EXPECT_THAT((InStandardPackOrder<Pack<B<2>>, Pack<RatioPow<B<3>, -1, 38>>>::value), IsTrue());
}

TEST(InStandardPackOrder, IfLeadingBasesEqualUsesSmallerExpoBreakTie) {
    EXPECT_THAT((InStandardPackOrder<Pack<Pow<B<2>, -1>>, Pack<B<2>>>::value), IsTrue());

    EXPECT_THAT((InStandardPackOrder<Pack<B<2>>, Pack<RatioPow<B<2>, 11, 10>>>::value), IsTrue());
    EXPECT_THAT((InStandardPackOrder<Pack<B<2>>, Pack<RatioPow<B<2>, 9, 10>>>::value), IsFalse());
}

TEST(InStandardPackOrder, IfLeadingBaseAndExpEqualRecursesToRemainder) {
    EXPECT_THAT((InStandardPackOrder<Pack<Pow<B<2>, 4>, B<3>>, Pack<Pow<B<2>, 4>, B<5>>>::value),
                IsTrue());

    EXPECT_THAT(
        (InStandardPackOrder<Pack<Pow<B<2>, 4>, B<3>>, Pack<Pow<B<2>, 4>, B<3>, B<5>>>::value),
        IsTrue());

    EXPECT_THAT((InStandardPackOrder<Pack<Pow<B<2>, 4>, B<3>>,
                                     Pack<Pow<B<2>, 4>, RatioPow<B<3>, 99, 100>, B<5>>>::value),
                IsFalse());
}

TEST(IsValidPack, TrueForEmptyPack) { EXPECT_THAT((IsValidPack<Pack, Pack<>>::value), IsTrue()); }

TEST(IsValidPack, TrueForSingleElementPack) {
    EXPECT_THAT((IsValidPack<Pack, Pack<B<3>>>::value), IsTrue());
    EXPECT_THAT((IsValidPack<Pack, Pack<B<4>>>::value), IsTrue());
    EXPECT_THAT((IsValidPack<Pack, Pack<Pie>>::value), IsTrue());
}

TEST(IsValidPack, TrueForMoreComplicatedButValidExample) {
    EXPECT_THAT((IsValidPack<Pack, Pack<B<2>, Pow<B<3>, -2>, RatioPow<Pie, 1, 3>>>::value),
                IsTrue());
}

TEST(IsValidPack, BecomesFalseIfAnyExponentZeroedOut) {
    ASSERT_THAT((IsValidPack<Pack, Pack<B<2>, Pow<B<3>, -2>, RatioPow<Pie, 1, 3>>>::value),
                IsTrue());

    EXPECT_THAT((IsValidPack<Pack, Pack<B<2>, Pow<B<3>, 0>, RatioPow<Pie, 1, 3>>>::value),
                IsFalse());
    EXPECT_THAT((IsValidPack<Pack, Pack<B<2>, Pow<B<3>, -2>, RatioPow<Pie, 0, 3>>>::value),
                IsFalse());
}

TEST(AreBasesInOrder, TrueForEmptyPack) {
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<>>::value), IsTrue());
}

TEST(AreBasesInOrder, TrueForSingleElementPack) {
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<B<3>>>::value), IsTrue());
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<B<4>>>::value), IsTrue());
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<Pie>>::value), IsTrue());
}

TEST(AreBasesInOrder, DependsOnOrderingForTwoElementPack) {
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<B<3>, B<4>>>::value), IsTrue());
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<B<4>, B<3>>>::value), IsFalse());
}

TEST(AreBasesInOrder, CanMixIntegerAndFloatingPointCorrectly) {
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<B<3>, Pie, B<4>>>::value), IsTrue());
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<Pie, B<3>, B<4>>>::value), IsFalse());
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<B<3>, B<4>, Pie>>::value), IsFalse());
}

TEST(AreBasesInOrder, FalseIfAnyElementRepeated) {
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<B<3>, B<3>, B<4>>>::value), IsFalse());
    EXPECT_THAT((AreBasesInOrder<Pack, Pack<B<3>, B<4>, B<4>>>::value), IsFalse());
}

TEST(AreAllPowersNonzero, TrueForEmptyPack) {
    EXPECT_THAT((AreAllPowersNonzero<Pack, Pack<>>::value), IsTrue());
}

TEST(AreAllPowersNonzero, DependsOnPowerForSingleElementPack) {
    EXPECT_THAT((AreAllPowersNonzero<Pack, Pack<B<3>>>::value), IsTrue());
    EXPECT_THAT((AreAllPowersNonzero<Pack, Pack<Pow<B<3>, 0>>>::value), IsFalse());

    EXPECT_THAT((AreAllPowersNonzero<Pack, Pack<Pie>>::value), IsTrue());
    EXPECT_THAT((AreAllPowersNonzero<Pack, Pack<Pow<Pie, 0>>>::value), IsFalse());
}

TEST(AreAllPowersNonzero, AllMustSatisfyForMultiElementPack) {
    EXPECT_THAT(
        (AreAllPowersNonzero<Pack, Pack<RatioPow<B<3>, -1, 2>, Pow<Pie, 8>, Pow<B<5>, 2>>>::value),
        IsTrue());

    EXPECT_THAT((AreAllPowersNonzero<Pack, Pack<Pow<B<3>, 0>, Pow<Pie, 8>, Pow<B<5>, 2>>>::value),
                IsFalse());

    EXPECT_THAT(
        (AreAllPowersNonzero<Pack, Pack<RatioPow<B<3>, -1, 2>, Pow<Pie, 0>, Pow<B<5>, 2>>>::value),
        IsFalse());

    EXPECT_THAT(
        (AreAllPowersNonzero<Pack, Pack<RatioPow<B<3>, -1, 2>, Pow<Pie, 8>, Pow<B<5>, 0>>>::value),
        IsFalse());
}

namespace detail {

TEST(SimplifyBasePowers, SimplifiesEachIndividualBasePower) {
    StaticAssertTypeEq<SimplifyBasePowers<Pack<B<2>,                      // A. Leave alone.
                                               Pow<B<3>, 1>,              // B. Reduce to base.
                                               Pow<B<5>, 3>,              // C. Leave alone.
                                               RatioPow<B<7>, 1, 1>,      // D. Reduce to base.
                                               RatioPow<B<11>, -4, 1>,    // E. Reduce to int pow.
                                               RatioPow<B<13>, -4, 5>>>,  // F. Leave alone.

                       Pack<B<2>,                        // A. Unchanged.
                            B<3>,                        // B. Reduced to base.
                            Pow<B<5>, 3>,                // C. Unchanged.
                            B<7>,                        // D. Reduced to base.
                            Pow<B<11>, -4>,              // E. Reduced to integer pow.
                            RatioPow<B<13>, -4, 5>>>();  // F. Unchanged.
}

TEST(NumeratorPart, PullsOutPositivePowers) {
    StaticAssertTypeEq<NumeratorPart<Pack<>>, Pack<>>();
    StaticAssertTypeEq<NumeratorPart<Pack<B<2>>>, Pack<B<2>>>();

    StaticAssertTypeEq<NumeratorPart<Pack<Pow<B<2>, 3>>>, Pack<Pow<B<2>, 3>>>();
    StaticAssertTypeEq<NumeratorPart<Pack<Pow<B<2>, -3>>>, Pack<>>();

    StaticAssertTypeEq<NumeratorPart<Pack<RatioPow<B<2>, 3, 2>>>, Pack<RatioPow<B<2>, 3, 2>>>();
    StaticAssertTypeEq<NumeratorPart<Pack<RatioPow<B<2>, -3, 2>>>, Pack<>>();

    StaticAssertTypeEq<NumeratorPart<Pack<Pow<B<2>, 2>, Pow<B<3>, -6>, Pow<B<5>, -3>, B<7>>>,
                       Pack<Pow<B<2>, 2>, B<7>>>();
}

TEST(DenominatorPart, PullsOutAndInvertsNegativePowers) {
    StaticAssertTypeEq<DenominatorPart<Pack<>>, Pack<>>();

    StaticAssertTypeEq<DenominatorPart<Pack<Pow<B<2>, -1>>>, Pack<B<2>>>();

    StaticAssertTypeEq<DenominatorPart<Pack<Pow<B<2>, 3>>>, Pack<>>();
    StaticAssertTypeEq<DenominatorPart<Pack<Pow<B<2>, -3>>>, Pack<Pow<B<2>, 3>>>();

    StaticAssertTypeEq<DenominatorPart<Pack<RatioPow<B<2>, 3, 2>>>, Pack<>>();
    StaticAssertTypeEq<DenominatorPart<Pack<RatioPow<B<2>, -3, 2>>>, Pack<RatioPow<B<2>, 3, 2>>>();

    StaticAssertTypeEq<DenominatorPart<Pack<Pow<B<2>, 2>, Pow<B<3>, -6>, Pow<B<5>, -3>, B<7>>>,
                       Pack<Pow<B<3>, 6>, Pow<B<5>, 3>>>();
}

}  // namespace detail
}  // namespace au
