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

#include "au/power_aliases.hh"

#include <cstdint>

#include "au/packs.hh"
#include "gtest/gtest.h"

using ::testing::StaticAssertTypeEq;

namespace au {

// An arbitrary monovalue type which we'll enable raising to powers.
//
// We'll use the `:packs` machinery as an easy way to write meaningful tests.
template <typename... BPs>
struct Vector {};

// "B" is for "Base".
template <int N>
struct B {
    static constexpr int index = N;
};
template <int N>
constexpr int B<N>::index;

// Defining `pow<N>(Vector<...>)` is what unlocks `squared`, `cubed`, and `inverse`.
template <std::intmax_t N, typename... BPs>
constexpr PackPowerT<Vector, Vector<BPs...>, N> pow(Vector<BPs...>) {
    return {};
}

// Defining `root<N>(Vector<...>)` is what unlocks `sqrt` and `cbrt`.
template <std::intmax_t N, typename... BPs>
constexpr PackPowerT<Vector, Vector<BPs...>, 1, N> root(Vector<BPs...>) {
    return {};
}

TEST(Inverse, RaisesToPowerNegativeOne) {
    StaticAssertTypeEq<decltype(inverse(Vector<>{})), Vector<>>();

    StaticAssertTypeEq<decltype(inverse(Vector<B<2>, Pow<B<3>, 8>, RatioPow<B<5>, 1, 2>>{})),
                       Vector<Pow<B<2>, -1>, Pow<B<3>, -8>, RatioPow<B<5>, -1, 2>>>();
}

TEST(Inverse, TypeBasedFormGivesExpectedResult) {
    StaticAssertTypeEq<Inverse<Vector<>>, Vector<>>();

    StaticAssertTypeEq<Inverse<Vector<B<2>, Pow<B<3>, 8>, RatioPow<B<5>, 1, 2>>>,
                       Vector<Pow<B<2>, -1>, Pow<B<3>, -8>, RatioPow<B<5>, -1, 2>>>();
}

TEST(Squared, RaisesToPowerTwo) {
    StaticAssertTypeEq<decltype(squared(Vector<>{})), Vector<>>();

    StaticAssertTypeEq<decltype(squared(Vector<B<2>, Pow<B<3>, 8>, RatioPow<B<5>, 1, 2>>{})),
                       Vector<Pow<B<2>, 2>, Pow<B<3>, 16>, B<5>>>();
}

TEST(Squared, TypeBasedFormGivesExpectedResult) {
    StaticAssertTypeEq<Squared<Vector<>>, Vector<>>();

    StaticAssertTypeEq<Squared<Vector<B<2>, Pow<B<3>, 8>, RatioPow<B<5>, 1, 2>>>,
                       Vector<Pow<B<2>, 2>, Pow<B<3>, 16>, B<5>>>();
}

TEST(Cubed, RaisesToPowerThree) {
    StaticAssertTypeEq<decltype(cubed(Vector<>{})), Vector<>>();

    StaticAssertTypeEq<decltype(cubed(Vector<B<2>, Pow<B<3>, 8>, RatioPow<B<5>, 1, 3>>{})),
                       Vector<Pow<B<2>, 3>, Pow<B<3>, 24>, B<5>>>();
}

TEST(Cubed, TypeBasedFormGivesExpectedResult) {
    StaticAssertTypeEq<Cubed<Vector<>>, Vector<>>();

    StaticAssertTypeEq<Cubed<Vector<B<2>, Pow<B<3>, 8>, RatioPow<B<5>, 1, 3>>>,
                       Vector<Pow<B<2>, 3>, Pow<B<3>, 24>, B<5>>>();
}

TEST(Sqrt, TakesSecondRoot) {
    StaticAssertTypeEq<decltype(sqrt(Vector<>{})), Vector<>>();

    StaticAssertTypeEq<decltype(sqrt(Vector<B<2>, Pow<B<3>, 8>, Pow<B<5>, 2>>{})),
                       Vector<RatioPow<B<2>, 1, 2>, Pow<B<3>, 4>, B<5>>>();
}

TEST(Sqrt, TypeBasedFormGivesExpectedResult) {
    StaticAssertTypeEq<Sqrt<Vector<>>, Vector<>>();

    StaticAssertTypeEq<Sqrt<Vector<B<2>, Pow<B<3>, 8>, Pow<B<5>, 2>>>,
                       Vector<RatioPow<B<2>, 1, 2>, Pow<B<3>, 4>, B<5>>>();
}

TEST(Cbrt, TakesThirdRoot) {
    StaticAssertTypeEq<decltype(cbrt(Vector<>{})), Vector<>>();

    StaticAssertTypeEq<decltype(cbrt(Vector<B<2>, Pow<B<3>, 9>, Pow<B<5>, 3>>{})),
                       Vector<RatioPow<B<2>, 1, 3>, Pow<B<3>, 3>, B<5>>>();
}

TEST(Cbrt, TypeBasedFormGivesExpectedResult) {
    StaticAssertTypeEq<Cbrt<Vector<>>, Vector<>>();

    StaticAssertTypeEq<Cbrt<Vector<B<2>, Pow<B<3>, 9>, Pow<B<5>, 3>>>,
                       Vector<RatioPow<B<2>, 1, 3>, Pow<B<3>, 3>, B<5>>>();
}

}  // namespace au
