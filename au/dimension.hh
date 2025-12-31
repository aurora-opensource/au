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

#pragma once

#include "au/fwd.hh"
#include "au/packs.hh"
#include "au/power_aliases.hh"

namespace au {

template <typename... BPs>
struct Dimension {
    // Having separate `static_assert` instances for the individual conditions produces more
    // readable errors if we fail.
    static_assert(AreAllPowersNonzero<Dimension, Dimension<BPs...>>::value,
                  "All powers must be nonzero");
    static_assert(AreBasesInOrder<Dimension, Dimension<BPs...>>::value,
                  "Bases must be listed in ascending order");

    // We also want to use the "full" validity check.  This should be equivalent to the above
    // conditions, but if we add more conditions later, we want them to get picked up here
    // automatically.
    static_assert(IsValidPack<Dimension, Dimension<BPs...>>::value, "Ill-formed Dimension");
};

// Define readable operations for product, quotient, power, inverse on Dimensions.
template <typename... BPs>
using DimProduct = PackProductT<Dimension, BPs...>;
template <typename... BPs>
using DimProductT = DimProduct<BPs...>;
template <typename T, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using DimPower = PackPowerT<Dimension, T, ExpNum, ExpDen>;
template <typename T, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using DimPowerT = DimPower<T, ExpNum, ExpDen>;

template <typename T, typename U>
using DimQuotient = PackQuotientT<Dimension, T, U>;
template <typename T, typename U>
using DimQuotientT = DimQuotient<T, U>;

template <typename T>
using DimInverse = PackInverseT<Dimension, T>;
template <typename T>
using DimInverseT = DimInverse<T>;

template <typename... BP1s, typename... BP2s>
constexpr auto operator*(Dimension<BP1s...>, Dimension<BP2s...>) {
    return DimProduct<Dimension<BP1s...>, Dimension<BP2s...>>{};
}

template <typename... BP1s, typename... BP2s>
constexpr auto operator/(Dimension<BP1s...>, Dimension<BP2s...>) {
    return DimQuotient<Dimension<BP1s...>, Dimension<BP2s...>>{};
}

// Roots and powers for Dimension instances.
template <std::intmax_t N, typename... BPs>
constexpr DimPower<Dimension<BPs...>, N> pow(Dimension<BPs...>) {
    return {};
}
template <std::intmax_t N, typename... BPs>
constexpr DimPower<Dimension<BPs...>, 1, N> root(Dimension<BPs...>) {
    return {};
}

template <typename... Dims>
struct CommonDimensionImpl;
template <typename... Dims>
using CommonDimension = typename CommonDimensionImpl<Dims...>::type;
template <typename... Dims>
using CommonDimensionT = CommonDimension<Dims...>;

template <typename... BaseDims>
struct CommonDimensionImpl<Dimension<BaseDims...>> : stdx::type_identity<Dimension<BaseDims...>> {};
template <typename Head, typename... Tail>
struct CommonDimensionImpl<Head, Tail...> : CommonDimensionImpl<Tail...> {
    static_assert(std::is_same<Head, CommonDimension<Tail...>>::value,
                  "Common dimension only defined when all dimensions are identical");
};

namespace base_dim {

template <int64_t I>
struct BaseDimension {
    static constexpr int64_t base_dim_index = I;
};
template <int64_t I>
constexpr int64_t BaseDimension<I>::base_dim_index;

template <typename T, typename U>
struct OrderByBaseDimIndex : stdx::bool_constant<(T::base_dim_index < U::base_dim_index)> {};

struct Length : BaseDimension<-99> {};
struct Mass : BaseDimension<-98> {};
struct Time : BaseDimension<-97> {};
struct Current : BaseDimension<-96> {};
struct Temperature : BaseDimension<-95> {};
struct Angle : BaseDimension<-94> {};
struct Information : BaseDimension<-93> {};
struct AmountOfSubstance : BaseDimension<-92> {};
struct LuminousIntensity : BaseDimension<-91> {};

}  // namespace base_dim

template <typename A, typename B>
struct InOrderFor<Dimension, A, B>
    : LexicographicTotalOrdering<A, B, base_dim::OrderByBaseDimIndex> {};

// The types we want to expose to the rest of the library internals are the full-fledged Dimensions,
// not the Base Dimensions, because Dimensions are easier to work with (we can take products,
// quotients, powers, etc.).
using Length = Dimension<base_dim::Length>;
using Mass = Dimension<base_dim::Mass>;
using Time = Dimension<base_dim::Time>;
using Current = Dimension<base_dim::Current>;
using Temperature = Dimension<base_dim::Temperature>;
using Angle = Dimension<base_dim::Angle>;
using Information = Dimension<base_dim::Information>;
using AmountOfSubstance = Dimension<base_dim::AmountOfSubstance>;
using LuminousIntensity = Dimension<base_dim::LuminousIntensity>;

}  // namespace au
