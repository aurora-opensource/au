# Reference Docs

This section contains detailed reference documentation on `Quantity`, `QuantityPoint`, units,
dimensions, magnitudes, and other core library abstractions.  The main folder is reserved for things
that end users will routinely interact with.  Implementation details will be documented in
[a `detail` sub-folder](./detail/index.md).

Here's a guide to the main reference pages.

- **[`Quantity`](./quantity.md).**  This is **the main type** you'll use to replace raw numeric types.
  It acts like a numeric type, but it keeps track of its units.

    - **[`QuantityPoint`](./quantity_point.md).**  Similar to `Quantity`, but for modeling [affine
      space types](http://videocortex.io/2018/Affine-Space-Types/).  This means you can subtract two
      `QuantityPoint`s, and you'll get a `Quantity` (just like subtracting two _points_ gives
      a _displacement_).  Practically speaking, this is **essential for dealing with temperatures**,
      and useful for a couple other dimensions such as pressures and distances.

- **[`Unit`](./unit.md).**  A type which represents a _unit of measure_.

- **[`Magnitude`](./magnitude.md).**  A special kind of compile-time number, which we use to
  represent ratios between pairs of units.  Can handle many challenging use cases which `std::ratio`
  can't, and which are necessary for units libraries --- such as irrational numbers, or extremely
  large or small numbers.

- **[`Prefix`](./prefix.md).**  A term that can be applied to the beginning of a unit, to make a new
  unit with a specified _relative size_.  For example, `centi` is a prefix whose relative magnitude
  is $1/100$, and a `centi(meter)` is a new unit which is one one-hundredth of a `meter`.

- **[`Math functions`](./math.md).**  We provide many common mathematical functions out of the box.

See the sidebar for the complete list of pages.
