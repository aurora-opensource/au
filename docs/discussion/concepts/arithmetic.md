# Arithmetic Operations

Quantity arithmetic operations fall into two categories.

- **Arbitrary-unit operations** work with any units---even if there are multiple input units with
  different dimensions!  Examples include products, quotients, powers, and roots.

- **Common-unit operations** require the inputs to be in the same unit.  (Note that this is only
  _possible_ if they have the same _dimension_.)  Examples include addition, subtraction, and
  relational operators.

We'll explain how the library handles each category below.  First, a brief refresher.

## Quantities, values, and units 101

A _quantity_ is an abstract concept: some physical property that can be measured.  To _work with_
that quantity, we need to pick a _unit of measure_, or "unit".  Once we've done so, the quantity
obtains a _value_ with respect to that unit.

Different choices of unit will lead to different values for the same quantity.  For example, suppose
we have a rod that is one foot long.  The distance between its endpoints is `feet(1.0)`, where
`feet` is the unit, and `1.0` is the value.  It's also equal to `inches(12.0)`, where `inches` is
another unit we could have chosen, and `12.0` is the value we'd obtain if we did.

Note that `feet(1.0)` and `inches(12.0)` are the _same quantity_, conceptually!  But when we pick
different units, we'll end up _storing different values_ (i.e., `1.0` vs. `12.0`) in the bits and
bytes of our computer program.

## Arbitrary-unit operations {#arbitrary-unit}

For arbitrary-unit operations (products, quotients, powers, and roots), there is a simple, golden
rule: apply the operation (say, multiplication) _independently_ to both the _value_ and the _unit_.
Consider this example:

$$(150 \,\,\text{mi}) \div (2 \,\,\text{h}) = (75 \,\,\text{mi} / \text{h})$$

Notice how the operation applies independently to the value and the unit:

- **values:** $150 \div 2 = 75$
- **units:** $\text{mi} \div \text{h} = \text{mi} / \text{h}$

Au works the same way.  Here's how that computation would look in our library:

```cpp
miles(150.0) / hours(2.0) == (miles / hour)(75.0)
```

Here, `(miles / hour)` creates a new, compound _unit_ on the fly, and calling it on `75.0` stores
that _value_ to create a _quantity_.

### Mixed Units and Conversions

Au makes it easy to do complicated, multi-step operations.  For example:

> How many "g's" of acceleration will a car experience, going from 0 to 60 MPH in 3 seconds?

Assuming constant acceleration, we have:

```cpp
QuantityD<StandardGravity> accel = (miles / hour)(60.0) / seconds(3.0);
```

It's worth breaking down the operations the library performs, and the order in which it performs
them.  In particular, remember that division happens _first_, before the assignment.

For the division, we know we can reason _independently_ about the value; so, the value is `20.0`.
Similarly, the unit is `miles / hour / second`.  Altogether, the division step yields
$20 \,\,\text{MPH} / \text{s}$.

Next comes assignment.  Since standard gravity ($g_0$) and $\text{MPH}/\text{s}$ both have the same
_dimension_ (namely, acceleration), then $\frac{g_0}{\text{MPH}/\text{s}}$ is some real number:
namely, $\frac{980665}{44704}$.  The library computes this number _at compile time_, and performs
only a single multiplication or division at runtime.  The final printed result is roughly
`0.911708 g_0`.

And this makes sense!  We know intuitively that going from 0 to 60 MPH in 3 seconds "feels very
fast".  We can see that in fact, it's pulling almost a full "g".

## Common unit operations {#common-unit}

If two quantities have the same dimension, it's _physically_ meaningful to add, subtract, or compare
them.  Consider two lengths: we can _physically_ add them by placing them in series and measuring
the total distance, even if we had measured one in "feet", and the other in "inches".

_Computationally_, it's a different story: we need to express them in a [**common
unit**](./common_unit.md).  Once we do, it's easy to add/subtract/compare the _quantities_: we
simply add/subtract/compare the _values_.

The "common unit" of two units is the _largest_ unit that _evenly divides both_[^1]---essentially,
the _greatest common divisor_.  This convention has two benefits:

1. Converting each input quantity to the common unit involves _multiplying the value by an integer_.
   This is really important for quantities with integral storage types!

2. The _value_ of that multiplier will be as _small as possible_ (relative to other units that
   evenly divide both input units).  This minimizes the risk of integer overflow.

Let's look at a few examples.

- `feet(1) + inches(6)`: This converts each input to their common unit, which is `inches`, resulting
  in `inches(18)`.

- `inches(100) > centi(meters)(200)`: The common unit doesn't have a name, so let's call it
  $\text{com}$.  An inch is $127 \,\text{com}$, and a centimeter is $50 \,\text{com}$.  After
  implicitly converting each argument, the input reduces to `coms(12700) > coms(10000)`, which
  clearly evaluates to `true`.

Check your understanding: what would we get if we _added_ these last values instead of _comparing_
them?

=== "Question"

    ```cpp
    inches(100) + centi(meters)(200) // -> ...?
    ```

=== "Answer and discussion"

    ```cpp
    inches(100) + centi(meters)(200) // -> coms(22700), using above notation
    ```

    We know that addition is a "common unit operation", which means we have to convert both inputs
    to the common unit before performing it.  As above, let's temporarily assume we've given a name,
    `coms`, to this anonymous unit.  We already saw that these inputs convert to `coms(12700)` and
    `coms(10000)`, respectively, so all we have to do is add them together to get `coms(22700)`.

    Of course, in real code, this unit wouldn't have a name.  We'd just have some `Quantity` of an
    anonymous length unit with this magnitude, storing the value `22700`.  So: what can we _do_ with
    this result?  Well, we can't assign it to an _integer_ Quantity of `inches` _or_
    `centi(meters)`, because it's not an integer in either of those units.  However, we _can_...

      - ...**compare** it to an integer Quantity of `inches` or `centi(meters)`
      - ...assign it to a **floating point** Quantity of `inches` or `centi(meters)`
      - ...assign it to an integer Quantity of some **smaller divisor unit**, such as
        `micro(meters)`

    So, for example, this would work:

    ```cpp
    QuantityI32<Micro<Meters>> x = inches(100) + centi(meters)(200)
    ```

    The value of `x` would be `micro(meters)(4'540'000)`.


[^1]: Of course, this definition only applies to units which _have_ a common divisor.  This happens
whenever their ratio (a dimensionless number) is an exact rational number.  This won't work for unit
pairs such as `degrees` and `radians`, whose ratio is the irrational number $(\pi / 180)$.  For
"irrational ratios", no convention can provide the benefits in this section, so we make an arbitrary
choice based on convenience of implementation.  For more details, see the [common unit
docs](./common_unit.md).
