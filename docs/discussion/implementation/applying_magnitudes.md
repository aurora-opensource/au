# Applying Magnitudes

Every unit conversion factor is a [_magnitude_](../../reference/magnitude.md) --- a positive real
number.  When we apply it to a value, conceptually, we're just multiplying the value by that number.
However, that doesn't mean that multiplying by a number is the best _implementation_!  Consider
these examples.

- **Factor: $\frac{5}{8}$; value: `12` (type: `int`).**
    - Computationally, we don't want to leave the integral domain.  But of course, $\frac{5}{8}$
      can't be represented as an integer!  This suggests we should perform _two_ operations: first
      multiply by `5`, and then divide by `8`, yielding `7`.
- **Factor: $\frac{1}{13}$; value: `91.0f` (type: `float`).**
    - Conceptually, this has an exactly representable answer: $91\left(\frac{1}{13}\right) = 7$.
      However, if we multiply by the single number `(1.0f / 13.0f)`, we obtain the approximation
      `7.0000004768371582`!  This suggests that for $\frac{1}{13}$, at least, it would be better to
      divide by `13.0f`.

Au is thoughtful about how we apply conversion factors.  We first compute a _category_ for the
factor, which dictates the best strategy for applying it.  We may also take into account whether
we're dealing with an integral or floating point type.

## Magnitude categories

We represent conversion factors with [magnitudes](../../reference/magnitude.md).  These
representations support _exact symbolic math_ for products and rational powers.  They also support
querying for _numeric properties_ of the number, such as whether it is an integer, whether it's
irrational, and so on.

For purposes of _applying to a value_, we find four useful categories of magnitude.

1. Integers.
2. Reciprocal integers.
3. Rational numbers (other than the first two categories).
4. Irrational numbers.

These categories are mutually exclusive and exhaustive.  Below, we'll explain the best strategy for
each one.

### Integers

Applying an integer magnitude to a type `T` is simple: we multiply by that integer's representation
in `T`.

This always compiles to a single instruction, and always produces exact answers whenever they are
representable in the type `T`.

### Reciprocal integers

If a magnitude is _not_ an integer, but its _reciprocal is_, then we divide by its reciprocal.  For
example, in converting a value from `inches` to `feet`, we will divide by $12$, instead of
multiplying by the representation of $\frac{1}{12}$, which would be inexact.

As with integers, this always compiles to a single instruction, and always produces exact answers
whenever they are representable in the type `T`.

### Rational numbers

Again, to be clear, this category only includes rationals that are _neither_ integers _nor_
reciprocal integers.  So, for example, neither $2$ nor $\frac{1}{5}$ falls in this category, but
$\frac{2}{5}$ does.

This category is interesting, because it's the first instance where our strategy _depends on the
type `T`_ to which we're applying the factor.  The best approach differs between integral and
non-integral types.

#### Integral types

Here, we multiply by the numerator, then divide by the denominator.  This compiles to _two_
operations instead of one, but it's the only way to get reasonable accuracy.

There's another issue: the multiplication operation can overflow.  This means we can produce wrong
answers in some instances, even when the correct answer is representable in the type!  For example,
let's say our value is `std::numeric_limits<uint64_t>::max()`, and we apply the magnitude
$\frac{2}{3}$: by the time we divide by $3$, the multiplication by $2$ has already lost our value to
overflow.

We might be tempted to prevent this by doing the division first.  In the above example, this would
certainly give us a much closer result!  However, the cost would be reduced accuracy for _smaller_
values, which are far more common.  Consider applying $\frac{2}{3}$ to a smaller number, such as
`5`.  The exact rational answer is $\frac{10}{3}$, which truncates to `3`.  If we perform the
multiplication first, this is what we get, but doing the division first would give `2`.

If you know that your final answer is representable, _and_ you have an integer type with more bits
than your type `T`, then you can work around this issue manually by casting to the wider type,
applying the magnitude, and casting back to `T`. However, if you _don't_ have a wider integer types,
we know of no _general_ "solution" that wouldn't do more harm then good.

#### Floating point types

Applying a rational magnitude $\frac{N}{D}$ to a value of floating point type `T` presents a genuine
tradeoff. On the one hand, we could take the same approach as for the integers, and perform two
operations: multiplying by $N$, then dividing by $D$.  On the other hand, we could simply multiply
by the single number which best represents $\frac{N}{D}$.  Here's a summary of the tradeoffs:

Criterion | Weighting | Multiply-and-divide: `(val * N) / D` | Single number: `val * (N / D)`
---|---|---|---
Instructions | medium | 2 | 1
Overflow | low | More vulnerable | Less vulnerable
Exact answers for multiples of $D$ | low | Guaranteed | Not guaranteed

Overall, we aren't worried much about missing out on exact answers.  Users of floating point know
they need to handle the possibility that a calculation's result can be one or two representable
values away from the best possible result.  (This is commonly called the "usual floating point
error".)

We also aren't very worried about overflow.  Even float has a range of $10^{38}$, while going from
femtometers to Astronomical Units (AU) spans a range of "only" about $10^{26}$.

Going from 1 instruction to 2 is a moderate concern, which means that it outweighs the other two
considerations.  It represents a runtime penalty relative to the usual approach people take without
a units library, which is to compute a single conversion factor.  We always strive to avoid runtime
penalties in units libraries! The reason we don't consider this even more serious is that unit
conversions should never occur in the "hot loop" for a program; thus, this performance hit isn't
really meaningful.

**Outcome:** we represent a rational conversion factor $\frac{N}{D}$ with a **single number** when
applying it to a floating point variable.

### Irrational numbers

There is no reason to try splitting an irrational number into parts to get an exact answer.  Since
we're multiplying our variable by an irrational number, we know the result won't be exactly
representable.  Therefore, we always simply multiply by the closest representation of this
conversion factor.

The one difference is that we forbid this operation for integral types, because it makes no sense.

## Summary and conclusion

Applying a conversion factor to a numeric variable of type `T` can be a tricky and subtle business.
Au takes a thoughtful, tailored approach, which can be summarized as follows:

- If the conversion factor multiplies --- _or divides_ --- by an exact integer, then we do that.
- Otherwise, if it's a rational number $\frac{N}{D}$, and `T` is integral, then we multiply by $N$
  and divide by $D$ (each represented in `T`).
- Otherwise, we simply multiply by the nearest representation of the conversion factor in `T` ---
  with the exception that if `T` is integral, we raise a compiler error for irrational factors.
