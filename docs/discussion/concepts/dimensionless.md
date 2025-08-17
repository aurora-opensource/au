# Dimensionless Units and Quantities

Every dimension has a variety of units available to measure its quantities.  This is no less true
for the "null" dimension!  Example units include "dozen", "score", "percent", and others.  We call
these units (and their quantities) "dimensionless".

One big difference (compared to units of other dimensions) is that the magnitudes of _dimensionless_
units are _objectively meaningful_.  Recall that for, say, length-dimensioned units, there is no
such thing as "the" magnitude of `Feet`.  We can choose any number we like, as long as it's 12 times
the magnitude of `Inches`.  By contrast, `Percent` has a definite magnitude: it's $1/100$.

## `Unos`: the "unit 1"

Dimensionless units are special, but one is more special still.  Literally, _one_ --- the
dimensionless _unit_ whose _magnitude_ is 1.  It is the only unit equal to its own square, and the
only unit whose quantities are completely and unambiguously interchangeable with raw numbers.

In our library, we named this unit "unos", after an [SI proposal from the 1990s][unos].  Although
the proposal failed, the concept turns out to suit software libraries much better than scientific
prose. It is short, greppable, and reasonably intuitive.  It also lets us enter and exit the library
boundaries in just the same way as for other units: `q = unos(x)` turns a numeric value `x` into
a Quantity `q`, and `q.in(unos)` retrieves the raw number.

This is particularly useful when working with _non-`unos`_ dimensionless units.  For example: say we
wanted to "express `0.75` as a quantity of `percent`". Instead of trying to remember whether to
multiply or divide by 100, we can simply write `x = unos(0.75).as(percent)`.  And if we have
something that's already a `percent`, but we want its "true" value, we can simply write
`x.in(unos)`.

## Exact cancellation and types {#exact-cancellation}

Sometimes a computation exactly cancels all units (like the ratio of two lengths, each measured in
`Feet`).  As a units library, we have two options: return a Quantity of `Unos`, or a raw number.
Presently, we opt for the latter; here is why.

Users generally tend to expect the result of a perfectly unit-cancelling expression to behave
exactly like a raw number, in _every_ respect.  Although a `Quantity<Unos, T>` implicitly converts
to `T`, this conversion turns out to get triggered in only a subset of use cases; many edge cases
remain.  The only way to _perfectly_ mimic a raw number is to return one.

The downside is that this incurs some complexity.  This mainly impacts _generic_ code, where we
can't know whether a product or quotient of Quantities is a Quantity, or a raw number.  People
writing generic code are generally more advanced users, and thus better able to work around this
inconsistency.  For example, one could write an `ensure_quantity(T x)` function template, which
returns `unos(x)` in the generic case, but has an overload for when `T` matches `Quantity<U, R>`
that simply returns `x`.

We may someday be able to improve the ergonomics of `Quantity<Unos, T>` to the point that we'd feel
comfortable returning it, thus making the library more consistent.  However, returning a raw number
feels like the right compromise solution for us to start with.

!!! note
    For results that are dimensionless but _not_ "unitless", we **always** return a Quantity.

    For example, `milli(seconds)(50) * hertz(10)` produces a numeric value of `50 * 10 -> 500`, in
    a dimensionless unit whose magnitude is $1 / 1000$.  This is equivalent to a raw numeric value
    of $1 / 2$ --- but it's not the library's place to decide how or when to perform the lossy
    conversion of this integral Quantity.  Rather, the library's job is to safely hold the obtained
    numeric value of `500`.  The Magnitude attached to the Quantity is what lets us do so.

## Raw number conversions

Converting dimensionless quantities to raw numbers raises several interesting and subtle questions.
For dimensionless quantities that are also _unitless_, it's straightforward: implicit conversion is
always safe, and we always permit it.  However, dimensionless quantities with _non-trivial
magnitudes_ require more careful consideration.

### Implicit conversions {#implicit-conversions}

A common choice among units libraries is to support implicit conversions with dimensionless units.
This is intuitively appealing: after all, a Quantity like `percent(75.0)` represents the value
`0.75`.  Shouldn't we handle that conversion automatically, just as happily as we turn `feet(3)`
into `inches(36)`?

While the appeal is obvious, we believe this does more harm than good.  The reason is that
a Quantity has _two different_ notions of value, and for dimensionless units specifically, these
become ambiguous.  Consider something like `inches(24)`.  By "value", we might mean:

- the numeric variable `24`, stored safely within the Quantity object, as if in a container.
- the _quantity value itself_ --- in this case, the extent of the physical length, which is identical
  with `feet(2)`.

With dimensioned quantities, the library prevents confusion: we can't use either in contexts where
the other belongs.  But dimensionless quantities lack this safeguard.  This opens the door to
decisions which are individually reasonable, but which interact badly together.  For instance,
a `Quantity<Percent, T>` may be implicitly constructible _and_ convertible with `T`, but could pick
up stray factors of 100 in the round trip!

It is safer (and not _much_ less convenient!) to use separate, unambiguous idioms for these two
notions of "value".

### Explicit conversions

We have seen why _implicit_ conversions are problematic, but it's still important to support
_explicit_ conversions robustly.  For this, we have the
[`as_raw_number()`](../../reference/quantity.md#as-raw-number) utility.  The name disambiguates
between the two notions of "value", making it clear that we are considering the _quantity_
holistically (as with any other `as`-named interface).  So, something like
`as_raw_number(percent(75.0))` could only ever mean `0.75`.

Additionally, `as_raw_number` gets all the same safety checks as any other conversion function,
guarding against both truncation and overflow.  For example, `as_raw_number(percent(150))` would
fail to compile, because the true value of 1.5 cannot be represented in `int`.  We can even use the
same mechanisms to turn off [conversion risks](./conversion_risks.md): so,
`as_raw_number(percent(150), ignore(TRUNCATION_RISK))` _would_ compile, and produce `1`.

## Summary

Any time your computation produces a dimensionless result, consider keeping it as a `Quantity` type.
You get the benefit of having the compiler keep track of any scale factors, such as the
$\frac{1}{100}$ associated with `Percent`.  When you do want just a raw number, though, pass the
result to `as_raw_number`.  You'll make your intent clear, get all the usual safety checks, and even
get the ability to selectively override safety checks when you know that's the right move for your
use case.

[unos]: https://archive.iupap.org/commissions/interunion/iu14/ga-99.html
