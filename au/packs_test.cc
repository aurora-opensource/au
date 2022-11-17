// Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

#include "au/packs.hh"

#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {

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

TEST(BaseT, IdentityForArbitraryTypes) {
    StaticAssertTypeEq<BaseT<int>, int>();
    StaticAssertTypeEq<BaseT<char>, char>();
}

TEST(BaseT, FirstArgumentOfPow) {
    StaticAssertTypeEq<BaseT<Pow<int, 3>>, int>();
    StaticAssertTypeEq<BaseT<Pow<char, -1>>, char>();
}

TEST(BaseT, FirstArgumentOfRatioPow) {
    StaticAssertTypeEq<BaseT<RatioPow<int, 3, 2>>, int>();
    StaticAssertTypeEq<BaseT<RatioPow<char, -1, 4>>, char>();
}

TEST(ExpT, Ratio1ForArbitraryTypes) {
    StaticAssertTypeEq<ExpT<int>, std::ratio<1>>();
    StaticAssertTypeEq<ExpT<char>, std::ratio<1>>();
}

TEST(ExpT, SecondArgumentOfPowAsRatio) {
    StaticAssertTypeEq<ExpT<Pow<int, 3>>, std::ratio<3>>();
    StaticAssertTypeEq<ExpT<Pow<char, -1>>, std::ratio<-1>>();
}

TEST(ExpT, RatioOfFinalTwoArgumentsOfRatioPow) {
    StaticAssertTypeEq<ExpT<RatioPow<int, 3, 2>>, std::ratio<3, 2>>();
    StaticAssertTypeEq<ExpT<RatioPow<char, -1, 4>>, std::ratio<-1, 4>>();
}

TEST(AsPackT, IdentityForPackOfSameType) {
    StaticAssertTypeEq<AsPackT<Pack, Pack<>>, Pack<>>();
    StaticAssertTypeEq<AsPackT<Pack, Pack<int>>, Pack<int>>();
    StaticAssertTypeEq<AsPackT<Pack, Pack<int, char>>, Pack<int, char>>();
}

TEST(AsPackT, WrapsOtherTypes) {
    StaticAssertTypeEq<AsPackT<Pack, int>, Pack<int>>();
    StaticAssertTypeEq<AsPackT<Pack, char>, Pack<char>>();
}

TEST(UnpackIfSoloT, ReturnsEnclosedElementIfExactlyOne) {
    StaticAssertTypeEq<UnpackIfSoloT<Pack, Pack<>>, Pack<>>();

    StaticAssertTypeEq<UnpackIfSoloT<Pack, Pack<int>>, int>();
    StaticAssertTypeEq<UnpackIfSoloT<Pack, Pack<char>>, char>();

    StaticAssertTypeEq<UnpackIfSoloT<Pack, Pack<int, char>>, Pack<int, char>>();
}

TEST(PackProductT, UnaryProductIsIdentity) {
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
    EXPECT_FALSE((IsValidPack<Pack, int>::value));
    EXPECT_FALSE((IsValidPack<Pack, std::ratio<3, 2>>::value));
}

TEST(LexicographicTotalOrdering, SecondArgumentIgnoredIfFirstIsUnambiguous) {
    EXPECT_TRUE((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<1, 0>>::value));
    EXPECT_TRUE((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<1, 8>>::value));
    EXPECT_TRUE((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<1, -8>>::value));

    EXPECT_FALSE((InOrderFor<LexiPack, TwoIndex<1, 0>, TwoIndex<0, 0>>::value));
    EXPECT_FALSE((InOrderFor<LexiPack, TwoIndex<1, 0>, TwoIndex<0, 8>>::value));
    EXPECT_FALSE((InOrderFor<LexiPack, TwoIndex<1, 0>, TwoIndex<0, -8>>::value));
}

TEST(LexicographicTotalOrdering, SecondArgumentBreaksTieInFirst) {
    EXPECT_FALSE((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<0, 0>>::value));
    EXPECT_TRUE((InOrderFor<LexiPack, TwoIndex<0, 0>, TwoIndex<0, 1>>::value));
}

TEST(InStandardPackOrder, NullPackComesBeforeEveryNonNullPack) {
    EXPECT_FALSE((InStandardPackOrder<Pack<>, Pack<>>::value));

    EXPECT_TRUE((InStandardPackOrder<Pack<>, Pack<B<2>>>::value));
    EXPECT_TRUE(
        (InStandardPackOrder<Pack<>, Pack<B<2>, Pow<B<3>, 8>, RatioPow<B<7>, 1, 4>>>::value));
}

TEST(InStandardPackOrder, IfLeadingBasesUnequalPicksWhicheverComesFirst) {
    EXPECT_TRUE((InStandardPackOrder<Pack<B<2>>, Pack<B<3>>>::value));
    EXPECT_TRUE((InStandardPackOrder<Pack<Pow<B<2>, 99>>, Pack<B<3>>>::value));
    EXPECT_TRUE((InStandardPackOrder<Pack<B<2>>, Pack<RatioPow<B<3>, -1, 38>>>::value));
}

TEST(InStandardPackOrder, IfLeadingBasesEqualUsesSmallerExpToBreakTie) {
    EXPECT_TRUE((InStandardPackOrder<Pack<Pow<B<2>, -1>>, Pack<B<2>>>::value));

    EXPECT_TRUE((InStandardPackOrder<Pack<B<2>>, Pack<RatioPow<B<2>, 11, 10>>>::value));
    EXPECT_FALSE((InStandardPackOrder<Pack<B<2>>, Pack<RatioPow<B<2>, 9, 10>>>::value));
}

TEST(InStandardPackOrder, IfLeadingBaseAndExpEqualRecursesToRemainder) {
    EXPECT_TRUE((InStandardPackOrder<Pack<Pow<B<2>, 4>, B<3>>, Pack<Pow<B<2>, 4>, B<5>>>::value));

    EXPECT_TRUE(
        (InStandardPackOrder<Pack<Pow<B<2>, 4>, B<3>>, Pack<Pow<B<2>, 4>, B<3>, B<5>>>::value));

    EXPECT_FALSE((InStandardPackOrder<Pack<Pow<B<2>, 4>, B<3>>,
                                      Pack<Pow<B<2>, 4>, RatioPow<B<3>, 99, 100>, B<5>>>::value));
}

TEST(IsValidPack, TrueForEmptyPack) { EXPECT_TRUE((IsValidPack<Pack, Pack<>>::value)); }

TEST(IsValidPack, TrueForSingleElementPack) {
    EXPECT_TRUE((IsValidPack<Pack, Pack<B<3>>>::value));
    EXPECT_TRUE((IsValidPack<Pack, Pack<B<4>>>::value));
    EXPECT_TRUE((IsValidPack<Pack, Pack<Pie>>::value));
}

TEST(IsValidPack, TrueForMoreComplicatedButValidExample) {
    EXPECT_TRUE((IsValidPack<Pack, Pack<B<2>, Pow<B<3>, -2>, RatioPow<Pie, 1, 3>>>::value));
}

TEST(IsValidPack, BecomesFalseIfAnyExponentZeroedOut) {
    ASSERT_TRUE((IsValidPack<Pack, Pack<B<2>, Pow<B<3>, -2>, RatioPow<Pie, 1, 3>>>::value));

    EXPECT_FALSE((IsValidPack<Pack, Pack<B<2>, Pow<B<3>, 0>, RatioPow<Pie, 1, 3>>>::value));
    EXPECT_FALSE((IsValidPack<Pack, Pack<B<2>, Pow<B<3>, -2>, RatioPow<Pie, 0, 3>>>::value));
}

TEST(AreBasesInOrder, TrueForEmptyPack) { EXPECT_TRUE((AreBasesInOrder<Pack, Pack<>>::value)); }

TEST(AreBasesInOrder, TrueForSingleElementPack) {
    EXPECT_TRUE((AreBasesInOrder<Pack, Pack<B<3>>>::value));
    EXPECT_TRUE((AreBasesInOrder<Pack, Pack<B<4>>>::value));
    EXPECT_TRUE((AreBasesInOrder<Pack, Pack<Pie>>::value));
}

TEST(AreBasesInOrder, DependsOnOrderingForTwoElementPack) {
    EXPECT_TRUE((AreBasesInOrder<Pack, Pack<B<3>, B<4>>>::value));
    EXPECT_FALSE((AreBasesInOrder<Pack, Pack<B<4>, B<3>>>::value));
}

TEST(AreBasesInOrder, CanMixIntegerAndFloatingPointCorrectly) {
    EXPECT_TRUE((AreBasesInOrder<Pack, Pack<B<3>, Pie, B<4>>>::value));
    EXPECT_FALSE((AreBasesInOrder<Pack, Pack<Pie, B<3>, B<4>>>::value));
    EXPECT_FALSE((AreBasesInOrder<Pack, Pack<B<3>, B<4>, Pie>>::value));
}

TEST(AreBasesInOrder, FalseIfAnyElementRepeated) {
    EXPECT_FALSE((AreBasesInOrder<Pack, Pack<B<3>, B<3>, B<4>>>::value));
    EXPECT_FALSE((AreBasesInOrder<Pack, Pack<B<3>, B<4>, B<4>>>::value));
}

TEST(AreAllPowersNonzero, TrueForEmptyPack) {
    EXPECT_TRUE((AreAllPowersNonzero<Pack, Pack<>>::value));
}

TEST(AreAllPowersNonzero, DependsOnPowerForSingleElementPack) {
    EXPECT_TRUE((AreAllPowersNonzero<Pack, Pack<B<3>>>::value));
    EXPECT_FALSE((AreAllPowersNonzero<Pack, Pack<Pow<B<3>, 0>>>::value));

    EXPECT_TRUE((AreAllPowersNonzero<Pack, Pack<Pie>>::value));
    EXPECT_FALSE((AreAllPowersNonzero<Pack, Pack<Pow<Pie, 0>>>::value));
}

TEST(AreAllPowersNonzero, AllMustSatisfyForMultiElementPack) {
    EXPECT_TRUE(
        (AreAllPowersNonzero<Pack, Pack<RatioPow<B<3>, -1, 2>, Pow<Pie, 8>, Pow<B<5>, 2>>>::value));

    EXPECT_FALSE((AreAllPowersNonzero<Pack, Pack<Pow<B<3>, 0>, Pow<Pie, 8>, Pow<B<5>, 2>>>::value));

    EXPECT_FALSE(
        (AreAllPowersNonzero<Pack, Pack<RatioPow<B<3>, -1, 2>, Pow<Pie, 0>, Pow<B<5>, 2>>>::value));

    EXPECT_FALSE(
        (AreAllPowersNonzero<Pack, Pack<RatioPow<B<3>, -1, 2>, Pow<Pie, 8>, Pow<B<5>, 0>>>::value));
}

namespace detail {

TEST(SimplifyBasePowersT, SimplifiesEachIndividualBasePower) {
    StaticAssertTypeEq<SimplifyBasePowersT<Pack<B<2>,                      // A. Leave alone.
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

TEST(NumeratorPartT, PullsOutPositivePowers) {
    StaticAssertTypeEq<NumeratorPartT<Pack<>>, Pack<>>();
    StaticAssertTypeEq<NumeratorPartT<Pack<B<2>>>, Pack<B<2>>>();

    StaticAssertTypeEq<NumeratorPartT<Pack<Pow<B<2>, 3>>>, Pack<Pow<B<2>, 3>>>();
    StaticAssertTypeEq<NumeratorPartT<Pack<Pow<B<2>, -3>>>, Pack<>>();

    StaticAssertTypeEq<NumeratorPartT<Pack<RatioPow<B<2>, 3, 2>>>, Pack<RatioPow<B<2>, 3, 2>>>();
    StaticAssertTypeEq<NumeratorPartT<Pack<RatioPow<B<2>, -3, 2>>>, Pack<>>();

    StaticAssertTypeEq<NumeratorPartT<Pack<Pow<B<2>, 2>, Pow<B<3>, -6>, Pow<B<5>, -3>, B<7>>>,
                       Pack<Pow<B<2>, 2>, B<7>>>();
}

TEST(DenominatorPartT, PullsOutAndInvertsNegativePowers) {
    StaticAssertTypeEq<DenominatorPartT<Pack<>>, Pack<>>();

    StaticAssertTypeEq<DenominatorPartT<Pack<Pow<B<2>, -1>>>, Pack<B<2>>>();

    StaticAssertTypeEq<DenominatorPartT<Pack<Pow<B<2>, 3>>>, Pack<>>();
    StaticAssertTypeEq<DenominatorPartT<Pack<Pow<B<2>, -3>>>, Pack<Pow<B<2>, 3>>>();

    StaticAssertTypeEq<DenominatorPartT<Pack<RatioPow<B<2>, 3, 2>>>, Pack<>>();
    StaticAssertTypeEq<DenominatorPartT<Pack<RatioPow<B<2>, -3, 2>>>, Pack<RatioPow<B<2>, 3, 2>>>();

    StaticAssertTypeEq<DenominatorPartT<Pack<Pow<B<2>, 2>, Pow<B<3>, -6>, Pow<B<5>, -3>, B<7>>>,
                       Pack<Pow<B<3>, 6>, Pow<B<5>, 3>>>();
}

}  // namespace detail
}  // namespace au
