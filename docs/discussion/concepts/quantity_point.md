# Quantity Point

While `Quantity` works well for most units library operations, there are some situations where it
struggles.  The most important example is _temperature_: as we'll soon see in detail, `Quantity`
alone could never handle all temperature use cases simultaneously.  The tool that solves this
problem, `QuantityPoint`, also helps similar use cases, such as atmospheric pressure.  It even
improves seemingly unrelated use cases as well, such as along-path positions ("mile markers").

Overall, `QuantityPoint` is a subtle but critically important tool in a units library toolbox.
Let's dive into the main motivating problem, and then learn about the properties of its solution.

## Temperatures are error prone

Let's look at a use case where `Quantity` struggles: _temperature_.  Consider: is 20 degrees Celsius
the same as 20 Kelvins?

Answer: _it depends_.

- If we're talking about a _change_ in temperature, they're completely equivalent.  If the
  temperature **increased by** 20 degrees Celsius, then it increased by 20 Kelvins.

- If we're asking what the temperature _is_, they're very different.  If the temperature **is** 20
  degrees Celsius, then it's **not** 20 Kelvins --- it's _293.15_ Kelvins!

What's going on is that the Celsius scale has _shifted the origin_, that is, the temperature which
we consider to be "zero".  This shift helps humans: it labels the temperatures we encounter in the
environment with numbers that are easier to work with.  That's an important property of
a well-chosen system of units for a given domain.

!!! note
    Note especially that this origin shift is _not an option_ for temperature _changes_!  There
    could never be a unit that assigns "zero" to any amount other than "no change".

Let's ponder the implications for a C++ units library.  Assume that `Quantity` is all we have, and
consider: how should this library convert from degrees Celsius, to Kelvins?  We have two choices for
our policy: we can either take the origin offset into account, or not.  But it's a no-win situation:
either choice produces wrong answers for perfectly legitimate use cases!  We can't possibly handle
both temperatures and temperature changes --- at least, not if `Quantity` is our only tool.

## Mile marker math

Viewing temperatures as _points_, rather than "amounts", changes the set of operations that make
sense for them.  To grasp this, let's consider another kind of labeled points: _mile markers_.
(These will make it easier to understand, because the terminology is less confusing than for
temperatures.)

Mile markers label the points along a linear path using distance units.  The choice of which point
is "mile zero" is completely arbitrary, but once we make that choice, the labels for the rest of the
points are determined.

Here are some examples showing how "mile marker math" works.  Let's imagine we have a function,
`mile_marker()`.  Say it produces some type that models "points", which we'll call `QuantityPoint`
(as opposed to the "displacements" or "amounts" which `Quantity` models).  Here are some examples
showing how we should expect it to behave:

??? example "Example: `mile_marker(8) - mile_marker(5)`"
    **Result:** `miles(3)`

    **General Principle:** Subtracting two points is **meaningful**, but it produces a **quantity**,
    not a point.

??? example "Example: `mile_marker(8) + mile_marker(5)`"
    **Result:** :warning: Nonsense: no result. :warning:

    **General Principle:** Adding two points is **meaningless**.

??? example "Example: `mile_marker(8) + miles(5)`"
    - **Result:** `mile_marker(13)`

    - **General Principle:** Adding a quantity to a point is **meaningful**, and it produces another
      **point**.

??? example "Example: `mile_marker(8) - miles(5)`"
    - **Result:** `mile_marker(3)`

    - **General Principle:** Subtracting a quantity from a point is **meaningful**, and it produces
      another **point**.

These examples show the symbiotic relationship between `Quantity` and `QuantityPoint`.  A `Quantity`
is just the displacement between two `QuantityPoint` instances.  And we can add or subtract such
a `Quantity`, to go from one `QuantityPoint` to another.

!!! tip
    The technical term for `QuantityPoint` is "affine space type".  If you're interested to learn
    more, check out [this accessible introduction to Affine Space
    Types](http://videocortex.io/2018/Affine-Space-Types/).

## Temperatures revisited

Armed with `QuantityPoint`, our formerly confusing temperature use cases have become a breeze.  If
we always use `QuantityPoint` for temperatures, and `Quantity` for temperature _changes_, we can
express ourselves with effortless clarity.

In Au, we use the `_pt` suffix for functions that make `QuantityPoint`, and the `_qty` suffix (or,
more commonly, no suffix[^1]) for those that produce `Quantity`.  Let's rephrase our challenge cases
using these new tools.

[^1]: In practice, `Quantity` is overwhelmingly more common, so we prefer to omit the suffix: we
write `meters(2)` instead of `meters_qty(2)`, for example.  However, this policy would fail for
temperatures: if we wrote `celsius(20)`, should it refer to the _temperature_, or temperature
_change_?  Clearly, this would be far too error prone.  Therefore, for temperatures, we always
include the `_qty` suffix for `Quantity` makers.

- Temperature _changes_: `celsius_qty(20) == kelvins_qty(20)`.  (Result: `true`)

- _Temperatures_: `celsius_pt(20) == kelvins_pt(20)`.  (Result: `false`)

These tools are so clear and reliable that we can even mix and match different temperature scales at
will!  For example: if we started the day at -40 degrees Fahrenheit, and it warmed up by 60 degrees
Celsius, would it be hotter than 300 Kelvins?  Doing this by hand, we might make use of a few facts:

- -40 degrees Fahrenheit happens to be the same temperature as -40 degrees Celsius, so the final
  temperature is equivalent to 20 degrees Celsius.
- The offset between Kelvins and Celsius is 273.15 K, so the final temperature of 20 degrees Celsius
  is 293.15 K.
- Therefore, the answer is "no": it's not hotter than 300 Kelvins.

Now we can see whether Au comes to the same conclusion.  Note how easy it is to express the question
with clarity, despite mixing _three_ different temperature scales:

```cpp
EXPECT_THAT(fahrenheit_pt(-40) + celsius_qty(60), Lt(kelvins_pt(300)));
```

As hoped, this test passes.

!!! note "Fun fact"
    Au would perform the above calculation _without ever leaving the integer domain_, even though
    the offset between Kelvins and Fahrenheit (or Celsius) is not an integer!  As an exercise,
    ponder how we might do that.

## `QuantityPoint` and `std::chrono::time_point`

Readers familiar with the `std::chrono` library may recognize this kind of interface: it's similar
to `std::chrono::time_point`.  This class has the same relationship to `std::chrono::duration` as
`QuantityPoint` has to `Quantity`. In each case, we have a "point" type and a "displacement" type.
And the allowed operations are similar, for example:

- You can subtract two points to get a displacement.
- You can add (or subtract) a displacement to (or from) a point.
- You **can't** add two **points**; that's a meaningless operation.

These similarities may tempt the reader to reach for a time-units `QuantityPoint` to replace
`std::chrono::time_point`, just as a time-units `Quantity` makes a very capable replacement for
`std::chrono::duration`.  However, experience doesn't support this choice.

There's much more to `std::chrono::time_point` than just providing arithmetic operations with point
semantics.  It also models different kinds of clocks, preventing unintended inter-conversion between
them.  And it handles real-world clock subtleties, such as modeling whether a clock can ever produce
an earlier value at a later time (think: daylight saving time).  By contrast, `QuantityPoint` can
only handle measurement scales that are identical up to a constant offset --- and that offset must
be known at compile time.

**Bottom line**: when you need to track timestamps, you're better off using a special purpose
library like `std::chrono`.  But once you subtract two `time_point` instances to get a `duration`,
it's often useful to convert it to Au's `Quantity` --- whether
[implicitly](../../reference/corresponding_quantity.md#chrono-duration), or
[explicitly](../../reference/corresponding_quantity.md#as-quantity) --- so that it can participate
in equations with other units (such as speeds and distances).

## Summary

`QuantityPoint` is largely a refinement for C++ units libraries.  Most use cases don't need it, and
we don't even bother to define it for almost all units.  However, it is useful in a few use cases,
such as mile markers or atmospheric pressure.  And for some use cases, such as temperatures or
atmospheric pressure, it's absolutely essential.
