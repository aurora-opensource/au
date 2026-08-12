# Eigen Compatibility

Au works with [Eigen](https://eigen.tuxfamily.org/) out of the box.  You can use Eigen vector and
matrix types as the [rep](./quantity.md) of a `Quantity`, and the core library operations ---
arithmetic, unit conversions, [element access](./quantity.md#element-access), and so on --- will
just work.  See our [Eigen how-to guide](../howto/interop/eigen.md) for an introduction.

This page documents the additional utilities in `"au/compatibility/eigen.hh"`.  Eigen provides much
of its functionality as _member functions_ (`v.norm()`, `v.transpose()`, and so on), and `Quantity`
does not forward arbitrary member calls.  This header fills that gap with unit-aware _free function_
equivalents: for example, `norm(q)` instead of `v.norm()`.  Each function delegates to the
corresponding member function on the underlying rep, and tracks how the operation transforms the
unit.

!!! note
    This header does **not** depend on Eigen: it does not include any Eigen headers, and you can
    include it in any project, in any order, without adding a dependency.  Each function is
    a template that only requires the relevant member function to exist when it's actually used.
    (This also means these functions aren't _intrinsically_ specific to Eigen: they will work with
    any rep whose member functions follow Eigen's conventions.)

    One consequence: since the functions are unconstrained templates, we deliberately follow Eigen's
    `camelCase` naming (`squaredNorm`, `cwiseProduct`, ...) rather than Au's usual `snake_case`, so
    that each Au function reads as the free-function form of the Eigen member it wraps.

Many Eigen operations are _lazy_: instead of computing a result, they return an [expression
template](../discussion/concepts/eigen_safety.md) that refers to its operands.  The
functions on this page preserve that behavior, along with both its performance benefits and its
lifetime risks.  Every lazy function below carries a warning admonition to that effect; functions
without the warning are evaluated eagerly, and their results are always safe to store.

## Forcing evaluation

### `eval` {#eval}

Force evaluation of a `Quantity` whose rep is an Eigen expression template, producing a `Quantity`
of the same unit whose rep owns its own data (for example, a concrete `Eigen::Vector3d`).  This is
the free function form of Eigen's `.eval()` member function, and it is the standard tool for
avoiding expression template lifetime issues.

```cpp
template <typename U, typename R>
auto eval(const Quantity<U, R> &q);
```

??? example "Example: materializing a unit conversion result"
    ```cpp
    auto q = meters(Eigen::Vector3d{1.0, 2.0, 3.0});

    auto lazy = q.as(centi(meters));        // Expression template: refers to `q`.
    auto safe = eval(q.as(centi(meters)));  // Concrete vector: owns its data.
    ```

## Scalar type conversion

### `cast` {#cast}

Cast the rep's _scalar_ type, preserving the unit.  This is the free function form of Eigen's
`.cast<NewScalar>()` member function.

```cpp
template <typename NewScalar, typename U, typename R>
auto cast(const Quantity<U, R> &q);
```

Note that the template parameter is the new _scalar_ type, not the new rep: as in Eigen, the shape
and options of the rep carry over automatically.  So casting a `Quantity<Meters, Eigen::Vector3f>`
with `cast<double>` produces a `Quantity<Meters, Eigen::Vector3d>`.

This is the only way to change the scalar type of an Eigen-backed `Quantity`.  Au's usual tools for
changing the rep --- [`.as<NewRep>(unit)`](./quantity.md#as), which is preferred because it checks
for [conversion risk](../discussion/concepts/conversion_risks.md), and the forcing
[`rep_cast<NewRep>`](./quantity.md#rep_cast) --- both fail here.  `.as<NewRep>(unit)` cannot form
a common rep for two Eigen types with different scalars, and `rep_cast<NewRep>` performs
a `static_cast`, which Eigen _deliberately_ rejects between types with different scalars, directing
users to `.cast<T>()` instead.

!!! warning
    Like every function on this page, `cast` is a pure port of the Eigen member function.  It does
    **not** consult Au's [conversion risk](../discussion/concepts/conversion_risks.md) machinery, so
    a lossy cast is permitted and will silently lose information, exactly as it does in Eigen.  For
    instance, `cast<int>` on a `Quantity<Meters, Eigen::Vector3d>` holding `{1.5, -2.5, 3.9}` yields
    `{1, -2, 3}`.

??? example "Example: widening a float vector to double"
    ```cpp
    auto q = meters(Eigen::Vector3f{1.5f, 2.5f, 3.5f});

    auto widened = eval(cast<double>(q));  // Quantity<Meters, Eigen::Vector3d>
    ```

--8<-- "eigen-lifetime-risk-lazy.md"

When `NewScalar` is already the rep's scalar type, Eigen returns a reference to the original rather
than an expression.  `Quantity` stores a decayed copy of that reference, so this case is always safe
to store without `eval()`.

## Reductions

These operations reduce a vector or matrix to a single scalar.  They are all evaluated eagerly, so
their results are always safe to store.

### `norm`

The Euclidean (L2) norm.  The result has the same unit as the input.

```cpp
template <typename U, typename R>
auto norm(const Quantity<U, R> &q);
```

### `squaredNorm`

The squared Euclidean norm (the sum of squared coefficients).  The result unit is the _square_ of
the input unit.  Use this instead of `norm` when you can, to avoid a square root.

```cpp
template <typename U, typename R>
auto squaredNorm(const Quantity<U, R> &q);
```

### `stableNorm`, `blueNorm`, `hypotNorm`

Numerically robust alternatives to `norm`, corresponding to the Eigen member functions of the same
names.  Each result has the same unit as the input.

```cpp
template <typename U, typename R>
auto stableNorm(const Quantity<U, R> &q);

template <typename U, typename R>
auto blueNorm(const Quantity<U, R> &q);

template <typename U, typename R>
auto hypotNorm(const Quantity<U, R> &q);
```

### `lpNorm`

The $L^p$ norm, for a compile-time integer `p`.  Pass `Eigen::Infinity` for the maximum-absolute
norm.  The result has the same unit as the input.

```cpp
template <int p, typename U, typename R>
auto lpNorm(const Quantity<U, R> &q);
```

### `sum`, `mean`, `minCoeff`, `maxCoeff`, `trace`

The sum of all coefficients; the mean of all coefficients; the smallest coefficient; the largest
coefficient; and the trace (the sum of the diagonal coefficients).  Each result has the same unit as
the input.

```cpp
template <typename U, typename R>
auto sum(const Quantity<U, R> &q);

template <typename U, typename R>
auto mean(const Quantity<U, R> &q);

template <typename U, typename R>
auto minCoeff(const Quantity<U, R> &q);

template <typename U, typename R>
auto maxCoeff(const Quantity<U, R> &q);

template <typename U, typename R>
auto trace(const Quantity<U, R> &q);
```

### `prod`

The product of all coefficients.  The result unit is the input unit raised to the power of the
_number of coefficients_.

Because that unit must be computed at compile time, this function requires a **fixed-size** operand:
you will get a `static_assert` failure for dynamic-size types.

```cpp
template <typename U, typename R>
auto prod(const Quantity<U, R> &q);
```

### `determinant`

The determinant.  For an `N`-by-`N` matrix, the result unit is the input unit raised to the power
`N`.

Because that unit must be computed at compile time, this function requires a **fixed-size** operand:
you will get a `static_assert` failure for dynamic-size types.

```cpp
template <typename U, typename R>
auto determinant(const Quantity<U, R> &q);
```

### `dot`

The dot (inner) product.  When both operands are `Quantity`, the result unit is the _product_ of the
operand units.  We also provide overloads where one operand is a raw (that is, dimensionless) Eigen
object; there, the result carries the unit of the `Quantity` operand.

```cpp
template <typename U1, typename R1, typename U2, typename R2>
auto dot(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b);

template <typename U, typename R, typename V>
auto dot(const Quantity<U, R> &a, const V &b);

template <typename V, typename U, typename R>
auto dot(const V &a, const Quantity<U, R> &b);
```

## Vector geometry

### `cross`

The cross product.  When both operands are `Quantity`, the result unit is the _product_ of the
operand units.  We also provide overloads where one operand is a raw (that is, dimensionless) Eigen
object; there, the result carries the unit of the `Quantity` operand.

```cpp
template <typename U1, typename R1, typename U2, typename R2>
auto cross(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b);

template <typename U, typename R, typename V>
auto cross(const Quantity<U, R> &a, const V &b);

template <typename V, typename U, typename R>
auto cross(const V &a, const Quantity<U, R> &b);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `normalized`

The normalized (unit-length) vector.  Evaluated eagerly.

Uniquely on this page, the result is a _raw Eigen vector_, not a `Quantity`.  The reason is that
this result is unitless _by definition of the operation, for all inputs_: the norm always carries
the operand's unit, so the units cancel exactly.  Since there is no operand-dependent unit for
a `Quantity` wrapper to track, we return the raw vector directly.

```cpp
template <typename U, typename R>
auto normalized(const Quantity<U, R> &q);
```

## Coefficient-wise operations

### `cwiseProduct`

The coefficient-wise (Hadamard) product.  When both operands are `Quantity`, the result unit is the
_product_ of the operand units.  We also provide overloads where one operand is a raw (that is,
dimensionless) Eigen object; there, the result carries the unit of the `Quantity` operand.

```cpp
template <typename U1, typename R1, typename U2, typename R2>
auto cwiseProduct(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b);

template <typename U, typename R, typename V>
auto cwiseProduct(const Quantity<U, R> &a, const V &b);

template <typename V, typename U, typename R>
auto cwiseProduct(const V &a, const Quantity<U, R> &b);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `cwiseQuotient`

The coefficient-wise quotient.  When both operands are `Quantity`, the result unit is the _quotient_
of the operand units.  We also provide overloads where one operand is a raw (that is, dimensionless)
Eigen object; there, the result unit is the `Quantity` operand's unit (if it's the numerator), or
its inverse (if it's the denominator).

```cpp
template <typename U1, typename R1, typename U2, typename R2>
auto cwiseQuotient(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b);

template <typename U, typename R, typename V>
auto cwiseQuotient(const Quantity<U, R> &a, const V &b);

template <typename V, typename U, typename R>
auto cwiseQuotient(const V &a, const Quantity<U, R> &b);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `cwiseAbs`

The coefficient-wise absolute value.  The result has the same unit as the input.

```cpp
template <typename U, typename R>
auto cwiseAbs(const Quantity<U, R> &q);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `cwiseSqrt`

The coefficient-wise square root.  The result unit is the _square root_ of the input unit.

```cpp
template <typename U, typename R>
auto cwiseSqrt(const Quantity<U, R> &q);
```

--8<-- "eigen-lifetime-risk-lazy.md"

## Views and accessors

Every operation in this section preserves the unit of its input.

### `transpose`

The transpose.

```cpp
template <typename U, typename R>
auto transpose(const Quantity<U, R> &q);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `diagonal`

The main diagonal; or, with an `index` argument, the `index`-th diagonal (positive for
super-diagonals, negative for sub-diagonals).

```cpp
template <typename U, typename R>
auto diagonal(const Quantity<U, R> &q);

template <typename U, typename R>
auto diagonal(const Quantity<U, R> &q, std::ptrdiff_t index);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `row`, `col`

The `i`-th row, or the `j`-th column.

```cpp
template <typename U, typename R>
auto row(const Quantity<U, R> &q, std::ptrdiff_t i);

template <typename U, typename R>
auto col(const Quantity<U, R> &q, std::ptrdiff_t j);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `reverse`

The coefficients in reverse order.

```cpp
template <typename U, typename R>
auto reverse(const Quantity<U, R> &q);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `conjugate`

The complex conjugate (a no-op for real reps).

```cpp
template <typename U, typename R>
auto conjugate(const Quantity<U, R> &q);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `head`, `tail`, `segment`

Portions of a vector: the first `N` (or `n`) coefficients; the last `N` (or `n`) coefficients; or
a length-`N` (or `n`) portion starting at `start`.  As in Eigen, sizes given as template arguments
are fixed at compile time, and sizes given as function arguments are dynamic.

```cpp
template <int N, typename U, typename R>
auto head(const Quantity<U, R> &q);

template <typename U, typename R>
auto head(const Quantity<U, R> &q, std::ptrdiff_t n);

template <int N, typename U, typename R>
auto tail(const Quantity<U, R> &q);

template <typename U, typename R>
auto tail(const Quantity<U, R> &q, std::ptrdiff_t n);

template <int N, typename U, typename R>
auto segment(const Quantity<U, R> &q, std::ptrdiff_t start);

template <typename U, typename R>
auto segment(const Quantity<U, R> &q, std::ptrdiff_t start, std::ptrdiff_t n);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `block`

A `Rows`-by-`Cols` (or `rows`-by-`cols`) block of a matrix, whose top-left corner is at (`i`, `j`).
As in Eigen, sizes given as template arguments are fixed at compile time, and sizes given as
function arguments are dynamic.

```cpp
template <int Rows, int Cols, typename U, typename R>
auto block(const Quantity<U, R> &q, std::ptrdiff_t i, std::ptrdiff_t j);

template <typename U, typename R>
auto block(const Quantity<U, R> &q,
           std::ptrdiff_t i,
           std::ptrdiff_t j,
           std::ptrdiff_t rows,
           std::ptrdiff_t cols);
```

--8<-- "eigen-lifetime-risk-lazy.md"

### `replicate`

The input, tiled into a `RowFactor`-by-`ColFactor` (or `row_factor`-by-`col_factor`) grid of copies.
As in Eigen, factors given as template arguments are fixed at compile time, and factors given as
function arguments are dynamic.

```cpp
template <int RowFactor, int ColFactor, typename U, typename R>
auto replicate(const Quantity<U, R> &q);

template <typename U, typename R>
auto replicate(const Quantity<U, R> &q, std::ptrdiff_t row_factor, std::ptrdiff_t col_factor);
```

--8<-- "eigen-lifetime-risk-lazy.md"

## Matrix operations

### `inverse`

The matrix inverse.  The result unit is the _inverse_ of the input unit (so that the product of
a matrix and its inverse is dimensionless).

```cpp
template <typename U, typename R>
auto inverse(const Quantity<U, R> &q);
```

--8<-- "eigen-lifetime-risk-lazy.md"
