# Corresponding Quantity

Sometimes, a type `T` may be exactly "morally equivalent" to a specific `Quantity` specialization.
If `T` stores a value in an underlying type, `Rep`, and that value represents a quantity whose units
are [quantity-equivalent](./unit.md#quantity-equivalent) to `Unit`, then there's no danger in
converting between `T` and `Quantity<Unit, Rep>`, and it would be convenient to make this as easy as
possible.

The `CorrespondingQuantity<T>` type trait makes this possible.  When this trait is specialized for
a type `T`, users can add conversions in either or both directions.  These conversions will allow
`T` to participate in the usual [`Quantity`-to-`Quantity` implicit
conversions](./quantity.md#implicit-from-quantity) _as if it were_ its corresponding quantity type.

## How to specialize

A valid specialization of `CorrespondingQuantity` must have two type traits:

- `Unit` is the unit type for the units of `T`.
- `Rep` is the underlying storage type in `T`.

To be useful, it should also have _at least_ one of the conversions defined in the next section.

### Conversions

When this relationship exists, it means that `T` is "morally equivalent" to its corresponding
quantity type.  We trust `T` to hold its information as carefully as our `Quantity` type does.
Therefore, we enable implicit conversions.

#### From `T` to `Quantity` {#t-to-quantity}

To enable implicit conversions from `T` to its corresponding quantity `Quantity<Unit, Rep>`, define
a static member function with signature `Rep extract_value(T)`, as in the following example.

??? example "Example of setting up implicit conversion from `T` to `Quantity`"

    Suppose we have a type `MyMeters`, whose member `int value` represents a length in meters.  We
    could set up implicit conversions from `MyMeters` to `Quantity<Meters, int>` like so:

    ```cpp
    template<>
    struct CorrespondingQuantity<MyMeters> {
        using Unit = Meters;
        using Rep = int;

        static Rep extract_value(MyMeters x) { return x.value; }
    };
    ```

Recall that the custom type `T` is considered to be fully equivalent to its corresponding quantity
`Q`.  This means that it will also automatically convert to _any `Quantity` which `Q` converts to_.
See the following example.

??? example "Example of 'two-hop' conversion, continued from above"

    ```cpp
    QuantityD<Milli<Meters>> x = MyMeters{3};
    ```

    Here we have a "two-hop" conversion.  `MyMeters{3}` is treated as equivalent to
    a `Quantity<Meters, int>` holding a `3`.  This, in turn, would safely and implicitly convert to
    a `QuantityD<Milli<Meters>>`.  Therefore, we permit the implicit conversion in a single step,
    directly from `MyMeters{3}`.

    The final result would be `milli(meters)(3000.0)`.

#### From `Quantity` to `T`

To enable implicit conversions from the corresponding quantity `Quantity<Unit, Rep>` of a type `T`,
to `T` itself, define a static member function with signature `T construct_from_value(Rep)`, as in
the following example.

??? example "Example of setting up implicit conversion from `Quantity` to `T`"

    Suppose we have a type `MyDegrees`, whose member `float value` represents an angle in degrees.
    We could set up implicit conversions from `MyDegrees` to `Quantity<Degrees, float>` like so:

    ```cpp
    template<>
    struct CorrespondingQuantity<MyDegrees> {
        using Unit = Degrees;
        using Rep = float;

        static MyDegrees construct_from_value(float x) { return MyDegrees{x}; }
    };
    ```

Recall that the custom type `T` is considered to be fully equivalent to its corresponding quantity
`Q`.  This means that _any `Quantity` which converts to `Q`_ will also convert to `T`.  See the
following example.

??? example "Example of 'two-hop' conversion, continued from above"

    ```cpp
    MyDegrees angle = radians(get_value<double>(Magnitude<Pi>{} / mag<2>()));
    ```

    Here we have a "two-hop" conversion.  The corresponding quantity for `MyDegrees` is
    `Quantity<Degrees, float>`.  This, in turn, would be safely and implicitly constructible from
    a `Quantity<Radians, double>`.  Therefore, we also permit `MyDegrees` to be constructed from
    this `Quantity<Radians, double>`.

    The final result would be `MyDegrees{90.0f}`, within floating point rounding error.

## `as_quantity()` {#as-quantity}

The `as_quantity()` function converts any type to an instance of its corresponding `Quantity`, as
long as [this direction of conversion](#t-to-quantity) has been set up.  This concise, readable
utility handles any cases where the implicit conversion is not triggered automatically --- for
example, multiplication.

??? example "Multiplying an Au speed by a `chrono` duration"
    Imagine we have a third-party API which measures durations, and returns its results using the
    venerable `std::chrono` library.

    ```cpp
    namespace third_party {
    std::chrono::nanoseconds measure_duration();
    }
    ```

    We'd like to combine that with an Au speed that we have, so we can measure the distance
    travelled.  Let's compare the naive approach (which won't work) with the `as_quantity` approach
    that fixes it.  (Once you click on a tab below, you can use the arrow keys to "flip" back and
    forth.)

    === "Naive approach (broken)"
        ```cpp
        const auto speed = (miles / hour)(65.0);
        const QuantityD<Meters> dist = speed * measure_duration();
        // Compiler error! ------------------^
        ```

        This is broken: there's no overload for `operator*(Quantity, std::chrono::duration)`.

    === "`as_quantity()` (fixed)"
        ```cpp
        const auto speed = (miles / hour)(65.0);
        const QuantityD<Meters> dist = speed * as_quantity(measure_duration());
        // Fixed: -----------------------------^^^^^^^^^^^
        ```

        `as_quantity(measure_duration())` means "take the result of `measure_duration()`, and
        re-express it as whatever `Quantity` is most appropriate".  At this point, Au's machinery
        takes over.  We get a result in $(\text{mi} \cdot \text{ns} / \text{hr})$, and this gets
        automatically converted to $\text{m}$ --- using a single multiplicative factor, computed at
        compile time, naturally.

`as_quantity()` is [SFINAE](https://en.cppreference.com/w/cpp/language/sfinae)-friendly.  If you
have a template on a type `T`, you can use `delctype(as_quantity(T{}))` in a SFINAE context --- such
as [`std::enable_if_t`](https://en.cppreference.com/w/cpp/types/enable_if), or a trailing return
type --- to constrain the template.  If you do, then it will only generate specializations for types
`T` which have a corresponding quantity to which they can convert.  There are [some
examples](https://github.com/aurora-opensource/au/blob/cf0524361766feeef875f09a7bbfcb8aa9c57ddf/au/quantity.hh#L569-L635)
in the library itself.

## Built-in corresponding quantities

Au strives to minimize dependencies, but we do depend on C++14.  Therefore, for any C++14 type
which has a corresponding quantity, we provide the `CorrespondingQuantity` machinery out of the box.

Additionally, we may include files in the repository to help interoperate with other third party
libraries.  Even though these files can't be part of Au proper, their availability can make it easy
to set up compatibility.

Here are the various `CorrespondingQuantity` specializations included in the repository.

### `std::chrono::duration` {#chrono-duration}

[`std::chrono::duration`](https://en.cppreference.com/w/cpp/chrono/duration) has two template
parameters: `Rep`, and `Period`.  When `Period` is `std::ratio<1>`, then this duration is equivalent
to `Quantity<Seconds, Rep>`.  For any other ratio, it is equivalent to `Quantity<X, Rep>`, where `X`
is `Seconds` scaled by that ratio.

We include this correspondence out of the box.  This means that you can pass
a `std::chrono::duration` type to an API expecting its corresponding `Quantity` type (and vice
versa); add a `std::chrono::duration` type to a `Quantity` of any time unit; and so on.

### nholthaus/units library

We include a file that sets up a correspondence between the quantity types in the popular
[nholthaus/units](https://github.com/nholthaus/units) library, and those in Au.

This file is **not** "active" by default.  You will need to set it up in your project, following our
[detailed how-to guide](../howto/interop/nholthaus.md).

<script src="../../assets/hrh4.js" async=false defer=false></script>
