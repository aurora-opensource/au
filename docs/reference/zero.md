# Zero

`Zero` is a type that represents a quantity of "0" in any units.  See [the `Zero` discussion
doc](../discussion/concepts/zero.md) for more background motivation and use cases.

`Zero` is a [monovalue type](./detail/monovalue_types.md).  Every instance of `Zero` is equivalent
to every other instance.  In practice, it's easiest and most idiomatic to use the predefined
instance, `ZERO`.

## Conversions

The goal is for `ZERO` to be implicitly convertible to any type for which the value of "0" is
completely unambiguous.  For types in the language or the standard library, which cannot possibly
know about `Zero`, we do this via implicit conversions.  For other types, we do this by adding
a constructor which takes `Zero`.

### Implicit conversions

We provide implicit conversions to the following categories of types:

- Any arithmetic type (`int`, `double`, `std::size_t`, ...).

- Any `std::chrono::duration` type.

### `Quantity` constructor

`Quantity` is implicitly constructible from `Zero`.

This means you can efficiently check the sign of any `Quantity` by comparing to `ZERO`.

### `QuantityPoint` constructor

This is explicitly deleted.  There is no unambiguous notion of which point is labeled as "0"; it
depends on the choice of units.  Therefore, we delete this constructor to prevent users from relying
on this dubious notion.

## I/O

If you include I/O support, then `Zero` will be streamed as `"0"`.
