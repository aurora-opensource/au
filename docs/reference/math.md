# Math functions

We provide many common mathematical functions out of the box.  If you think we're missing
a particular math function, and you'd like to see it added, [reach out to us and
ask](https://github.com/aurora-opensource/au/issues)!

## General usage advice

Prefer to _make unqualified calls_ to these functions.  So for example: if you're using unit types
and you want the "max", just write plain `max(...)`.

- Don't write `std::max(...)`, because that would give the wrong function.
- Don't write `au::max(...)`, because that's neither necessary nor idiomatic.

!!! warning
    For some functions, including `min`, `max`, and `clamp`, this advice is _mandatory_ in many
    cases, such as when the arguments have the same type.

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

!!! warning
    You _must_ use _unqualified_ calls to `min` and `max` in many cases, including the common case
    where the arguments have the same type.  Write `min(a, b)`, not `au::min(a, b)`: the latter will
    frequently result in the right overload not being found.

#### `clamp`

"Clamp" the first parameter to the range defined by the second and third.  This is a _unit-aware_
analogue to [`std::clamp`](https://en.cppreference.com/w/cpp/algorithm/clamp), which was introduced
in C++17.  However, this version differs in several respects from `std::clamp`, in order to handle
quantities more effectively.  We'll explain these differences at the end of the `clamp` section.

**Signatures:**

```cpp
// 1. `Quantity` inputs
template <typename UV , typename RV ,
          typename ULo, typename RLo,
          typename UHi, typename RHi>
constexpr auto clamp(
    Quantity<UV , RV > v,
    Quantity<ULo, RLo> lo,
    Quantity<UHi, RHi> hi);

// 2. `QuantityPoint` inputs
template <typename UV , typename RV ,
          typename ULo, typename RLo,
          typename UHi, typename RHi>
constexpr auto clamp(
    QuantityPoint<UV , RV > v,
    QuantityPoint<ULo, RLo> lo,
    QuantityPoint<UHi, RHi> hi);
```

??? note "A note on custom comparators"
    [`std::clamp`](https://en.cppreference.com/w/cpp/algorithm/clamp) includes a four-parameter
    version, where the fourth parameter is a custom comparator.  `std::clamp` provides this because
    it must support an unknowably wide range of custom types.  However, `au::clamp` only supports
    `Quantity` and `QuantityPoint` types, whose notions of comparison is unambiguous.  Therefore, we
    opt to keep the library simple, and omit this four-parameter version.

**Returns:** The closest quantity (or quantity point) to `v` which is between `lo` and `hi`,
inclusive --- that is, greater than or equal to `lo`, and less than or equal to `hi`.  If `lo > hi`,
the behaviour is undefined.  The return type will be expressed in the appropriate unit and rep;
expand the note below for further details.

??? info "Details on the unit and rep for the return type"
    Comparison is a [common-unit operation](../discussion/concepts/arithmetic.md#common-unit).  We
    must convert all inputs to their common unit before we compare, and therefore the output must
    also be expressed in this same common unit.

    The rep of the return type will be the common type of the input reps.  Specifically, given the
    above signatures, it will be `std::common_type_t<RV, RLo, RHi>`.

    The unit of the return type depends on whether we are working with `Quantity` inputs, or
    `QuantityPoint`.

    - For `Quantity`, the return type's unit is `CommonUnitT<UV, ULo, UHi>`.
    - For `QuantityPoint`, the return type's unit is `CommonPointUnitT<UV, ULo, UHi>`: this is the
      [common point unit](../discussion/concepts/common_unit.md#common-quantity-point), which takes
      relative origin offsets into account.

??? info "Differences from `std::clamp`"
    Here are the main changes which stem from handling quantities instead of simple numbers.

    - unlike `std::clamp`, we return by **value**, not by reference.  This is because we support
      combining different units.  This means the return type will generally be different from the
      types of the inputs.

    - The return type **can be different from the type of `v`**, because we must express it in the
      common unit and rep of the input parameter types.

    - We do not currently plan to provide the four-parameter overload, unless we get a compelling
      use case.

!!! warning
    You _must_ use _unqualified_ calls to `clamp` in many cases, including the common case where the
    arguments have the same type.  Write `clamp(a, b, c)`, not `au::clamp(a, b, c)`: the latter will
    frequently result in the right overload not being found.

### Interpolating functions

#### `lerp` (C++20) {#lerp}

!!! warning
    `lerp`, based on [std::lerp], is only available for C++20 and later.

    For the special case where `t = 0.5`, [`mean`](#mean) (see below) presents an alternative.

Linearly interpolate between two `Quantity` or `QuantityPoint` values, based on a parameter `t`,
such that `t=0` corresponds to the first argument, and `t=1` corresponds to the second argument.
That is, `lerp(a, b, t)` is logically equivalent to `a + (b - a) * t`, but with all of the special
case handling found in [std::lerp].

**Signatures:**

```cpp
// 1. `Quantity` inputs
template <typename U1, typename R1, typename U2, typename R2, typename T>
constexpr auto lerp(Quantity<U1, R1> a, Quantity<U2, R2> b, T t);

// 2. `QuantityPoint` inputs
template <typename U1, typename R1, typename U2, typename R2, typename T>
constexpr auto lerp(QuantityPoint<U1, R1> a, QuantityPoint<U2, R2> b, T t);
```

**Returns:** The value notionally equivalent to $a + t(b - a)$, subject to all of the special case
handling outlined in [std::lerp].  The return value will be expressed in the common unit of the
units of the inputs `a` and `b`.

[std::lerp]: https://en.cppreference.com/w/cpp/numeric/lerp

#### `mean` {#mean}

Produce the arithmetic mean of two or more `Quantity` or `QuantityPoint` values.

**Signatures:**

```cpp
// 1. `Quantity` inputs
template <typename U0, typename R0, typename... Us, typename... Rs>
constexpr auto mean(Quantity<U0, R0> q0, Quantity<Us, Rs>... qs);

// 2. `QuantityPoint` inputs
template <typename U0, typename R0, typename... Us, typename... Rs>
constexpr auto mean(QuantityPoint<U0, R0> p0, QuantityPoint<Us, Rs>... ps);
```

**Returns:** The arithmetic mean of all inputs.  The return value will be expressed in the common
unit of the units of the inputs, and the rep will be the common type of the reps of all inputs.

!!! note
    `mean` has overlap with `lerp`: `mean(a, b)` is similar to `lerp(a, b, 0.5)`.  Here is
    a comparison table to help you decide which to use.

    | Criterion | `mean` | `lerp` |
    |-----------|--------|--------|
    | Number of inputs | 2 or more | Exactly 2 |
    | Weights | All equal | Arbitrary |
    | Special case handling | Avoids overflows | Delegates to [std::lerp], which handles many special cases |
    | Approach to integer types | Use integer arithmetic | Delegates to [std::lerp], which always converts to floating point |
    | C++ version compatibility | All versions of Au (C++14 and later) | C++20 and later |

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

#### `sqrt`, `cbrt`

A unit-aware adaptation of [`std::sqrt`](https://en.cppreference.com/w/cpp/numeric/math/sqrt) and
[`std::cbrt`](https://en.cppreference.com/w/cpp/numeric/math/cbrt).  Both the input and output are
`Quantity` types.  Since `sqrt` and `cbrt` are [arbitrary-unit
operations](../discussion/concepts/arithmetic.md#arbitrary-unit), the root applies
_independently_ to the unit and to the value.

We mirror `std::sqrt` and `std::cbrt` in selecting our output rep.  That is to say: the output rep
for `sqrt` and `cbrt` will be the return type of `std::sqrt` or `std::cbrt`, respectively, when
called with a value of our input rep.  For example, if the input quantity has `int` rep, then the
output will be `double`.

**Signature:**

```cpp
template <typename U, typename R>
auto sqrt(Quantity<U, R> q);

template <typename U, typename R>
auto cbrt(Quantity<U, R> q);
```

**Returns:** A `Quantity` whose unit is the square root (for `sqrt`) or cube root (for `cbrt`) of
the input quantity's unit, and whose value is the square root (for `sqrt`) or cube root (for `cbrt`)
of the input quantity's value.

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
    EXPECT_THAT(sin(degrees(30)), DoubleNear(0.5, 1e-15));
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
// round_as(): return a Quantity or QuantityPoint (depending on the input type)
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `round_as(units, quantity)`

// a) For `Quantity` inputs
template <typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, QuantityPoint<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `round_as<Type>(units, quantity)`

// a) For `Quantity` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_as(RoundingUnits rounding_units, QuantityPoint<U, R> q);


//
// round_in(): return a raw number
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `round_in(units, quantity)`

// a) For `Quantity` inputs
template <typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, QuantityPoint<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `round_in<Type>(units, quantity)`

// a) For `Quantity` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto round_in(RoundingUnits rounding_units, QuantityPoint<U, R> q);
```

**Returns:** A `Quantity` or `QuantityPoint` (depending on the input type), expressed in the
requested units, which has an integer value in those units. We return the nearest such quantity to
the original input quantity.

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
// ceil_as(): return a Quantity or QuantityPoint (depending on the input type)
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `ceil_as(units, quantity)`

// a) For `Quantity` inputs
template <typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, QuantityPoint<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `ceil_as<Type>(units, quantity)`

// a) For `Quantity` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_as(RoundingUnits rounding_units, QuantityPoint<U, R> q);


//
// ceil_in(): return a raw number
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `ceil_in(units, quantity)`

// a) For `Quantity` inputs
template <typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, QuantityPoint<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `ceil_in<Type>(units, quantity)`

// a) For `Quantity` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto ceil_in(RoundingUnits rounding_units, QuantityPoint<U, R> q);
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
// floor_as(): return a Quantity or QuantityPoint (depending on the input type)
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `floor_as(units, quantity)`

// a) For `Quantity` inputs
template <typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, QuantityPoint<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `floor_as<Type>(units, quantity)`

// a) For `Quantity` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_as(RoundingUnits rounding_units, QuantityPoint<U, R> q);


//
// floor_in(): return a raw number
//

// 1. Unit-only version (including safety checks).  Typical callsites look like:
//    `floor_in(units, quantity)`

// a) For `Quantity` inputs
template <typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, QuantityPoint<U, R> q);

// 2. Explicit-rep version (overriding; ignores safety checks).  Typical callsites look like:
//    `floor_in<Type>(units, quantity)`

// a) For `Quantity` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, Quantity<U, R> q);

// b) For `QuantityPoint` inputs
template <typename OutputRep, typename RoundingUnits, typename U, typename R>
auto floor_in(RoundingUnits rounding_units, QuantityPoint<U, R> q);
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
// 1. `Quantity` inputs
template <typename U, typename R>
constexpr bool isnan(Quantity<U, R> q);

// 2. `QuantityPoint` inputs
template <typename U, typename R>
constexpr bool isnan(QuantityPoint<U, R> q);
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

A unit-aware adaptation of `std::fmod`, giving the positive remainder of the division of the two
inputs.

As with the [integer modulus](./quantity.md#mod), we first express the inputs in their [common
unit](../discussion/concepts/common_unit.md).

**Signature:**

```cpp
template <typename U1, typename R1, typename U2, typename R2>
auto fmod(Quantity<U1, R1> q1, Quantity<U2, R2> q2);
```

**Returns:** The remainder of `q1 / q2`, in the type `Quantity<U, R>`, where `U` is the common unit
of `U1` and `U2`, and `R` is the common type of `R1` and `R2`.

#### `remainder`

A unit-aware adaptation of `std::remainder`, giving the zero-centered remainder of the division of
the two inputs.

As with the [integer modulus](./quantity.md#mod), we first express the inputs in their [common
unit](../discussion/concepts/common_unit.md).

**Signature:**

```cpp
template <typename U1, typename R1, typename U2, typename R2>
auto remainder(Quantity<U1, R1> q1, Quantity<U2, R2> q2);
```

**Returns:** The remainder of `q1 / q2`, in the type `Quantity<U, R>`, where `U` is the common unit
of `U1` and `U2`, and `R` is the common type of `R1` and `R2`.
