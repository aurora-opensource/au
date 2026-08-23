# Eigen: 3D vector kinematics

A body has a position, a velocity, and an acceleration, each a 3D vector.  Where is it one timestep
later, and how far from the origin is it?

<!--
  AUTHORING NOTE.  The two tabs below are a blink comparison: readers flip between them and compare
  corresponding lines in place.  Keep each banner to one short line, or the code will start at
  a different height in each tab and the comparison stops working.  The real explanation belongs
  under "What's happening", not in the banner.
-->

=== "⚠️ Before: raw C++"

    ⚠️ **Before** --- every vector is a bare `Vector3d`, and the units live only in the names.
    { .ab-banner .ab-before }

    ??? note "Includes and usings"

        ```cpp
        --8<-- "examples/eigen_kinematics/raw.cc:frontmatter"
        ```

    ```cpp
    --8<-- "examples/eigen_kinematics/raw.cc:example"
    ```

=== "✅ After: with Au"

    ✅ **After** --- each vector is a `Quantity` whose rep is `Vector3d`, so the units are checked.
    { .ab-banner .ab-after }

    ??? note "Includes and usings"

        ```cpp
        --8<-- "examples/eigen_kinematics/au.cc:frontmatter"
        ```

    ```cpp
    --8<-- "examples/eigen_kinematics/au.cc:example"
    ```

!!! note
    The two tabs are aligned for comparison: blank lines where one version needs fewer
    statements, and extra spaces so that corresponding expressions sit in the same column.
    Neither is a spelling we'd recommend writing --- they're here so that flipping between
    the tabs shows only the real differences.

Both programs print the same two lines[^1]:

```
      5       0 119.694 m
119.798 m
```

[^1]: All of our examples get compiled and run in CI, and we check that they produce the same
    output.

## What's happening

The physics is the same on both sides: $x + v \, \Delta t + \frac{1}{2} a \, \Delta t^2$.  What
changes is who is responsible for the units.  The raw code has three different unit conversions:

- `km/h` to `m/s` for the velocity,
- `ms` to `s` for the timestep, and
- `g_0` to `m/s^2` for the acceleration.

The first two are at least visible in the source.  The third one isn't, because the raw version
doesn't convert anything.  Instead, it uses `-9.80665` directly as a magic number, leaving users to
guess the intent.

All of those conversions vanish from the source code in the Au version, because the library
automatically generates the correct conversion factors --- _at compile time_.

The names simplify, too: unit-suffixed names such as `x_m`, which force the human to keep track of
the units, get replaced by the simpler `x`.  In fact, for velocity, we get _even more_
simplification: both `v_mps` and `v_kph` get replaced by a single `v`.  Its units happen to be `km
/ h`, but we don't need to worry about that; we know the library will produce any necessary
conversions.  These simpler names really pay off in the `advanced_position()` function body: when
the suffixes vanish, the underlying physics shows through more clearly.

The output lines simplify for the same reason.  The raw version types the unit label by hand ---
`<< " m"`, twice, with nothing checking that it still matches what the number means.  The Au version
streams the quantities themselves, and the label comes from the type: `norm(x_new)` is a length, so
it prints `m`, and `transpose(x_new)` is a whole vector, so it prints Eigen's formatting of the
elements followed by the one unit they all share.

### Unit symbols and constants

This example leans on [Unit symbols], such as `m` and `km`, and [Constants], such as
`STANDARD_GRAVITY`.  They both have the same effect here: when you _multiply_ or _divide_ by them,
they _change the units_, but **not** the _underlying stored value_.  If the input is already
a `Quantity`, you get another `Quantity`; and if it's not, then it _becomes_ one.

Unit symbols are a handy, _concise_ way to annotate your variables with their units.  Writing `12.34
* m / s` has exactly the same effect as `(meters / second)(12.34)`; it's just a little shorter.

Again, keep in mind that unit symbols and constants do _not_ change the underlying value.  So, if
you're following our Eigen [safety guide], these do _not_ count as "operations" that create risk for
dangling references.  That's why the variable assignments here are perfectly safe, even without
`eval()`.

??? note "More nuance on lifetime risk"
    To be clear: we mean that multiplying by symbols or constants doesn't _add_ lifetime risk.  We
    _don't_ mean unit symbols and constants _preclude_ lifetime risk.  If there is _pre-existing_
    lifetime risk, these won't magically remove it.

    Consider this example.  Suppose we have two utility functions that return `Eigen::Vector3d`
    instances:

    ```cpp
    Eigen::Vector3d v1();
    Eigen::Vector3d v2();
    ```

    The following example is guaranteed to dangle:

    ```cpp
    auto q = (v1() + v2()) * m;
    ```

    The sum holds references to its operands, which in this case are the temporary objects `v1()`
    and `v2()`, neither of which survives past the semicolon at the end of the line.  `m` doesn't
    make this safe, but it's also not the root of the problem: the following simpler example is also
    guaranteed to dangle!

    ```cpp
    auto q = v1() + v2();
    ```

    We hope this discussion clarifies how unit symbols and constants relate to lifetime risk.  For
    a fuller treatment of this topic, we recommend that all users read and understand the Eigen
    [safety guide] before using Au with Eigen.

### Alias names

In this example, we went with `Position`, `Velocity`, and `Acceleration`.  This is a fine approach,
but not the only possible one.  If you want to use mixed units in your interfaces, you could also
define a custom "rep-named alias" for `Eigen::Vector3d`:

```cpp
template <typename U>
using QuantityV3 = Quantity<U, Eigen::Vector3d>;
```

Then you could write `QuantityV3<Meters>` for a position, `QuantityV3<KilometersPerHour>` (after
defining a suitable `KilometersPerHour` alias), and so on.

This question is mostly a matter of taste; Au is safe either way.

### Eigen safety

Eigen's famously fast performance comes in part from *lazy evaluation*.  The equally famous _cost_
of this speed is an elevated risk of object lifetime bugs.  We have a whole Eigen [safety guide]
devoted to this topic in general.  We'll hit the highlights relevant to this example here.

First, it's important to appreciate that Au has "risk parity" with Eigen.  This means that when you
add Au to Eigen, you still have all the same risks, but you _don't_ get _new_ ones.  The [safety
guide] explains the details, but the upshot is that you should still be looking for the same warning
signs as raw Eigen, and you'll still use the same strategies to mitigate the issues (even if some of
the particulars might change, such as `eval()` being a free function instead of a member function).

In this example, there are three places where expression templates occur, and thus three places that
may carry object lifetime risk.

- The arguments `v` and `a` that we pass to `advanced_position()` both need unit conversions.
    - These are safe because they're assigned to a _concrete type_: `Velocity` and `Acceleration`,
      respectively.

- The _return value_ of `advanced_position()`, `x + v * dt + 0.5 * a * dt * dt`, is also an
  expression template.
    - This is safe because the return value is a concrete type: `Position`.  Evaluation happens when
      we convert to the concrete type.

- The `transpose(x_new)` that we stream on the last line is an expression template too: Eigen's
  "view" functions are operations, just like arithmetic.
    - This one is _not_ assigned to a concrete type.  It's safe for the other reason: we consume it
      inside the same full expression, so `x_new` cannot have died or changed in the meantime.

So, using concrete types for the input parameters and the return value automatically guarantees
lifetime safety at those boundaries --- and an expression you compute and consume on the spot is
safe, because nothing gets deferred past its inputs.

## Summary

Au's Eigen support makes it easier to get your units right _robustly_, and often makes your code
easier to read.  Lifetime safety is the one thing it leaves exactly as it found it, for better and
for worse, so make sure you're familiar with the Eigen [safety guide] before you start using Au with
Eigen.

## Related reading

- [Eigen how-to guide](../howto/interop/eigen.md), for creating and using Eigen-backed quantities.
- [Eigen safety][safety guide], on expression templates and object lifetime.
- [Eigen compatibility reference](../reference/eigen.md), for the full list of free functions.
- [Unit symbols], including the prefix-applier form used for `km`.
- [Constants], such as the `STANDARD_GRAVITY` used here.
- [Element access](../reference/quantity.md#element-access), for reading and writing one component
  of a vector quantity.

[safety guide]: ../discussion/concepts/eigen_safety.md
[Unit symbols]: ../reference/unit.md#symbols
[Constants]: ../reference/constant.md
