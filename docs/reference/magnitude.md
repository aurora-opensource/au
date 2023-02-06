# Magnitude

`Magnitude` is a family of [monovalue types](./detail/monovalue_types.md) representing positive real
numbers.  These values can be multiplied, divided, and raised to (rational) powers, and this
arithmetic always takes place at compile time.  Values can also be converted to more standard
numeric types, such as `double` and `int`, as long as the receiving type can represent the
magnitude's value faithfully.

The core motivation is to represent ratios of different units that have the same dimension.  As
a corollary, any unit can be scaled by a `Magnitude` to make a new unit of the same dimension.

## Forming magnitudes

There are 3 **valid** ways for end users to form a `Magnitude` instance.

1. :heavy_check_mark: Using the `mag<N>()` helper to form the canonical representation of the
   integer `N`.
2. :heavy_check_mark: Writing `Magnitude<MyConstant>{}`, where `MyConstant` is a valid _irrational
   magnitude base_.  (See the _custom bases_ section below for more details.)
3. :heavy_check_mark: Forming products, quotients, powers, and roots of other valid `Magnitude`
   instances.

End users can also use pre-formed `Magnitude` instances from the library, such as `PI` and `ONE`.

The following is a **valid, but dis-preferred way** to form a `Magnitude`.

- :warning: `Magnitude<>`.
    - **Explanation:**  This represents the number 1, but it's less readable than writing
      `mag<1>()`.

The following are **not valid** ways to form a `Magnitude`.

- :x: `Magnitude<Pi, MyConstant>`.
    - **Explanation:** Do not supply a manual sequence of template parameters.  `Magnitude` has
      strict ordering requirements on its template parameters.  The approved methods listed above
      are guaranteed to satisfy these requirements.
- :x: `Magnitude<Prime<3>>`.
    - **Explanation:** Do not supply integer bases manually.  Integers are represented by their
      prime factorization, which is performed automatically.  Instead, form integers, rationals, and
      their powers only by starting with valid `Magnitude` instances, and performing arithmetic
      operations as in option 3 above.

Below, we give more details on several concepts mentioned above.

### `mag<N>()`

`mag<N>()` gives an instance of the unique, canonical `Magnitude` type that represents the positive
integer `N`.

??? info "More detail on integral `Magnitude` representations"
    Integers are stored as their prime factorization.  For example, `18` would be stored as the type
    `Magnitude<Prime<2>, Pow<Prime<3>, 2>>`, because $18 = 2 \cdot 3^2$.

    `mag<N>()` automatically performs the prime factorization of `N`, and constructs a well-formed
    `Magnitude`.

### Custom bases

`Magnitude` can handle some irrational numbers.  This even includes some transcendental numbers,
such as $\pi$.  Because `Magnitude` is closed under products and rational powers, this means that we
also automatically support related values such as $\pi^2$, $\frac{1}{\sqrt{2\pi}}$, and so on.

??? question "What irrational numbers can `Magnitude` _not_ handle?"
    A common example is any that are formed by addition.  For example, $(1 + \sqrt{2})$ cannot be
    represented by `Magnitude`.  Recall that `Magnitude` is designed to support products and
    rational powers, since these are the most important operations in quantity calculus.

    It is tempting to want a better representation --- one which supports full symbolic algebra.
    Perhaps such a representation could be designed.  However, we haven't seen any real world use
    cases for it.  The current `Magnitude` implementation already handles the most critical use
    cases, such as handling $\pi$, which most units libraries have traditionally struggled to
    support.

Because of its importance for angular variables, $\pi$ is supported natively in the library --- you
don't need to define it yourself.  The constant `PI` is a `Magnitude` _instance_.  It's based on the
(natively included) irrational magnitude base, `Pi`.  (Concretely: `PI` is defined as
`Magnitude<Pi>{}`, in accordance with option 2 above.)

If you need to represent an irrational number which can't be formed via any product of powers of the
existing `Magnitude` types --- namely, integers and $\pi$ --- then you can define a new irrational
magnitude base.  This is a `struct` with the following member:

- `static constexpr long double value()`: the best approximation of your constant's value in the
  `long double` storage type.

??? warning "Important information for defining your own constant"
    If you return a literal, you must add `L` on the end.  Otherwise it will be interpreted as
    `double`, and will lose precision.

    Here are the results of one example which was run on an arbitrary development machine.

    | | No suffix | `L` suffix |
    |-|------------------------|-------------------------|
    | Literal | `3.141592653589793238` | `3.141592653589793238L` |
    | Actual Value | `3.141592653589793115` | `3.141592653589793238` |

    The un-suffixed version has lost several digits of precision.  (The precise amount will depend
    on the computer architecture being used.)

Each time you add a new irrational magnitude base, you must make sure that it's **independent:**
that is, that it can't be formed as any product of rational powers of existing `Magnitude` types.

## Extracting values

As a [monovalue type](./detail/monovalue_types.md), `Magnitude` can only hold one value.  There are
no computations we can perform at runtime; everything happens at compile time.  What we _can_ do is
to extract that represented value, and store it in a more conventional numeric type, such as `int`
or `double`.

To extract the value of a `Magnitude` instance `m` into a given numeric type `T`, call
`get_value<T>(m)`.  Here are some important aspects of this utility.

1. The computation takes place completely at compile time.
2. The computation takes place in the widest type of the same kind.  (That is, when `T` is floating
   point we use `long double`, and when `T` is integral we use `std::intmax_t` or `std::uintmax_t`
   according to the signedness of `T`.)
3. If `T` cannot hold the value represented by `m`, we produce a compile time error.

??? example "Example: `float` and $\pi^3$"
    Suppose you are running on an architecture which has hardware support for `float`, but uses slow
    software emulation for `double` and `long double`.  With `Magnitude` and `get_value`, you can
    get the best of both worlds:

    - The **computation** gets performed _at compile time_ in `long double`, giving extra precision.
    - The **result** gets cast to `float` and stored as a program constant.

    Thus, `get_value<float>(pow<3>(PI))` will be much more accurate than storing $\pi$ in a `float`,
    and cubing it --- yet, there will be no loss in performance.

## Operations

These are the operations which `Magnitude` supports.  Because it is a [monovalue
type](./detail/monovalue_types.md), the value can take the form of either a _type_ or an _instance_.
In what follows, we'll use this convention:

- **Capital** identifiers (`M`, `M1`, `M2`, ...) refer to **types**.
- **Lowercase** identifiers (`m`, `m1`, `m2`, ...) refer to **instances**.

### Equality comparison

**Result:** A `bool` indicating whether two `Magnitude` values represent the same number.

**Syntax:**

- For _types_ `M1` and `M2`:
    - `std::is_same<M1, M2>::value`
- For _instances_ `m1` and `m2`:
    - `m1 == m2` (equality comparison)
    - `m1 != m2` (inequality comparison)

### Multiplication

**Result:** The product of two `Magnitude` values.

**Syntax:**

- For _types_ `M1` and `M2`:
    - `MagProductT<M1, M2>`
- For _instances_ `m1` and `m2`:
    - `m1 * m2`

### Division

**Result:** The quotient of two `Magnitude` values.

**Syntax:**

- For _types_ `M1` and `M2`:
    - `MagQuotientT<M1, M2>`
- For _instances_ `m1` and `m2`:
    - `m1 / m2`

### Powers

**Result:** A `Magnitude` raised to an integral power.

**Syntax:**

- For a _type_ `M`, and an integral power `N`:
    - `MagPowerT<M, N>`
- For an _instance_ `m`, and an integral power `N`:
    - `pow<N>(m)`

### Roots

**Result:** An integral root of a `Magnitude`.

**Syntax:**

- For a _type_ `M`, and an integral root `N`:
    - `MagPowerT<M, 1, N>` (because the $N^\text{th}$ root is equivalent to the
      $\left(\frac{1}{N}\right)^\text{th}$ power)
- For an _instance_ `m`, and an integral root `N`:
    - `root<N>(m)`

## Traits

These traits provide information, at compile time, about the number represented by a `Magnitude`.

### Integer test

**Result:** A `bool` indicating whether a `Magnitude` represents an _integer_ (`true` if it does;
`false` otherwise).

**Syntax:**

- For a _type_ `M`:
    - `IsInteger<M>::value`
- For an _instance_ `m`:
    - `is_integer(m)`

### Rational test

**Result:** A `bool` indicating whether a `Magnitude` represents a _rational number_ (`true` if it
does; `false` otherwise).

**Syntax:**

- For a _type_ `M`:
    - `IsRational<M>::value`
- For an _instance_ `m`:
    - `is_rational(m)`

### Numerator (integer part)

**Result:** The integer part of the numerator we would have if a `Magnitude` were written as
a fraction.  This result is another `Magnitude`.

For example, the "numerator" of $\frac{3\sqrt{3}}{5\pi}$ would be $3$, because it is the integer
part of $3\sqrt{3}$.

**Syntax:**

- For a _type_ `M`:
    - `NumeratorT<M>`
- For an _instance_ `m`:
    - `numerator(m)`

!!! warning
    This name and/or convention may be subject to change; see
    [#83](https://github.com/aurora-opensource/au/issues/83).

### Denominator (integer part)

**Result:** The integer part of the denominator we would have if a `Magnitude` were written as
a fraction.  This result is another `Magnitude`.

For example, the "denominator" of $\frac{3\sqrt{3}}{5\pi}$ would be $5$, because it is the integer
part of $5\pi$.

**Syntax:**

- For a _type_ `M`:
    - `DenominatorT<M>`
- For an _instance_ `m`:
    - `denominator(m)`

!!! warning
    This name and/or convention may be subject to change; see
    [#83](https://github.com/aurora-opensource/au/issues/83).
