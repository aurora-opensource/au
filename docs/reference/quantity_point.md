# QuantityPoint

`QuantityPoint` is our [affine space type](http://videocortex.io/2018/Affine-Space-Types/).  The
core use cases include:

- **Temperatures.**  Specifically, when you want to represent "what temperature is it?" rather than
  "how much did the temperature change?"

- **Mile markers.**  That is to say, points along a linear path which are indexed by distances.  It
  wouldn't make sense to _add_ two mile markers, but you can _subtract_ them and obtain a _distance_
  (which is a [`Quantity`](./quantity.md), not a `QuantityPoint`).

!!! warning "TODO: this page is a stub"

We will provide a full-fledged reference for quantity points later.  For now, here are the basics.

1. `QuantityPoint<U, R>` stores a value of numeric type `R` (called the "Rep"), whose units are the
   [unit type](./unit.md) `U`.

    - **Example:** `QuantityPoint<Meters, double>` represents an along-path point, whose distance is
      measured in `Meters`, in a `double` variable.

2. We provide "Rep-named aliases" for better ergonomics.

    - **Example:** `QuantityPointD<Meters>` is an alias for `QuantityPoint<Meters, double>`.

3. You cannot get values into or out of `QuantityPoint` without _explicitly naming the unit, at the
   callsite_.  We do not have tutorials for this yet, but it works the same as `Quantity`.  In the
   meantime, you can read [the `QuantityPoint` unit
   tests](https://github.com/aurora-opensource/au/blob/main/au/quantity_point_test.cc) to see some
   practical examples.
