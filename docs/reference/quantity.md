# Quantity

`Quantity` is our workhorse type for safely storing quantities of a given unit in some underlying
numeric variable.

!!! warning "TODO: this page is a stub"

We will provide a full-fledged reference for quantities later.  For now, here are the basics.

1. `Quantity<U, R>` stores a value of numeric type `R` (called the "Rep"), whose units are the [unit
   type](./unit.md) `U`.

    - **Example:** `Quantity<Meters, double>` stores a length in `Meters`, in a `double` variable.

2. We provide "Rep-named aliases" for better ergonomics.

    - **Example:** `QuantityD<Meters>` is an alias for `Quantity<Meters, double>`.

3. You cannot get values into or out of `Quantity` without _explicitly naming the unit, at the
   callsite_.  For full details, see our tutorials, starting with the first: [Au 101: Quantity
   Makers](../tutorial/101-quantity-makers.md).

??? tip "Handling temperatures"
    If you are working with temperatures --- in the sense of "what temperature is it?", rather than
    "how much did the temperature change?" --- you will want to use
    [`QuantityPoint`](./quantity_point.md) instead of `Quantity`.
