# Truncation

_Truncation_ occurs when the result of a unit conversion lands _in-between_ representable values in
the destination type.  Along with [overflow], it's one of the two main [conversion risks] in units
libraries.

## Au's approach

Au implements unit conversions as _sequences of operations_.  Some operations multiply or divide by
a particular number.  Others cast a value from one type to another.

Au checks every operation in the sequence to see whether any input values might cause truncation.
If we find _any_ risk, we forbid the conversion --- at least, by default.  There may be valid
reasons to perform such a conversion anyway.  If users are confident this is what they want, they
can pass `ignore(TRUNCATION_RISK)` as a second argument to any conversion operator.

Usually, even conversions with truncation risk will truncate for only some values, and not for
others.  Users can check _individual values_ at runtime to see if that particular value is safe.
`will_conversion_truncate(q, u)` examines converting the _specific_ `Quantity` value `q` to the new
unit `u`.  If the conversion being checked also changes the representation type (the "rep"), to
a new type `T`, `will_conversion_truncate<T>(q, u)` is the right form to use.

These tools complement each other.  A common pattern is to check an individual value at runtime, and
then call the actual conversion function with `ignore(TRUNCATION_RISK)` if the check passes.
Remember, the `TRUNCATION_RISK` flag refers to the risk of the _conversion as a whole_.  If you've
just proved that _your particular value_ is safe, then ignoring that _general_ risk is clearly fine.

## Truncation in mathematical operations

By "mathematical operations", we mainly mean multiplying or dividing by numerical constants.  To
understand the truncation risk, we first put the rep into one of several categories, because the
risk profile strongly depends on the category.

The main categories are the arithmetic[^1] types: integral, and floating point.  We'll also briefly
discuss how to think about other numeric types that we hope to support more fully in the future
([#52]).

[^1]: "Arithmetic" types are C++'s built-in numeric types: `int`, `double`, `uint64_t`, and so on.

### Integral types

Integral types are considered _exact_.  To see the implications for truncation, we need to consider
various [types of conversion factors](../implementation/applying_magnitudes.md).

First, we have multiplying by an integer.  This is easy: it stays within the domain of the integers,
so it can never truncate.

Next, we have multiplying by a non-integer rational (including reciprocal integers).  This will
truncate for any input that isn't an exact integer multiple of the denominator, so the _conversion
as a whole_ clearly has truncation risk.  When checking _individual values_, we can use the built-in
`%` operator.

Finally, we have multiplication by an irrational number.  For integral types, this truncates for
every nonzero input, because integers are exact types, and the only integer that produces an integer
when multiplied by an irrational is zero.

### Floating point types {#float}

Mathematical operations on floating point types are governed by a very simple philosophy: _floating
point never truncates_.

This may surprise the reader: after all, truncation means the result falls in-between representable
values, and floating point types certainly have gaps!  But consider the design philosophy of
floating point.  The goal is to _emulate_ a continuous real line.  Floating point values implicitly
come with a _relative tolerance_: one that is small, but _not_ zero.  Each representable value
effectively stands in for a small range of real values around it, and the exact size of that range
depends on the precise calculations.

Or, more simply: floating point types are considered _inexact_.

Now, we return to the problem of truncation in units libraries.  The very act of choosing a floating
point representation is a statement, by the user, that _exact values do not matter_ for their
application.  It would be inappropriate for us to raise warnings about perfectly routine properties
of the user's chosen type.  Hence: _floating point never truncates_.

### Other types

Currently, Au has only limited support for non-arithmetic rep types (full support is tracked in
[#52]). As a stopgap, Au treats any non-arithmetic type conservatively, and assumes that it can
truncate.  We hope to refine this approach when we strengthen our support for more rep types.

## Truncation in casting

Besides mathematical operations, the other main operation in unit conversions is casting from one
type to another.  We assess truncation risk for these operations as well.

### Arithmetic to arithmetic

Casting from one _arithmetic_ type to another is governed by simple rules.

If the source and destination types are in the same _category_ --- that is, either both integral, or
both floating point --- then the cast never truncates.  The reason for integral types is
straightforward: clearly, the source can't hold a non-integer value.  As for floating point types,
they are governed by the [philsophy explained above](#float).

Casting from a floating point type to an integral type does have truncation risk: we would truncate
for any non-integer input.  We can check this for individual values by discarding the fractional
part, and checking whether this changes the value.

Casting from an integral type to a floating point type is an interesting case.  As floating point
values get larger, they grow farther apart.  At some point, consecutive representable values can
differ by _multiple_ integers.  If we cast an input integer in this range, it may seem that we
should consider this to truncate.  But recall the [philosophy above](#float): floating point types
are all about _relative_ position.  For this reason, we consider casting from integral to floating
point as a **non-truncating** operation.

### Non-arithmetic types

Here, too, our support for non-arithmetic rep types is limited (see [#52]), and we take
a conservative approach.  Any cast involving a non-arithmetic type, either as source or destination,
is considered to have truncation risk, and will not be allowed by default: users must pass
`ignore(TRUNCATION_RISK)` as a second argument to override this.  We hope to have better default
behavior once we support non-arithmetic types more fully.

## Summary

Truncation happens when the result of an operation falls in-between representable values in the
numeric type where we're storing it.  If any input values for a unit conversion can truncate --- or,
if we _don't know_ whether any can --- then we forbid the conversion.  Users can override this
behavior by passing `ignore(TRUNCATION_RISK)` as a second argument.  Users can also check
whether _individual values_ truncate using the `will_conversion_truncate` function.

Truncation risk depends strongly on the types involved.  Integral types are vulnerable to truncation
for non-integer scale factors.  On the other hand, we treat floating point types as though they
never "truncate", because they're already inexact.  Finally, since we don't yet fully support
non-arithmetic types, we treat them conservatively and assume that they carry truncation risk.

[overflow]: ./overflow.md
[conversion risks]: ./conversion_risks.md
[#52]: https://github.com/aurora-opensource/au/issues/52
