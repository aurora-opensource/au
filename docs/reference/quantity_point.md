# QuantityPoint

`QuantityPoint` is our [affine space type](http://videocortex.io/2018/Affine-Space-Types/).  Common
example use cases include temperatures and mile markers.  See our [`QuantityPoint`
discussion](../discussion/concepts/quantity_point.md) for a more detailed understanding.

`QuantityPoint` is a template, `QuantityPoint<U, R>`, with two parameters.  Both are typenames.

- `U` is the _unit_: a type representing the [unit of measure](./unit.md).
- `R` is the _"rep"_, a term we borrow from the [`std::chrono`
  library](https://en.cppreference.com/w/cpp/chrono/duration).  It's the underlying raw numeric type
  which holds the wrapped value.

## Naming `QuantityPoint` in code

You can use both template parameters directly (for example, `QuantityPoint<Meters, int>`).  However,
multiple template parameters can be awkward to read.  It can also give the parser some trouble when
you're forced to use macros, as in googletest.  For that reason, we also provide "Rep-named
aliases", so you can express the rep more precisely, and put the visual focus on the unit:

| Rep-named alias | Equivalent to: |
|-----------------|----------------|
| `QuantityPointD<U>` | `QuantityPoint<U, double>` |
| `QuantityPointF<U>` | `QuantityPoint<U, float>` |
| `QuantityPointI<U>` | `QuantityPoint<U, int>` |
| `QuantityPointU<U>` | `QuantityPoint<U, unsigned int>` |
| `QuantityPointI32<U>` | `QuantityPoint<U, int32_t>` |
| `QuantityPointU32<U>` | `QuantityPoint<U, uint32_t>` |
| `QuantityPointI64<U>` | `QuantityPoint<U, int64_t>` |
| `QuantityPointU64<U>` | `QuantityPoint<U, uint64_t>` |

## Constructing `QuantityPoint` {#constructing}

There are three ways to construct a `QuantityPoint` object.

- The **preferred** way, which we'll explain below, is to use a _quantity point maker_.
- The other two ways are both normal C++ constructors.

### Quantity point maker (preferred) {#quantity-point-maker}

The preferred way to construct a `QuantityPoint` of a given unit is to use the _quantity point
maker_ for that unit.  This is a callable whose name is the plural form of that unit, expressed in
"snake_case", and suffixed with `_pt` for "point": for example, `fahrenheit_pt`.  When you pass
a raw numeric variable of type `T` to the quantity point maker, it returns a `QuantityPoint` of the
unit it represents, whose rep type is `T`.

!!! example
    `fahrenheit_pt(75)` returns a `QuantityPoint<Fahrenheit, int>`.

### Implicit constructor from another `QuantityPoint` {#implicit-from-quantity}

This constructor performs a unit conversion.  The result will represent the _same point_ as the
input, but expressed in the _units_ of the newly constructed object's type.  It will also convert
the stored value to the _rep_ of the constructed object, if necessary.

Here is the signature of the constructor, slightly simplified for illustration purposes.  (We've
enclosed it in the class template definition to be clear about `Unit` and `Rep` in the discussion
that follows.)

```cpp
template <typename Unit, typename Rep>
class QuantityPoint {

    // Implicit constructor signature (for illustration purposes):
    template <typename OtherUnit, typename OtherRep>
    QuantityPoint(QuantityPoint<OtherUnit, OtherRep> other);
};
```

This constructor only exists when this unit-and-rep conversion is both meaningful and safe.  It can
fail to exist in several ways.

1. If the input quantity has a **different dimension**, then the operation is intrinsically
   meaningless and we forbid it.

2. If the _constructed_ quantity's rep (that is, `Rep` in the code snippet above) is **not floating
   point**, then we forbid any conversion that might produce a non-integer value.  Examples include:

    a. When `OtherRep` _is floating point_, we forbid this conversion.

    b. When `UnitRatioT<OtherUnit, Unit>` is _not an integer_, we forbid this conversion.

    c. When the origin of `OtherUnit` is _additively offset_ from the origin of `Unit` by an amount
       that can't be represented as an integer in the target units, `Unit`, we forbid this
       conversion.

Note that case `c` doesn't occur for `Quantity`; it is unique to `QuantityPoint`.

We also inherit the "overflow safety surface" from the `Quantity` member inside of `QuantityPoint`
(discussed as point 3 of the [`Quantity` constructor docs](./quantity.md#implicit-from-quantity)).
This can prevent certain quantity point conversions which have excessive overflow risk.

Here are several examples to illustrate the conditions under which implicit conversions are allowed.

<!--
What's up with the `&#8288 {: style="padding:0"} |` entries in the table below?  See:
https://github.com/mkdocs/mkdocs/issues/1198#issuecomment-1253896100
-->

!!! example "Examples of `QuantityPoint` to `QuantityPoint` conversions"
    | Source type | Target type | Implicit construction outcome |
    |-------------|-------------|---------|
    | `QuantityPoint<Meters, double>` | `QuantityPoint<Meters, int>` {: rowspan=3} | :cross_mark: **Forbidden:** `double` source not guaranteed to hold an integer value |
    | `QuantityPoint<Milli<Meters>, int>` | :cross_mark: **Forbidden:** point measured in $\text{mm}$ not generally an integer in $\text{m}$ | &#8288 {: style="padding:0"} |
    | `QuantityPoint<Kilo<Meters>, int>` | :check_mark: **Permitted:** point measured in $\text{km}$ guaranteed to be integer in $\text{m}$ | &#8288 {: style="padding:0"} |
    | `QuantityPoint<Celsius, int>` {: rowspan=3} | `QuantityPoint<Kelvins, int>` | :cross_mark: **Forbidden:** Zero point offset from Kelvins to Celsius is $273.15\,\, \text{K}$, a non-integer number of Kelvins |
    | `QuantityPoint<Kelvins, double>` | :check_mark: **Permitted:** target Rep is floating point, and can represent offset of $273.15\,\, \text{K}$ |
    | `QuantityPoint<Milli<Kelvins>, int>` | :check_mark: **Permitted:** offset in target units is $273,\!150\,\, \text{mK}$, which is an integer |

Note that every case in the above table is _physically_ meaningful (because the source and target
have the same dimension), but some conversions are forbidden due to the risk of larger-than-usual
errors.  The library can still perform these conversions, but not via this constructor, and it must
be "forced" to do so. See [`.coerce_as(unit)`](#coerce) for more details.

### Default constructor

Here is the signature of the constructor, enclosed in the class template definition for context.

```cpp
template <typename Unit, typename Rep>
class QuantityPoint {
    QuantityPoint();
};
```

A default-constructed `QuantityPoint` is initialized to some value, which helps avoid certain kinds
of memory safety bugs.  However, **the value is contractually unspecified**.  You can of course look
up that value by reading the source code, but we may change it in the future, and **we would not
consider this to be a breaking change**.  The only valid operation on a default-constructed
`QuantityPoint` is to assign to it later on.

## Extracting the stored value

In order to access the raw numeric value stored inside of `QuantityPoint`, you must explicitly name
the unit at the callsite.  There are two functions which can do this, depending on whether you want
to access by value or by reference.

### By value: `.in(unit)` {#extracting-with-in}

This function returns the underlying stored value, by value.  See the [unit
slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: extracting `4.56` from `meters_pt(4.56)`"
    Consider this `QuantityPoint<Meters, double>`:

    ```cpp
    auto p = meters_pt(4.56);
    ```

    You can retrieve the underlying value by writing either `p.in(meters_pt)` (passing the
    `QuantityPointMaker`), or `p.in(Meters{})` (passing an instance of the unit).

### By reference: `.data_in(unit)`

This function returns a reference to the underlying stored value.  See the [unit
slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: incrementing the underlying stored value"
    ```cpp
    auto temperature = celsius_pt(20);
    temperature.data_in(celsius_pt)++;
    ```

    Since `temperature` is not `const`, the reference returned by `.data_in()` will be mutable, and
    we can treat it like any other `int` lvalue.  The above would result in `celsius_pt(21)`.

## Performing unit conversions

We have two methods for performing unit conversions.  They have identical APIs, but their names are
different.  `as` returns another `QuantityPoint`, but `in` exits the library and returns a raw
number.

### `.as(unit)`, `.as<T>(unit)` {#as}

This function produces a new representation of the input `QuantityPoint`, converted to the new unit.
See the [unit slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: converting `meters_pt(3)` to `centi(meters_pt)`"
    Consider this `QuantityPoint<Meters, int>`:

    ```cpp
    auto point = meters_pt(3);
    ```

    Then `point.as(centi(meters_pt))` re-expresses this quantity in units of centimeters.
    Specifically, it returns a `QuantityPoint<Centi<Meters>, int>`, which is equal to
    `centi(meters_pt)(300)`.

    The above example used the quantity maker, `centi(meters_pt)`.  One could also use an instance
    of the unit type `Centi<Meters>`, writing `point.as(Centi<Meters>{})`.  The former is generally
    preferable; the latter is mainly useful in generic code where the unit type may be all you have.

**Without** a template argument, `.as(unit)` obeys the same safety checks as for the [implicit
constructors](#implicit-from-quantity): conversions at high risk for integer overflow or truncation
are forbidden.  Additionally, the `Rep` of the output is identical to the `Rep` of the input.

**With** a template argument, `.as<T>(unit)` has two differences.

1. The output `Rep` will be `T`.
2. The conversion is considered "forcing", and will be permitted in spite of any overflow or
   truncation risk.  The semantics are similar to `static_cast<T>`.

However, note that we may change this second property in the future.  The version with the template
arguments may be changed later so that it _does_ prevent lossy conversions.  If you want this
"forcing" semantic, prefer to use [`.coerce_as(unit)`](#coerce), and add the explicit template
parameter only if you want to change the rep.  See
[#122](https://github.com/aurora-opensource/au/issues/122) for more details.

??? example "Example: forcing a conversion from centimeters to meters"
    `centi(meters_pt)(200).as(meters_pt)` is not allowed.  This conversion will divide the
    underlying value, `200`, by `100`.  Now, it so happens that this _particular_ value _would_
    produce an integer result. However, the compiler must decide whether to permit this operation
    _at compile time_, which means we don't yet know the value.  Since most `int` values would _not_
    produce integer results, we forbid this.

    `centi(meters_pt)(200).as<int>(meters_pt)` _is_ allowed.  The "explicit rep" template parameter
    has "forcing" semantics.  This would produce `meters_pt(2)`. However, note that this operation
    uses integer division, which truncates: so, for example,
    `centi(meters_pt)(199).as<int>(meters_pt)` would produce `meters_pt(1)`.

!!! tip
    Prefer to **omit** the template argument if possible, because you will get more safety checks.
    The risks which the no-template-argument version warns about are real.

### `.in(unit)`, `.in<T>(unit)`

This function produces the value of the `QuantityPoint`, re-expressed in the new unit.  `unit` can
be either a `QuantityPointMaker` for the quantity's unit, or an instance of the unit itself. See the
[unit slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: getting the value of `meters_pt(3)` in `centi(meters_pt)`"
    Consider this `QuantityPoint<Meters, int>`:

    ```cpp
    auto point = meters_pt(3);
    ```

    Then `point.in(centi(meters_pt))` converts this quantity to centimeters, and returns the value,
    `300`, as an `int`.

    The above example used the quantity maker, `centi(meters_pt)`.  One could also use an instance
    of the unit type `Centi<Meters>`, writing `point.in(Centi<Meters>{})`.  The former is generally
    preferable; the latter is mainly useful in generic code where the unit type may be all you have.

**Without** a template argument, `.in(unit)` obeys the same safety checks as for the [implicit
constructors](#implicit-from-quantity): conversions at high risk for integer overflow or truncation
are forbidden.  Additionally, the `Rep` of the output is identical to the `Rep` of the input.

**With** a template argument, `.in<T>(unit)` has two differences.

1. The output type will be `T`.
2. The conversion is considered "forcing", and will be permitted in spite of any overflow or
   truncation risk.  The semantics are similar to `static_cast<T>`.

However, note that we may change this second property in the future.  The version with the template
arguments may be changed later so that it _does_ prevent lossy conversions.  If you want this
"forcing" semantic, prefer to use [`.coerce_in(unit)`](#coerce), and add the explicit template
parameter only if you want to change the rep.  See
[#122](https://github.com/aurora-opensource/au/issues/122) for more details.

??? example "Example: forcing a conversion from centimeters to meters"
    `centi(meters_pt)(200).in(meters_pt)` is not allowed.  This conversion will divide the
    underlying value, `200`, by `100`.  Now, it so happens that this _particular_ value _would_
    produce an integer result. However, the compiler must decide whether to permit this operation
    _at compile time_, which means we don't yet know the value.  Since most `int` values would _not_
    produce integer results, we forbid this.

    `centi(meters_pt)(200).in<int>(meters_pt)` _is_ allowed.  The "explicit rep" template parameter
    has "forcing" semantics (at least for now; see
    [#122](https://github.com/aurora-opensource/au/issues/122)).  This would produce `2`. However,
    note that this operation uses integer division, which truncates: so, for example,
    `centi(meters_pt)(199).in<int>(meters_pt)` would produce `1`.

!!! tip
    Prefer to **omit** the template argument if possible, because you will get more safety checks.
    The risks which the no-template-argument version warns about are real.

### Forcing lossy conversions: `.coerce_as(unit)`, `.coerce_in(unit)` {#coerce}

This function performs the exact same kind of unit conversion as if the string `coerce_` were
removed.  However, it will ignore any safety checks for overflow or truncation.

??? example "Example: forcing a conversion from centimeters to meters"
    `centi(meters_pt)(200).in(meters_pt)` is not allowed.  This conversion will divide the
    underlying value, `200`, by `100`.  Now, it so happens that this _particular_ value _would_
    produce an integer result. However, the compiler must decide whether to permit this operation
    _at compile time_, which means we don't yet know the value.  Since most `int` values would _not_
    produce integer results, we forbid this.

    `centi(meters_pt)(200).coerce_in(meters_pt)` _is_ allowed.  The `coerce_` prefix has "forcing"
    semantics.  This would produce `2`. However, note that this operation uses integer division,
    which truncates: so, for example, `centi(meters_pt)(199).coerce_in(meters_pt)` would produce
    `1`.

These functions also support an explicit template parameter: so, `.coerce_as<T>(unit)` and
`.coerce_in<T>(unit)`.  If you supply this parameter, it will be the rep of the result.

??? example "Example: simultaneous unit and type conversion"
    `centi(meters_pt)(271.8).coerce_as<int>(meters_pt)` will return `meters_pt(2)`.

!!! tip
    Prefer **not** to use the "coercing versions" if possible, because you will get more safety
    checks.  The risks which the "base" versions warn about are real.

## Operations

Au includes as many common operations as possible.  Our goal is to avoid incentivizing users to
leave the safety of the library.

Recall that for `Quantity`, there are [two main categories](../discussion/concepts/arithmetic.md) of
operation: "arbitrary unit" operations, and "common unit" operations.  However, `QuantityPoint` is
different.  Since multiplication, division, and powers are generally meaningless, we don't have any
"arbitrary unit" operations: every operation is a ["common unit"
operation](../discussion/concepts/arithmetic.md#common-unit).

### Comparison

Comparison is a [common unit operation](../discussion/concepts/arithmetic.md#common-unit).  If the
input `QuantityPoint` types are not identical --- same `Unit` _and_ `Rep` --- then we first convert
them to their [common type](#common-type). Then, we perform the comparison by delegating to the
comparison operator on the underlying values.

We support the following binary comparison operators:

- `==`
- `!=`
- `>`
- `>=`
- `<`
- `<=`

### Addition

Addition between any two `QuantityPoint` instances is not defined, because it is not meaningful ---
this is intrinsic to the [core design of quantity point
types](../discussion/concepts/quantity_point.md).  Addition is only defined between
a `QuantityPoint` and a `Quantity` of the same dimension.

Addition is a [common unit operation](../discussion/concepts/arithmetic.md#common-unit). If the
input `Quantity` and `QuantityPoint` types don't have the same `Unit` _and_ `Rep`, then we first
convert them to their [common types](#common-type) --- that is, we use the common unit and common
rep for each.  The result is a `QuantityPoint` of this common unit and rep, whose value is the sum
of the input values (after conversions).

### Subtraction {#subtraction}

Subtraction is a [common unit operation](../discussion/concepts/arithmetic.md#common-unit).  If the
input `QuantityPoint` types are not identical --- same `Unit` _and_ `Rep` --- then we first convert
them to their [common type](#common-type).  The result is a `Quantity` --- note: **not**
a `QuantityPoint` --- of this common unit and Rep, whose value is the difference of the input values
(after any common type conversions).

### Shorthand addition and subtraction (`+=`, `-=`)

The input must be a `Quantity` which is [implicitly
convertible](./quantity.md#implicit-from-quantity) to the `Unit` and `Rep` of the target
`QuantityPoint` type.  These operations first perform that conversion, and then replace the target
`QuantityPoint` with the result of that addition (for `+=`) or subtraction (for `-=`).

## `rep_cast`

`rep_cast` performs a `static_cast` on the underlying value of a `QuantityPoint`.  It is used to
change the rep.

Given any `QuantityPoint<U, R> p` whose rep is `R`, then `rep_cast<T>(p)` gives a `QuantityPoint<U,
T>`, whose underlying value is `static_cast<T>(p.in(U{}))`.

## Templates and traits

### Matching: `typename U, typename R`

To specialize a template to match any `QuantityPoint`, declare two template parameters: one for the
unit, one for the rep.

??? example "Example: function template"
    ```cpp
    template <typename U, typename R>
    constexpr auto refine_scale(QuantityPoint<U, R> p) {
        return p.as(U{} / mag<10>());
    }
    ```

    This function template will match any `QuantityPoint` specialization, and nothing else.

??? example "Example: type template"
    ```cpp
    // First, we need to declare the generic type template.  It matches a single type, `T`.
    template <typename T>
    struct Size;

    // Now we can declare a specialization that matches any `QuantityPoint`, by templating on its
    // unit and rep.
    template <typename U, typename R>
    struct Size<QuantityPoint<U, R>> {

        // Note: this example uses inline variables, a C++17 feature.  It's just for illustration.
        static constexpr inline std::size_t value = sizeof(R);
    };
    ```

    In this way, `Size<T>::value` will exist only when `T` is some `QuantityPoint` type (unless, of
    course, other specializations get defined elsewhere).

### `::Rep`

Every `QuantityPoint` type has a public static alias, `::Rep`, which indicates its underlying
storage type (or, its "rep").

??? example
    `decltype(meters_pt(5))::Rep` is `int`.

### `::Unit`

Every `QuantityPoint` type has a public static alias, `::Unit`, which indicates its unit type.

??? example
    `decltype(meters_pt(5))::Unit` is `Meters`.

### `::unit`

Every `QuantityPoint` type has a public static member variable, `::unit`, which is an instance of its
unit type.

??? example
    `decltype(meters_pt(5))::unit` is `Meters{}`.

### `std::common_type` specialization {#common-type}

For two `QuantityPoint` types, one with unit `U1` and rep `R1`, and the other with unit `U2` and rep
`R2`, then `std::common_type_t<QuantityPoint<U1, R1>, QuantityPoint<U2, R2>>` has the following properties.

1. It exists if and only if `U1` and `U2` have the same dimension.

2. When it exists, it is `QuantityPoint<U, R>`, where `U` is the [common
   point-unit](../discussion/concepts/common_unit.md#common-quantity-point) of `U1` and `U2`, and
   `R` is `std::common_type_t<R1, R2>`.

As [required by the standard](https://en.cppreference.com/w/cpp/types/common_type), our
`std::common_type` specializations are
[SFINAE](https://en.cppreference.com/w/cpp/language/sfinae)-friendly: improper combinations will
simply not be present, rather than producing a hard error.

### AreQuantityPointTypesEquivalent

**Result:** Indicates whether two `QuantityPoint` types are equivalent.  Equivalent types may be
freely converted to each other, and no arithmetic operations will be performed in doing so.

More precisely, `QuantityPoint<U1, R1>` and `QuantityPoint<U2, R2>` are equivalent if and only if
**both** of the following conditions hold.

1. The units `U1` and `U2` are [point-equivalent](./unit.md#point-equivalent).

2. The reps `R1` and `R2` are the same type.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `AreQuantityPointTypesEquivalent<U1, U2>::value`
