// Copyright 2024 Aurora Operations, Inc.
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

#include <cstdint>

namespace au {

struct Zero;

template <typename B, std::intmax_t N>
struct Pow;

template <typename B, std::intmax_t N, std::intmax_t D>
struct RatioPow;

template <typename... BPs>
struct Dimension;

template <typename... BPs>
struct Magnitude;

template <typename UnitT>
struct QuantityMaker;

template <typename Unit>
struct SingularNameFor;

template <typename UnitT>
struct QuantityPointMaker;

template <typename UnitT, typename RepT>
class Quantity;

//
// Machinery for forward-declaring a unit product.
//
// To use, make an alias with the correct unit powers in the correct order, in the `_fwd.hh` file.
// In the `.hh` file, call `is_forward_declared_unit_valid(...)` (defined in `unit_of_measure.hh`)
// on an instance of that alias.
//
template <typename... UnitPowers>
struct UnitProduct;
template <typename... UnitPowers>
struct ForwardDeclareUnitProduct {
    using unit_type = UnitProduct<UnitPowers...>;
};

//
// Machinery for forward-declaring a unit power.
//
// To use, make an alias with the same unit and power(s) that `UnitPowerT` would produce, in the
// `_fwd.hh` file.  In the `.hh` file, call `is_forward_declared_unit_valid(...)` (defined in
// `unit_of_measure.hh`) on that alias.
//
template <typename U, std::intmax_t N, std::intmax_t D = 1>
struct ForwardDeclareUnitPow {
    using unit_type = RatioPow<U, N, D>;
};
template <typename U, std::intmax_t N>
struct ForwardDeclareUnitPow<U, N, 1> {
    using unit_type = Pow<U, N>;
};

//
// Quantity aliases to set a particular Rep.
//
// This presents a less cumbersome interface for end users.
//
template <typename UnitT>
using QuantityD = Quantity<UnitT, double>;
template <typename UnitT>
using QuantityF = Quantity<UnitT, float>;
template <typename UnitT>
using QuantityI = Quantity<UnitT, int>;
template <typename UnitT>
using QuantityU = Quantity<UnitT, unsigned int>;
template <typename UnitT>
using QuantityI32 = Quantity<UnitT, int32_t>;
template <typename UnitT>
using QuantityU32 = Quantity<UnitT, uint32_t>;
template <typename UnitT>
using QuantityI64 = Quantity<UnitT, int64_t>;
template <typename UnitT>
using QuantityU64 = Quantity<UnitT, uint64_t>;

template <typename T>
struct CorrespondingQuantity;

template <typename UnitT, typename RepT>
class QuantityPoint;

//
// QuantityPoint aliases to set a particular Rep.
//
// This presents a less cumbersome interface for end users.
//
template <typename UnitT>
using QuantityPointD = QuantityPoint<UnitT, double>;
template <typename UnitT>
using QuantityPointF = QuantityPoint<UnitT, float>;
template <typename UnitT>
using QuantityPointI = QuantityPoint<UnitT, int>;
template <typename UnitT>
using QuantityPointU = QuantityPoint<UnitT, unsigned int>;
template <typename UnitT>
using QuantityPointI32 = QuantityPoint<UnitT, int32_t>;
template <typename UnitT>
using QuantityPointU32 = QuantityPoint<UnitT, uint32_t>;
template <typename UnitT>
using QuantityPointI64 = QuantityPoint<UnitT, int64_t>;
template <typename UnitT>
using QuantityPointU64 = QuantityPoint<UnitT, uint64_t>;

template <typename Unit>
struct Constant;

template <typename Unit>
struct SymbolFor;

template <template <class U> class Prefix>
struct PrefixApplier;

// SI Prefixes.
template <typename U>
struct Quetta;
template <typename U>
struct Ronna;
template <typename U>
struct Yotta;
template <typename U>
struct Zetta;
template <typename U>
struct Exa;
template <typename U>
struct Peta;
template <typename U>
struct Tera;
template <typename U>
struct Giga;
template <typename U>
struct Mega;
template <typename U>
struct Kilo;
template <typename U>
struct Hecto;
template <typename U>
struct Deka;
template <typename U>
struct Deci;
template <typename U>
struct Centi;
template <typename U>
struct Milli;
template <typename U>
struct Micro;
template <typename U>
struct Nano;
template <typename U>
struct Pico;
template <typename U>
struct Femto;
template <typename U>
struct Atto;
template <typename U>
struct Zepto;
template <typename U>
struct Yocto;
template <typename U>
struct Ronto;
template <typename U>
struct Quecto;

// Binary Prefixes.
template <typename U>
struct Yobi;
template <typename U>
struct Zebi;
template <typename U>
struct Exbi;
template <typename U>
struct Pebi;
template <typename U>
struct Tebi;
template <typename U>
struct Gibi;
template <typename U>
struct Mebi;
template <typename U>
struct Kibi;

}  // namespace au
