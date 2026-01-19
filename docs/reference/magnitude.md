# Magnitude

`Magnitude` is a family of [monovalue types](./detail/monovalue_types.md) representing nonzero real
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

Because of its importance for angular variables, $\pi$ is supported natively in the library, via the
irrational magnitude base, `Pi`.  To define a magnitude _instance_ for $\pi$, you can write:

```cpp
constexpr auto PI = Magnitude<Pi>{};
```

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

    Thus, if you have a magnitude instance `PI`, then `get_value<float>(pow<3>(PI))` will be much
    more accurate than storing $\pi$ in a `float`, and cubing it --- yet, there will be no loss in
    runtime performance.

### Checking for representability

If you need to check whether your magnitude `m` can be represented in a type `T`, you can call
`representable_in<T>(m)`.  This function is `constexpr` compatible.

??? example "Example: integer and non-integer values"

    Here are some example test cases which will pass.

    ```cpp
    EXPECT_THAT(representable_in<int>(mag<1>()), IsTrue());

    // (1 / 2) is not an integer.
    EXPECT_THAT(representable_in<int>(mag<1>() / mag<2>()), IsFalse());

    EXPECT_THAT(representable_in<float>(mag<1>() / mag<2>()), IsTrue());
    ```

??? example "Example: range of the type"
    Here are some example test cases which will pass.

    ```cpp
    EXPECT_THAT(representable_in<uint32_t>(mag<4'000'000'000>()), IsTrue());

    // 4 billion is larger than the max value representable in `int32_t`.
    EXPECT_THAT(representable_in<int32_t>(mag<4'000'000'000>()), IsFalse());
    ```

Note that this function's return value also depends on _whether we can compute_ the value, not just
whether it is representable.   For example, `representable_in<double>(sqrt(mag<2>()))` is currently
`false`, because we haven't yet added support for computing rational base powers.

## Compile-time arithmetic limitations

Some `Magnitude` operations need to perform arithmetic at compile time to compute their results.  For
example, to compare two magnitudes, we need to determine which one is larger.  It is extremely hard,
if not impossible, to do this for _all_ magnitudes, which may involve arbitrary rational powers, and
even transcendental numbers.  However, others can easily be computed using only 64-bit integer
operations.

We provide support for the subset of inputs where we can confidently produce exact answers, and
guard other inputs behind compile-time errors.  This enables many practical use cases, while
conservatively avoiding producing an incorrect program.

Our concrete policy is to support only operations where we can produce exact answers using only
64-bit integer arithmetic.  This does _not_ mean that we only support exact integer magnitudes.
These operations are _binary_, so the feasibility depends on the _relationship_ between the two
inputs.

??? example
    Suppose we had this variable:

    ```cpp
    constexpr auto PI = Magnitude<Pi>{};
    ```

    Here are some examples of comparisons that would or would not be supported.

    - `PI > mag<3>()` would **not** be supported: it would result in a compiler error.
        - **Reason:** We cannot use the exact value of $\pi$ in computations; we only have access to
          the nearest representable floating point number.  Floating point arithmetic is inexact, so
          this does not meet our criteria.
    - `PI > (mag<3>() * PI / mag<2>())` **would** be supported, and would evaluate to `false`.
        - **Reason:** Regardless of the exact value of $\pi$, we can see that $\pi$ cancels out, so
          this comparison is equivalent to checking whether $1 > \frac{3}{2}$, which we can convert
          to $2 > 3$.  This can be seen to be `false` using only integer arithmetic.

When an operation cannot be computed --- either because the relationship involves irrational
factors, or because the integers involved are too large --- you will get a **compile-time error**
with a message explaining the problem.

!!! note
    Operations subject to this limitation will be marked with † in their documentation.

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

### Ordering comparison †

**Result:** A `bool` indicating the relative ordering of two `Magnitude` values.

Unlike equality comparison (which simply checks whether two types are the same), ordering comparison
computes which magnitude is larger.

† _This feature is subject to [compile-time arithmetic
limitations](#compile-time-arithmetic-limitations)._

**Syntax:**

- For _instances_ `m1` and `m2`:
    - `m1 < m2`
    - `m1 > m2`
    - `m1 <= m2`
    - `m1 >= m2`

`Zero` can also be compared with any `Magnitude`:

- `ZERO < m` is `true` when `m` is positive
- `ZERO > m` is `true` when `m` is negative

### Multiplication

**Result:** The product of two `Magnitude` values.

**Syntax:**

- For _types_ `M1` and `M2`:
    - `MagProduct<M1, M2>`
- For _instances_ `m1` and `m2`:
    - `m1 * m2`

!!! note
    Older releases used `MagProductT` (with the `T` suffix) instead of `MagProduct`.  Prefer
    `MagProduct`.  `MagProductT` is deprecated, and will be removed in future releases.

### Division

**Result:** The quotient of two `Magnitude` values.

**Syntax:**

- For _types_ `M1` and `M2`:
    - `MagQuotient<M1, M2>`
- For _instances_ `m1` and `m2`:
    - `m1 / m2`

!!! note
    Older releases used `MagQuotientT` (with the `T` suffix) instead of `MagQuotient`.  Prefer
    `MagQuotient`.  `MagQuotientT` is deprecated, and will be removed in future releases.

### Negation

**Result:** The negative of a `Magnitude`.

**Syntax:**

- For a _type_ `M`:
    - No special support, but you can form the product with `Magnitude<Negative>`, which represents
      `-1`.
- For an _instance_ `m`:
    - `-m`

### Powers

**Result:** A `Magnitude` raised to an integral power.

**Syntax:**

- For a _type_ `M`, and an integral power `N`:
    - `MagPower<M, N>`
- For an _instance_ `m`, and an integral power `N`:
    - `pow<N>(m)`

!!! note
    Older releases used `MagPowerT` (with the `T` suffix) instead of `MagPower`.  Prefer
    `MagPower`.  `MagPowerT` is deprecated, and will be removed in future releases.

### Roots

**Result:** An integral root of a `Magnitude`.

**Syntax:**

- For a _type_ `M`, and an integral root `N`:
    - `MagPower<M, 1, N>` (because the $N^\text{th}$ root is equivalent to the
      $\left(\frac{1}{N}\right)^\text{th}$ power)
- For an _instance_ `m`, and an integral root `N`:
    - `root<N>(m)`

!!! note
    If `m` is negative, and `N` is even, then `root<N>(m)` produces a hard compiler error, because
    the result cannot be represented as a `Magnitude`.

### Helpers for powers and roots

Magnitudes support all of the [power helpers](./powers.md#helpers).  So, for example, for
a magnitude instance `m`, you can write `sqrt(m)` as a more readable alternative to `root<2>(m)`.

## Traits

These traits provide information, at compile time, about the number represented by a `Magnitude`.

### Is Integer?

**Result:** A `bool` indicating whether a `Magnitude` represents an _integer_ (`true` if it does;
`false` otherwise).

**Syntax:**

- For a _type_ `M`:
    - `IsInteger<M>::value`
- For an _instance_ `m`:
    - `is_integer(m)`

### Is Rational?

**Result:** A `bool` indicating whether a `Magnitude` represents a _rational number_ (`true` if it
does; `false` otherwise).

**Syntax:**

- For a _type_ `M`:
    - `IsRational<M>::value`
- For an _instance_ `m`:
    - `is_rational(m)`

### Is Positive?

**Result:** A `bool` indicating whether a `Magnitude` represents a _positive number_ (`true` if it
does; `false` otherwise).

**Syntax:**

- For a _type_ `M`:
    - `IsPositive<M>::value`
- For an _instance_ `m`:
    - `is_positive(m)`

### Integer part

**Result:** The integer part of a `Magnitude`, which is another `Magnitude`.

For example, the "integer part" of $\frac{\sqrt{18}}{5\pi}$ would be $3$, because $\sqrt{27}
= 3\sqrt{2}$, and $3$ is the integer part of $3\sqrt{2}$.

If the input magnitude is an integer, then this operation is the identity.

If the input magnitude is _not_ an integer, then this operation produces the largest integer factor
that can be extracted from the numerator (that is, the base powers with positive exponent).[^1]

[^1]: The concept `integer_part()` is conceptually ambiguous when applied to non-integers.  So, too,
for `numerator()` and `denominator()` applied to irrational numbers.  These utilities serve two
purposes.  First, they provide a means for checking whether a given magnitude is a member of the
unambiguous set --- that is, we can check whether a magnitude is an integer by checking whether it's
equal to its "integer part".  Second, they enable us to automatically construct labels for
magnitudes, by breaking them into the same kinds of pieces that a human reader would expect.

**Syntax:**

- For a _type_ `M`:
    - `IntegerPart<M>`
- For an _instance_ `m`:
    - `integer_part(m)`

!!! note
    Older releases used `IntegerPartT` (with the `T` suffix) instead of `IntegerPart`.  Prefer
    `IntegerPart`.  `IntegerPartT` is deprecated, and will be removed in future releases.

### Numerator (integer part)

**Result:** The numerator we would have if a `Magnitude` were written as a fraction.  This result is
another `Magnitude`.

For example, the "numerator" of $\frac{3\sqrt{3}}{5\pi}$ would be $3\sqrt{3}$.

**Syntax:**

- For a _type_ `M`:
    - `Numerator<M>`
- For an _instance_ `m`:
    - `numerator(m)`

!!! note
    Older releases used `NumeratorT` (with the `T` suffix) instead of `Numerator`.  Prefer
    `Numerator`.  `NumeratorT` is deprecated, and will be removed in future releases.

### Denominator (integer part)

**Result:** The denominator we would have if a `Magnitude` were written as a fraction.  This result is
another `Magnitude`.

For example, the "denominator" of $\frac{3\sqrt{3}}{5\pi}$ would be $5\pi$.

**Syntax:**

- For a _type_ `M`:
    - `Denominator<M>`
- For an _instance_ `m`:
    - `denominator(m)`

!!! note
    Older releases used `DenominatorT` (with the `T` suffix) instead of `Denominator`.  Prefer
    `Denominator`.  `DenominatorT` is deprecated, and will be removed in future releases.

### Absolute value

**Result:** The absolute value of a `Magnitude`, which is another `Magnitude`.

**Syntax:**

- For a _type_ `M`:
    - `Abs<M>`
- For an _instance_ `m`:
    - `abs(m)`

### Sign

**Result:** A `Magnitude`: `1` if the input is positive, and `-1` if the input is negative.

We expect that the relation `m == sign(m) * abs(m)` will hold for every `Magnitude` `m`.

**Syntax:**

- For a _type_ `M`:
    - `Sign<M>`
- For an _instance_ `m`:
    - `sign(m)`
