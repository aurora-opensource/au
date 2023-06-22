# Dimension

`Dimension` is a family of [monovalue types](./monovalue_types.md) representing equivalence classes
of units.  Each class defines a collection of units which can be meaningfully added, subtracted, and
compared with each other.  Familiar examples of `Dimension` include length, time, temperature,
speed, and so on.

Dimensions form a [vector space](../../discussion/implementation/vector_space.md).  We choose
certain "base dimensions" as the basis vectors for this space.  As with other vector spaces in our
library, `Dimension` values can be multiplied, divided, and raised to (rational) powers, and this
arithmetic always takes place at compile time.

_`Dimension` is an implementation detail._  Most end users will never name dimensions in their code,
and never see them in their compiler errors.  Instead, users will work with [_units_](../unit.md),
which each carry their own dimension information.  The main situation where an end user would use
`Dimension` directly is to define the _first_ unit for a novel _base_ dimension.

## Operations

### Multiplication

**Result:** The product of two `Dimension` values.

**Syntax:**

- For _types_ `D1` and `D2`:
    - `DimProductT<D1, D2>`
- For _instances_ `d1` and `d2`:
    - `d1 * d2`

### Division

**Result:** The quotient of two `Dimension` values.

**Syntax:**

- For _types_ `D1` and `D2`:
    - `DimQuotientT<D1, D2>`
- For _instances_ `d1` and `d2`:
    - `d1 / d2`

### Powers

**Result:** A `Dimension` raised to an integral power.

**Syntax:**

- For a _type_ `D`, and an integral power `N`:
    - `DimPowerT<D, N>`
- For an _instance_ `d`, and an integral power `N`:
    - `pow<N>(d)`

### Roots

**Result:** An integral root of a `Dimension`.

**Syntax:**

- For a _type_ `D`, and an integral root `N`:
    - `DimPowerT<D, 1, N>` (because the $N^\text{th}$ root is equivalent to the
      $\left(\frac{1}{N}\right)^\text{th}$ power)
- For an _instance_ `d`, and an integral root `N`:
    - `root<N>(d)`

### Helpers for powers and roots

Dimensions support all of the [power helpers](../powers.md#helpers).  So, for example, for
a dimension instance `d`, you can write `sqrt(d)` as a more readable alternative to `root<2>(d)`.

## Included base dimensions

Au includes the following base dimensions:

- `Length`
- `Mass`
- `Time`
- `Current`
- `Temperature`
- `Angle`
- `Information`
- `AmountOfSubstance`
- `LuminousIntensity`

These comprise each of the seven base dimensions in the SI, with the addition of `Angle` and
`Information`.
