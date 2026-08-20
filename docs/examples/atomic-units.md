# Atomic units

This example is different from the others.  Instead of showing how Au can improve raw numeric code,
this one shows how _existing_ users of Au can _extend_ it to support a new system of units.

Most units --- including _all_ units shipped with Au --- are defined by _exact_ relationships with
the SI base units.  The Hartree atomic units in this example are very different, in an interesting
way:

- **Within** Hartree atomic units, relationships are (still) _exact_.

- **Between** Hartree atomic units and SI units, relationships are _relative to measured constants_.

These constants have associated measurement uncertainty, and future improvements in physics will
surely update their values.  Unfortunately, Au's entire machinery is built on representing _exact_
relationships between units, not approximate or uncertain ones.

This makes it sound like Hartree units should be impossible with Au.  However, we'll see that in
fact, we can handle them just fine!  The one thing we _can't_ do is _ship these definitions with the
library_.  That would mean choosing one-size-fits-all values for these uncertain and measured
constants, preventing each project from choosing the measurement version best suited to its needs.

!!! tip
    This example provides a complete demonstration of creating a new system of units.  You can use
    it for inspiration if your project has any units with inexact relationships.  And if your
    project uses Hartree atomic units specifically, you can even copy paste these files and use them
    directly!

## The definitions

Hartree atomic units are the working units of quantum chemistry.  They are convenient precisely
because they are defined so that the quantities in the Schrödinger equation come out around 1.

The definitions themselves are a library of two files --- a header (`.hh`) and an implementation
file (`.cc`) --- which a program then includes.  We show all three below.

!!! note
    If you are using C++17 or later, you can skip the `.cc` file entirely, by using inline variable
    definitions for the labels.  See [How to define new units](../howto/new-units.md) for details.

### Header (`atomic_units.hh`)

We'll first present the code, and then discuss the design choices that went into it.

```cpp
--8<-- "examples/atomic_units/atomic_units.hh:definitions"
```

Since this is a header file, we qualify Au names with the `au::` prefix, rather than importing them.
This avoids namespace pollution; see [Namespaces and
includes](../discussion/idioms/namespaces-and-includes.md) for more details.  (The one exception is
utilities found by argument-dependent lookup, discussed below.)

The header also needs the two measured values.  By far the best way to express them is using Au
literals: the [magnitude literal](../reference/magnitude.md#_mag-literal) `7.297'352'5693e-3_mag`,
and the [prefixed unit literal](../reference/constant.md#prefixed-unit-literals)
`kilo(9.109'383'7015e-31_g)`.  This lets the source code carry the published CODATA digits
_verbatim_, making it easy to check at a glance.[^1]  Since it's a header file, we do still need to
avoid namespace pollution, so each one goes inside its own function, where the using-directive
expires at the closing brace and never reaches a consumer.

[^1]: Compare this readability to the alternative, `mag<72973525693>() * pow<-13>(mag<10>())`, with
its manually shifted powers of 10.

From this point, we simply follow our standard approach for defining [new
units](../howto/new-units.md) and [constants](../howto/new-constants.md).  One key thing to note is
that when we define a new unit, the type it inherits from must be
a [**unit**](../reference/unit.md), not a [constant](../reference/constant.md) or anything else.
The most ergonomic way to do this is to write the most convenient expression, and then pass it
through the `associated_unit` utility: this converts any [unit
slot](../discussion/idioms/unit-slots.md) into an actual unit type.

Also, as a minor point of style, note that we do not need the `au::` prefix on utilities that take
`au::` types as arguments (`associated_unit`, `squared`).  The compiler uses argument-dependent
lookup (ADL) to find these functions.

### Implementation (`atomic_units.cc`)

In C++14, the unit labels need one out-of-line definition each, in a `.cc` file:

```cpp
--8<-- "examples/atomic_units/atomic_units.cc:labels"
```

## Using them

Here's an example showing how you would _use_ these definitions in your program.  As usual, we
provide "front matter" (includes and `using` statements) collapsed by default, and then show the
core code itself.

??? note "Includes and usings"

    ```cpp
    --8<-- "examples/atomic_units/main.cc:frontmatter"
    ```

```cpp
--8<-- "examples/atomic_units/main.cc:usage"
```

This prints:

```
a_0 = 5.29177e-11 m
E_h = 4.35974e-18 J
t_a = 2.41888e-17 s
e = 1.60218e-19 C
h_bar = 1 E_h * t_a
```

Every name in that output is derived, not typed.  `a_0`, `E_h`, `t_a`, `e` and `h_bar` come from the
constants themselves; `m`, `J`, `s`, `C` and `E_h * t_a` come from the units they were converted
into.

Note that the charge line is the only one whose conversion to SI is *exact*: the elementary charge
is an SI-defining constant, so neither measured input is involved.

The last line is the interesting one, and it makes a different kind of claim.  `h_bar` comes out as
exactly `1` in units of `E_h * t_a` --- a relationship *inside* the system, with no measured input
anywhere in it.  Below, we'll see how the program gets that checked by the compiler, rather than
asking you to trust a printed digit.

## What's happening

Let's highlight the most salient points about how this new system works.

**Everything derives from exactly five inputs.**  Three of them --- the speed of light, the reduced
Planck constant, and the elementary charge --- are SI-*defining* constants.  They have no
uncertainty, because the SI defines the kilogram, meter and second in terms of them rather than the
other way round.  The other two --- the electron mass and the fine structure constant --- are
measured.  Separating them this way means that updating to a newer CODATA release is a two-line
change, and you can see at a glance exactly which numbers in your program are experimental inputs.

**Each unit is defined by its formula, not by a decimal value.**  The hartree is written as
`m_e * squared(c * alpha)`, not as `4.3597447e-18` joules.  That is the difference that makes the
last line of the output read `1 E_h * t_a` rather than `0.999999999...`.  The atomic time unit is
*defined* as $\hbar/E_h$, so within the system that relationship is exact --- Au carries it as an
exact rational [magnitude](../reference/magnitude.md), never as a rounded double.

The two core properties we outlined at the beginning follow from these definitions:

- **Inside the system, relationships are exact.**  No accumulated error from round-tripping through
  SI, no drift, no epsilon comparisons.

- **Crossing out to SI is as accurate as physics currently allows.**  Au composes the entire
  definition chain --- through the hartree, through $\alpha^2$, through the electron mass --- into a
  single exact rational factor, and applies it once. The only error is the uncertainty in the two
  measured inputs.

**The first property is checked by the compiler:**  The header closes with a `static_assert` that
asks for $\hbar$'s value in `hartrees * atomic_time_units` **as an `int`**.

```cpp
--8<-- "examples/atomic_units/atomic_units.hh:assert"
```

The `int` output is what gives this teeth.  Au refuses to compile a conversion it knows would
[truncate](../discussion/concepts/truncation.md), so the fact that this line compiles at all
means the conversion factor is exactly 1, and not a double that happens to round to `1.0`.  And this
property is completely independent of the actual measured values, so it won't break when we update
them after newer experiments give us more precise values.

**The labels make output readable.**  Because each unit declares a `label`, printing a quantity
gives `a_0` or `E_h` rather than an unwieldy composite of SI base units --- and composite units get
composite labels, which is where the `E_h * t_a` on the last line comes from.  Streaming a
`Constant` prints its label alone, so `std::cout << HARTREE` writes `E_h`.  That is why the program
above never spells a unit name out in a string literal.

**Nothing here is special-cased.**  These are ordinary Au units, using the same mechanism as
[any other custom unit](../howto/new-units.md).  They compose with the built-in units, participate
in the same conversion checks, and cost nothing at runtime.

## Related reading

- [How to define new units](../howto/new-units.md), including the C++14 versus C++17 label forms.
- [How to define new constants](../howto/new-constants.md).
- [Magnitude](../reference/magnitude.md), the exact compile-time arithmetic that makes the
  in-system relationships exact.
- [Unit literals](../reference/constant.md#unit-literals) and
  [prefixed unit literals](../reference/constant.md#prefixed-unit-literals), for writing a measured
  value exactly as it was published.
- [Applying a prefix to a `Constant`](../reference/prefix.md#constant), which is what makes
  `kilo(...e-31_g)` mean what you would want it to mean.
- [Namespaces and includes](../discussion/idioms/namespaces-and-includes.md), including why a `.cc`
  imports names individually and a `.hh` does not.
