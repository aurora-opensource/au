# Prefix

A "prefix" scales a [unit](./unit.md) by some [magnitude](./magnitude.md), and prepends a _prefix
symbol_ to the unit's label.

!!! warning "TODO: this page is a stub"

We will provide a full-fledged reference for prefixes later.  For now, here are the basics:

1. We support every [SI prefix](https://www.nist.gov/pml/owm/metric-si-prefixes) and [binary
   prefix](https://en.wikipedia.org/wiki/Binary_prefix).

2. To apply a prefix to a [_unit type_](./unit.md), spell the prefix using `CamelCase` (just like
   any other type in Au), and pass the unit type as a _template parameter_.

    - **Example:** `Meters` is a unit type; so is `Centi<Meters>`.  You can form
      a `QuantityD<Centi<Meters>>`.

3. To apply a prefix to a [_quantity maker_](../tutorial/101-quantity-makers.md), spell the prefix
   using `snake_case` (just like the quantity maker itself), and pass the quantity maker as
   a _function parameter_.

    - **Example:** `meters` is a quantity maker; so is `centi(meters)`.  If you call
      `centi(meters)(2.54)`, it will create a quantity of centimeters.
