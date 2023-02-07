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
