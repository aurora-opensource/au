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

#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/unit_of_measure.hh"

namespace au {

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
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SI Prefixes.

template <typename U>
struct Quetta : decltype(U{} * pow<30>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Q", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Quetta<U>::label;
constexpr auto quetta = PrefixApplier<Quetta>{};

template <typename U>
struct Ronna : decltype(U{} * pow<27>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("R", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Ronna<U>::label;
constexpr auto ronna = PrefixApplier<Ronna>{};

template <typename U>
struct Yotta : decltype(U{} * pow<24>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Y", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Yotta<U>::label;
constexpr auto yotta = PrefixApplier<Yotta>{};

template <typename U>
struct Zetta : decltype(U{} * pow<21>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("Z", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Zetta<U>::label;
constexpr auto zetta = PrefixApplier<Zetta>{};

template <typename U>
struct Exa : decltype(U{} * pow<18>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("E", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Exa<U>::label;
constexpr auto exa = PrefixApplier<Exa>{};

template <typename U>
struct Peta : decltype(U{} * pow<15>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("P", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Peta<U>::label;
constexpr auto peta = PrefixApplier<Peta>{};

template <typename U>
struct Tera : decltype(U{} * pow<12>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("T", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Tera<U>::label;
constexpr auto tera = PrefixApplier<Tera>{};

template <typename U>
struct Giga : decltype(U{} * pow<9>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("G", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Giga<U>::label;
constexpr auto giga = PrefixApplier<Giga>{};

template <typename U>
struct Mega : decltype(U{} * pow<6>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("M", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Mega<U>::label;
constexpr auto mega = PrefixApplier<Mega>{};

template <typename U>
struct Kilo : decltype(U{} * pow<3>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("k", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Kilo<U>::label;
constexpr auto kilo = PrefixApplier<Kilo>{};

template <typename U>
struct Hecto : decltype(U{} * pow<2>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("h", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Hecto<U>::label;
constexpr auto hecto = PrefixApplier<Hecto>{};

template <typename U>
struct Deka : decltype(U{} * pow<1>(mag<10>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("da", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Deka<U>::label;
constexpr auto deka = PrefixApplier<Deka>{};

template <typename U>
struct Deci : decltype(U{} * pow<-1>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("d", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Deci<U>::label;
constexpr auto deci = PrefixApplier<Deci>{};

template <typename U>
struct Centi : decltype(U{} * pow<-2>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("c", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Centi<U>::label;
constexpr auto centi = PrefixApplier<Centi>{};

template <typename U>
struct Milli : decltype(U{} * pow<-3>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("m", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Milli<U>::label;
constexpr auto milli = PrefixApplier<Milli>{};

template <typename U>
struct Micro : decltype(U{} * pow<-6>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("u", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Micro<U>::label;
constexpr auto micro = PrefixApplier<Micro>{};

template <typename U>
struct Nano : decltype(U{} * pow<-9>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("n", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Nano<U>::label;
constexpr auto nano = PrefixApplier<Nano>{};

template <typename U>
struct Pico : decltype(U{} * pow<-12>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("p", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Pico<U>::label;
constexpr auto pico = PrefixApplier<Pico>{};

template <typename U>
struct Femto : decltype(U{} * pow<-15>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("f", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Femto<U>::label;
constexpr auto femto = PrefixApplier<Femto>{};

template <typename U>
struct Atto : decltype(U{} * pow<-18>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("a", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Atto<U>::label;
constexpr auto atto = PrefixApplier<Atto>{};

template <typename U>
struct Zepto : decltype(U{} * pow<-21>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("z", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Zepto<U>::label;
constexpr auto zepto = PrefixApplier<Zepto>{};

template <typename U>
struct Yocto : decltype(U{} * pow<-24>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("y", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Yocto<U>::label;
constexpr auto yocto = PrefixApplier<Yocto>{};

template <typename U>
struct Ronto : decltype(U{} * pow<-27>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("r", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Ronto<U>::label;
constexpr auto ronto = PrefixApplier<Ronto>{};

template <typename U>
struct Quecto : decltype(U{} * pow<-30>(mag<10>())) {
    static constexpr detail::ExtendedLabel<1, U> label = detail::concatenate("q", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<1, U> Quecto<U>::label;
constexpr auto quecto = PrefixApplier<Quecto>{};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary Prefixes.

template <typename U>
struct Yobi : decltype(U{} * pow<80>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Yi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Yobi<U>::label;
constexpr auto yobi = PrefixApplier<Yobi>{};

template <typename U>
struct Zebi : decltype(U{} * pow<70>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Zi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Zebi<U>::label;
constexpr auto zebi = PrefixApplier<Zebi>{};

template <typename U>
struct Exbi : decltype(U{} * pow<60>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ei", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Exbi<U>::label;
constexpr auto exbi = PrefixApplier<Exbi>{};

template <typename U>
struct Pebi : decltype(U{} * pow<50>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Pi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Pebi<U>::label;
constexpr auto pebi = PrefixApplier<Pebi>{};

template <typename U>
struct Tebi : decltype(U{} * pow<40>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ti", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Tebi<U>::label;
constexpr auto tebi = PrefixApplier<Tebi>{};

template <typename U>
struct Gibi : decltype(U{} * pow<30>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Gi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Gibi<U>::label;
constexpr auto gibi = PrefixApplier<Gibi>{};

template <typename U>
struct Mebi : decltype(U{} * pow<20>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Mi", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Mebi<U>::label;
constexpr auto mebi = PrefixApplier<Mebi>{};

template <typename U>
struct Kibi : decltype(U{} * pow<10>(mag<2>())) {
    static constexpr detail::ExtendedLabel<2, U> label = detail::concatenate("Ki", unit_label<U>());
};
template <typename U>
constexpr detail::ExtendedLabel<2, U> Kibi<U>::label;
constexpr auto kibi = PrefixApplier<Kibi>{};

}  // namespace au
