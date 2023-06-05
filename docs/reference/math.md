# Math functions

We provide many common mathematical functions out of the box.  If you think we're missing
a particular math function, and you'd like to see it added, [reach out to us and
ask](https://github.com/aurora-opensource/au/issues)!

## General usage advice

Prefer to _make unqualified calls_ to these functions.  So for example: if you're using unit types
and you want the "max", just write plain `max(...)`.

- Don't write `std::max(...)`, because that would give the wrong function.
- Don't write `au::max(...)`, because that's neither necessary nor idiomatic.

## Function categories

Here are the functions we provide, grouped roughly into related categories.

### Sign-based functions:

#### Checking signs, comparing to 0

`Quantity` cannot be compared to `0` or `0.0`, since these are raw numeric types.  However, any
`Quantity` _can_ be compared to `ZERO`, which is a built-in constant of the library.  See [our
`Zero` discussion](../discussion/concepts/zero.md) for more background.

#### `abs`

Adapts `std::abs` to `Quantity` types.  Covers both
[integral](https://en.cppreference.com/w/cpp/numeric/math/abs) and [floating
point](https://en.cppreference.com/w/cpp/numeric/math/fabs) overloads of `std::abs`.

**Signature:**

```cpp
template <typename U, typename R>
auto abs(Quantity<U, R> q);
```

**Returns:** The input quantity, but with `std::abs` applied to its underlying value.

#### `copysign`

Adapts [`std::copysign`](https://en.cppreference.com/w/cpp/numeric/math/copysign) to
`Quantity` types.

**Signatures:**

```cpp
// 1: First argument Quantity, second argument raw numeric
template <typename U, typename R, typename T>
constexpr auto copysign(Quantity<U, R> mag, T sgn);

// 2: First argument raw numeric, second argument Quantity
template <typename T, typename U, typename R>
constexpr auto copysign(T mag, Quantity<U, R> sgn);

// 3: Both arguments Quantity
template <typename U1, typename R1, typename U2, typename R2>
constexpr auto copysign(Quantity<U1, R1> mag, Quantity<U2, R2> sgn);
```

**Returns:** The first argument, with the sign from the second argument applied to it.

### Comparison-based functions

#### `min`, `max`

Select the smaller (`min`) or larger (`max`) of the two inputs.  This operation is _unit-aware_, and
supports mixing different input units, as long as they have the same dimension.  These functions
support both `Quantity` and `QuantityPoint` inputs.

**Signatures:**[^1]

```cpp
//
// min()
//

// 1. `Quantity` inputs
template <typename U1, typename U2, typename R1, typename R2>
auto min(Quantity<U1, R1> q1, Quantity<U2, R2> q2);

// 2. `QuantityPoint` inputs
template <typename U1, typename U2, typename R1, typename R2>
auto min(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2);

//
// max()
//

// 1. `Quantity` inputs
template <typename U1, typename U2, typename R1, typename R2>
auto max(Quantity<U1, R1> q1, Quantity<U2, R2> q2);

// 2. `QuantityPoint` inputs
template <typename U1, typename U2, typename R1, typename R2>
auto max(QuantityPoint<U1, R1> p1, QuantityPoint<U2, R2> p2);
```

[^1]: These signatures are for purposes of illustration, not completeness.  In the real code, there
are additional signatures covering the case of identical inputs.  We need these in order to
disambiguate our `min` or `max` implementations with respect to `std::min` and `std::max`.

**Returns:** The value of the smallest (`min`) or largest (`max`) of the inputs, expressed in their
[common type](./quantity.md#common-type).

!!! note
    unlike `std::min` and `std::max`, we return by **value**, not by reference.  This is because we
    support combining different units.  This means the return type will generally be different from
    the types of the inputs.

### Exponentiation

#### `int_pow`

Raise a `Quantity` to an integer power.  Since this is an [arbitrary-unit
operation](../discussion/concepts/arithmetic.md#arbitrary-unit), the power applies _independently_
to the unit and to the value.

If the input has an integral rep (storage type), then the exponent cannot be negative.

**Signature:**

```cpp
template <int Exp, typename U, typename R>
constexpr auto int_pow(Quantity<U, R> q);
```

**Returns:**  A `Quantity` whose unit is the input unit raised to the given power, and whose value
is the input value raised to the given power.

#### `sqrt`

A unit-aware adaptation of [`std::sqrt`](https://en.cppreference.com/w/cpp/numeric/math/sqrt).  Both
the input and output are `Quantity` types.  Since `sqrt` is an [arbitrary-unit
operation](../discussion/concepts/arithmetic.md#arbitrary-unit), the square root applies
_independently_ to the unit and to the value.

We mirror `std::sqrt` in selecting our output rep.  That is to say: the output rep will be the
return type of `std::sqrt` when called with a value of our input rep.

**Signature:**

```cpp
template <typename U, typename R>
auto sqrt(Quantity<U, R> q);
```

**Returns:** A `Quantity` whose unit is the square root of the input quantity's unit, and whose
value is the square root of the input quantity's value.

??? warning "Warning: not all unit conversions are currently supported"
    There is one edge case to be aware of with `sqrt`: we don't yet support any **conversion** which
    picks up a radical factor.  This is because all conversion factors get computed at compile time,
    and we don't have a way to compute rational powers at compile time.  To fix this, we would need
    a `constexpr`-compatible implementation of `std::powl`.

    Let's clarify what you can and can't do in today's library, with an example.

    ```cpp
    // Taking the square root of "weird" units: this works.
    const auto geo_mean_length = sqrt(inches(1) * meters(1));

    // Now let's look at retrieving the value in different units.

    // Using a Quantity-equivalent Unit just retrieves the stored value.
    // This _always_ works.  (In this case, it gives `1.0`.)
    const auto retrieved_value = geo_mean_length.in(sqrt(inch * meters));

    // This conversion is non-trivial, but it's also OK.
    // The reason is that the conversion factor doesn't have any rational powers.
    // (In this case, it gives `10.0`.)
    const auto rationally_converted_value = geo_mean_length.in(sqrt(inch * centi(meters)));

    // This test case doesn't currently work.
    // Later, if we can compute radical conversion factors at compile time, it will.
    // (It should give roughly 6.274558...)
    // const auto radically_converted_value = geo_mean_length.in(inches);
    ```

### Trigonometric functions

#### `sin`, `cos`, `tan`

The value of the named trigonometric function ($\sin$, $\cos$, or $\tan$), evaluated at an input
`Quantity` representing an angle.

If called with any `Quantity` which is not an angle, we produce a hard compiler error.

**Signatures:**

```cpp
//
// sin()
//
template <typename U, typename R>
auto sin(Quantity<U, R> q);

//
// cos()
//
template <typename U, typename R>
auto cos(Quantity<U, R> q);

//
// tan()
//
template <typename U, typename R>
auto tan(Quantity<U, R> q);
```

**Returns:** The result of converting the input to `Radians`, and then calling the corresponding STL
function (that is, `std::sin()` for `sin()`, and so on).

In converting to radians, we mirror the corresponding STL functions in how we handle the rep.  For
floating point rep (`float`, `double`, and so on), the return type is the `rep`.  For integral
inputs (`int`, `uint32_t`, and so on), we cast to `double` and return `double`.  See, for instance,
the [`std::sin` documentation](https://en.cppreference.com/w/cpp/numeric/math/sin).

??? example "Example: using angles of integer degrees"
    This example is taken from a test case in the library.

    ```cpp
    EXPECT_NEAR(sin(degrees(30)), 0.5, 1e-15);
    ```

#### `arcsin`, `arccos`, `arctan`

The standard inverse trigonometric functions, each returning a `Quantity` of `Radians`.

Each function corresponds to an STL function, except with `arc` replaced by `a`.  For example,
`arcsin()` corresponds to `std::asin()`, and so on.  This library's functions return
`Quantity<Radians, T>` whenever the corresponding STL function would return a `T`.

Their names are slightly different than the corresponding STL functions, because in C++ it's
impermissible to have two functions whose signatures differ only in their return type.

!!! note
    For more flexibility and robustness in dealing with arctangent use cases, see
    [`arctan2`](#arctan2) below.

**Signatures:**

```cpp
//
// arcsin()
//
template <typename T>
auto arcsin(T x);

//
// arccos()
//
template <typename T>
auto arccos(T x);

//
// arctan()
//
template <typename T>
auto arctan(T x);
```

**Returns:** `radians(stl_func(x))`, where `stl_func` is the corresponding STL function (that is,
`std::acos()` for `arccos()`, and so on).

??? example "Example: getting the result in degrees"
    The fact that we return a `Quantity`, not a raw number, makes these functions far more flexible
    than their STL counterparts.  For example, it's easy to get the result in degrees using fluent,
    readable code:

    ```cpp
    // It's easy to express the answer in your preferred angular units.
    const auto angle = arcsin(0.5).as(degrees);

    // This test verifies that the result has the value we'd expect from basic trigonometry.
    constexpr auto TOL = degrees(1e-12);
    EXPECT_THAT(angle, IsNear(degrees(30.0), TOL));
    ```

#### `arctan2` {#arctan2}

The two-argument arctangent function, which determines the in-plane angle based on the $y$ and $x$
coordinates of a point in the plane.

`arctan2` corresponds to `std::atan2`, but returns a `Quantity` of `Radians` instead of a raw
number.

This two-argument version is more robust than the single-argument version.  `arctan2(y, x)` is
equivalent to `arctan(y / x)`, but it avoids the problems faced by the latter whenever `x` is zero.

Unlike the other inverse trigonometric functions, which only support raw numeric inputs, `arctan2`
also supports `Quantity` inputs.  These inputs must have the same dimension, or else we will produce
a hard compiler error.  We convert them to their common unit, if necessary, before delegating to
`std::atan2`.

**Signatures:**

```cpp
// 1. Raw numeric inputs.
template <typename T, typename U>
auto arctan2(T y, U x);

// 2. Quantity inputs (must be same dimension).
template <typename U1, typename R1, typename U2, typename R2>
auto arctan2(Quantity<U1, R1> y, Quantity<U2, R2> x);
```

**Returns:** `radians(std::atan2(y, x))`.  If the inputs are `Quantity` types, then instead of
passing `y` and `x`, we first convert them to their common unit, and pass their values in that unit.

### Rounding functions

#### `round_as`, `round_in`

Round a `Quantity` to the nearest integer value, using units that are specified explicitly at the
callsite.

These functions are intended as unit-aware analogues to
[`std::round`](https://en.cppreference.com/w/cpp/numeric/math/round).  However, we firmly oppose the
idea of providing the same (single-argument) API as `std::round` for `Quantity`, because a quantity
has no single well-defined result: it depends on the units.  (For example, `std::round(height)` is
an intrinsically ill-formed concept: what is an "integer height"?)

As with everything else in the library, `"as"` is a word that means "return a `Quantity`", and
`"in"` is a word that means "return a raw number".

**Signatures:**

```cpp
//
// round_as(): return a Quantity
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `round_as(units, quantity)`
template <typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `round_as<Type>(units, quantity)`
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q);


//
// round_in(): return a raw number
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `round_in(units, quantity)`
template <typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `round_in<Type>(units, quantity)`
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q);
```

**Returns:** A `Quantity`, expressed in the requested units, which has an integer value in those
units.  We return the nearest such quantity to the original input quantity.

The policy for the rep is consistent with
[`std::round`](https://en.cppreference.com/w/cpp/numeric/math/round).  The output rep is the same as
the return type of applying `std::round` to the input rep.

#### `ceil_in`, `ceil_as`

Round a `Quantity` up to the smallest integer value which is at least as big as that quantity, using
units that are specified explicitly at the callsite.

These functions are intended as unit-aware analogues to
[`std::ceil`](https://en.cppreference.com/w/cpp/numeric/math/ceil).  However, we firmly oppose the
idea of providing the same (single-argument) API as `std::ceil` for `Quantity`, because a quantity
has no single well-defined result: it depends on the units.  (For example, `std::ceil(height)` is an
intrinsically ill-formed concept: what is an "integer height"?)

As with everything else in the library, `"as"` is a word that means "return a `Quantity`", and
`"in"` is a word that means "return a raw number".

**Signatures:**

```cpp
//
// ceil_as(): return a Quantity
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `ceil_as(units, quantity)`
template <typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `ceil_as<Type>(units, quantity)`
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q);


//
// ceil_in(): return a raw number
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `ceil_in(units, quantity)`
template <typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `ceil_in<Type>(units, quantity)`
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q);
```

**Returns:** A `Quantity`, expressed in the requested units, which has an integer value in those
units.  We return the smallest such quantity which is no smaller than the original input quantity.

The policy for the rep is consistent with
[`std::ceil`](https://en.cppreference.com/w/cpp/numeric/math/ceil).  The output rep is the same as
the return type of applying `std::ceil` to the input rep.

#### `floor_in`, `floor_as`

Round a `Quantity` down to the largest integer value which is no bigger than that quantity, using
the units that are specified explicitly at the callsite.

These functions are intended as unit-aware analogues to
[`std::floor`](https://en.cppreference.com/w/cpp/numeric/math/floor).  However, we firmly oppose the
idea of providing the same (single-argument) API as `std::floor` for `Quantity`, because a quantity
has no single well-defined result: it depends on the units.  (For example, `std::floor(height)` is an
intrinsically ill-formed concept: what is an "integer height"?)

As with everything else in the library, `"as"` is a word that means "return a `Quantity`", and
`"in"` is a word that means "return a raw number".

**Signatures:**

```cpp
//
// floor_as(): return a Quantity
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `floor_as(units, quantity)`
template <typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `floor_as<Type>(units, quantity)`
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q);


//
// floor_in(): return a raw number
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `floor_in(units, quantity)`
template <typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `floor_in<Type>(units, quantity)`
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q);
```

**Returns:** A `Quantity`, expressed in the requested units, which has an integer value in those
units.  We return the largest such quantity which is no larger than the original input quantity.

The policy for the rep is consistent with
[`std::floor`](https://en.cppreference.com/w/cpp/numeric/math/floor).  The output rep is the same as
the return type of applying `std::floor` to the input rep.

### Inverse functions

#### `inverse_as`, `inverse_in`

A unit-aware computation of $1 / x$.

"Unit-aware" means that you specify the desired target unit, and the library will figure out the
appropriate units for representing the $1$ in $1 / x$.  This intelligent choice enables it to
automatically handle many conversions with integer types, without the computation ever needing to
leave the integer domain.

??? example "Example: inverse of $250 \,\text{Hz}$"
    The inverse of $250 \,\text{Hz}$ is $0.004 \,\text{s}$.  If we are using integer types, of
    course this would truncate down to $0$. However, we could choose an alternate unit --- say,
    $\text{µs}$ --- and we would get a "nicer" answer of $4000 \,\text{µs}$.

    Now for Au.  If you request the inverse of `hertz(250)` in `micro(seconds)`, the library will
    indeed return `micro(seconds)(4000)` --- and it can perform this computation without ever
    leaving the integer domain!  What happens under the hood is that the value of `250` is divided
    into a value of `1'000'000`, not `1`.

    To see how we came up with this value, let's re-express the fundamental equation.  Let $x$ be
    the original quantity, and $y$ its inverse.  We have:

    $$
    \begin{align}
    y &= 1 / x \\
    1 &= xy
    \end{align}
    $$

    This is a quantity equation.  And since multiplication is an [arbitrary-unit
    operation](../discussion/concepts/arithmetic.md#arbitrary-unit), we can _reason independently
    about the unit_ and the value.  The units on the right hand side are `hertz * micro(seconds)`.
    This is a dimensionless unit with a magnitude of $10^{-6}$.  The units on the left hand side
    must match; therefore, we must express $1$ in these units.  When we do, we find its value in
    these units is $10^6$ --- or, in C++ code, `1'000'000`.

    That is how the library knows to divide `250` into `1'000'000` to get an answer of `4'000` ---
    all without ever leaving the integer domain.

These functions include safety checks.

- `Quantity` inputs with floating point rep are always allowed.
- `Quantity` inputs with integral rep are allowed only when the product of the input and target
  units --- which is necessarily dimensionless --- has a magnitude not greater than $10^{-6}$.  We
  chose this threshold because it means that the round-trip double inversion will be lossless for
  any `Quantity` whose underlying value is not greater than `1'000`.

As with all other library functions, you can circumvent the safety checks by using one of the
"explicit-rep" versions, which are forcing in the same way as `static_cast`.

**Signatures:**

```cpp
//
// inverse_as(): return a Quantity
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `inverse_as(units, quantity)`
template <typename TargetUnits, typename U, typename R>
constexpr auto inverse_as(TargetUnits target_units, Quantity<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `inverse_as<Type>(units, quantity)`
template <typename TargetRep, typename TargetUnits, typename U, typename R>
constexpr auto inverse_as(TargetUnits target_units, Quantity<U, R> q);


//
// inverse_in(): return a raw number
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `inverse_in(units, quantity)`
template <typename TargetUnits, typename U, typename R>
constexpr auto inverse_in(TargetUnits target_units, Quantity<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `inverse_in<Type>(units, quantity)`
template <typename TargetRep, typename TargetUnits, typename U, typename R>
constexpr auto inverse_in(TargetUnits target_units, Quantity<U, R> q);
```

**Returns:** The inverse of the input `Quantity`, expressed in the requested units.

### Special values and language features

#### `isnan`

Indicates whether the underlying value of a `Quantity` is a NaN ("not-a-number") value.

**Signature:**

```cpp
template <typename U, typename R>
constexpr bool isnan(Quantity<U, R> q);
```

**Returns:** `true` if `q` is NaN; `false` otherwise.

#### `std::numeric_limits` specializations

Specializations for `std::numeric_limits<Quantity<...>>`.

For any `Quantity<UnitT, Rep>`, we simply delegate to `std::numeric_limits<Rep>` in the appropriate
way, being careful to follow the [rules for specializing], and adapt the result we get.  For
example, `std::numeric_limits<Quantity<Hours, int>>::max()` is exactly equal to
`hours(std::numeric_limits<int>::max())`.

!!! warning
    Be careful about using these limits in the presence of **different** Units of the same
    **Dimension**.  Comparison operations will _compile_, but may not do what you expect.  Consider
    this example:

    ``` C++
    seconds(0) < std::numeric_limits<Quantity<Hours, int>>::max()
    ```

    Clearly, we'd want this to be `true`... but, in converting both sides to their common type, we'd
    end up multiplying the max-int on the right by 3600.  What answer would we get for the
    comparison?  It's far from clear.

    If you use these for a **single** `Quantity` type (i.e., same Unit and Rep), they should be just
    fine.  (Then again---perhaps this is a good opportunity to ask yourself what you're _really_
    trying to accomplish, and whether using the largest finite value of a particular type is the
    best way to achieve it!)

[rules for specializing]: https://en.cppreference.com/w/cpp/language/extending_std
[round]: https://en.cppreference.com/w/cpp/numeric/math/round


<script src="../../assets/hrh4.js" async=false defer=false></script>
### Miscellaneous

#### `fmod`

A unit-aware adaptation of `std::fmod`, giving the remainder of the division of the two inputs.

As with the [integer modulus](./quantity.md#mod), we first express the inputs in their [common
unit](../discussion/concepts/common_unit.md).

**Signature:**

```cpp
template <typename U1, typename R1, typename U2, typename R2>
auto fmod(Quantity<U1, R1> q1, Quantity<U2, R2> q2);
```

**Returns:** The remainder of `q1 / q2`, in the type `Quantity<U, R>`, where `U` is the common unit
of `U1` and `U2`, and `R` is the common type of `R1` and `R2`.

