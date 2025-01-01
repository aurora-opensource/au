# Constant

`Constant` is a family of template types, each of which represents a single specific quantity value.

Recall that the usual way to represent quantity values is with a [`Quantity<Unit, Rep>`](./quantity.md)
type. This holds a numeric variable of type `Rep` inside, letting it represent
many possible quantity values. By contrast, `Constant` is an empty class and has no "rep": the
single value it can represent is fully encoded in its type.  This makes it an example of
a [monovalue type](./detail/monovalue_types.md).

Because the value is always fully known at compile time, we do not need to use a heuristic like the
[overflow safety surface](../discussion/concepts/overflow.md) to determine which conversions are
allowed.  Instead, we can achieve a perfect conversion policy: we allow converting to any `Quantity`
that can represent the value exactly, and disallow all other conversions.

The main use of `Constant` is to multiply and divide raw numbers or `Quantity` values.  When we do
this, the constant is applied _symbolically_, and affects the _units_ of the resulting quantity.
For example, multiplying a duration in seconds by a constant representing the speed of light
produces a _length_, measured in units of _light-seconds_.  Notably, _the underlying stored numeric
value does not change_: whether a duration of `5` seconds, or a length of `5` light-seconds, we
still store `5` under the hood.

This approach means that if subsequent operations cancel out the constant, this cancellation is both
_exact_ and has _zero runtime cost_.

## Built-in constants included with Au {#built-in}

Au includes a number of built-in constants.  Each constant is in its own include file, in the folder
`"au/constant"` --- for example, `"au/constant/speed_of_light.hh"`.

The constant object itself is in the top-level `au::` namespace, and uses an UPPERCASE naming
convention, as with all other constants in the library --- for example, `au::SPEED_OF_LIGHT`.

We spell out the constant name in full to avoid ambiguity.  However, this can be overly verbose.  We
encourage users to define their own copy of each constant, with a more usable name --- for example:

```cpp
constexpr auto c = au::SPEED_OF_LIGHT;
```

This "copy" is essentially free, because the constant is a [monovalue
type](./detail/monovalue_types.md), and therefore empty.

Here are the constants that we include with Au:

| Name | Symbol | Value | Include (under `"au/constants/"`) | Object name (under `au::` namespace) |
| ---- | ------ | ----- | ------- | ----------- |
| Avogadro constant | $N_A$ | $6.022\,140\,76 \times 10^{23}\,\, \text{mol}^{-1}$ | `avogadro_constant.hh` | `AVOGADRO_CONSTANT` |
| Boltzmann constant | $k_B$ | $1.380\,649 \times 10^{-23}\,\, \text{J} / \text{K}$ | `boltzmann_constant.hh` | `BOLTZMANN_CONSTANT` |
| Cesium hyperfine transition frequency | $\Delta \nu_{Cs}$ | $9\,192\,631\,770\,\, \text{Hz}$ | `cesium_hyperfine_transition_frequency.hh` | `CESIUM_HYPERFINE_TRANSITION_FREQUENCY` |
| Elementary charge | $e$ | $1.602\,176\,634 \times 10^{-19}\,\, \text{C}$ | `elementary_charge.hh` | `ELEMENTARY_CHARGE` |
| Luminous efficacy of light at $540\,\, \text{THz}$ | $K_{cd}$ | $683\,\, \text{lm} / \text{W}$ | `luminous_efficacy_540_terahertz.hh` | `LUMINOUS_EFFICACY_540_TERAHERTZ` |
| Planck constant | $h$ | $6.626\,070\,15 \times 10^{-34}\,\, \text{J} \cdot \text{s}$ | `planck_constant.hh` | `PLANCK_CONSTANT` |
| Reduced Planck constant | $\hbar$ | $1.054\,571\,817 \times 10^{-34}\,\, \text{J} \cdot \text{s}$ | `reduced_planck_constant.hh` | `REDUCED_PLANCK_CONSTANT` |
| Speed of light | $c$ | $299\,792\,458\,\, \text{m} / \text{s}$ | `speed_of_light.hh` | `SPEED_OF_LIGHT` |
| Standard Gravity | $g_0$ | $9.806\,65\,\, \text{m} / \text{s}^2$ | `standard_gravity.hh` | `STANDARD_GRAVITY` |

Our policy is to include only exactly defined constants with the library.  This rules out many
useful constants, such as the universal gravitational constant $G$, the _new_ (post-2019) permeability
of free space $\mu_0$, and so on.  For these, we can't reasonably provide values that will satisfy
all users at all times.  However, defining custom constants for your own project is straightforward,
as we explain in the next section, and in our [how-to guide for custom
constants](../howto/new-constants.md).


## Constructing `Constant`

`Constant` encodes all information about the value in its type.  Moreover, it has only a single
template parameter, which is a [unit](./unit.md).  Therefore, the first step is to encode your
quantity as a unit --- that is, to define the unit "U" such that your quantity has a value of
"1 U".

To do this, follow [the usual instructions for creating new units](../howto/new-units.md).  Note
that you can use a much simpler definition that omits most of the optional features.  The only
important ones are those labeled `[1]` (the strong type definition) and `[2]` (the unit label).

Having defined your unit, you can pass an instance to the `make_constant` function.  If the unit you
defined above is called `YourUnits`, and the constant is called `YOUR_CONSTANT`, then the constant
definition will look like this:

```cpp
constexpr auto YOUR_CONSTANT = make_constant(YourUnits{});
```

Finally, note that the argument to `make_constant()` is a [unit
slot](../discussion/idioms/unit-slots.md), so you can pass "unit-like" alternatives such as
`QuantityMaker` or `SymbolFor` instances as well.

??? example "Full worked example: speed of light"
    Let's look at an example of defining a constant for the speed of light.  Both the name of the
    instance and the label will be `c`.

    === "C++14"

        ```cpp
        // In .hh file:
        struct SpeedOfLight : decltype(Meters{} / Seconds{} * mag<299'792'458>()) {
            static constexpr const char label[] = "c";
        };

        constexpr auto c = make_constant(SpeedOfLight{});
        ```

        ```cpp
        // In .cc file:
        constexpr const char SpeedOfLight::label[];
        ```

    === "C++17 or later"

        ```cpp
        // In .hh file:
        struct SpeedOfLight : decltype(Meters{} / Seconds{} * mag<299'792'458>()) {
            static constexpr inline const char label[] = "c";
        };

        constexpr auto c = make_constant(SpeedOfLight{});
        ```

### Ad hoc constants

You can obtain many of the benefits of `Constant` even if you don't formally define a new unit.
Because `make_constant` has a unit slot API, you can pass an ad hoc expression to it.  For example:

```cpp
constexpr auto c = make_constant(meters / second * mag<299'792'458>());
```

The main advantage of doing this is its conciseness: the constant definition is a single, readable
line.  The built constant also has all of the multiplication and division operators types that
`Constant` supports, as well as its perfect conversion policy to any `Quantity` type.

The only disadvantage is that an ad hoc `Constant` gets an ad hoc unit symbol, instead of a simple,
concise symbol.  The example below demonstrates the difference.

??? example "Example: ad hoc constants get ad hoc labels"
    Let's take the following speed, and express it in terms of the speed of light, using a constant:

    ```cpp
    constexpr auto v = (miles / hour)(65.0);
    ```

    First, an ad hoc constant:

    ```cpp
    constexpr auto c = make_constant(meters / second * mag<299'792'458>());

    std::cout << v.as(c) << std::endl;
    // Output:
    // "9.69257e-08 [299792458 m / s]"
    ```

    Note how the program is correct, and the label is accurate, but clunky: it prints
    `[299792458 m / s]` instead of a concise label such as `c`.

    By contrast, we can use a fully defined constant.  This is generally a little more effort, but
    some constants, such as `SPEED_OF_LIGHT`, are included out of the box:

    ```cpp
    // Found in `"au/constants/speed_of_light.hh"`:
    std::cout << v.as(SPEED_OF_LIGHT) << std::endl;
    // Output:
    // "9.69257e-08 c"
    ```

    There's no difference in the program that gets executed, but the printed label is a lot easier
    to understand.

## `Constant` and unit slots

`Constant` can be passed to any API that takes a [unit slot](../discussion/idioms/unit-slots.md).

## Converting to `Quantity`

`Constant` can be converted to any `Quantity` type of the same dimension.

By default, this conversion policy is _perfect_.  This means that it permits converting to any
`Quantity` that can represent the value exactly, and disallows all other conversions.  Users can
also override this policy by choosing the "coerce" variant of any API (say, using `.coerce_as()`
instead of `.as()`).

Finally, it's important to appreciate that `Constant` has no rep, no underlying numeric type.
Therefore, every `Quantity` conversion API requires an explicit template parameter to specify the
desired rep.

### `.as<T>()`

This function expresses the constant as a `Quantity` in "units of this constant".  Therefore, the
underlying stored value will be `T{1}`, and the rep will be `T`.

### `.as<T>(unit)` {#as-T-unit}

This function expresses the constant as a `Quantity` in the requested unit, using a rep of `T`.  It
has a perfect conversion policy, which means that it compiles if and only if the constant's value in
the requested unit can be exactly represented in the type `T`.

The argument `unit` is a [unit slot](../discussion/idioms/unit-slots.md) API, so it accepts a unit
instance, quantity maker instance, or any other instance compatible with a unit slot.

### `.coerce_as<T>(unit)`

This function expresses the constant as a `Quantity` in the requested unit, using a rep of `T`.  It
is similar to [`.as<T>(unit)`](#as-T-unit), except that it will ignore the safety checks that
prevent truncation and overflow.

!!! warning
    Because `.as<T>(unit)` has a perfect conversion policy, we know that this function either
    produces the exact same result (in which case you could simply _call_ `.as<T>(unit)`), _or_ it
    produces a result which is **guaranteed to be lossy**.  Therefore, be very judicious in using
    this function.

### `.in<T>(unit)` {#in-T-unit}

This function produces a raw numeric value, of type `T`, holding the value of the constant in the
requested unit.  It has a perfect conversion policy, which means that it compiles if and only if the
constant's value in the requested unit can be exactly represented in the type `T`.

The argument `unit` is a [unit slot](../discussion/idioms/unit-slots.md) API, so it accepts a unit
instance, quantity maker instance, or any other instance compatible with a unit slot.

### `.coerce_in<T>(unit)`

This function produces a raw numeric value, of type `T`, holding the value of the constant in the
requested unit.  It is similar to [`.in<T>(unit)`](#in-T-unit), except that it will ignore the
safety checks that prevent truncation and overflow.

!!! warning
    Because `.in<T>(unit)` has a perfect conversion policy, we know that this function either
    produces the exact same result (in which case you could simply _call_ `.in<T>(unit)`), _or_ it
    produces a result which is **guaranteed to be lossy**.  Therefore, be very judicious in using
    this function.

### Implicit `Quantity` conversion

`Constant` will implicitly convert to any `Quantity` type which passes the safety checks on
truncation and overflow.  Essentially: any time [`.as<T>(unit)`](#as-T-unit) produces a result, that
same result can be obtained via implicit conversion.

This provides great flexibility and confidence in passing `Constant` values to APIs that take
`Quantity`.

!!! note
    The fact that `Constant` has a perfect conversion policy means that we can use it with APIs
    where the corresponding `Quantity` would not work, because `Quantity` is forced to use the
    [overflow safety surface](../discussion/concepts/overflow.md), which is a more conservative
    heuristic.

    For example, suppose you have an API accepting `Quantity<UnitQuotientT<Meters, Seconds>, int>`,
    and a constant `c` representing the speed of light.

    You will be able to pass `c` to this API, because the constant-to-quantity conversion operation
    knows the exact value at compile time, and can verify that it fits in an `int`.

    By contrast, you would not be able to pass `c.as<int>()` (which is a `Quantity`).  Even though
    it would work for _this specific value_ (which is `1`), this quantity-to-quantity conversion is
    too dangerous for `int` in general.

## Operations

Each operation with a `Constant` consists in multiplying or dividing with some other family of
types.

### Raw numeric type `T`

Multiplying or dividing `Constant<Unit>` with a raw numeric type `T` produces a `Quantity` whose rep
is `T`, and whose unit is derived from `Unit`.

In the following table, we will use `x` to represent the value that was stored in the input of type
`T`.

| Operation | Resulting Type | Underlying Value | Notes |
| --------- | -------------- | ---------------- | ----- |
| `Constant<Unit> * T` | `Quantity<Unit, T>` | `x` | |
| `Constant<Unit> / T` | `Quantity<Unit, T>` | `T{1} / x` | Disallowed for integral `T` |
| `T * Constant<Unit>` | `Quantity<Unit, T>` | `x` | |
| `T / Constant<Unit>` | `Quantity<UnitInverseT<Unit>, T>` | `x` | |

### `Quantity<U, R>`

Multiplying or dividing `Constant<Unit>` with a `Quantity<U, R>` produces a `Quantity` whose rep is
`R`, and whose unit is derived from `Unit` and `U`.

In the following table, we will use `x` to represent the underlying value in the input quantity ---
that is, if the input quantity was `q`, then `x` is `q.in(U{})`.

| Operation | Resulting Type | Underlying Value | Notes |
| --------- | -------------- | ---------------- | ----- |
| `Constant<Unit> * Quantity<U, R>` | `Quantity<UnitProductT<Unit, U>, R>` | `x` | |
| `Constant<Unit> / Quantity<U, R>` | `Quantity<UnitQuotientT<Unit, U>, R>` | `R{1} / x` | Disallowed for integral `R` |
| `Quantity<U, R> * Constant<Unit>` | `Quantity<UnitProductT<U, Unit>, R>` | `x` | |
| `Quantity<U, R> / Constant<Unit>` | `Quantity<UnitQuotientT<U, Unit>, R>` | `x` | |

### `Constant<U>`

Constants compose: the product or quotient of two `Constant` instances is a new `Constant` instance.

| Operation | Resulting Type |
| --------- | -------------- |
| `Constant<Unit> * Constant<U>` | `Constant<UnitProductT<Unit, U>>` |
| `Constant<Unit> / Constant<U>` | `Constant<UnitQuotientT<Unit, U>>` |

### `QuantityMaker<U>`

Multiplying or dividing `Constant<Unit>` with a `QuantityMaker<U>` produces a new `QuantityMaker`
whose unit is derived from `Unit` and `U`.

| Operation | Resulting Type |
| --------- | -------------- |
| `Constant<Unit> * QuantityMaker<U>` | `QuantityMaker<UnitProductT<Unit, U>>` |
| `Constant<Unit> / QuantityMaker<U>` | `QuantityMaker<UnitQuotientT<Unit, U>>` |
| `QuantityMaker<U> * Constant<Unit>` | `QuantityMaker<UnitProductT<U, Unit>>` |
| `QuantityMaker<U> / Constant<Unit>` | `QuantityMaker<UnitQuotientT<U, Unit>>` |

### `SingularNameFor<U>`

Multiplying or dividing `Constant<Unit>` with a `SingularNameFor<U>` produces a new
`SingularNameFor` whose unit is derived from `Unit` and `U`.

| Operation | Resulting Type |
| --------- | -------------- |
| `Constant<Unit> * SingularNameFor<U>` | `SingularNameFor<UnitProductT<Unit, U>>` |
| `Constant<Unit> / SingularNameFor<U>` | `SingularNameFor<UnitQuotientT<Unit, U>>` |
| `SingularNameFor<U> * Constant<Unit>` | `SingularNameFor<UnitProductT<U, Unit>>` |
| `SingularNameFor<U> / Constant<Unit>` | `SingularNameFor<UnitQuotientT<U, Unit>>` |

### `Magnitude<BPs...>`

Multiplying or dividing `Constant<Unit>` with a `Magnitude` produces a new `Constant` which is
scaled by that magnitude.

In the following table, let `m` be an instance of `Magnitude<BPs...>`.

| Operation | Resulting Type |
| --------- | -------------- |
| `Constant<Unit> * Magnitude<BPs...>` | `Constant<decltype(Unit{} * m)>` |
| `Constant<Unit> / Magnitude<BPs...>` | `Constant<decltype(Unit{} / m)>` |
| `Magnitude<BPs...> * Constant<Unit>` | `Constant<decltype(Unit{} * m)>` |
| `Magnitude<BPs...> / Constant<Unit>` | `Constant<decltype(UnitInverseT<Unit>{} * m)>` |

### `QuantityPointMaker<U>` (deleted)

Multiplying or dividing `Constant<Unit>` with a `QuantityPointMaker<U>` is explicitly deleted,
because quantity points do not support multiplication.

### `QuantityPoint<U, R>` (deleted)

Multiplying or dividing `Constant<Unit>` with a `QuantityPoint<U, R>` is explicitly deleted,
because quantity points do not support multiplication.
