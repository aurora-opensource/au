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
struct UnitRatioImpl : stdx::type_identity<MagQuotientT<detail::MagT<U1>, detail::MagT<U2>>> {
    static_assert(HasSameDimension<U1, U2>::value,
                  "Can only compute ratio of same-dimension units");
};
template <typename U1, typename U2>
using UnitRatio = typename UnitRatioImpl<U1, U2>::type;
template <typename U1, typename U2>
using UnitRatioT = UnitRatio<U1, U2>;

// The sign of a unit: almost always `mag<1>()`, but `-mag<1>()` for "negative" units.
template <typename U>
using UnitSign = Sign<detail::MagT<U>>;

template <typename U>
struct AssociatedUnitImpl : stdx::type_identity<U> {};
template <typename U>
using AssociatedUnit = typename AssociatedUnitImpl<U>::type;
template <typename U>
using AssociatedUnitT = AssociatedUnit<U>;

template <typename U>
struct AssociatedUnitForPointsImpl : stdx::type_identity<U> {};
template <typename U>
using AssociatedUnitForPoints = typename AssociatedUnitForPointsImpl<U>::type;
template <typename U>
using AssociatedUnitForPointsT = AssociatedUnitForPoints<U>;

// `CommonUnit`: the largest unit that evenly divides all input units.
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
using CommonUnit = typename ComputeCommonUnit<Us...>::type;
template <typename... Us>
using CommonUnitT = CommonUnit<Us...>;

// `CommonPointUnit`: the largest-magnitude, highest-origin unit which is "common" to the units of
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
using CommonPointUnit = typename ComputeCommonPointUnit<Us...>::type;
template <typename... Us>
using CommonPointUnitT = CommonPointUnit<Us...>;

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

// Type trait for the sign of a Unit (represented as a Magnitude).
template <typename U>
constexpr UnitSign<AssociatedUnitT<U>> unit_sign(U) {
    return {};
}

template <typename U>
constexpr auto associated_unit(U) {
    return AssociatedUnitT<U>{};
}

template <typename U>
constexpr auto associated_unit_for_points(U) {
    return AssociatedUnitForPointsT<U>{};
}

template <typename... Us>
constexpr auto common_unit(Us...) {
    return CommonUnitT<AssociatedUnitT<Us>...>{};
}

template <typename... Us>
constexpr auto common_point_unit(Us...) {
    return CommonPointUnitT<AssociatedUnitForPointsT<Us>...>{};
}

template <template <class> class Utility, typename... Us>
constexpr auto make_common(Utility<Us>...) {
    return Utility<CommonUnitT<AssociatedUnitT<Us>...>>{};
}

template <template <class> class Utility, typename... Us>
constexpr auto make_common_point(Utility<Us>...) {
    return Utility<CommonPointUnitT<AssociatedUnitForPointsT<Us>...>>{};
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
struct UnitProductPack {
    using Dim = DimProductT<detail::DimT<UnitPows>...>;
    using Mag = MagProductT<detail::MagT<UnitPows>...>;
};

// Helper to make a canonicalized product of units.
//
// On the input side, we treat every input unit as a UnitProductPack.  Once we get our final result,
// we simplify it using `UnpackIfSoloT`.  (The motivation is that we don't want to return, say,
// `UnitProductPack<Meters>`; we'd rather just return `Meters`.)
template <typename... UnitPows>
using UnitProduct =
    UnpackIfSoloT<UnitProductPack,
                  PackProductT<UnitProductPack, AsPackT<UnitProductPack, UnitPows>...>>;
template <typename... UnitPows>
using UnitProductT = UnitProduct<UnitPows...>;

// Raise a Unit to a (possibly rational) Power.
template <typename U, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using UnitPower =
    UnpackIfSoloT<UnitProductPack,
                  PackPowerT<UnitProductPack, AsPackT<UnitProductPack, U>, ExpNum, ExpDen>>;
template <typename U, std::intmax_t ExpNum, std::intmax_t ExpDen = 1>
using UnitPowerT = UnitPower<U, ExpNum, ExpDen>;

// Compute the inverse of a unit.
template <typename U>
using UnitInverse = UnitPower<U, -1>;
template <typename U>
using UnitInverseT = UnitInverse<U>;

// Compute the quotient of two units.
template <typename U1, typename U2>
using UnitQuotient = UnitProductT<U1, UnitInverse<U2>>;
template <typename U1, typename U2>
using UnitQuotientT = UnitQuotient<U1, U2>;

template <typename... Us>
constexpr bool is_forward_declared_unit_valid(ForwardDeclareUnitProduct<Us...>) {
    return std::is_same<typename ForwardDeclareUnitProduct<Us...>::unit_type,
                        UnitProductT<Us...>>::value;
}

template <typename U, std::intmax_t ExpNum, std::intmax_t ExpDen>
constexpr bool is_forward_declared_unit_valid(ForwardDeclareUnitPow<U, ExpNum, ExpDen>) {
    return std::is_same<typename ForwardDeclareUnitPow<U, ExpNum, ExpDen>::unit_type,
                        UnitPowerT<U, ExpNum, ExpDen>>::value;
}

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
template <std::intmax_t Deg, typename U, typename = std::enable_if_t<IsUnit<U>::value>>
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

// Support `SingularNameFor` in (quantity) unit slots.
template <typename U>
struct AssociatedUnitImpl<SingularNameFor<U>> : stdx::type_identity<U> {};

template <int Exp, typename Unit>
constexpr auto pow(SingularNameFor<Unit>) {
    return SingularNameFor<UnitPowerT<Unit, Exp>>{};
}

//
// Specialize `UnitOrderTiebreaker<YourCustomUnit>` as below, but with a different constant, in
// order to reduce the chance of hitting "distinct input types compare equal" errors.
//
template <typename U>
struct UnitOrderTiebreaker;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Origin displacement implementation.

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// `ValueDisplacementMagnitude` utility.
namespace detail {

// `ValueDisplacementMagnitude<T1, T2>` is a type that can be instantiated, and is either a
// `Magnitude` type or else `Zero`.  It represents the magnitude of the unit that takes us from
// `T1::value()` to `T2::value()` (and is `Zero` if and only if these values are equal).
//
// This is fully encapsulated inside of the `detail` namespace because we don't want end users
// reasoning in terms of "the magnitude" of a unit.  This concept makes no sense generally.
// However, it's useful to us internally, because it helps us compute the largest possible magnitude
// of a common point unit.  Being fully encapsulated, we ourselves can be careful not to misuse it.
enum class AreValuesEqual { YES, NO };
template <typename U1, typename U2, AreValuesEqual>
struct ValueDisplacementMagnitudeImpl;
template <typename U1, typename U2>
using ValueDisplacementMagnitude = typename ValueDisplacementMagnitudeImpl<
    U1,
    U2,
    (U1::value() == U2::value() ? AreValuesEqual::YES : AreValuesEqual::NO)>::type;

// Equal values case.
template <typename U1, typename U2>
struct ValueDisplacementMagnitudeImpl<U1, U2, AreValuesEqual::YES> : stdx::type_identity<Zero> {
    static_assert(U1::value() == U2::value(), "Mismatched instantiation (internal library error)");
};

// Prep for handling unequal values: it's useful to be able to turn a signed integer into a
// Magnitude.
//
// The `bool` template parameter in the `MagSign` interface has poor callsite readability, but it
// doesn't matter because we're only using it right here.
template <bool IsNeg>
struct MagSign : stdx::type_identity<Magnitude<>> {};
template <>
struct MagSign<true> : stdx::type_identity<Magnitude<Negative>> {};
template <std::intmax_t N>
constexpr auto signed_mag() {
    constexpr auto sign = typename MagSign<(N < 0)>::type{};
    return sign * mag<static_cast<std::size_t>(N < 0 ? (-N) : N)>();
}

// Unequal values case implementation: scale up the magnitude of the diff's _unit_ by the diff's
// _value in_ that unit.
template <typename U1, typename U2>
struct ValueDisplacementMagnitudeImpl<U1, U2, AreValuesEqual::NO> {
    static_assert(U1::value() != U2::value(), "Mismatched instantiation (internal library error)");
    static constexpr auto mag() {
        constexpr auto diff = U2::value() - U1::value();
        using D = typename decltype(diff)::Unit;
        return MagT<D>{} * signed_mag<diff.in(D{})>();
    }
    using type = decltype(mag());
};

}  // namespace detail

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
struct HasSameOrigin : stdx::bool_constant<(OriginOf<U1>::value() == OriginOf<U2>::value())> {};
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
struct CommonUnitPack {
    static_assert(AreElementsInOrder<CommonUnitPack, CommonUnitPack<Us...>>::value,
                  "Elements must be listed in ascending order");
    static_assert(HasSameDimension<Us...>::value,
                  "Common unit only meaningful if units have same dimension");

    using Dim = CommonDimensionT<detail::DimT<Us>...>;
    using Mag = CommonMagnitudeT<detail::MagT<Us>...>;
};

template <typename A, typename B>
struct InOrderFor<CommonUnitPack, A, B> : InOrderFor<UnitProductPack, A, B> {};

template <typename... Us>
struct UnitList {};
template <typename A, typename B>
struct InOrderFor<UnitList, A, B> : InOrderFor<UnitProductPack, A, B> {};

namespace detail {
// This machinery searches a unit list for one that "matches" a target unit.
//
// If none do, it will produce the target unit.

// Generic template.
template <template <class, class> class Matcher,
          typename TargetUnit,
          typename UnitListT = TargetUnit>
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
                                            stdx::conjunction<IsInteger<UnitRatioT<U1, U2>>,
                                                              IsPositive<UnitRatioT<U1, U2>>>>> {};

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
    static_assert(sizeof...(Us) > 1u, "Common unit label only makes sense for multiple units");
    static_assert(AllUnitsQuantityEquivalent<Us...>::value,
                  "Must pre-reduce units before constructing common-unit label");

    using LabelT = ExtendedLabel<7u + 2u * (sizeof...(Us) - 1u), Us...>;
    static constexpr LabelT value = concatenate("EQUIV{", join_by(", ", unit_label<Us>()...), "}");
};
template <typename... Us>
constexpr typename CommonUnitLabelImpl<Us...>::LabelT CommonUnitLabelImpl<Us...>::value;

template <typename U>
struct CommonUnitLabelImpl<U> : UnitLabel<U> {};

template <typename U>
struct UnscaledUnitImpl : stdx::type_identity<U> {};
template <typename U, typename M>
struct UnscaledUnitImpl<ScaledUnit<U, M>> : stdx::type_identity<U> {};
template <typename U>
using UnscaledUnit = typename UnscaledUnitImpl<U>::type;

template <typename U>
struct DistinctUnscaledUnitsImpl : stdx::type_identity<UnitList<UnscaledUnit<U>>> {};
template <typename U>
using DistinctUnscaledUnits = typename DistinctUnscaledUnitsImpl<U>::type;
template <typename... Us>
struct DistinctUnscaledUnitsImpl<CommonUnitPack<Us...>>
    : stdx::type_identity<FlatDedupedTypeListT<UnitList, UnscaledUnit<Us>...>> {};

template <typename U, typename DistinctUnits>
struct SimplifyIfOnlyOneUnscaledUnitImpl;
template <typename U>
using SimplifyIfOnlyOneUnscaledUnit =
    typename SimplifyIfOnlyOneUnscaledUnitImpl<U, DistinctUnscaledUnits<U>>::type;
template <>
struct SimplifyIfOnlyOneUnscaledUnitImpl<Zero, UnitList<Zero>> : stdx::type_identity<Zero> {};
template <typename U, typename SoleUnscaledUnit>
struct SimplifyIfOnlyOneUnscaledUnitImpl<U, UnitList<SoleUnscaledUnit>>
    : stdx::type_identity<decltype(SoleUnscaledUnit{} * UnitRatioT<U, SoleUnscaledUnit>{})> {};
template <typename U, typename... Us>
struct SimplifyIfOnlyOneUnscaledUnitImpl<U, UnitList<Us...>> : stdx::type_identity<U> {};

// Explicit specialization to short-circuit `FirstMatchingUnit` machinery for `Zero`.
template <>
struct FirstMatchingUnit<AreUnitsQuantityEquivalent, Zero, Zero> : stdx::type_identity<Zero> {};

template <typename U>
struct ReplaceCommonPointUnitWithCommonUnitImpl : stdx::type_identity<U> {};
template <typename U>
using ReplaceCommonPointUnitWithCommonUnit =
    typename ReplaceCommonPointUnitWithCommonUnitImpl<U>::type;
}  // namespace detail

template <typename A, typename B>
struct InOrderFor<detail::CommonUnitLabelImpl, A, B> : InOrderFor<UnitProductPack, A, B> {};

template <typename... Us>
using CommonUnitLabel = FlatDedupedTypeListT<detail::CommonUnitLabelImpl, Us...>;

template <typename... Us>
struct ComputeCommonUnitImpl
    : stdx::type_identity<detail::EliminateRedundantUnits<
          FlatDedupedTypeListT<CommonUnitPack,
                               detail::ReplaceCommonPointUnitWithCommonUnit<Us>...>>> {};
template <>
struct ComputeCommonUnitImpl<> : stdx::type_identity<Zero> {};

template <typename T>
struct IsNonzero : stdx::negation<std::is_same<T, Zero>> {};

template <typename... Us>
struct ComputeCommonUnit
    : stdx::type_identity<detail::SimplifyIfOnlyOneUnscaledUnit<typename detail::FirstMatchingUnit<
          AreUnitsQuantityEquivalent,
          typename detail::IncludeInPackIf<IsNonzero, ComputeCommonUnitImpl, Us...>::type>::type>> {
};

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

// `UnitOfLowestOrigin<Us...>` is any unit among `Us` whose origin equals `CommonOrigin<Us...>`.
template <typename... Us>
struct UnitOfLowestOriginImpl;
template <typename... Us>
using UnitOfLowestOrigin = typename SortAs<UnitProductPack, UnitOfLowestOriginImpl<Us...>>::type;
template <typename U>
struct UnitOfLowestOriginImpl<U> : stdx::type_identity<U> {};
template <typename U, typename U1, typename... Us>
struct UnitOfLowestOriginImpl<U, U1, Us...>
    : std::conditional<(OriginOf<U>::value() == CommonOrigin<U, U1, Us...>::value()),
                       U,
                       UnitOfLowestOrigin<U1, Us...>> {};

template <typename U1, typename U2>
struct OriginDisplacementUnit {
    static_assert(OriginOf<U1>::value() != OriginOf<U2>::value(),
                  "OriginDisplacementUnit must be an actual unit, so it must be nonzero.");

    using Dim = CommonDimensionT<DimT<U1>, DimT<U2>>;
    using Mag = ValueDisplacementMagnitude<OriginOf<U1>, OriginOf<U2>>;
};

// `ComputeOriginDisplacementUnit<U1, U2>` produces an ad hoc unit equal to the displacement from
// the origin of `U1` to the origin of `U2`.  If `U1` and `U2` have equal origins, then it is
// `Zero`.  Otherwise, it will be `OriginDisplacementUnit<U1, U2>`.
template <typename U1, typename U2>
using ComputeOriginDisplacementUnit =
    std::conditional_t<(OriginOf<U1>::value() == OriginOf<U2>::value()),
                       Zero,
                       OriginDisplacementUnit<U1, U2>>;

template <typename U1, typename U2>
constexpr auto origin_displacement_unit(U1, U2) {
    return ComputeOriginDisplacementUnit<AssociatedUnitForPointsT<U1>,
                                         AssociatedUnitForPointsT<U2>>{};
}

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

template <typename U1, typename U2>
struct UnitLabel<detail::OriginDisplacementUnit<U1, U2>> {
    using LabelT = detail::ExtendedLabel<15u, U1, U2>;
    static constexpr LabelT value =
        detail::concatenate("(@(0 ", UnitLabel<U2>::value, ") - @(0 ", UnitLabel<U1>::value, "))");
};
template <typename U1, typename U2>
constexpr typename UnitLabel<detail::OriginDisplacementUnit<U1, U2>>::LabelT
    UnitLabel<detail::OriginDisplacementUnit<U1, U2>>::value;

// This exists to be the "named type" for the common unit of a bunch of input units.
//
// To be well-formed, the units must be listed in the same order every time.  End users cannot be
// responsible for this; thus, they should never name this type directly.  Rather, they should name
// the `CommonPointUnitT` alias, which will handle the canonicalization.
template <typename... Us>
using CommonAmongUnitsAndOriginDisplacements =
    CommonUnitT<Us...,
                detail::ComputeOriginDisplacementUnit<detail::UnitOfLowestOrigin<Us...>, Us>...>;
template <typename... Us>
struct CommonPointUnitPack : CommonAmongUnitsAndOriginDisplacements<Us...> {
    static_assert(AreElementsInOrder<CommonPointUnitPack, CommonPointUnitPack<Us...>>::value,
                  "Elements must be listed in ascending order");
    static_assert(HasSameDimension<Us...>::value,
                  "Common unit only meaningful if units have same dimension");

    static constexpr auto origin() { return detail::CommonOrigin<Us...>::value(); }
};

namespace detail {
template <typename... Us>
struct ReplaceCommonPointUnitWithCommonUnitImpl<CommonPointUnitPack<Us...>>
    : stdx::type_identity<CommonAmongUnitsAndOriginDisplacements<Us...>> {};
}  // namespace detail

template <typename A, typename B>
struct InOrderFor<CommonPointUnitPack, A, B> : InOrderFor<UnitProductPack, A, B> {};

template <typename... Us>
using ComputeCommonPointUnitImpl = FlatDedupedTypeListT<CommonPointUnitPack, Us...>;

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
struct CompoundLabel<UnitProductPack<Us...>, Policy> {
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
struct QuotientLabeler<N, UnitProductPack<>, T> {
    using LabelT = StringConstant<CompoundLabel<N, ParensPolicy::OMIT>::value().size()>;
    static constexpr LabelT value = CompoundLabel<N, ParensPolicy::OMIT>::value();
};
template <typename N, typename T>
constexpr typename QuotientLabeler<N, UnitProductPack<>, T>::LabelT
    QuotientLabeler<N, UnitProductPack<>, T>::value;

// Special case for numerator of 1.
template <typename D, typename T>
struct QuotientLabeler<UnitProductPack<>, D, T> {
    using LabelT = StringConstant<CompoundLabel<D>::value().size() + 4>;
    static constexpr LabelT value = concatenate("1 / ", CompoundLabel<D>::value());
};
template <typename D, typename T>
constexpr typename QuotientLabeler<UnitProductPack<>, D, T>::LabelT
    QuotientLabeler<UnitProductPack<>, D, T>::value;

// Special case for numerator _and_ denominator of 1 (null product).
template <typename T>
struct QuotientLabeler<UnitProductPack<>, UnitProductPack<>, T> {
    static constexpr const char value[] = "";
};
template <typename T>
constexpr const char QuotientLabeler<UnitProductPack<>, UnitProductPack<>, T>::value[];
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

// Implementation for UnitProductPack: split into positive and negative powers.
template <typename... Us>
struct UnitLabel<UnitProductPack<Us...>>
    : detail::QuotientLabeler<detail::NumeratorPartT<UnitProductPack<Us...>>,
                              detail::DenominatorPartT<UnitProductPack<Us...>>,
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

// Special case for unit scaled by (-1).
template <typename U>
struct UnitLabel<ScaledUnit<U, Magnitude<Negative>>> {
    using LabelT = detail::ExtendedLabel<3u, U>;
    static constexpr LabelT value = detail::concatenate("[-", UnitLabel<U>::value, "]");
};
template <typename U>
constexpr typename UnitLabel<ScaledUnit<U, Magnitude<Negative>>>::LabelT
    UnitLabel<ScaledUnit<U, Magnitude<Negative>>>::value;

// Implementation for CommonUnitPack: give size in terms of each constituent unit.
template <typename... Us>
struct UnitLabel<CommonUnitPack<Us...>>
    : CommonUnitLabel<decltype(Us{} *
                               (detail::MagT<CommonUnitPack<Us...>>{} / detail::MagT<Us>{}))...> {};

// Implementation for CommonPointUnitPack: give size in terms of each constituent unit, taking any
// origin displacements into account.
template <typename... Us>
struct UnitLabel<CommonPointUnitPack<Us...>>
    : UnitLabel<CommonAmongUnitsAndOriginDisplacements<Us...>> {};

template <typename Unit>
constexpr const auto &unit_label(Unit) {
    return detail::as_char_array(UnitLabel<AssociatedUnitT<Unit>>::value);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UnitProductPack` implementation.
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

// OrderAsUnitProductPack<A, B> can only be true if both A and B are unit products, _and_ they are
// in the standard pack order for unit products.  This default case handles the usual case where
// either A or B (or both) is not a UnitProductPack<...> in the first place.
template <typename A, typename B>
struct OrderAsUnitProductPack : std::false_type {};

// This specialization handles the non-trivial case, where we do have two UnitProductPack instances.
template <typename... U1s, typename... U2s>
struct OrderAsUnitProductPack<UnitProductPack<U1s...>, UnitProductPack<U2s...>>
    : InStandardPackOrder<UnitProductPack<U1s...>, UnitProductPack<U2s...>> {};

// OrderAsOriginDisplacementUnit<A, B> can only be true if both A and B are `OriginDisplacementUnit`
// specializations, _and_ their first units are in order, or their first units are identical and
// their second units are in order.  This default case handles the usual case where either A or B
// (or both) is not a `OriginDisplacementUnit` specialization in the first place.
template <typename A, typename B>
struct OrderAsOriginDisplacementUnit : std::false_type {};

template <typename A, typename B>
struct OrderByFirstInOriginDisplacementUnit;
template <typename A1, typename A2, typename B1, typename B2>
struct OrderByFirstInOriginDisplacementUnit<OriginDisplacementUnit<A1, A2>,
                                            OriginDisplacementUnit<B1, B2>>
    : InOrderFor<UnitProductPack, A1, B1> {};

template <typename A, typename B>
struct OrderBySecondInOriginDisplacementUnit;
template <typename A1, typename A2, typename B1, typename B2>
struct OrderBySecondInOriginDisplacementUnit<OriginDisplacementUnit<A1, A2>,
                                             OriginDisplacementUnit<B1, B2>>
    : InOrderFor<UnitProductPack, A2, B2> {};

template <typename A1, typename A2, typename B1, typename B2>
struct OrderAsOriginDisplacementUnit<OriginDisplacementUnit<A1, A2>, OriginDisplacementUnit<B1, B2>>
    : LexicographicTotalOrdering<OriginDisplacementUnit<A1, A2>,
                                 OriginDisplacementUnit<B1, B2>,
                                 OrderByFirstInOriginDisplacementUnit,
                                 OrderBySecondInOriginDisplacementUnit> {};

template <typename A, typename B>
struct OrderByOrigin
    : stdx::bool_constant<(detail::OriginOf<A>::value() < detail::OriginOf<B>::value())> {};

// "Unit avoidance" is a tiebreaker for quantity-equivalent units.  Anonymous units, such as
// `UnitImpl<...>`, `ScaledUnit<...>`, and `UnitProductPack<...>`, are more "avoidable" than units
// which are none of these, because the latter are likely explicitly named and thus more
// user-facing.  The relative ordering among these built-in template types is probably less
// important than the fact that there _is_ a relative ordering among them (because we need to have a
// strict total ordering).
template <typename T>
struct CoarseUnitOrdering : std::integral_constant<int, 0> {};

template <typename A, typename B>
struct OrderByCoarseUnitOrdering
    : stdx::bool_constant<(CoarseUnitOrdering<A>::value < CoarseUnitOrdering<B>::value)> {};

template <typename... Ts>
struct CoarseUnitOrdering<UnitProductPack<Ts...>> : std::integral_constant<int, 1> {};

template <typename... Ts>
struct CoarseUnitOrdering<UnitImpl<Ts...>> : std::integral_constant<int, 2> {};

template <typename... Ts>
struct CoarseUnitOrdering<ScaledUnit<Ts...>> : std::integral_constant<int, 3> {};

template <typename B, std::intmax_t N>
struct CoarseUnitOrdering<Pow<B, N>> : std::integral_constant<int, 4> {};

template <typename B, std::intmax_t N, std::intmax_t D>
struct CoarseUnitOrdering<RatioPow<B, N, D>> : std::integral_constant<int, 5> {};

template <typename... Us>
struct CoarseUnitOrdering<CommonUnitPack<Us...>> : std::integral_constant<int, 6> {};

template <typename... Us>
struct CoarseUnitOrdering<CommonPointUnitPack<Us...>> : std::integral_constant<int, 7> {};

template <typename A, typename B>
struct OrderByUnitOrderTiebreaker
    : stdx::bool_constant<(UnitOrderTiebreaker<A>::value < UnitOrderTiebreaker<B>::value)> {};

template <typename U>
struct UnitAvoidance : std::integral_constant<int, 0> {};

}  // namespace detail

template <typename U>
struct UnitOrderTiebreaker : detail::UnitAvoidance<U> {};

template <typename A, typename B>
struct InOrderFor<UnitProductPack, A, B>
    : LexicographicTotalOrdering<A,
                                 B,
                                 detail::OrderByCoarseUnitOrdering,
                                 detail::OrderByDim,
                                 detail::OrderByMag,
                                 detail::OrderByScaleFactor,
                                 detail::OrderByOrigin,
                                 detail::OrderAsUnitProductPack,
                                 detail::OrderAsOriginDisplacementUnit,
                                 detail::OrderByUnitOrderTiebreaker> {};

}  // namespace au
