# Unit slots

Many APIs in the library have a parameter for you to name the desired units.  We call these
parameters "unit slots".

Let's look at some common examples.  In what follows, assume `q` is some `Quantity` type, and `T` is
some [arithmetic](https://en.cppreference.com/w/cpp/types/is_arithmetic) type.  Then `u` will be
what goes in the unit slot:

- `q.as(u)`
- `round_as(u, q)`
- `inverse_as(u, q)`
- "Explicit-Rep" versions of the above (such as `q.as<T>(u)`)
- "`in`" versions of the above, which exit the library (`q.in(u)`, `round_in(u, q)`, etc.)

This page will explain what kinds of things "fit" in the slot, and which styles to prefer in
different situations.

## What "fits" in a unit slot?

The golden rule --- the reason these slots exist --- is that you should be naming your units at the
callsite, concisely but explicitly.  This makes the code easier to read and understand at a glance.

It turns out that there are multiple styles for explicitly naming units.  For example, if our target
unit is `Meters`, then we could either pass `Meters{}` or `meters` in the unit slot, and get the
same result.  While they both fulfill the main goal --- namely, they name the unit explicitly ---
they are two different kinds of objects.

- `Meters{}` is a _unit expression_.
- `meters` is a _quantity maker expression_.

Let's explore these concepts in more detail.

### Unit expression {#unit-expression}

A unit expression is simply an instance of a [unit type](../../reference/unit.md).  This could be
something as simple as `Meters{}`, an instance of the unit type `Meters`.

It could also be the result of _combining several_ such instances, via arithmetic.  For example,
`Meters{} / squared(Seconds{})` is also a unit expression.  It's an instance of the type
`UnitQuotientT<Meters, UnitPowerT<Seconds, 2>>`.  However, the unit expression is much easier to
write and to read than the `UnitQuotientT<...>` version!  That's why we recommend using them to
[create new units](../../howto/new-units.md).

### Quantity maker expression

A quantity maker expression is similar to the unit expression defined just above, except that it
combines instances of `QuantityMaker` (such as `meters`) and, optionally, `SingularNameFor` (such as
`meter`).  The result of this expression will be an instance of a `QuantityMaker`.

Quantity maker expressions support all of the same arithmetic operations as unit expressions, but
they have two advantages that make them easier to read:

1. You don't need to add the `{}`, since the participating elements are _already_ instances rather
   than types.

2. You can use grammatically correct names, such as `meters / squared(second)` (note: `second` is
   singular), rather than `Meters{} / squared(Seconds{})`.

## Examples: rounding to RPM

Let's look at some examples, using this quantity variable:

```cpp
constexpr auto angular_velocity = (radians / second)(10.0);
```

Our goal will be to round it to the nearest value in revolutions per minute (RPM).  Let's look at
our options for doing that.

### Best choice: quantity maker expression

`QuantityMaker` instances have the _name of the unit_, so they meet our core criterion (which is,
again, to name the units explicitly at the callsite).  They also compose just as naturally as unit
instances.  In fact, they also work nicely with `SingularNameFor` instances --- such as `minute`,
for the unit `Minutes` --- to further enhance readability.

Here's how it looks to pass a `QuantityMaker` --- fluently composed, on the fly --- to this unit
expression API.

```cpp
constexpr auto rpm = round_as(revolutions / minute, angular_velocity);
//                            ^^^^^^^^^^^^^^^^^^^^
//                          Quantity maker expression
```

### Also acceptable: unit expression

We could, of course, also pass a unit expression to the unit expression slot.

```cpp
// Usual unit-expression approach (doing arithmetic on *instances*):
constexpr auto rpm = round_as(Revolutions{} / Minutes{}, angular_velocity);
//                            ^^^^^^^^^^^^^^^^^^^^^^^^^
//                                 unit expression

// Alternative, clunkier unit-expression approach (doing arithmetic on *types*):
constexpr auto rpm = round_as(
   UnitQuotientT<Revolutions, Minutes>{}, angular_velocity);
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//            unit expression
```

These are OK, but they have a couple of drawbacks compared to the quantity maker approach.

1. They're grammatically incorrect: "revolutions per _minutes_", instead of "revolutions per
   _minute_".

2. You need to sprinkle extra `{}` throughout to turn the unit types into instances.

The main reason to use this method is for _generic code_.  In these cases, you don't usually _have_
a quantity maker handy, but you _do_ know the unit type.

### Poor choice: manually constructed `QuantityMaker`

This may look counterintuitive, but we mention it because we've seen a few instances of this
approach in the wild!  Here is an example:

```cpp
//
// !!   Do not do this!   !!
//
constexpr auto rpm = round_as(
    QuantityMaker<UnitQuotientT<Revolutions, Minutes>>{}, angular_velocity);
//  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//           manual QuantityMaker instance
```

Presumably, this mistake comes from reading the signatures in the source code
without understanding their core design goal: namely, to provide a place to specify the units,
concisely and explicitly, at a callsite.

This provides no benefit at all.  We could replace the `QuantityMaker<UnitQuotientT<Revolutions,
Minutes>>` with its contents (`UnitQuotientT<Revolutions, Minutes>`) and it would be strictly
better.

The reason we endorse the `QuantityMaker` overloads is because of the convention to provide "canned"
`QuantityMaker` instances which are named after their corresponding units.  If you have to construct
a new `QuantityMaker` on the fly, then this benefit vanishes.  (This is why unit expressions are
preferred for generic code.)

## Summary

Many Au APIs have a "unit slot".  These are designed for you to name the units explicitly at the
callsite.  Pass whatever is the simplest construct that meets that goal --- usually, this will be
a pre-existing `QuantityMaker`, or several of them composed together.
