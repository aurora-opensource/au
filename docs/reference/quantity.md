# Quantity

`Quantity` is our workhorse type.  It combines a runtime numeric value with compile-time information
about the units of measure in which that quantity is expressed.

`Quantity` comes with "batteries included".  We try to support as many operations as possible
natively, to avoid incentivizing users to leave the safety of the units library.

`Quantity` is a template, `Quantity<U, R>`, with two parameters.  Both are typenames.

- `U` is the _unit_: a type representing the [unit of measure](./unit.md).
- `R` is the _"rep"_, a term we borrow from the [`std::chrono`
  library](https://en.cppreference.com/w/cpp/chrono/duration).  It's the underlying raw numeric type
  which holds the wrapped value.

??? tip "Handling temperatures"
    If you are working with temperatures --- in the sense of "what temperature is it?", rather than
    "how much did the temperature change?" --- you will want to use
    [`QuantityPoint`](./quantity_point.md) instead of `Quantity`.  For further useful background
    reading, see our [quantity point discussion](../discussion/concepts/quantity_point.md).

## Naming `Quantity` in code

You can use both template parameters directly (for example, `Quantity<Meters, int>`).  However,
multiple template parameters can be awkward to read.  It can also give the parser some trouble when
you're forced to use macros, as in googletest.  For that reason, we also provide "Rep-named
aliases", so you can express the rep more precisely, and put the visual focus on the unit:

| Rep-named alias | Equivalent to: |
|-----------------|----------------|
| `QuantityD<U>` | `Quantity<U, double>` |
| `QuantityF<U>` | `Quantity<U, float>` |
| `QuantityI<U>` | `Quantity<U, int>` |
| `QuantityU<U>` | `Quantity<U, unsigned int>` |
| `QuantityI32<U>` | `Quantity<U, int32_t>` |
| `QuantityU32<U>` | `Quantity<U, uint32_t>` |
| `QuantityI64<U>` | `Quantity<U, int64_t>` |
| `QuantityU64<U>` | `Quantity<U, uint64_t>` |

For more practice with this, see [Tutorial 102: API Types](../tutorial/102-api-types.md).

## Constructing `Quantity` {#constructing}

There are several ways to construct a `Quantity` object.  For concreteness, we'll use this skeleton
to show the signatures.

```cpp
template <typename Unit, typename Rep>
class Quantity {

   // A) Implicit constructor from another Quantity
   template <typename OtherUnit, typename OtherRep>
   Quantity(Quantity<OtherUnit, OtherRep> other);

   // B) Construct from `Zero`
   Quantity(Zero);

   // C) Default constructor
   Quantity();

   // D) Construct from equivalent type
   template <typename T>
   Quantity(T &&x);
   // NOTE: only exists when `T` is an "exactly-equivalent" type.  One example:
   // `std::chrono::nanoseconds` can construct an `au::QuantityI64<Nano<Seconds>>`.
};
```

However, the _preferred_ way to construct a `Quantity` is actually none of these.  It's the
_quantity maker_, which we describe next.

### Quantity Maker (preferred)

The preferred way to construct a `Quantity` of a given unit is to use the _quantity maker_ for that
unit.  This is a callable whose name is the plural form of that unit, expressed in "snake_case": for
example, `nautical_miles`.  When you pass a raw numeric variable of type `T` to the quantity maker,
it returns a `Quantity` of the unit it represents, whose rep type is `T`.

!!! example
    `nautical_miles(15)` returns a `Quantity<NauticalMiles, int>`.

For more practice with quantity makers, see [Tutorial 101: Quantity
Makers](../tutorial/101-quantity-makers.md).

### Implicit constructor from another `Quantity` {#implicit-from-quantity}

This is option `A)` from the [previous section](#constructing).

```cpp
template <typename OtherUnit, typename OtherRep>
Quantity<Unit, Rep>(Quantity<OtherUnit, OtherRep> other);
```

This constructor performs a unit conversion.  The result will represent the _same quantity_ as the
input, but expressed in the _units_ of the newly constructed object's type.  It will also convert
the stored value to the _rep_ of the constructed object, if necessary.

This constructor only exists when this unit-and-rep conversion is both meaningful and safe.  It can
fail to exist in several ways.

1. If the input quantity has a **different dimension**, then the operation is intrinsically
   meaningless and we forbid it.

2. If the _constructed_ quantity's rep (that is, `Rep` in the code snippet above) is **not floating
   point**, then we forbid any conversion that might produce a non-integer value.  Examples include:

    a. When `OtherRep` _is floating point_, we forbid this conversion.

    b. When `UnitRatioT<OtherUnit, Unit>` is _not an integer_, we forbid this conversion.

3. If we're performing an integer-to-integer conversion, and the conversion factor carries
   a significant risk of overflowing the rep, we forbid the conversion.  This is the adaptive
   "overflow safety surface" which we featured in our [Au announcement blog
   post](https://blog.aurora.tech/engineering/introducing-au-our-open-source-c-units-library).

These last two are examples of conversions that are physically meaningful, but forbidden due to the
risk of larger-than-usual errors.  The library can still perform these conversions, but not via this
constructor, and it must be "forced" to do so.  See [`.as<T>(unit)`](#as) for more details.

### Constructing from `Zero`

This is option `B)` from the [previous section](#constructing).

```cpp
Quantity(Zero);
```

This constructs a `Quantity` with a value of 0.

!!! note
    `Zero` is the name of the type.  At the callsite, you would pass an _instance_ of that type,
    such as the constant `ZERO`.  For example:

    ```cpp
    QuantityD<Meters> height{ZERO};
    ```

### Default constructor

This is option `C)` from the [previous section](#constructing).

```cpp
Quantity();
```

A default-constructed `Quantity` is initialized to some value, which helps avoid certain kinds of
memory safety bugs.  However, the value is contractually unspecified.  You can look up that value by
reading the source code, but we may change it in the future, and we would not consider this to be
a breaking change.  The only valid operation on a default-constructed `Quantity` is to assign to it
later on.

### Constructing from corresponding quantity

This is option `D)` from the [previous section](#constructing).

```cpp
template <typename T>
Quantity(T &&x);
// NOTE: only exists when `T` is an "exactly-equivalent" type.  One example:
// `std::chrono::nanoseconds` can construct an `au::QuantityI64<Nano<Seconds>>`.
```

This constructor will only exist when `T` has a ["corresponding
quantity"](./corresponding_quantity.md), and this `Quantity` type is _implicitly constructible_ from
that "corresponding quantity" type by [the above mechanism](#implicit-from-quantity).

## Extracting the stored value

In order to access the raw numeric value stored inside of `Quantity`, you must explicitly name the
unit at the callsite.  There are two functions which can do this, depending on whether you want to
access by value or by reference.

### `.in(unit)` {#extracting-with-in}

This function returns the underlying stored value, by value.  See the [unit
slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: extracting `4.56` from `seconds(4.56)`"
    Consider this `Quantity<Seconds, double>`:

    ```cpp
    auto t = seconds(4.56);
    ```

    You can retrieve the underlying value by writing either `t.in(seconds)` (passing the
    `QuantityMaker`), or `t.in(Seconds{})` (passing an instance of the unit).

### `.data_in(unit)`

This function returns a reference to the underlying stored value.  See the [unit
slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: incrementing the underlying stored value"
    ```cpp
    auto length = inches(60);
    length.data_in(inches)++;
    ```

    Since `length` is not `const`, the reference returned by `.data_in()` will be mutable, and we
    can treat it like any other `int` lvalue.  The above would result in `inches(61)`.

## Performing unit conversions

We have two methods for performing unit conversions.  They have identical APIs, but their names are
different.  `as` returns another `Quantity`, but `in` exits the library and returns a raw number.

### `.as(unit)`, `.as<T>(unit)` {#as}

This function produces a new representation of the input `Quantity`, converted to the new unit. See
the [unit slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: converting `feet(3)` to `inches`"
    Consider this `Quantity<Feet, int>`:

    ```cpp
    auto length = feet(3);
    ```

    Then `length.as(inches)` re-expresses this quantity in units of inches.  Specifically, it
    returns a `Quantity<Inches, int>`, which is equal to `inches(36)`.

    The above example used the quantity maker, `inches`.  One could also use an instance of the unit
    type `Inches`, writing `length.as(Inches{})`.  The former is generally preferable; the latter is
    mainly useful in generic code where the unit type may be all you have.

**Without** a template argument, `.as(unit)` obeys the same safety checks as for the [implicit
constructors](#implicit-from-quantity): conversions at high risk for integer overflow or truncation
are forbidden.  Additionally, the `Rep` of the output is identical to the `Rep` of the input.

**With** a template argument, `.as<T>(unit)` has two differences.

1. The output `Rep` will be `T`.
2. The conversion is considered "forcing", and will be permitted in spite of any overflow or
   truncation risk.  The semantics are similar to `static_cast<T>`.

??? example "Example: forcing a conversion from inches to feet"
    `inches(24).as(feet)` is not allowed.  This conversion will divide the underlying value, `24`,
    by `12`.  While this particular value would produce an integer result, most other `int` values
    would not.  Because our result uses `int` for storage --- same as the input --- we forbid this.

    `inches(24).as<int>(feet)` _is_ allowed.  The "explicit rep" template parameter has "forcing"
    semantics.  This would produce `feet(2)`.

    However, note that this operation uses integer division, which truncates: so, for example,
    `inches(23).as<int>(feet)` would produce `feet(1)`.

!!! tip
    Prefer to **omit** the template argument if possible, because you will get more safety checks.
    The risks which the no-template-argument version warns about are real.

### `.in(unit)`, `.in<T>(unit)`

This function produces the value of the `Quantity`, re-expressed in the new unit.  `unit` can be
either a `QuantityMaker` for the quantity's unit, or an instance of the unit itself. See the [unit
slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: getting the value of `feet(3)` in `inches`"
    Consider this `Quantity<Feet, int>`:

    ```cpp
    auto length = feet(3);
    ```

    Then `length.in(inches)` converts this quantity to inches, and returns the value, `36`, as an
    `int`.

    The above example used the quantity maker, `inches`.  One could also use an instance of the unit
    type `Inches`, writing `length.in(Inches{})`.  The former is generally preferable; the latter is
    mainly useful in generic code where the unit type may be all you have.

**Without** a template argument, `.in(unit)` obeys the same safety checks as for the [implicit
constructors](#implicit-from-quantity): conversions at high risk for integer overflow or truncation
are forbidden.  Additionally, the `Rep` of the output is identical to the `Rep` of the input.

**With** a template argument, `.in<T>(unit)` has two differences.

1. The output type will be `T`.
2. The conversion is considered "forcing", and will be permitted in spite of any overflow or
   truncation risk.  The semantics are similar to `static_cast<T>`.

??? example "Example: forcing a conversion from inches to feet"
    `inches(24).in(feet)` is not allowed.  This conversion will divide the underlying value, `24`,
    by `12`.  While this particular value would produce an integer result, most other `int` values
    would not.  Because our result uses `int` --- same as the input's rep --- we forbid this.

    `inches(24).in<int>(feet)` _is_ allowed.  The "explicit rep" template parameter has "forcing"
    semantics.  This would produce `2`.

    However, note that this operation uses integer division, which truncates: so, for example,
    `inches(23).in<int>(feet)` would produce `1`.

!!! tip
    Prefer to **omit** the template argument if possible, because you will get more safety checks.
    The risks which the no-template-argument version warns about are real.

## Operations

Au includes as many common operations as possible.  Our goal is to avoid incentivizing users to
leave the safety of the library.

Recall that there are [two main categories](../discussion/concepts/arithmetic.md) of operation:
"arbitrary unit" operations, and "common unit" operations.

### Comparison

Comparison is a [common unit operation](../discussion/concepts/arithmetic.md#common-unit).  If the
input `Quantity` types are not identical --- same `Unit` _and_ `Rep` --- then we first convert them
to their [common type](#common-type). Then, we perform the comparison by delegating to the
comparison operator on the underlying values.

If either input is a non-`Quantity` type `T`, but it has
a [`CorrespondingQuantity`](./corresponding_quantity.md), then that input can participate _as if it
were_ its corresponding quantity.

??? example "Example of corresponding quantity comparison"
    We provide built-in [equivalence with `std::chrono::duration`
    types](./corresponding_quantity.md#chrono-duration).  Therefore, this comparison will work:

    ```cpp
    const bool greater = std::chrono::duration<double>{1.0} > milli(seconds)(999.9f);
    ```

    Here, the `std::chrono::duration` variable will be treated identically to `seconds(1.0)`. The
    comparison will take place in units of `milli(seconds)` (since that is the common unit), and
    using a Rep of `double` (since that is `std::common_type_t<double, float>`).  The variable
    `greater` will hold the value `true`.

We support the following binary comparison operators:

- `==`
- `!=`
- `>`
- `>=`
- `<`
- `<=`

### Addition

Addition is a [common unit operation](../discussion/concepts/arithmetic.md#common-unit).  If the
input `Quantity` types are not identical --- same `Unit` _and_ `Rep` --- then we first convert them
to their [common type](#common-type).  The result is a `Quantity` of this common type, whose value
is the sum of the input values (after any common type conversions).

If either input is a non-`Quantity` type `T`, but it has
a [`CorrespondingQuantity`](./corresponding_quantity.md), then that input can participate _as if it
were_ its corresponding quantity.

### Subtraction {#subtraction}

Subtraction is a [common unit operation](../discussion/concepts/arithmetic.md#common-unit).  If the
input `Quantity` types are not identical --- same `Unit` _and_ `Rep` --- then we first convert them
to their [common type](#common-type).  The result is a `Quantity` of this common type, whose value
is the difference of the input values (after any common type conversions).

If either input is a non-`Quantity` type `T`, but it has
a [`CorrespondingQuantity`](./corresponding_quantity.md), then that input can participate _as if it
were_ its corresponding quantity.

### Multiplication

Multiplication is an [arbitrary unit
operation](../discussion/concepts/arithmetic.md#arbitrary-unit).  This means you can reason
independently about the units and the values.

- The output unit is the product of the input units.
- The output value is the product of the input values.
    - The output rep (storage type) is the same as the type of the product of the input reps.

The output is always a `Quantity` with this unit and rep, _unless_ the units **completely** cancel
out (returning [a unitless unit](./unit.md#unitless-unit)).  If they do, then [we return a raw
number](../discussion/concepts/dimensionless.md#exact-cancellation).

If either _input_ is a raw number, then it only affects the value, not the unit.  It's equivalent to
a `Quantity` whose unit is [a unitless unit](./unit.md#unitless-unit).

### Division

Division is an [arbitrary unit operation](../discussion/concepts/arithmetic.md#arbitrary-unit).
This means you can reason independently about the units and the values.

- The output unit is the quotient of the input units.
- The output value is the quotient of the input values.
    - The output rep (storage type) is the same as the type of the quotient of the input reps.

The output is always a `Quantity` with this unit and rep, _unless_ the units **completely** cancel
out (returning [a unitless unit](./unit.md#unitless-unit)).  If they do, then [we return a raw
number](../discussion/concepts/dimensionless.md#exact-cancellation).

If either _input_ is a raw number, then it only affects the value, not the unit.  It's equivalent to
a `Quantity` whose unit is [a unitless unit](./unit.md#unitless-unit).

#### `integer_quotient()`

Experience has shown that raw integer division can be dangerous in a units library context.  It
conflicts with intuitions, and can produce code that is silently and grossly incorrect: see the
[integer division section](../troubleshooting.md#integer-division-forbidden) of the troubleshooting
guide for an example.

To use integer division, you must ask for it explicitly by name, with the `integer_quotient()`
function.

??? example "Using `integer_quotient()` to explicitly opt in to integer division"

    This will not work:

    ```cpp
    miles(125) / hours(2);
    //        ^--- Forbidden!  Compiler error.
    ```

    However, this will work just fine:

    ```cpp
    integer_quotient(miles(125), hours(2));
    ```

    It produces `(miles / hour)(62)`.

### Unary `+` and `-`

For a `Quantity` instance `q`, you can apply a "unary plus" (`+q`) or "unary minus" (`-q`).  These
produce a `Quantity` of the same type, with the unary plus or minus operator applied to the
underlying value.

### Shorthand addition and subtraction (`+=`, `-=`)

The input must be a `Quantity` which is [implicitly convertible](#implicit-from-quantity) to the
target `Quantity` type.  These operations first perform that conversion, and then replace the target
`Quantity` with the result of that addition (for `+=`) or subtraction (for `-=`).

### Shorthand multiplication and division (`*=`, `/=`)

The input must be a _raw number_ which is implicitly convertible to the target `Quantity`'s rep.
These operations first perform that conversion, and then replace the target `Quantity`
with the result of that multiplication (for `*=`) or division (for `/=`).

### Automatic conversion to `Rep`

For any `Quantity<U, Rep>`, if `U` is a [unitless unit](./unit.md#unitless-unit), we provide
implicit conversion to `Rep`, which simply returns the underlying value.  This enables users to pass
such a `Quantity` to an API expecting `Rep`.

We do _not_ provide this functionality for quantities of any other unit.  In particular --- and
unlike many other units libraries --- we do not automatically convert other dimensionless units
(such as `Percent`) to a raw number.  While this operation is intuitively appealing, experience
shows that it [does more harm than
good](../discussion/concepts/dimensionless.md#implicit-conversions).

### Mod (`%`) {#mod}

The modulo operator, `%`, is the remainder of an integer division.  Au only defines this operator
between two `Quantity` types, not between a `Quantity` and a raw number.

More precisely, suppose we have instances of two `Quantity` types: `Quantity<U1, R1> q1` and
`Quantity<U2, R2> q2`.  Then `q1 % q2` is defined only when:

1. `(R1{} % R2{})` is defined (that is, both `R1` and `R2` are integral types).
2. `CommonUnitT<U1, U2>` is defined (that is, `U1` and `U2` have the same dimension).

When these conditions hold, the result is equivalent to first converting `q1` and `q2` to their
common unit, and then computing the remainder from performing integer division on their values.

??? info "Why this policy?"
    We restrict to same-dimension quantities because this not only gives a meaningful answer, but
    the meaning of that answer is independent of the choice of units.  Take length as an example: we
    could imagine repeatedly subtracting `q2` from `q1` until we get a result that is smaller than
    `q2`.  This final result is the answer we seek.  It does not depend on the units for either
    quantity.

    The reason we express that answer in the common unit of `q1` and `q2` is because it's the
    simplest unit in which we _can_ express it.  (Note that this is the same unit we would get from
    the operation of repeated [subtraction](#subtraction) as well, so this choice is consistent.)

## `rep_cast`

`rep_cast` performs a `static_cast` on the underlying value of a `Quantity`.  It is used to change
the rep.

Given any `Quantity<U, R> q` whose rep is `R`, then `rep_cast<T>(q)` gives a `Quantity<U, T>`, whose
underlying value is `static_cast<T>(q.in(U{}))`.

## Templates and Traits

### Matching: `typename U, typename R`

To specialize a template to match any `Quantity`, declare two template parameters: one for the unit,
one for the rep.

??? example "Example: function template"
    ```cpp
    template <typename U, typename R>
    Quantity<U, R> negate(Quantity<U, R> q) {
        return -q;
    }
    ```

    This function template will match any `Quantity` specialization, and nothing else.

??? example "Example: type template"
    ```cpp
    // First, we need to declare the generic type template.  It matches a single type, `T`.
    template <typename T>
    struct Size;

    // Now we can declare a specialization that matches any `Quantity`, by templating on its unit
    // and rep.
    template <typename U, typename R>
    struct Size<Quantity<U, R>> {

        // Note: this example uses inline variables, a C++17 feature.  It's just for illustration.
        static constexpr inline std::size_t value = sizeof(R);
    };
    ```

    In this way, `Size<T>::value` will exist only when `T` is some `Quantity` type (unless, of
    course, other specializations get defined elsewhere).

### `::Rep`

Every `Quantity` type has a public static alias, `::Rep`, which indicates its underlying storage
type (or, its "rep").

??? example
    `decltype(meters(5))::Rep` is `int`.

### `::Unit`

Every `Quantity` type has a public static alias, `::Unit`, which indicates its unit type.

??? example
    `decltype(meters(5))::Unit` is `Meters`.

### `::unit`

Every `Quantity` type has a public static member variable, `::unit`, which is an instance of its
unit type.

??? example
    `decltype(meters(5))::unit` is `Meters{}`.

### `std::common_type` specialization {#common-type}

For two `Quantity` types, one with unit `U1` and rep `R1`, and the other with unit `U2` and rep
`R2`, then `std::common_type_t<Quantity<U1, R1>, Quantity<U2, R2>>` has the following properties.

1. It exists if and only if `U1` and `U2` have the same dimension.

2. When it exists, it is `Quantity<U, R>`, where `U` is the [common
   unit](../discussion/concepts/common_unit.md) of `U1` and `U2`, and `R` is `std::common_type_t<R1,
   R2>`.

As [required by the standard](https://en.cppreference.com/w/cpp/types/common_type), our
`std::common_type` specializations are
[SFINAE](https://en.cppreference.com/w/cpp/language/sfinae)-friendly: improper combinations will
simply not be present, rather than producing a hard error.

### AreQuantityTypesEquivalent

**Result:** Indicates whether two `Quantity` types are equivalent.  Equivalent types may be freely
converted to each other, and no arithmetic operations will be performed in doing so.

More precisely, `Quantity<U1, R1>` and `Quantity<U2, R2>` are equivalent if and only if **both** of
the following conditions hold.

1. The units `U1` and `U2` are [quantity-equivalent](./unit.md#quantity-equivalent).

2. The reps `R1` and `R2` are the same type.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `AreQuantityTypesEquivalent<U1, U2>::value`
