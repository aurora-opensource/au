# Unit expression APIs

Many APIs in the library accept a _Unit expression_, `u`.  Common examples include (for some
Quantity `q` and [arithmetic](https://en.cppreference.com/w/cpp/types/is_arithmetic) type `T`):

- `q.as(u)`
- `round_as(u, q)`
- `inverse_as(u, q)`
- "Explicit-Rep" versions of the above (such as `q.as<T>(u)`)
- "`in`" versions of the above, which exit the library (`q.in(u)`, `round_in(u, q)`, etc.)

## What is a unit expression?

A unit expression is simply an instance of a [unit type](../../reference/unit.md).  This could be
something as simple as `Meters{}`, an instance of the unit type `Meters`.

It could also be the result of _combining several_ such instances, via arithmetic.  For example,
`Meters{} / squared(Seconds{})` is also a unit expression.  It's an instance of the type
`UnitQuotientT<Meters, UnitPowerT<Seconds, 2>>`.  However, the unit expression is much easier to
write and to read than the `UnitQuotientT<...>` version!  That's why we recommend using them to
[create new units](../../howto/new-units.md).

## What "fits" in a unit expression "slot"?

The golden rule --- the reason these slots exist --- is that you should be naming your units at the
callsite, concisely and readably.

The Unit expression is _not the only thing_ that meets this description.  In fact, it's _usually_
not even the _best_ choice!  Anywhere you can pass an instance of a Unit `U`, you're usually better
off passing a `QuantityMaker<U>`, because it reads better.

## Examples: rounding to RPM

Let's look at some examples, using this quantity variable:

```cpp
constexpr auto angular_velocity = (radians / second)(10.0);
```

Our goal will be to round it to the nearest value in revolutions per minute (RPM).  Let's look at
our options for doing that.

### Best choice: `QuantityMaker`

`QuantityMaker` instances have the _name of the unit_, so they meet our core criterion (which is,
again, to name the units explicitly at the callsite).  They also compose just as naturally as unit
instances.  In fact, they also work nicely with `SingularNameFor` instances --- such as `minute`,
for the unit `Minutes` --- to further enhance readability.

Here's how it looks to pass a `QuantityMaker` --- fluently composed, on the fly --- to this unit
expression API.

```cpp
constexpr auto rpm = round_as(revolutions / minute, angular_velocity);
// QuantityMaker -------------^^^^^^^^^^^^^^^^^^^^
```

### Also acceptable: unit expression

We could, of course, also pass a unit expression to the unit expression slot.

```cpp
// Usual unit-expression approach (doing arithmetic on *instances*):
constexpr auto rpm = round_as(Revolutions{} / Minutes{}, angular_velocity);
// Unit expression -----------^^^^^^^^^^^^^^^^^^^^^^^^^

// Alternative, clunkier unit-expression approach (doing arithmetic on *types*):
constexpr auto rpm = round_as(UnitQuotientT<Revolutions, Minutes>{}, angular_velocity);
// Unit expression -----------^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
```

These are OK, but they have a couple of drawbacks compared to the quantity maker approach.

1. They're grammatically incorrect: "revolutions per _minutes_", instead of "revolutions per
   _minute_".

2. You need to sprinkle extra `{}` throughout to turn the unit types into instances.

The main reason to use this method is for _generic code_.  In these cases, you don't usually _have_
a quantity maker handy, but you _do_ know the unit type.

### Poor choice: manually constructed `QuantityMaker`

This may look counter-intuitive, but we have seen a few instances of this approach in the wild!
Presumably, this mistake comes from reading the signatures in the source code without understanding
their core design goal: namely, to provide a place to specify the units, concisely and explicitly,
at a callsite.  Here is an example:

```cpp
//
// !!   Do not do this!   !!
//
constexpr auto rpm = round_as(
    QuantityMaker<UnitQuotientT<Revolutions, Minutes>>{}, angular_velocity);
//  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^--- manual QuantityMaker instance
```

This provides no benefit at all.  We could replace the `QuantityMaker<...>` with its contents
(`...`) and it would be strictly better.  The reason we endorse the `QuantityMaker` overloads is
because of the convention to provide "canned" `QuantityMaker` instances which are named after their
corresponding units.

## Summary

Many Au APIs have a "unit expression" slot.  These are designed for you to name the units explicitly
at the callsite.  Pass whatever is the simplest construct that meets that goal --- usually, this
will be a `QuantityMaker`, or several of them composed together.
