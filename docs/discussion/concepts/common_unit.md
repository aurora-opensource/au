# Common Units

Some operations (multiplication, division, powers) work natively with _arbitrary_ units.  Others
(addition, subtraction, comparison) require converting to a "common" unit.  This page explains the
concept, the requirements, and how we implement it.

## Concepts

In this section, we'll use square bracket notation, $[x]$, to refer to a **unit**.  Units can be
multiplied by [_magnitudes_](../../reference/magnitude.md) (i.e., nonzero real numbers) to form
_new_ units: so, $[12x]$ is a unit which is 12 times the size of the unit $[x]$.

A _quantity_ is some property which can be measured.  A measurement result has two parts: the _unit
of measure_, and the _value_ of the quantity, which is the ratio of the quantity to that unit.  For
example: suppose we had some physical length, and we found that if we placed four yardsticks
end-to-end, they would exactly coincide with this length.  Then the measured quantity would be $4
[\text{yd}]$: $4$ is the value, and $[\text{yd}]$ is the unit.

We should be careful not to say that the quantity "is" the unit/value pair!  We can convert
a quantity to any other unit of the same [dimension](../../reference/detail/dimension.md) by
"trading off" numeric factors between the value and the unit.  For example, using the fact that
a yard is three feet, we can convert the above quantity to this new unit like so:

$$
\begin{align}
4 [\text{yd}] &= 4 \left[ \text{yd} \left(\frac{3\, \text{ft}}{\text{yd}}\right) \right] \\
&= 4 [3\, \text{ft}] \\
&= (4 \cdot 3) [\text{ft}] \\
&= 12 [\text{ft}]
\end{align}
$$

Notice that in going from line 2 to 3, we pulled the factor of 3 out of the unit, and applied it to
the value.  This changes the unit, _and_ the value, but the _overall quantity_ is unchanged.  This
is the key point: _one quantity; many representations_.

### The need for common units

_Physically_, we can compare any two quantities of the same dimension.  It doesn't matter if one is
measured in feet, and the other in yards; we can place the physical lengths next to each other, and
see which one is longer.  _Computationally_, we need to express them in the same unit, so that our
notion of `<` for _quantities_ can simply "inherit" from our notion of `<` for their _values_.

!!! tip
    This is exactly analogous to the need for common _denominators_ when working with fractions.
    Each fraction can be expressed in many different denominators, and all of those representations
    represent the _same_ number, the _same_ element of the set $\mathbb{Q}$.

    However, before we can add, subtract, or compare different fractions, we need to express them in
    the _same_, common denominator (analogous to units).  Once we do, we can simply apply these
    operations directly to the numerators (analogous to values).

In principle, _any_ unit of the same dimension can serve as the "common unit".  However, just as we
tend to prefer the _lowest_ common denominator for fractions, there is also a preferred common unit
for quantities.  The usual choice is _the largest (i.e., greatest magnitude) unit which evenly
divides both input units_.  This has some very nice properties.

- Since it _evenly divides_ both units, each conversion will end up simply _multiplying by an
  integer_ (as in our example above).  This lets us **stay in the integer domain** if we started out
  there.

- Since it's the _largest_ such unit, we'll be multiplying by the _smallest integers_ that still get
  the job done.  Not only are smaller numbers easier to work with, but when we move to the
  programming domain, they also **reduce the risk of overflow**.

Now, this isn't always possible: for example, _no_ unit evenly divides both degrees and radians!  In
those cases, our choice matters less, and it can be driven by convenience.

## C++ considerations (`Quantity`)

!!! note
    This section only applies to `Quantity` types.  We follow a similar strategy for
    `QuantityPoint`, but with a few differences we'll explain at the end.

The "common unit" is the unit of the _common type_ of two or more `Quantity` instances, in the sense
of [`std::common_type`](https://en.cppreference.com/w/cpp/types/common_type).  What properties
should it have?

### Requirements

1. **Symmetry**.  The common unit of any collection of input units must be independent of their
   ordering.
    - This flows directly from the requirements for specializing `std::common_type`, [which
      state](https://en.cppreference.com/w/cpp/types/common_type):

        > Additionally, `std::common_type<T1, T2>::type` and `std::common_type<T2, T1>::type` must
        > denote the same type.

2. **Deduplication.**  Any given input unit can appear at most once in the resulting unit type.
    - This is to keep compiler errors as concise and readable as possible.
3. **Flattening.**  If an input unit is a `CommonUnit<...>` type, "unpack" it and replace it with
   its constituent Units.
    - To see why, let `c(...)` be "the common unit", and `x`, `y`, and `z` be units.  We wouldn't
      want `c(x, c(y, z))` to be different from `c(x, y, z)`!
4. **Semantic.**  Prefer user-meaningful units, because they show up in compiler errors. Thus, if
   any input unit is _equivalent_ to the "common unit", we'll prefer that input unit.
    - The common unit of `Inches` and `Feet` is just `Inches`, not `CommonUnit<Inches, Feet>`!

### User-facing types

There are two main abstractions for common units which users might encounter.

- **`CommonUnit<...>`**.  This is a template that defines new units from old ones, just like
  `UnitProduct<...>` or `ScaledUnit<...>`.
    - This should _rarely, if ever_ be named in code.
        - In implementations, we need to do this, for example, for defining the _unit label_ of
          a `CommonUnit<...>`, or defining its ordering relative to other units.
        - In end user code, this should probably _never_ be named.
        - In either case: **never** write `CommonUnit<...>` with _specific_ template arguments!
          Only use it for matching.
    - Remember: `CommonUnit<...>` can arise only as _the result of some type computation_.
- **`CommonUnitT<...>`**.  This _computes_ the common unit of the provided units.

Let's clarify this relationship with an example.  Suppose you're writing a function based on two
arbitrary (but same-dimension) units, `U1` and `U2`, and you need their "common unit".

- What you would _write_ is `CommonUnitT<U1, U2>`, **not** `CommonUnit<U1, U2>`.
    - `CommonUnitT<...>` says "_please calculate_ the common unit".
    - `CommonUnit<...>` says "_this is the result_ of calculating the common unit".
- What you _get_ depends on the specific units.
    - For `CommonUnitT<Inches, Meters>`, the result might be `CommonUnit<Inches, Meters>`.[^1]  This
      is because the greatest common divisor for `Inches` and `Meters` is smaller than both of them.
    - For `CommonUnitT<Inches, Feet>`, the result would simply be `Inches`, because `Inches` is
      quantity-equivalent to this common unit (it evenly divides both `Inches` and `Feet`).

[^1]:  It might also be `CommonUnit<Meters, Inches>`.  The ordering is deterministic, but
unspecified.

??? info "Implementation approach details (deep in the weeds)"

    There are two main tools we use to implement `CommonUnitT`.

    1. `FlatDedupedTypeList`.  For a given variadic pack `List<...>` (which, for us, will be
       `CommonUnit<...>`), `FlatDedupedTypeList<List, Ts...>` will produce a `List<...>`, whose
       elements are `Ts...`, but sorted according to `InOrderFor<List, ...>`, and with duplicates
       removed.

        - If any of the `Ts` are already `List<Us...>`, we effectively "unpack" it, replacing it with
          `Us...`.  This is the "flat" part in `FlatDedupedTypeList`.

    2. `FirstQuantityEquivalentUnit`.  The above step produces a `CommonUnit<...>` specialization, which
       itself meets [the definition of a unit](../../reference/unit.md).  But is it the unit we
       really want to provide?  Not if there's a simpler one!
       `FirstQuantityEquivalentUnit<CommonUnit<Us...>>` searches through the unit list `Us...`, and
       returns the first quantity-equivalent one it finds. If no such unit is available, then we
       fall back to returning `CommonUnit<Us...>`.

## Changes for `QuantityPoint` {#common-quantity-point}

The common unit for [`QuantityPoint`](./quantity_point.md) operations is different from the common
unit for `Quantity`.  To see why we can't just reuse the same "common unit", consider `Celsius`
and `Kelvins`.

- For a **`Quantity`**, these two units are identical.  The "common unit" will be
  [(quantity-)equivalent](../../reference/unit.md#quantity-equivalent) to both of
  them.

- For a **`QuantityPoint`**, these units are very different.  A "temperature point" of 0 degrees
  `Celsius` is [(point-)equivalent](../../reference/unit.md#point-equivalent) to a temperature point
  of 273.15 `Kelvins`.  This additive offset means that we'll need to convert both to
  `Centi<Kelvins>` before we can subtract and/or compare them!

Thus, what we've been calling `CommonUnitT` is really more like `CommonQuantityUnitT` (although
we've kept the name short because `Quantity` is by far the typical use case).  For `QuantityPoint`
operations, we have the `CommonPointUnitT<Us...>` alias, which typically creates some instance of
`CommonPointUnit<Us...>` with the `Us...` in their canonical ordering.

So: what _is_ the "common quantity _point_ unit"?  Well, we can start with the "common _quantity_
unit," but the origin adds a new complication.  We'll need to choose a convention.

- With "common _quantity_ units," our convention ensured that conversions could only **multiply** by
  a **nonzero integer**[^1].  This keeps us within the domain of the integers whenever we start
  there. And we chose the **smallest** such number to minimize overflow risk.

- Similarly, with "common quantity _point_ units," we should choose its origin such that we only
  **add** a **non-negative** integer.  This convention preserves and extends the previous one: not
  only are we keeping integers as integers, but we support **unsigned** integers as best we can.

So the _origin_ of the "common point unit" is simply the _lowest_ origin of any input unit.

To complete our definition, we need to determine the _precise_ effect that origin displacements
should have on the unit's _magnitude_.  Our approach is to take every pair of units with different
origins, and treat their origin displacement as a _new, implicitly created unit_.  This new unit
then participates in the common unit calculation in the usual way, on equal footing with the others.

??? example "Example: common point unit of degrees Celsius and Kelvins"
    The magnitude of the unit itself is 1 K, in both cases.  The _origin displacement_ is 273.15 K,
    and we treat this as a new, implicit unit.  Now we simply combine these units using the usual
    common unit algorithm, which is equivalent to [Euclid's
    algorithm](https://en.wikipedia.org/wiki/Euclidean_algorithm) for the greatest common divisor:

    | Higher | Lower | Remainder |
    |--------|-------|-----------|
    | 273.15 K | 1 K   | 0.15 K    |
    | 1 K    | 0.15 K | 0.10 K    |
    | 0.15 K | 0.10 K | 0.05 K    |
    | 0.10 K | 0.05 K | 0 K       |

    Therefore, the greatest common divisor has a magnitude of 0.05 K.  This means that our common
    point unit is a unit with a magnitude of 0.05 K, whose origin equals the origin of Kelvins
    (because it is lower than the origin of degrees Celsius).

    Au will automatically generate the label
    `"EQUIV{[(1 / 20) K], [(1 / 5463) (@(0 degC) - @(0 K))]}"`
    for this unit, which gives the value in terms of all relevant inputs.

[^1]: The overwhelmingly most common case here is that this integer is _positive_ as well, not just
nonzero, so that we usually don't change the sign, either.  If the integer happens to be negative,
then this unit conversion would produce a hard compiler error if applied to an unsigned type.
