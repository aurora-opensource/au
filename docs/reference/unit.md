# Unit

A _unit_ is a type which represents a unit of measure.  Examples include `Meters`, `Radians`,
`Hours`, and so on.

Users can work with units as either _types_ or _instances_, and can freely convert between these
representations.  That is to say: units are [_monovalue types_](./detail/monovalue_types.md).

## Identifying unit types

A unit is _not_ forced to be a specialization of some central type template, such as a hypothetical
`Unit<...>`. Rather, it's more open ended: a unit can be **any** type which fulfills certain
defining properties.

To be a unit, a type `U`:

1. **Must** contain a public type alias, `U::Dim`, which refers to a valid [dimension
   type](./detail/dimension.md).

2. **Must** contain a public type alias, `U::Mag`, which refers to a valid [magnitude
   type](./magnitude.md).

3. **Must** be a [monovalue type](./detail/monovalue_types.md).

4. **May** contain a `static constexpr` member named `label`, which is a C-style `const char[]`
   (**not** a `const char*`).[^1]

5. **May** contain a `static constexpr` member function `origin()`, which returns a quantity whose
   dimension type is `U::Dim`.

A custom `origin()` is very rarely needed.  Both [labels](#labels) and [origins](#origins) will be
discussed further below.

[^1]: Unit types defined by the library may also use `au::detail::StringConstant<N>` for some
integer length `N`.  Since this is in the `detail` namespace, we wanted to de-emphasize it in this
document.

### Making unit types

Although every unit type needs `Dim` and `Mag` members, users won't need to add them directly.
Rather, the best way to make a unit type is by _combining existing unit types_ via [supported
operations](#operations).  This approach has two key advantages relative to defining your unit as
a fully manual `struct`.

1. If your input types are all valid units, your output type will be too.

2. It makes the definition more readable and physically meaningful.

To give your unit type the best ergonomics, follow our [how-to guide for defining new
units](../howto/new-units.md).

## Unit labels {#labels}

Every unit has a label.  Its label is a `constexpr const char[]` of the appropriate size.

For a unit type `U`, or instance `u`, we can access the label as follows:

- `unit_label<U>()`
- `unit_label(u)`

This function returns a reference to the array, which again is a compile time constant.

Note especially that the type is an _array_ (`[]`).  A pointer (`*`) is _not_ acceptable.  This is
so that we can support `sizeof(unit_label(u))`.

Using C-style `char` arrays for our labels makes Au more friendly for embedded users, because it
gives them full access to the labels without forcing them to depend on `<string>` or `<iostream>`.

### `[UNLABELED_UNIT]`

If a unit does not have an explicit label, we will try to generate one automatically.  If we're
unable to do so, we fall back to the "default label", which is `"[UNLABELED_UNIT]"`.

This is a label just like any other: we do not attempt to "propagate the un-labeled-ness".  As
a concrete example, if `Foos` is an unlabeled unit, then the label for `Nano<Foos>{} / Seconds{}`
would be `"n[UNLABELED_UNIT] / s"`.  This is to preserve as much structure as possible for end
users, so they have the best chance of recognizing the offending unit, and perhaps upgrading it.

!!! note
    A key design goal is for every combination of meaningfully labeled units, by every supported
    operation, to produce a meaningfully labeled unit.  Right now, the only missing operation is
    scaling a unit by a magnitude.  We are tracking this in
    [#85](https://github.com/aurora-opensource/au/issues/85).

## Unit origins {#origins}

The "origin" of a unit is only useful for `QuantityPoint`, our [affine space
type](http://videocortex.io/2018/Affine-Space-Types/).  Even then, the origin by itself is not
meaningful.  Only the difference between the origins of two units is meaningful.

You would use this to implement an "offset" unit, such as `Celsius` or `Fahrenheit`.  However, note
that both of these are already implemented in the library.

The origin defaults to `ZERO` if not supplied.

## Types for combined units

A core tenet of Au's design philosophy is to avoid giving any units special status.  Every named
unit enters into a unit computation on equal footing.  We will keep track of the accumulated powers
of each named unit, cancelling as appropriate.  The final form will follow these rules.

1. Every power of a named unit will be represented according to the [representation
   table](./detail/packs.md#powers).  That is, it will be omitted if its power is zero, and will
   otherwise appear as one of `Pow`, `RatioPow`, or the bare unit itself.

2. If only one named unit remains with nonzero power, then that named unit power (as represented in
   the previous rule) is the _complete_ type.

3. If multiple named units remain with nonzero power, then their representations (according to rule
   1) are combined as the elements of a variadic `UnitProduct<...>` pack.

!!! warning
    The ordering of the bases is deterministic, but is implementation defined, and can change at any
    time.  It is a programming error to write code that assumes any specific ordering of the units
    in a pack.

??? example "A few examples"
    We have omitted the `au::` namespace in the following examples for greater clarity.

    | Unit expression | Resulting unit type |
    |-----------------|----------------|
    | `squared(Meters{})` | `Pow<Meters, 2>` |
    | `Meters{} / Seconds{}` | `UnitProduct<Meters, Pow<Seconds, -1>>` |
    | `Seconds{} * Meters{} / Seconds{}` | `Meters` |

## Operations {#operations}

These are the operations which each unit type supports.  Because a unit must be a [monovalue
type](./detail/monovalue_types.md), it can take the form of either a _type_ or an _instance_.
In what follows, we'll use this convention:

- **Capital** identifiers (`U`, `U1`, `U2`, ...) refer to **types**.
- **Lowercase** identifiers (`u`, `u1`, `u2`, ...) refer to **instances**.

### Multiplication

**Result:** The product of two units.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `UnitProductT<U1, U2>`
- For _instances_ `u1` and `u2`:
    - `u1 * u2`

### Division

**Result:** The quotient of two units.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `UnitQuotientT<U1, U2>`
- For _instances_ `u1` and `u2`:
    - `u1 / u2`

### Powers

**Result:** A unit raised to an integral power.

**Syntax:**

- For a _type_ `U`, and an integral power `N`:
    - `UnitPowerT<U, N>`
- For an _instance_ `u`, and an integral power `N`:
    - `pow<N>(u)`

### Roots

**Result:** An integral root of a unit.

**Syntax:**

- For a _type_ `U`, and an integral root `N`:
    - `UnitPowerT<U, 1, N>` (because the $N^\text{th}$ root is equivalent to the
      $\left(\frac{1}{N}\right)^\text{th}$ power)
- For an _instance_ `u`, and an integral root `N`:
    - `root<N>(u)`

### Helpers for powers and roots

Units support all of the [power helpers](./powers.md#helpers).  So, for example, for a unit instance
`u`, you can write `sqrt(u)` as a more readable alternative to `root<2>(u)`.

### Scaling by `Magnitude`

**Result:** A new unit which has been scaled by the given magnitude.  More specifically, for a unit
_instance_ `u` and magnitude instance `m`, this operation:

- **Preserves** the _dimension_ of `u`.
- **Scales** the _magnitude_ of `u` by a factor of `m`.
- **Deletes** the _label_ of `u`.
- **Preserves** the _origin_ of `u`.

**Syntax:**

- `u * m`

## Traits

Sections describing `bool` traits will be indicated with a trailing question mark, `"?"`.

### Is unit?

**Result:** Indicates whether the argument is a valid unit.

!!! warning
    We don't currently have a trait that can detect whether or not a type is a [monovalue
    type](./detail/monovalue_types.md).  Thus, the current implementation only checks whether the
    dimension and magnitude are valid.  Until we get such a trait, authors of unit types are
    responsible for satisfying the monovalue type requirement.

**Syntax:**

- For _type_ `U`:
    - `IsUnit<U>::value`
- For _instance_ `u`:
    - `is_unit(u)`

### Has same dimension?

**Result:** Indicates whether two units have the same dimension.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `HasSameDimension<U1, U2>::value`
- For _instances_ `u1` and `u2`:
    - `has_same_dimension(u1, u2)`

### Are units quantity-equivalent?

**Result:** Indicates whether two units are quantity-equivalent.  This means that they have the same
dimension and same magnitude.  Quantities of quantity-equivalent units may be trivially converted to
each other with no conversion factor.

For example, `Meters{} * Hertz{}` is not the _same unit_ as `Meters{} / Seconds{}`, but they _are_
**quantity-equivalent**.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `AreUnitsQuantityEquivalent<U1, U2>::value`
- For _instances_ `u1` and `u2`:
    - `are_units_quantity_equivalent(u1, u2)`

### Are units point-equivalent?

**Result:** Indicates whether two units are point-equivalent.  This means that they have the same
dimension, same magnitude, _and_ same origin.  `QuantityPoint` instances of point-equivalent units
may be trivially converted to each other with no conversion factor and no additive offset.

For example, while `Celsius` and `Kelvins` are quantity-equivalent, they are _not_
**point-equivalent**.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `AreUnitsPointEquivalent<U1, U2>::value`
- For _instances_ `u1` and `u2`:
    - `are_units_point_equivalent(u1, u2)`

### Is dimensionless?

**Result:** Indicates whether the argument is a dimensionless unit.

**Syntax:**

- For _type_ `U`:
    - `IsDimensionless<U>::value`
- For _instance_ `u`:
    - `is_dimensionless(u)`

### Is unitless unit?

**Result:** Indicates whether the argument is a "unitless unit": that is, a _dimensionless_ unit
whose _magnitude_ is 1.

**Syntax:**

- For _type_ `U`:
    - `IsUnitlessUnit<U>::value`
- For _instance_ `u`:
    - `is_unitless_unit(u)`

### Unit ratio

**Result:** The [magnitude](./magnitude.md) representing the ratio of the input units' magnitudes.

For units with non-trivial dimension, there is no such thing as "the" magnitude of a unit: it is not
physically meaningful or observable.  However, the _ratio_ of units' magnitudes _is_ well defined,
and that is what this trait produces.

For example, the unit ratio of `Feet` and `Inches` is `mag<12>()`, because a foot is 12 times as big
as an inch.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `UnitRatioT<U1, U2>::value`
- For _instances_ `u1` and `u2`:
    - `unit_ratio(u1, u2)`

### Origin displacement

**Result:** The displacement from the first unit's origin to the second unit's origin.

Recall that there is no such thing as "the" origin of a unit: it is not physically meaningful or
observable.  However, the _displacement_ from one unit's origin to another _is_ well defined, and
that is what this trait produces.

For example, the origin displacement from `Kelvins` to `Celsius` is equivalent to
$273.15 \,\text{K}$.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `OriginDisplacement<U1, U2>::value()`
- For _instances_ `u1` and `u2`:
    - `origin_displacement(u1, u2)`

### Associated unit

!!! warning "TODO: this is a stub."

### Common unit

!!! warning "TODO: this is a stub."

### Common point unit

!!! warning "TODO: this is a stub."
