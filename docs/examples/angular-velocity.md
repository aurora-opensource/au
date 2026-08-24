# Angular velocity in RPM

A wheel rolls along the ground at some speed.  How fast is it turning, in revolutions per minute?

<!--
  AUTHORING NOTE.  The two tabs below are a blink comparison: readers flip between them and compare
  corresponding lines in place.  Keep each banner to one short line, or the code will start at
  a different height in each tab and the comparison stops working.  The real explanation belongs
  under "What's happening", not in the banner.
-->

=== "⚠️ Before: raw C++"

    ⚠️ **Before** --- conversion factors are magic numbers, and units completely lack enforcement.
    { .ab-banner .ab-before }

    ??? note "Includes and usings"

        ```cpp
        --8<-- "examples/angular_velocity/raw.cc:frontmatter"
        ```

    ```cpp
    --8<-- "examples/angular_velocity/raw.cc:example"
    ```

=== "✅ After: with Au"

    ✅ **After** --- the types carry the units, and `rad / r` states the definition directly.
    { .ab-banner .ab-after }

    ??? note "Includes and usings"

        ```cpp
        --8<-- "examples/angular_velocity/au.cc:frontmatter"
        ```

    ```cpp
    --8<-- "examples/angular_velocity/au.cc:example"
    ```

Both programs print `409.256 rev / min`[^1].  Note where that unit label comes from: the Au version
derives it from the return type, while the raw version has it typed in by hand: one more thing to
keep in sync.

[^1]: All of our examples get compiled and run in CI, and we check that they produce the same
    output.

## What's happening

The raw version computes $\omega = \frac{v}{2 \pi r} \cdot 60$.  The underlying physics gets
obscured by manual unit bookkeeping:

- $2\pi$ converts the angle units between radians and revolutions

- $60$ converts the time units between seconds and minutes

It's up to whoever writes this function to get these constants right, _and_ to make sure they
don't accidentally multiply when they should divide (or vice versa).

The _real_ vulnerability is in the _interface_.  The function needs very specific units, but they
are only ever _described_ --- in a comment, and in the parameter name suffixes (`_mps` and `_m`).
Nothing there is checked.  The callers will be in some other file, far away, where neither the
comment nor the parameter names are easy to see.  Any mistakes will be hard to spot, and the
compiler will let them right on through.

Au tackles this vulnerability head-on.  With `QuantityF` parameters, we know that users can _only_
pass the right kind of quantity: the _compiler_ will produce a readable error if they get it wrong.
We even get extra flexibility: if we change the units at the callsite, the compiler will _generate
a correct and efficient conversion factor automatically_, and we'll still get the right answer!
We've seen this in action in the example above, where we pass a millimeter length to a function that
takes the length in meters.

The implementation gets a lot simpler, too.  All of the magic number conversion factors are gone.
Instead, we just state the underlying physics directly and clearly:

- angular velocity is proportional to linear velocity
- the conversion factor is just a ratio between angle and length
- "radian per radius" (`rad / r` in code) is exactly that ratio, _by definition of the radian unit_.

It's instructive to consider what's happening _on the C++ level_ in the `v * rad / r` expression.
`v` and `r` are both `QuantityF` types, so they store a `float` under the hood.  But `rad` is
different: it's a _unit symbol_, which means it takes up no space at all, and _never_ consumes
a runtime instruction.  Multiplying by `rad` only relabels the units, at compile time.  So if we
looked at the assembly generated for `v * rad / r`, we would see only one `float` divided by
another.

Of course, `v * rad / r` is _not_ expressed in revolutions per minute, but in radians per second.
To close the loop, Au compares this to the return type, and automatically generates, _at compile
time_, the _single_ floating point number that combines all these conversion factors.  Thus, the
only further runtime instruction is a single multiplication.

That is two floating point instructions in total, against three for the raw version, which spends
one on `2 * PI * r` before it can divide.

To summarize the overall comparison:

- **Runtime cost:** Au is generally _at least_ as good as the raw version.  In fact, in this
  specific case, _it's even slightly better_ (two floating point instructions vs three).

- **Conversion factor accuracy:** Same in both cases, _except Au's version is guaranteed to be
  correct, with no maintenance cost_.

- **Safety:** Au's version is both vastly safer (guaranteeing callers pass the right dimension), and
  more flexible (letting callers use any units they want).

## Related reading

- [Unit symbols](../reference/unit.md#symbols), such as the `rad` used here.
- [Quantity](../reference/quantity.md), and how conversions are applied.
- [Types for combined units](../reference/unit.md#types-for-combined-units), for expressions like
  `UnitQuotient<Revolutions, Minutes>`.
- [Namespaces and includes](../discussion/idioms/namespaces-and-includes.md), including why a `.cc`
  imports names individually and a `.hh` does not.
