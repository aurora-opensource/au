# Defining new dimensions

This page explains how to define a new base dimension.

Recall that [dimensions form a vector space](../discussion/implementation/vector_space.md).  In
plain terms, this means you don't need to add any new dimension that can be formed as a combination
of existing dimensions.  (For example, the "force" dimension can be composed of the base dimensions
"length", "time", and "mass", so you don't need to define it explicitly.)

Recall, too, that in Au, [dimensions are an implementation
detail](../reference/detail/dimension.md).  This means that although they are important for making
the library work, end users rarely name them directly in their code, and they rarely appear in
compiler errors.

Thus, defining a new base dimension is a much less common operation than [defining a new
unit](./new-units.md).  That said, it's still sometimes useful, and it is supported by the library,
so we'll explain how to do it.

## Definition features

Your definition will contain two "layers".

1. A new type representing the "base dimension".
2. A unit of this new dimension.

Your end users will typically only see the latter.  In Au, it's more idiomatic to work with _units_,
instead of naming dimensions explicitly.

To give a concrete example, we'll suppose we're defining a new base dimension for "pixels".

!!! note
    This doesn't imply anything about whether or not this would be a _good_ use case for a new base
    dimension; it's purely for illustration purposes.

### Base dimension type

In Au, a base dimension is a type with a static `int64_t` member, `base_dim_index`.  This index must
be unique among all other base dimensions in a program.  The easiest way to make this struct is to
inherit from `au::base_dim::BaseDimension<N>`, where `N` is the index.

To keep the index unique among all base dimensions in your program, we recommend using some integral
identifier that is unlikely to be duplicated.  This might be a GitHub issue number for your project,
or a current timestamp in seconds using the [Unix epoch](https://www.epoch101.com/).

Negative indices are reserved for the Au library.

Here's how we'd define the new base dimension for our example of "pixels".  (We've used the Unix
epoch approach to ensure uniqueness.)

```cpp
struct PixelBaseDim : au::base_dim::BaseDimension<1690384951> {};
```

### Unit of new dimension

The unit of your dimension is what you'll provide to end users.  They'll combine it with other units
via the usual operations.

The definition mostly follows the standard instructions for [defining new units](./new-units.md),
with one exception: instead of inheriting from a unit expression, you'll inherit from
`au::UnitImpl`.  This defines a new unit with the dimension that you pass it.

??? note "A note on the \"magnitude\" of this unit"
    The [magnitude](../reference/magnitude.md) of this unit will default to 1, because this is the
    simplest choice.  (Recall that [only magnitude _ratios_ are
    meaningful](../reference/unit.md#unit-ratio), and only for the same dimension.  Since there are
    no other units of this new dimension, we can assign whatever value we please.)

Here is how to create that new unit.  We'll show a relatively full-featured definition, including
unit label, quantity maker, and singular name.

=== "C++14"

    ```cpp
    // In .hh file:
    struct Pixels : au::UnitImpl<au::Dimension<PixelBaseDim>> {
        static constexpr const char label[] = "px";
    };
    constexpr auto pixel  = SingularNameFor<Pixels>{};
    constexpr auto pixels = QuantityMaker<Pixels>{};

    // In .cc file:
    constexpr const char Pixels::label[];
    ```

=== "C++17 or later"

    ```cpp
    // In .hh file:
    struct Pixels : au::UnitImpl<au::Dimension<PixelBaseDim>> {
        static constexpr inline const char label[] = "px";
    };
    constexpr auto pixel  = SingularNameFor<Pixels>{};
    constexpr auto pixels = QuantityMaker<Pixels>{};
    ```

## Usage example

This new unit will compose with other units in all of the usual ways.

Here's an example test case:

```cpp
constexpr auto resolution = (pixels / inch)(300);
EXPECT_THAT(resolution * inches(6), SameTypeAndValue(pixels(1800)));
```

If we followed the instructions in the previous section, this test should pass --- and you can use
the `pixels` unit in your project on the same footing as any other unit.
