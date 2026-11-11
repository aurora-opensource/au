// Copyright 2023 Aurora Operations, Inc.
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// HOW TO USE THIS FILE
//
// This file does all of the "heavy lifting" in establishing correspondence between equivalent types
// in Au and in the nholthaus units library.  HOWEVER, it does NOT include either of those libraries
// directly.  The reason is simple: there is a wide diversity in the ways people include each of the
// libraries.  Thus, if we included either directly here, it would fail to compile for many people.
//
// What you should do is to make a _new_ file, in your project, which does these things:
//
//   1. Include Au (in whatever manner is appropriate for your project).
//   2. Include the nholthaus units library (in whatever manner is appropriate for your project).
//   3. Include this present file, AFTER every other file.
//
// Relying on the order of includes is a fragile strategy in general.  However, if you confine it to
// a single file, and then only ever use that new file, then the strategy can work.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <cstdint>
#include <ratio>

namespace au {

namespace detail {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This utility lets us extract a single template parameter.

template <class T>
struct SoleTemplateParameter;
template <class T>
using SoleTemplateParameterT = typename SoleTemplateParameter<T>::type;
template <template <class> class Temp, class T>
struct SoleTemplateParameter<Temp<T>> {
    using type = T;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// These extract the exponent for each base unit from a coherent derived unit (which nholthaus calls
// "base_unit").

template <class U>
struct MeterExp;
template <class U>
using MeterExpT = typename MeterExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct MeterExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<M> {};

template <class U>
struct KilogramExp;
template <class U>
using KilogramExpT = typename KilogramExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct KilogramExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<Kg> {};

template <class U>
struct SecondExp;
template <class U>
using SecondExpT = typename SecondExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct SecondExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<S> {};

template <class U>
struct RadianExp;
template <class U>
using RadianExpT = typename RadianExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct RadianExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<R> {};

template <class U>
struct AmpExp;
template <class U>
using AmpExpT = typename AmpExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct AmpExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<A> {};

template <class U>
struct KelvinExp;
template <class U>
using KelvinExpT = typename KelvinExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct KelvinExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<Ke> {};

template <class U>
struct MoleExp;
template <class U>
using MoleExpT = typename MoleExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct MoleExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<Mo> {};

template <class U>
struct CandelaExp;
template <class U>
using CandelaExpT = typename CandelaExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct CandelaExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<C> {};

template <class U>
struct ByteExp;
template <class U>
using ByteExpT = typename ByteExp<U>::type;
template <class M, class Kg, class S, class R, class A, class Ke, class Mo, class C, class B>
struct ByteExp<units::base_unit<M, Kg, S, R, A, Ke, Mo, C, B>> : stdx::type_identity<B> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// These versions make sure derived units inherit their exponents from the corresponding base units.

template <class X1, class BaseUnit, class X3, class X4>
struct MeterExp<units::unit<X1, BaseUnit, X3, X4>> : MeterExp<BaseUnit> {};

template <class X1, class BaseUnit, class X3, class X4>
struct KilogramExp<units::unit<X1, BaseUnit, X3, X4>> : KilogramExp<BaseUnit> {};

template <class X1, class BaseUnit, class X3, class X4>
struct SecondExp<units::unit<X1, BaseUnit, X3, X4>> : SecondExp<BaseUnit> {};

template <class X1, class BaseUnit, class X3, class X4>
struct RadianExp<units::unit<X1, BaseUnit, X3, X4>> : RadianExp<BaseUnit> {};

template <class X1, class BaseUnit, class X3, class X4>
struct AmpExp<units::unit<X1, BaseUnit, X3, X4>> : AmpExp<BaseUnit> {};

template <class X1, class BaseUnit, class X3, class X4>
struct KelvinExp<units::unit<X1, BaseUnit, X3, X4>> : KelvinExp<BaseUnit> {};

template <class X1, class BaseUnit, class X3, class X4>
struct MoleExp<units::unit<X1, BaseUnit, X3, X4>> : MoleExp<BaseUnit> {};

template <class X1, class BaseUnit, class X3, class X4>
struct CandelaExp<units::unit<X1, BaseUnit, X3, X4>> : CandelaExp<BaseUnit> {};

template <class X1, class BaseUnit, class X3, class X4>
struct ByteExp<units::unit<X1, BaseUnit, X3, X4>> : ByteExp<BaseUnit> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Extract the magnitude of this unit relative to a coherent combination of base units.

// `MagFromRatioT<R>` computes the Magnitude which corresponds to a given `std::ratio` instance `R`.
template <typename RatioT>
struct MagFromRatio;
template <typename RatioT>
using MagFromRatioT = typename MagFromRatio<RatioT>::type;
template <std::intmax_t N, std::intmax_t D>
struct MagFromRatio<std::ratio<N, D>> : stdx::type_identity<decltype(mag<N>() / mag<D>())> {};

// `NholthausUnitMagT<U>` is the scale factor for the nholthaus unit `U`, relative to the coherent
// combination of base units which has the same dimension.
template <class NholthausUnit>
struct NholthausUnitMag;
template <class NholthausUnit>
using NholthausUnitMagT = typename NholthausUnitMag<NholthausUnit>::type;

// Implementation for derived units: apply top-level scaling factor to recursive result.
template <class RationalScale, class BaseUnit, class PiPower, class X4>
struct NholthausUnitMag<units::unit<RationalScale, BaseUnit, PiPower, X4>>
    : stdx::type_identity<MagProduct<MagFromRatioT<RationalScale>,
                                     MagPower<Magnitude<Pi>, PiPower::num, PiPower::den>,
                                     NholthausUnitMagT<BaseUnit>>> {};

// Implementation for base units: always 1 (i.e., the null Magnitude) by definition.
template <class... Es>
struct NholthausUnitMag<units::base_unit<Es...>> : stdx::type_identity<Magnitude<>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Compute the au unit which corresponds to a given nholthaus unit.

template <class NholthausUnit>
struct AuUnit {
    using NU = NholthausUnit;

    // If you want to use only a subset of units, you can avoid depending on the Au analogues for
    // all 9 nholthaus base units.  Simply delete the corresponding `UnitPower` from the
    // `decltype()` expression below.
    //
    // **NOTE:** For safety, if you do this, make sure that you also add a line like the following
    // (using moles as an example):
    //
    // static_assert(MoleExpT<NU>::num == 0, "Moles not supported");

    using type = decltype(UnitPower<Meters, MeterExpT<NU>::num, MeterExpT<NU>::den>{} *
                          UnitPower<Kilo<Grams>, KilogramExpT<NU>::num, KilogramExpT<NU>::den>{} *
                          UnitPower<Seconds, SecondExpT<NU>::num, SecondExpT<NU>::den>{} *
                          UnitPower<Radians, RadianExpT<NU>::num, RadianExpT<NU>::den>{} *
                          UnitPower<Amperes, AmpExpT<NU>::num, AmpExpT<NU>::den>{} *
                          UnitPower<Kelvins, KelvinExpT<NU>::num, KelvinExpT<NU>::den>{} *
                          UnitPower<Bytes, ByteExpT<NU>::num, ByteExpT<NU>::den>{} *
                          UnitPower<Candelas, CandelaExpT<NU>::num, CandelaExpT<NU>::den>{} *
                          UnitPower<Moles, MoleExpT<NU>::num, MoleExpT<NU>::den>{} *
                          NholthausUnitMagT<NU>{});
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Extract the nholthaus unit from a `units::unit_t`.

template <class NholthausType>
struct NholthausUnitType;
template <class U, class R, template <class> class S>
struct NholthausUnitType<units::unit_t<U, R, S>> : stdx::type_identity<U> {};

}  // namespace detail

// Define 1:1 mapping from each nholthaus type to its corresponding `au::Quantity` type.
template <class R, class RationalScale, class BaseUnit, class PiPower>
struct CorrespondingQuantity<
    units::unit_t<units::unit<RationalScale, BaseUnit, PiPower, std::ratio<0>>,
                  R,
                  units::linear_scale>> {
    using NholthausType = typename detail::SoleTemplateParameter<CorrespondingQuantity>::type;
    using NholthausUnit = typename detail::NholthausUnitType<NholthausType>::type;

    using Unit = typename detail::AuUnit<NholthausUnit>::type;
    using Rep = R;

    static constexpr Rep extract_value(NholthausType d) { return d.template to<Rep>(); }
    static constexpr NholthausType construct_from_value(Rep x) { return NholthausType{x}; }
};

// nholthaus handles dimensionless values inconsistently, so we must work around it.  See:
// https://github.com/nholthaus/units/issues/276
template <typename R, typename RationalScale>
struct CorrespondingQuantity<
    units::unit_t<units::unit<RationalScale, units::base_unit<>, std::ratio<0>, std::ratio<0>>,
                  R,
                  units::linear_scale>> {
    using NholthausType = typename detail::SoleTemplateParameter<CorrespondingQuantity>::type;
    using NholthausUnit = typename detail::NholthausUnitType<NholthausType>::type;
    using Mag = detail::NholthausUnitMagT<NholthausUnit>;

    using Unit = UnitImpl<Dimension<>, Mag>;
    using Rep = R;

    // This is the workaround: we must manually multiply the value by 100, because the nholthaus
    // library divides it by 100.  Note the asymmetry between `extract_value()` and
    // `construct_from_value()`: we must multiply by `inverse_mag` only in the former.
    static constexpr Rep extract_value(NholthausType d) {
        return get_value<Rep>(mag<1>() / Mag{}) * d.template to<Rep>();
    }

    static constexpr NholthausType construct_from_value(Rep x) { return NholthausType{x}; }
};

// If nholthaus, for whatever reason, defined a unit in terms of a non-`base_unit` specialization,
// unpack it one more level.  Eventually, we should recursively reach a `base_unit` specialization,
// and match one of the above `CorrespondingQuantity` specializations.
template <typename OuterRatio, typename InnerRatio, typename BaseUnit, typename R>
struct CorrespondingQuantity<units::unit_t<
    units::unit<OuterRatio, units::unit<InnerRatio, BaseUnit, std::ratio<0>, std::ratio<0>>>,
    R,
    units::linear_scale>>
    : CorrespondingQuantity<units::unit_t<units::unit<std::ratio_multiply<OuterRatio, InnerRatio>,
                                                      BaseUnit,
                                                      std::ratio<0>,
                                                      std::ratio<0>>,
                                          R,
                                          units::linear_scale>> {};

}  // namespace au
