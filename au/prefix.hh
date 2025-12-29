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
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/unit_of_measure.hh"
#include "au/unit_symbol.hh"

namespace au {

namespace detail {

// Trait to detect if a unit contains a Pow or RatioPow (i.e., has a non-trivial exponent).
// When applying a prefix to such a unit, we need brackets to disambiguate.
//
// This includes:
// - Direct Pow<B, N> or RatioPow<B, N, D> types
// - UnitProduct<...> containing any powered units (recursively)
template <typename U>
struct ContainsAnyPowers : std::false_type {};

template <typename B, std::intmax_t N>
struct ContainsAnyPowers<Pow<B, N>> : std::true_type {};

template <typename B, std::intmax_t N, std::intmax_t D>
struct ContainsAnyPowers<RatioPow<B, N, D>> : std::true_type {};

template <typename... Us>
struct ContainsAnyPowers<UnitProduct<Us...>> : stdx::disjunction<ContainsAnyPowers<Us>...> {};

// Helper to generate labels for prefixed units.
// Wraps unit label in brackets if the unit is a powered unit (Pow or RatioPow).
// This disambiguates labels like "m[X^(-1)]" (milli of per-X) from "mX^(-1)" (per milli-X).
template <std::size_t PrefixLen, typename U>
struct PrefixedUnitLabel {
    static constexpr std::size_t UNIT_LABEL_SIZE = concatenate(unit_label<U>()).size();
    static constexpr std::size_t BRACKETS_SIZE = ContainsAnyPowers<U>::value ? 2 : 0;
    using LabelT = StringConstant<PrefixLen + UNIT_LABEL_SIZE + BRACKETS_SIZE>;
};

template <std::size_t N, typename U>
constexpr auto make_prefixed_unit_label(const StringConstant<N> &prefix, U) {
    return concatenate(prefix, brackets_if<ContainsAnyPowers<U>::value>(unit_label<U>()));
}

}  // namespace detail

template <template <class U> class Prefix>
struct PrefixApplier {
    // Applying a Prefix to a Unit instance, creates an instance of the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(U) const {
        return Prefix<U>{};
    }

    // Applying a Prefix to a QuantityMaker instance, creates a maker for the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(QuantityMaker<U>) const {
        return QuantityMaker<Prefix<U>>{};
    }

    // Applying a Prefix to a QuantityPointMaker instance, changes it to make the Prefixed Unit.
    template <typename U>
    constexpr auto operator()(QuantityPointMaker<U>) const {
        return QuantityPointMaker<Prefix<U>>{};
    }

    // Applying a Prefix to a SingularNameFor instance, creates a singularly-named instance of the
    // Prefixed Unit.
    template <typename U>
    constexpr auto operator()(SingularNameFor<U>) const {
        return SingularNameFor<Prefix<U>>{};
    }

    // Applying a Prefix to a SymbolFor instance, creates a symbolically-named instance of the
    // Prefixed unit.
    template <typename U>
    constexpr auto operator()(SymbolFor<U>) const {
        return SymbolFor<Prefix<U>>{};
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SI Prefixes.

template <typename U>
struct Quetta : decltype(U{} * pow<30>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Q"), U{});
};
template <typename U>
constexpr typename Quetta<U>::LabelT Quetta<U>::label;
constexpr auto quetta = PrefixApplier<Quetta>{};

template <typename U>
struct Ronna : decltype(U{} * pow<27>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("R"), U{});
};
template <typename U>
constexpr typename Ronna<U>::LabelT Ronna<U>::label;
constexpr auto ronna = PrefixApplier<Ronna>{};

template <typename U>
struct Yotta : decltype(U{} * pow<24>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Y"), U{});
};
template <typename U>
constexpr typename Yotta<U>::LabelT Yotta<U>::label;
constexpr auto yotta = PrefixApplier<Yotta>{};

template <typename U>
struct Zetta : decltype(U{} * pow<21>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Z"), U{});
};
template <typename U>
constexpr typename Zetta<U>::LabelT Zetta<U>::label;
constexpr auto zetta = PrefixApplier<Zetta>{};

template <typename U>
struct Exa : decltype(U{} * pow<18>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("E"), U{});
};
template <typename U>
constexpr typename Exa<U>::LabelT Exa<U>::label;
constexpr auto exa = PrefixApplier<Exa>{};

template <typename U>
struct Peta : decltype(U{} * pow<15>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("P"), U{});
};
template <typename U>
constexpr typename Peta<U>::LabelT Peta<U>::label;
constexpr auto peta = PrefixApplier<Peta>{};

template <typename U>
struct Tera : decltype(U{} * pow<12>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("T"), U{});
};
template <typename U>
constexpr typename Tera<U>::LabelT Tera<U>::label;
constexpr auto tera = PrefixApplier<Tera>{};

template <typename U>
struct Giga : decltype(U{} * pow<9>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("G"), U{});
};
template <typename U>
constexpr typename Giga<U>::LabelT Giga<U>::label;
constexpr auto giga = PrefixApplier<Giga>{};

template <typename U>
struct Mega : decltype(U{} * pow<6>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("M"), U{});
};
template <typename U>
constexpr typename Mega<U>::LabelT Mega<U>::label;
constexpr auto mega = PrefixApplier<Mega>{};

template <typename U>
struct Kilo : decltype(U{} * pow<3>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("k"), U{});
};
template <typename U>
constexpr typename Kilo<U>::LabelT Kilo<U>::label;
constexpr auto kilo = PrefixApplier<Kilo>{};

template <typename U>
struct Hecto : decltype(U{} * pow<2>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("h"), U{});
};
template <typename U>
constexpr typename Hecto<U>::LabelT Hecto<U>::label;
constexpr auto hecto = PrefixApplier<Hecto>{};

template <typename U>
struct Deka : decltype(U{} * pow<1>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("da"), U{});
};
template <typename U>
constexpr typename Deka<U>::LabelT Deka<U>::label;
constexpr auto deka = PrefixApplier<Deka>{};

template <typename U>
struct Deci : decltype(U{} * pow<-1>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("d"), U{});
};
template <typename U>
constexpr typename Deci<U>::LabelT Deci<U>::label;
constexpr auto deci = PrefixApplier<Deci>{};

template <typename U>
struct Centi : decltype(U{} * pow<-2>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("c"), U{});
};
template <typename U>
constexpr typename Centi<U>::LabelT Centi<U>::label;
constexpr auto centi = PrefixApplier<Centi>{};

template <typename U>
struct Milli : decltype(U{} * pow<-3>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("m"), U{});
};
template <typename U>
constexpr typename Milli<U>::LabelT Milli<U>::label;
constexpr auto milli = PrefixApplier<Milli>{};

template <typename U>
struct Micro : decltype(U{} * pow<-6>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("u"), U{});
};
template <typename U>
constexpr typename Micro<U>::LabelT Micro<U>::label;
constexpr auto micro = PrefixApplier<Micro>{};

template <typename U>
struct Nano : decltype(U{} * pow<-9>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("n"), U{});
};
template <typename U>
constexpr typename Nano<U>::LabelT Nano<U>::label;
constexpr auto nano = PrefixApplier<Nano>{};

template <typename U>
struct Pico : decltype(U{} * pow<-12>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("p"), U{});
};
template <typename U>
constexpr typename Pico<U>::LabelT Pico<U>::label;
constexpr auto pico = PrefixApplier<Pico>{};

template <typename U>
struct Femto : decltype(U{} * pow<-15>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("f"), U{});
};
template <typename U>
constexpr typename Femto<U>::LabelT Femto<U>::label;
constexpr auto femto = PrefixApplier<Femto>{};

template <typename U>
struct Atto : decltype(U{} * pow<-18>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("a"), U{});
};
template <typename U>
constexpr typename Atto<U>::LabelT Atto<U>::label;
constexpr auto atto = PrefixApplier<Atto>{};

template <typename U>
struct Zepto : decltype(U{} * pow<-21>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("z"), U{});
};
template <typename U>
constexpr typename Zepto<U>::LabelT Zepto<U>::label;
constexpr auto zepto = PrefixApplier<Zepto>{};

template <typename U>
struct Yocto : decltype(U{} * pow<-24>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("y"), U{});
};
template <typename U>
constexpr typename Yocto<U>::LabelT Yocto<U>::label;
constexpr auto yocto = PrefixApplier<Yocto>{};

template <typename U>
struct Ronto : decltype(U{} * pow<-27>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("r"), U{});
};
template <typename U>
constexpr typename Ronto<U>::LabelT Ronto<U>::label;
constexpr auto ronto = PrefixApplier<Ronto>{};

template <typename U>
struct Quecto : decltype(U{} * pow<-30>(mag<10>())) {
    using LabelT = typename detail::PrefixedUnitLabel<1, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("q"), U{});
};
template <typename U>
constexpr typename Quecto<U>::LabelT Quecto<U>::label;
constexpr auto quecto = PrefixApplier<Quecto>{};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary Prefixes.

template <typename U>
struct Yobi : decltype(U{} * pow<80>(mag<2>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Yi"), U{});
};
template <typename U>
constexpr typename Yobi<U>::LabelT Yobi<U>::label;
constexpr auto yobi = PrefixApplier<Yobi>{};

template <typename U>
struct Zebi : decltype(U{} * pow<70>(mag<2>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Zi"), U{});
};
template <typename U>
constexpr typename Zebi<U>::LabelT Zebi<U>::label;
constexpr auto zebi = PrefixApplier<Zebi>{};

template <typename U>
struct Exbi : decltype(U{} * pow<60>(mag<2>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Ei"), U{});
};
template <typename U>
constexpr typename Exbi<U>::LabelT Exbi<U>::label;
constexpr auto exbi = PrefixApplier<Exbi>{};

template <typename U>
struct Pebi : decltype(U{} * pow<50>(mag<2>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Pi"), U{});
};
template <typename U>
constexpr typename Pebi<U>::LabelT Pebi<U>::label;
constexpr auto pebi = PrefixApplier<Pebi>{};

template <typename U>
struct Tebi : decltype(U{} * pow<40>(mag<2>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Ti"), U{});
};
template <typename U>
constexpr typename Tebi<U>::LabelT Tebi<U>::label;
constexpr auto tebi = PrefixApplier<Tebi>{};

template <typename U>
struct Gibi : decltype(U{} * pow<30>(mag<2>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Gi"), U{});
};
template <typename U>
constexpr typename Gibi<U>::LabelT Gibi<U>::label;
constexpr auto gibi = PrefixApplier<Gibi>{};

template <typename U>
struct Mebi : decltype(U{} * pow<20>(mag<2>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Mi"), U{});
};
template <typename U>
constexpr typename Mebi<U>::LabelT Mebi<U>::label;
constexpr auto mebi = PrefixApplier<Mebi>{};

template <typename U>
struct Kibi : decltype(U{} * pow<10>(mag<2>())) {
    using LabelT = typename detail::PrefixedUnitLabel<2, U>::LabelT;
    static constexpr LabelT label =
        detail::make_prefixed_unit_label(detail::as_string_constant("Ki"), U{});
};
template <typename U>
constexpr typename Kibi<U>::LabelT Kibi<U>::label;
constexpr auto kibi = PrefixApplier<Kibi>{};

}  // namespace au
