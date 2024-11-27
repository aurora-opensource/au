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

#include "au/dimension.hh"
#include "au/magnitude.hh"
#include "au/power_aliases.hh"
#include "au/stdx/type_traits.hh"
#include "au/utility/string_constant.hh"
#include "au/utility/type_traits.hh"
#include "au/zero.hh"

namespace au {

// A "unit" is any type which has:
// - a member typedef `Dim`, which is a valid Dimension; and,
// - a member typedef `Mag`, which is a valid Magnitude.
//
// These can be accessed by traits `detail::DimT` and `detail::MagT`, respectively.  The detail
// namespace is meant to discourage _end users_ from accessing these concepts directly.  For
// example, we don't want end users to ask _which dimension_ a Unit has.  We'd rather they ask
// whether it is the _same_ as some other unit.  (It's also meaningful to ask whether it is
// dimensionless.)  And we certainly don't want end users to try to reason about "the magnitude" of
// a Unit, since this is totally meaningless; rather, we want them to ask about the _relative_
// magnitude with another unit of _the same dimension_.

// A UnitImpl is one easy way (although not the only way) to make a "Unit".
template <typename D, typename M = Magnitude<>>
struct UnitImpl {
    using Dim = D;
    using Mag = M;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Printable labels for units.

// A printable label to indicate the unit for human readers.
//
// To name a unit explicitly, specialize this class template for the unit's type.  For any unit not
// manually labeled, we provide a default label so that this template is always defined.
//
// Valid ways to define the label include a C-style const char array, or a StringConstant<N>.
template <typename Unit>
struct UnitLabel;

// A sizeof()-compatible API to get the label for a unit.
template <typename Unit>
constexpr const auto &unit_label(Unit = Unit{});

// Default label for a unit which hasn't been manually labeled yet.
//
// The dummy template parameter exists to enable `au` to be a header-only library.
template <typename T = void>
struct DefaultUnitLabel {
    static constexpr const char value[] = "[UNLABELED UNIT]";
};
template <typename T>
constexpr const char DefaultUnitLabel<T>::value[];

namespace detail {
// To preserve support for C++14, we need to _name the type_ of the member variable.  However, the
// `StringConstant` template produces a different type for every length, and that length depends on
// _both_ the prefix _and_ the unit label.
//
// To minimize friction as much as possible, we create this alias, which computes the type we need
// for a given unit and prefix-length.
//
// While clunky, this approach is at least robust against errors.  If the user supplies the wrong
// prefix length, it will fail to compile, because there is no assignment operator between
// `StringConstant` instances of different lengths.
template <std::size_t ExtensionStrlen, typename... Us>
using ExtendedLabel = StringConstant<concatenate(unit_label<Us>()...).size() + ExtensionStrlen>;
}  // namespace detail

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type traits.

// Type trait to detect whether a type fulfills our definition of a "Unit".
template <typename T>
struct IsUnit : stdx::conjunction<IsValidPack<Dimension, detail::DimT<T>>,
                                  IsValidPack<Magnitude, detail::MagT<T>>> {};

// Type trait to detect whether two Units have the same Dimension.
template <typename... Us>
struct HasSameDimension;

// Type trait to detect whether two Units are quantity-equivalent.
//
// In this library, Units are "quantity-equivalent" exactly when they have the same Dimension and
// Magnitude.  Quantity instances whose Units are quantity-equivalent can be freely interconverted
// with each other.
template <typename U1, typename U2>
struct AreUnitsQuantityEquivalent;

// Type trait to detect whether two Units are point-equivalent.
//
// In this library, Units are "point-equivalent" exactly when they are quantity-equivalent (see
// above), _and_ they have the same origin.  QuantityPoint instances whose Units are
// point-equivalent can be freely interconverted with each other.
template <typename U1, typename U2>
struct AreUnitsPointEquivalent;

// Type trait to detect whether U is a Unit which is dimensionless.
template <typename U>
struct IsDimensionless : std::is_same<detail::DimT<U>, Dimension<>> {};

// Type trait to detect whether a Unit is "quantity-equivalent" to "the unitless unit".
//
// The "unitless unit" is a dimensionless unit of Magnitude 1 (as opposed to, say, other
// dimensionless units such as Percent).
template <typename U>
struct IsUnitlessUnit
    : stdx::conjunction<IsDimensionless<U>, std::is_same<detail::MagT<U>, Magnitude<>>> {};

// A Magnitude representing the ratio of two same-dimensioned units.
//
// Useful in doing unit conversions.
template <typename U1, typename U2>
struct UnitRatio : stdx::type_identity<MagQuotientT<detail::MagT<U1>, detail::MagT<U2>>> {
    static_assert(HasSameDimension<U1, U2>::value,
                  "Can only compute ratio of same-dimension units");
};
template <typename U1, typename U2>
using UnitRatioT = typename UnitRatio<U1, U2>::type;

// Some units have an "origin".  This is not meaningful by itself, but its difference w.r.t. the
// "origin" of another unit of the same Dimension _is_ meaningful.  This type trait provides access
// to that difference.
template <typename U1, typename U2>
struct OriginDisplacement;

template <typename U>
struct AssociatedUnit : stdx::type_identity<U> {};
template <typename U>
using AssociatedUnitT = typename AssociatedUnit<U>::type;

template <typename U>
struct AssociatedUnitForPoints : stdx::type_identity<U> {};
template <typename U>
using AssociatedUnitForPointsT = typename AssociatedUnitForPoints<U>::type;

// `CommonUnitT`: the largest unit that evenly divides all input units.
//
// A specialization will only exist if all input types are units.
//
// If the inputs are units, but their Dimensions aren't all identical, then the request is
// ill-formed and we will produce a hard error.
//
// It may happen that the input units have the same Dimension, but there is no unit which evenly
// divides them (because some pair of input units has an irrational quotient).  In this case, there
// is no uniquely defined answer, but the program should still produce _some_ answer.  We guarantee
// that the result is associative, and symmetric under any reordering of the input units.  The
// specific implementation choice will be driven by convenience and simplicity.
template <typename... Us>
struct ComputeCommonUnit;
template <typename... Us>
using CommonUnitT = typename ComputeCommonUnit<Us...>::type;

// `CommonPointUnitT`: the largest-magnitude, highest-origin unit which is "common" to the units of
// a collection of `QuantityPoint` instances.
//
// The key goal to keep in mind is that for a `QuantityPoint` of any unit `U` in `Us...`, converting
// its value to the common point-unit should involve only:
//
//   - multiplication by a _positive integer_
//   - addition of a _non-negative integer_
//
// This helps us support the widest range of Rep types (in particular, unsigned integers).
//
// As with `CommonUnitT`, this isn't always possible: in particular, we can't do this for units with
// irrational relative magnitudes or origin displacements.  However, we still provide _some_ answer,
// which is consistent with the above policy whenever it's achievable, and produces reasonable
// results in all other cases.
//
// A specialization will only exist if the inputs are all units, and will exist but produce a hard
// error if any two input units have different Dimensions.  We also strive to keep the result
// associative, and symmetric under interchange of any inputs.
template <typename... Us>
struct ComputeCommonPointUnit;
template <typename... Us>
using CommonPointUnitT = typename ComputeCommonPointUnit<Us...>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Type traits (instance-based interface).

// `is_unit(T)`: check whether this value is an instance of some Unit type.
template <typename T>
constexpr bool is_unit(T) {
    return IsUnit<T>::value;
}

// `fits_in_unit_slot(T)`: check whether this value is valid for a unit slot.
template <typename T>
constexpr bool fits_in_unit_slot(T) {
    return IsUnit<AssociatedUnitT<T>>::value;
}

// Check whether the units associated with these objects have the same Dimension.
template <typename... Us>
constexpr bool has_same_dimension(Us...) {
    return HasSameDimension<AssociatedUnitT<Us>...>::value;
}

// Check whether two Unit types are exactly quantity-equivalent.
template <typename U1, typename U2>
constexpr bool are_units_quantity_equivalent(U1, U2) {
    return AreUnitsQuantityEquivalent<AssociatedUnitT<U1>, AssociatedUnitT<U2>>::value;
}

// Check whether two Unit types are exactly point-equivalent.
template <typename U1, typename U2>
constexpr bool are_units_point_equivalent(U1, U2) {
    return AreUnitsPointEquivalent<AssociatedUnitT<U1>, AssociatedUnitT<U2>>::value;
}

// Check whether this value is an instance of a dimensionless Unit.
template <typename U>
constexpr bool is_dimensionless(U) {
    return IsDimensionless<AssociatedUnitT<U>>::value;
}

// Type trait to detect whether a Unit is "the unitless unit".
template <typename U>
constexpr bool is_unitless_unit(U) {
    return IsUnitlessUnit<AssociatedUnitT<U>>::value;
}

// A Magnitude representing the ratio of two same-dimensioned units.
//
// Useful in doing unit conversions.
template <typename U1, typename U2>
constexpr UnitRatioT<AssociatedUnitT<U1>, AssociatedUnitT<U2>> unit_ratio(U1, U2) {
    return {};
}

template <typename U1, typename U2>
constexpr auto origin_displacement(U1, U2) {
    return OriginDisplacement<AssociatedUnitT<U1>, AssociatedUnitT<U2>>::value();
}

template <typename U>
constexpr auto associated_unit(U) {
    return AssociatedUnitT<U>{};
}

template <typename U>
constexpr auto associated_unit_for_points(U) {
    return AssociatedUnitForPointsT<U>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unit arithmetic traits: products, powers, and derived operations.

// A Unit, scaled by some factor.
//
// Retains all of the member variables and typedefs of the existing Unit, except that the
// `detail::MagT` trait is appropriately scaled, and the unit label is erased.
//
// NOTE: This strategy will lead to long chains of inherited types when we scale a unit multiple
// times (say, going from Meters -> Centi<Meters> -> Inches -> Feet -> Miles).  What's more, each
// element in this chain yields _two_ types: one for the named opaque typedef (e.g., `Feet`), and
// one for the anonymous scaled unit (e.g., `Inches * mag<12>()`).  We explicitly assume that this
// will not cause any performance problems, because these should all be empty classes anyway.  If we
// find out we're mistaken, we'll need to revisit this idea.
template <typename Unit, typename ScaleFactor>
struct ScaledUnit;

template <typename Unit, typename ScaleFactor>
struct ComputeScaledUnitImpl : stdx::type_identity<ScaledUnit<Unit, ScaleFactor>> {};
template <typename Unit, typename ScaleFactor>
using ComputeScaledUnit = typename ComputeScaledUnitImpl<Unit, ScaleFactor>::type;
template <typename Unit, typename ScaleFactor, typename OldScaleFactor>
struct ComputeScaledUnitImpl<ScaledUnit<Unit, OldScaleFactor>, ScaleFactor>
    : ComputeScaledUnitImpl<Unit, MagProductT<OldScaleFactor, ScaleFactor>> {};
template <typename Unit>
struct ComputeScaledUnitImpl<Unit, Magnitude<>> : stdx::type_identity<Unit> {};
// Disambiguating specialization:
template <typename Unit, typename OldScaleFactor>
struct ComputeScaledUnitImpl<ScaledUnit<Unit, OldScaleFactor>, Magnitude<>>
    : stdx::type_identity<ScaledUnit<Unit, OldScaleFactor>> {};

template <typename Unit, typename ScaleFactor>
struct ScaledUnit : Unit {
    static_assert(IsValidPack<Magnitude, ScaleFactor>::value,
                  "Can only scale by a Magnitude<...> type");
    using Dim = detail::DimT<Unit>;
    using Mag = MagProductT<detail::MagT<Unit>, ScaleFactor>;
};

// Type template to hold the product of powers of Units.
template <typename... UnitPows>
struct UnitProduct {
    using Dim = DimProductT<detail::DimT<UnitPows>...>;
    using Mag = MagProductT<detail::MagT<UnitPows>...>;
};

// Helper to make a canonicalized product of units.
//
// On the input side, we treat every input unit as a UnitProduct.  Once we get our final result, we
// simplify it using `UnpackIfSoloT`.  (The motivation is that we don't want to return, say,
// `UnitProduct<Meters>`; we'd rather just return `Meters`.)
template <typename... UnitPows>
using UnitProductT =
    UnpackIfSoloT<UnitProduct, PackProductT<UnitProduct, AsPackT<UnitProduct, UnitPows>...>>;

// Raise a Unit to a (possibly rational) Power.
template <typename U, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using UnitPowerT =
    UnpackIfSoloT<UnitProduct, PackPowerT<UnitProduct, AsPackT<UnitProduct, U>, ExpNum, ExpDen>>;

// Compute the inverse of a unit.
template <typename U>
using UnitInverseT = UnitPowerT<U, -1>;

// Compute the quotient of two units.
template <typename U1, typename U2>
using UnitQuotientT = UnitProductT<U1, UnitInverseT<U2>>;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Unit arithmetic on _instances_ of Units and/or Magnitudes.

// Scale this Unit by multiplying by a Magnitude.
template <typename U, typename = std::enable_if_t<IsUnit<U>::value>, typename... BPs>
constexpr ComputeScaledUnit<U, Magnitude<BPs...>> operator*(U, Magnitude<BPs...>) {
    return {};
}

// Scale this Unit by dividing by a Magnitude.
template <typename U, typename = std::enable_if_t<IsUnit<U>::value>, typename... BPs>
constexpr ComputeScaledUnit<U, MagInverseT<Magnitude<BPs...>>> operator/(U, Magnitude<BPs...>) {
    return {};
}

// Compute the product of two unit instances.
template <typename U1,
          typename U2,
          typename = std::enable_if_t<stdx::conjunction<IsUnit<U1>, IsUnit<U2>>::value>>
constexpr UnitProductT<U1, U2> operator*(U1, U2) {
    return {};
}

// Compute the quotient of two unit instances.
template <typename U1,
          typename U2,
          typename = std::enable_if_t<stdx::conjunction<IsUnit<U1>, IsUnit<U2>>::value>>
constexpr UnitQuotientT<U1, U2> operator/(U1, U2) {
    return {};
}

// Raise a Unit to an integral power.
template <std::intmax_t Exp, typename U, typename = std::enable_if_t<IsUnit<U>::value>>
constexpr UnitPowerT<U, Exp> pow(U) {
    return {};
}

// Take the Root (of some integral degree) of a Unit.
template <std::uintmax_t Deg, typename U, typename = std::enable_if_t<IsUnit<U>::value>>
constexpr UnitPowerT<U, 1, Deg> root(U) {
    return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous interfaces.

// An instance which lets us refer to a unit by its singular name.
//
// To use this, whenever you define a new unit (e.g., `struct Meters`), follow it up with a line
// like the following:
//
//     constexpr auto meter = SingularNameFor<Meters>{};
//
// This is just to help us write grammatically natural code.  Examples:
//
//   - `torque.in(newton * meters)`
//                ^^^^^^
//   - `speed.as(miles / hour)`
//                       ^^^^
template <typename Unit>
struct SingularNameFor {

    // Multiplying `SingularNameFor` instances enables compound units such as:
    // `radians / (meter * second)`.
    template <typename OtherUnit>
    constexpr auto operator*(SingularNameFor<OtherUnit>) const {
        return SingularNameFor<UnitProductT<Unit, OtherUnit>>{};
    }
};

template <int Exp, typename Unit>
constexpr auto pow(SingularNameFor<Unit>) {
    return SingularNameFor<UnitPowerT<Unit, Exp>>{};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OriginDisplacement` implementation.

namespace detail {
// Callable type trait for the default origin of a unit: choose ZERO.
struct ZeroValue {
    static constexpr Zero value() { return Zero{}; }
};

template <typename U>
using OriginMemberType = decltype(U::origin());

// If any unit U has an explicit origin member, then treat that as its origin.
template <typename U>
struct OriginMember {
    static constexpr const OriginMemberType<U> value() { return U::origin(); }
};

template <typename U>
struct OriginOf : std::conditional_t<stdx::experimental::is_detected<OriginMemberType, U>::value,
                                     OriginMember<U>,
                                     ZeroValue> {};

template <typename T, typename U>
struct ValueDifference {
    static constexpr auto value() { return T::value() - U::value(); }
};
}  // namespace detail

// Why this conditional, instead of just using `ValueDifference` unconditionally?  The use case is
// somewhat subtle.  Without it, we would still deduce a displacement _numerically_ equal to 0, but
// it would be stored in specific _units_.  For example, for Celsius, the displacement would be "0
// millikelvins" rather than a generic ZERO.  This has implications for type deduction.  It means
// that, e.g., the following would fail!
//
//   celsius_pt(20).in(celsius_pt);
//
// The reason it would fail is because under the hood, we'd be subtracting a `QuantityI32<Celsius>`
// from a `QuantityI32<Milli<Kelvins>>`, yielding a result expressed in millikelvins for what should
// be an integer number of degrees Celsius.  True, that result happens to have a _value_ of 0... but
// values don't affect overload resolution!
//
// Using ZeroValue when the origins are equal fixes this problem, by expressing the "zero-ness" in
// the _type_.
template <typename U1, typename U2>
struct OriginDisplacement
    : std::conditional_t<detail::OriginOf<U1>::value() == detail::OriginOf<U2>::value(),
                         detail::ZeroValue,
                         detail::ValueDifference<detail::OriginOf<U2>, detail::OriginOf<U1>>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `HasSameDimension` implementation.

template <typename U>
struct HasSameDimension<U> : std::true_type {};

template <typename U1, typename U2, typename... Us>
struct HasSameDimension<U1, U2, Us...>
    : stdx::conjunction<std::is_same<detail::DimT<U1>, detail::DimT<U2>>,
                        HasSameDimension<U2, Us...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreUnitsQuantityEquivalent` implementation.

namespace detail {
// We don't want to advertise this utility, because "same magnitude" is meaningless unless the units
// also have the same dimension.
template <typename U1, typename U2>
struct HasSameMagnitude : std::is_same<detail::MagT<U1>, detail::MagT<U2>> {};
}  // namespace detail

template <typename U1, typename U2>
struct AreUnitsQuantityEquivalent
    : stdx::conjunction<HasSameDimension<U1, U2>, detail::HasSameMagnitude<U1, U2>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `AreUnitsPointEquivalent` implementation.

namespace detail {
template <typename U1, typename U2>
struct HasSameOrigin : stdx::bool_constant<(OriginDisplacement<U1, U2>::value() == ZERO)> {};
}  // namespace detail

template <typename U1, typename U2>
struct AreUnitsPointEquivalent
    : stdx::conjunction<AreUnitsQuantityEquivalent<U1, U2>, detail::HasSameOrigin<U1, U2>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonUnit` helper implementation.

// This exists to be the "named type" for the common unit of a bunch of input units.
//
// To be well-formed, the units must be listed in the same order every time.  End users cannot be
// responsible for this; thus, they should never name this type directly.  Rather, they should name
// the `CommonUnitT` alias, which will handle the canonicalization.
template <typename... Us>
struct CommonUnit {
    static_assert(AreElementsInOrder<CommonUnit, CommonUnit<Us...>>::value,
                  "Elements must be listed in ascending order");
    static_assert(HasSameDimension<Us...>::value,
                  "Common unit only meaningful if units have same dimension");

    using Dim = CommonDimensionT<detail::DimT<Us>...>;
    using Mag = CommonMagnitudeT<detail::MagT<Us>...>;
};

template <typename A, typename B>
struct InOrderFor<CommonUnit, A, B> : InOrderFor<UnitProduct, A, B> {};

namespace detail {
// This machinery searches a unit list for one that "matches" a target unit.
//
// If none do, it will produce the target unit.

// Generic template.
template <template <class, class> class Matcher,
          typename TargetUnit,
          typename UnitList = TargetUnit>
struct FirstMatchingUnit;

// Base case for an empty list: the target unit is the best match.
template <template <class, class> class Matcher,
          typename TargetUnit,
          template <class...>
          class List>
struct FirstMatchingUnit<Matcher, TargetUnit, List<>> : stdx::type_identity<TargetUnit> {};

// Recursive case for a non-empty list: return head if it matches, or else recurse.
template <template <class, class> class Matcher,
          typename TargetUnit,
          template <class...>
          class List,
          typename H,
          typename... Ts>
struct FirstMatchingUnit<Matcher, TargetUnit, List<H, Ts...>>
    : std::conditional_t<Matcher<TargetUnit, H>::value,
                         stdx::type_identity<H>,
                         FirstMatchingUnit<Matcher, TargetUnit, List<Ts...>>> {};

// A "redundant" unit, among a list of units, is one that is an exact integer multiple of another.
//
// If two units are identical, then each is redundant with the other.
//
// If two units are distinct, but quantity-equivalent, then the unit that comes later in the
// standard unit ordering (i.e., `InOrderFor<Pack, ...>`) is the redundant one.
template <typename Pack>
struct EliminateRedundantUnitsImpl;
template <typename Pack>
using EliminateRedundantUnits = typename EliminateRedundantUnitsImpl<Pack>::type;

// Base case: no units to eliminate.
template <template <class...> class Pack>
struct EliminateRedundantUnitsImpl<Pack<>> : stdx::type_identity<Pack<>> {};

// Helper for recursive case.
template <template <class...> class Pack, typename U1, typename U2>
struct IsFirstUnitRedundant
    : std::conditional_t<std::is_same<U1, U2>::value,
                         std::true_type,
                         std::conditional_t<AreUnitsQuantityEquivalent<U1, U2>::value,
                                            InOrderFor<Pack, U2, U1>,
                                            IsInteger<UnitRatioT<U1, U2>>>> {};

// Recursive case: eliminate first unit if it is redundant; else, keep it and eliminate any later
// units that are redundant with it.
template <template <class...> class Pack, typename H, typename... Ts>
struct EliminateRedundantUnitsImpl<Pack<H, Ts...>>
    : std::conditional<

          // If `H` is redundant with _any later unit_, simply omit it.
          stdx::disjunction<IsFirstUnitRedundant<Pack, H, Ts>...>::value,
          EliminateRedundantUnits<Pack<Ts...>>,

          // Otherwise, we know we'll need to keep `H`, so we prepend it to the remaining result.
          //
          // To get that result, we first replace any units _that `H` makes redundant_ with `void`.
          // Then, we drop all `void`, before finally recursively eliminating any units that are
          // redundant among those that remain.
          PrependT<
              EliminateRedundantUnits<DropAll<
                  void,

                  // `Pack<Ts...>`, but with redundant-with-`H` units replaced by `void`:
                  Pack<std::conditional_t<IsFirstUnitRedundant<Pack, Ts, H>::value, void, Ts>...>>>,

              H>> {};

template <typename U, typename... Us>
struct AllUnitsQuantityEquivalent : stdx::conjunction<AreUnitsQuantityEquivalent<U, Us>...> {};

template <typename... Us>
struct CommonUnitLabelImpl {
    static_assert(sizeof...(Us) > 0u, "Common unit label requires units");
    static_assert(AllUnitsQuantityEquivalent<Us...>::value,
                  "Must pre-reduce units before constructing common-unit label");

    using LabelT = ExtendedLabel<7u + 2u * (sizeof...(Us) - 1u), Us...>;
    static constexpr LabelT value = concatenate("EQUIV{", join_by(", ", unit_label<Us>()...), "}");
};
template <typename... Us>
constexpr typename CommonUnitLabelImpl<Us...>::LabelT CommonUnitLabelImpl<Us...>::value;

}  // namespace detail

template <typename A, typename B>
struct InOrderFor<detail::CommonUnitLabelImpl, A, B> : InOrderFor<UnitProduct, A, B> {};

template <typename... Us>
using CommonUnitLabel = FlatDedupedTypeListT<detail::CommonUnitLabelImpl, Us...>;

template <typename... Us>
using ComputeCommonUnitImpl =
    detail::EliminateRedundantUnits<FlatDedupedTypeListT<CommonUnit, Us...>>;

template <typename... Us>
struct ComputeCommonUnit
    : detail::FirstMatchingUnit<AreUnitsQuantityEquivalent, ComputeCommonUnitImpl<Us...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CommonPointUnitT` helper implementation.

namespace detail {

// For equal origins expressed in different units, we can compare the values in their native units
// as a way to decide which unit has the biggest Magnitude.  Bigger Magnitude, smaller value.  (We
// could have tried to assess the Magnitude directly, but this method works better with Zero, and we
// will often encounter Zero when dealing with origins.)
//
// This will be used as a tiebreaker for different origin types.  (For example, the origin of
// Celsius may be represented as Centikelvins or Millikelvins, and we want Centikelvins to "win"
// because it will result in smaller multiplications.)
template <typename T>
constexpr auto get_value_in_native_unit(const T &t) {
    return t.in(T::unit);
}

// If the input is "0", then its value _in any unit_ is 0.
constexpr auto get_value_in_native_unit(const Zero &) { return 0; }

// The common origin of a collection of units is the smallest origin.
//
// We try to keep the result symmetric under reordering of the inputs.
template <typename... Us>
struct CommonOrigin;

template <typename U>
struct CommonOrigin<U> : OriginOf<U> {};

template <typename Head, typename... Tail>
struct CommonOrigin<Head, Tail...> :
    // If the new value is strictly less than the common-so-far, then it wins, so choose it.
    std::conditional_t<
        (OriginOf<Head>::value() < CommonOrigin<Tail...>::value()),
        OriginOf<Head>,

        // If the new value is strictly greater than the common-so-far, it's worse, so skip it.
        std::conditional_t<
            (OriginOf<Head>::value() > CommonOrigin<Tail...>::value()),
            CommonOrigin<Tail...>,

            // If we're here, the origins represent the same _quantity_, but may be expressed in
            // different _units_.  We'd like the biggest unit, since it leads to the smallest
            // multiplications.  For equal quantities, "biggest unit" is equivalent to "smallest
            // value", so we compare the values.
            std::conditional_t<(get_value_in_native_unit(OriginOf<Head>::value()) <
                                get_value_in_native_unit(CommonOrigin<Tail...>::value())),
                               OriginOf<Head>,
                               CommonOrigin<Tail...>>>> {};

// MagTypeT<T> gives some measure of the size of the unit for this "quantity-alike" type.
//
// Zero acts like a quantity in this context, and we treat it as if its unit's Magnitude is Zero.
// This is specifically done for the `CommonPointUnit` implementation; there is no guarantee that
template <typename QuantityOrZero>
struct MagType : stdx::type_identity<MagT<typename QuantityOrZero::Unit>> {};
template <typename QuantityOrZero>
using MagTypeT = typename MagType<stdx::remove_cvref_t<QuantityOrZero>>::type;
template <>
struct MagType<Zero> : stdx::type_identity<Zero> {};

}  // namespace detail

// This exists to be the "named type" for the common unit of a bunch of input units.
//
// To be well-formed, the units must be listed in the same order every time.  End users cannot be
// responsible for this; thus, they should never name this type directly.  Rather, they should name
// the `CommonPointUnitT` alias, which will handle the canonicalization.
template <typename... Us>
struct CommonPointUnit {
    static_assert(AreElementsInOrder<CommonPointUnit, CommonPointUnit<Us...>>::value,
                  "Elements must be listed in ascending order");
    static_assert(HasSameDimension<Us...>::value,
                  "Common unit only meaningful if units have same dimension");

    // We need to store the origin member inside of a type, so that it will act "just enough" like a
    // unit to let us use `OriginDisplacement`.  (We'll link to this nested type's origin member for
    // our own origin member.)
    struct TypeHoldingCommonOrigin {
        using OriginT = decltype(detail::CommonOrigin<Us...>::value());
        static constexpr OriginT origin() { return detail::CommonOrigin<Us...>::value(); }
    };
    static constexpr auto origin() { return TypeHoldingCommonOrigin::origin(); }

    // This handles checking that all the dimensions are the same.  It's what lets us reason in
    // terms of pure Magnitudes below, whereas usually this kind of reasoning is meaningless.
    using Dim = CommonDimensionT<detail::DimT<Us>...>;

    // Now, for Magnitude reasoning.  `OriginDisplacementMagnitude` tells us how finely grained we
    // are forced to split our Magnitude to handle the additive displacements from the common
    // origin.  It might be `Zero` if there is no such constraint (which would mean all the units
    // have the _same_ origin).
    using OriginDisplacementMagnitude = CommonMagnitudeT<
        detail::MagTypeT<decltype(OriginDisplacement<TypeHoldingCommonOrigin, Us>::value())>...>;

    // The final Magnitude is just what it would have been before, except that we also take the
    // results of `OriginDisplacementMagnitude` into account.
    using Mag = CommonMagnitudeT<detail::MagT<Us>..., OriginDisplacementMagnitude>;
};

template <typename A, typename B>
struct InOrderFor<CommonPointUnit, A, B> : InOrderFor<UnitProduct, A, B> {};

template <typename... Us>
using ComputeCommonPointUnitImpl = FlatDedupedTypeListT<CommonPointUnit, Us...>;

template <typename... Us>
struct ComputeCommonPointUnit
    : detail::FirstMatchingUnit<AreUnitsPointEquivalent, ComputeCommonPointUnitImpl<Us...>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnitLabel` implementation.

namespace detail {

template <typename Unit>
using HasLabel = decltype(Unit::label);

// Implementation for units that do have a label.
template <typename T>
struct LabelRef {
    static constexpr auto &value = T::label;
};

// Utility for labeling a unit raised to some power.
template <typename ExpLabel, typename Unit>
struct PowerLabeler {
    using LabelT = ExtendedLabel<ExpLabel::value().size() + 1, Unit>;
    static constexpr LabelT value = join_by("^", unit_label<Unit>(), ExpLabel::value());
};
template <typename ExpLabeler, typename Unit>
constexpr typename PowerLabeler<ExpLabeler, Unit>::LabelT PowerLabeler<ExpLabeler, Unit>::value;

// Utility to generate the exponent label for a Pow.
template <std::intmax_t N>
struct ExpLabelForPow {
    static constexpr auto value() { return parens_if<(N < 0)>(IToA<N>::value); }
};

// Utility to generate the exponent label for a RatioPow.
template <std::intmax_t N, std::intmax_t D>
struct ExpLabelForRatioPow {
    static constexpr auto value() {
        return concatenate("(", IToA<N>::value, "/", IToA<D>::value, ")");
    }
};

enum class ParensPolicy {
    OMIT,
    ADD_IF_MULITPLE,
};

template <typename T, ParensPolicy Policy = ParensPolicy::ADD_IF_MULITPLE>
struct CompoundLabel;
template <typename... Us, ParensPolicy Policy>
struct CompoundLabel<UnitProduct<Us...>, Policy> {
    static constexpr auto value() {
        constexpr bool add_parens =
            (Policy == ParensPolicy::ADD_IF_MULITPLE) && (sizeof...(Us) > 1);
        return parens_if<add_parens>(join_by(" * ", unit_label<Us>()...));
    }
};

// Labeler for a quotient of products-of-Units: general case.
//
// The dummy template parameter exists to enable `au` to be a header-only library.
template <typename N, typename D, typename T = void>
struct QuotientLabeler {
    using LabelT =
        StringConstant<CompoundLabel<N>::value().size() + CompoundLabel<D>::value().size() + 3>;
    static constexpr LabelT value =
        join_by(" / ", CompoundLabel<N>::value(), CompoundLabel<D>::value());
};
template <typename N, typename D, typename T>
constexpr typename QuotientLabeler<N, D, T>::LabelT QuotientLabeler<N, D, T>::value;

// Special case for denominator of 1.
template <typename N, typename T>
struct QuotientLabeler<N, UnitProduct<>, T> {
    using LabelT = StringConstant<CompoundLabel<N, ParensPolicy::OMIT>::value().size()>;
    static constexpr LabelT value = CompoundLabel<N, ParensPolicy::OMIT>::value();
};
template <typename N, typename T>
constexpr typename QuotientLabeler<N, UnitProduct<>, T>::LabelT
    QuotientLabeler<N, UnitProduct<>, T>::value;

// Special case for numerator of 1.
template <typename D, typename T>
struct QuotientLabeler<UnitProduct<>, D, T> {
    using LabelT = StringConstant<CompoundLabel<D>::value().size() + 4>;
    static constexpr LabelT value = concatenate("1 / ", CompoundLabel<D>::value());
};
template <typename D, typename T>
constexpr typename QuotientLabeler<UnitProduct<>, D, T>::LabelT
    QuotientLabeler<UnitProduct<>, D, T>::value;

// Special case for numerator _and_ denominator of 1 (null product).
template <typename T>
struct QuotientLabeler<UnitProduct<>, UnitProduct<>, T> {
    static constexpr const char value[] = "";
};
template <typename T>
constexpr const char QuotientLabeler<UnitProduct<>, UnitProduct<>, T>::value[];
}  // namespace detail

// Unified implementation.
template <typename Unit>
struct UnitLabel
    : std::conditional_t<stdx::experimental::is_detected<detail::HasLabel, Unit>::value,
                         detail::LabelRef<Unit>,
                         DefaultUnitLabel<void>> {};

// Implementation for Pow.
template <typename Unit, std::intmax_t N>
struct UnitLabel<Pow<Unit, N>> : detail::PowerLabeler<detail::ExpLabelForPow<N>, Unit> {};

// Implementation for RatioPow.
template <typename Unit, std::intmax_t N, std::intmax_t D>
struct UnitLabel<RatioPow<Unit, N, D>>
    : detail::PowerLabeler<detail::ExpLabelForRatioPow<N, D>, Unit> {};

// Implementation for UnitProduct: split into positive and negative powers.
template <typename... Us>
struct UnitLabel<UnitProduct<Us...>>
    : detail::QuotientLabeler<detail::NumeratorPartT<UnitProduct<Us...>>,
                              detail::DenominatorPartT<UnitProduct<Us...>>,
                              void> {};

// Implementation for ScaledUnit: scaling unit U by M gets label `"[M U]"`.
template <typename U, typename M>
struct UnitLabel<ScaledUnit<U, M>> {
    using MagLab = MagnitudeLabel<M>;
    using LabelT = detail::
        ExtendedLabel<detail::parens_if<MagLab::has_exposed_slash>(MagLab::value).size() + 3u, U>;
    static constexpr LabelT value =
        detail::concatenate("[",
                            detail::parens_if<MagLab::has_exposed_slash>(MagLab::value),
                            " ",
                            UnitLabel<U>::value,
                            "]");
};
template <typename U, typename M>
constexpr typename UnitLabel<ScaledUnit<U, M>>::LabelT UnitLabel<ScaledUnit<U, M>>::value;

// Implementation for CommonUnit: give size in terms of each constituent unit.
template <typename... Us>
struct UnitLabel<CommonUnit<Us...>>
    : CommonUnitLabel<decltype(Us{} *
                               (detail::MagT<CommonUnit<Us...>>{} / detail::MagT<Us>{}))...> {};

// Implementation for CommonPointUnit: unite constituent labels.
template <typename... Us>
struct UnitLabel<CommonPointUnit<Us...>> {
    using LabelT = detail::ExtendedLabel<8 + 2 * (sizeof...(Us) - 1), Us...>;
    static constexpr LabelT value =
        detail::concatenate("COM_PT[", detail::join_by(", ", unit_label(Us{})...), "]");
};
template <typename... Us>
constexpr
    typename UnitLabel<CommonPointUnit<Us...>>::LabelT UnitLabel<CommonPointUnit<Us...>>::value;

template <typename Unit>
constexpr const auto &unit_label(Unit) {
    return detail::as_char_array(UnitLabel<AssociatedUnitT<Unit>>::value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnitProduct` implementation.
//
// It's just a standard pack product, so all we need to do is carefully define the total ordering.

namespace detail {
template <typename A, typename B>
struct OrderByDim : InStandardPackOrder<DimT<A>, DimT<B>> {};

template <typename A, typename B>
struct OrderByMag : InStandardPackOrder<MagT<A>, MagT<B>> {};

// Order by "scaledness" of scaled units.  This is always false unless BOTH are specializations of
// the `ScaledUnit<U, M>` template.  If they are, we *assume* we would never call this unless both
// `OrderByDim` and `OrderByMag` are tied.  Therefore, we go by the _scale factor itself_.
template <typename A, typename B>
struct OrderByScaledness : std::false_type {};
template <typename A, typename B>
struct OrderByScaleFactor : std::false_type {};
template <typename U1, typename M1, typename U2, typename M2>
struct OrderByScaleFactor<ScaledUnit<U1, M1>, ScaledUnit<U2, M2>> : InStandardPackOrder<M1, M2> {};

template <typename U1, typename M1, typename U2, typename M2>
struct OrderByScaledness<ScaledUnit<U1, M1>, ScaledUnit<U2, M2>>
    : LexicographicTotalOrdering<ScaledUnit<U1, M1>, ScaledUnit<U2, M2>, OrderByScaleFactor> {};

// OrderAsUnitProduct<A, B> can only be true if both A and B are unit products, _and_ they are in
// the standard pack order for unit products.  This default case handles the usual case where either
// A or B (or both) is not a UnitProduct<...> in the first place.
template <typename A, typename B>
struct OrderAsUnitProduct : std::false_type {};

// This specialization handles the non-trivial case, where we do have two UnitProduct instances.
template <typename... U1s, typename... U2s>
struct OrderAsUnitProduct<UnitProduct<U1s...>, UnitProduct<U2s...>>
    : InStandardPackOrder<UnitProduct<U1s...>, UnitProduct<U2s...>> {};

template <typename A, typename B>
struct OrderByOrigin : stdx::bool_constant<(OriginDisplacement<A, B>::value() < ZERO)> {};

// "Unit avoidance" is a tiebreaker for quantity-equivalent units.  Anonymous units, such as
// `UnitImpl<...>`, `ScaledUnit<...>`, and `UnitProduct<...>`, are more "avoidable" than units which
// are none of these, because the latter are likely explicitly named and thus more user-facing.  The
// relative ordering among these built-in template types is probably less important than the fact
// that there _is_ a relative ordering among them (because we need to have a strict total ordering).
template <typename T>
struct UnitAvoidance : std::integral_constant<int, 0> {};

template <typename A, typename B>
struct OrderByUnitAvoidance
    : stdx::bool_constant<(UnitAvoidance<A>::value < UnitAvoidance<B>::value)> {};

template <typename... Ts>
struct UnitAvoidance<UnitProduct<Ts...>> : std::integral_constant<int, 1> {};

template <typename... Ts>
struct UnitAvoidance<UnitImpl<Ts...>> : std::integral_constant<int, 2> {};

template <typename... Ts>
struct UnitAvoidance<ScaledUnit<Ts...>> : std::integral_constant<int, 3> {};

template <typename B, std::intmax_t N>
struct UnitAvoidance<Pow<B, N>> : std::integral_constant<int, 4> {};

template <typename B, std::intmax_t N, std::intmax_t D>
struct UnitAvoidance<RatioPow<B, N, D>> : std::integral_constant<int, 5> {};

template <typename... Us>
struct UnitAvoidance<CommonUnit<Us...>> : std::integral_constant<int, 6> {};

template <typename... Us>
struct UnitAvoidance<CommonPointUnit<Us...>> : std::integral_constant<int, 7> {};
}  // namespace detail

template <typename A, typename B>
struct InOrderFor<UnitProduct, A, B> : LexicographicTotalOrdering<A,
                                                                  B,
                                                                  detail::OrderByUnitAvoidance,
                                                                  detail::OrderByDim,
                                                                  detail::OrderByMag,
                                                                  detail::OrderByScaleFactor,
                                                                  detail::OrderByOrigin,
                                                                  detail::OrderAsUnitProduct> {};

}  // namespace au
