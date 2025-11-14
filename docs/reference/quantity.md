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

There are several ways to construct a `Quantity` object.

- The **preferred** way, which we'll explain below, is to use a _quantity maker_.
- The other ways are all normal C++ constructors.

One way you _cannot_ construct a `Quantity` is via a constructor that takes a single raw number.
Many users find this surprising at first, but it's important for safety and usability.  To learn
more about this policy, expand the box below.

??? note "The \"missing\" raw number constructor"
    New users often expect `Quantity<U, R>` to be constructible from a raw value of type `R`.  For
    example, they expect to be able to write something like:

    ```cpp
    Quantity<Meters, double> height{3.0};  // Does NOT work in Au
    ```

    This example looks innocuous, but enabling it would have other ill effects, and would be a net
    negative overall.

    First, we want to support a wide variety of reasonable usage patterns, _safely_.  One approach
    people sometimes take is to use _dimension-named aliases_ throughout the codebase, making the
    actual underlying unit an encapsulated implementation detail.  Here's an example of what this
    looks like, which shows why we must forbid the default constructor:

    ```cpp
    // Store all lengths in meters using `double`, but as an implementation detail.
    // End users will simply call their type `Length`.
    using Length = QuantityD<Meters>;

    // In some other file...
    Length l1{3.0};          // Does NOT work in Au --- good!
    Length l2 = meters(3.0); // Works; unambiguous.
    ```

    We hope the danger is clear: there's no such concept as a "length of 3".  For safety and
    clarity, the user must always name the unit at the callsite.

    The second reason is elaborated in the section, "[`explicit` is not explicit
    enough](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3045r0.html#explicit-is-not-explicit-enough)",
    from the standard units library proposal paper P3045R0.  Even if you don't use aliases for your
    quantities, you might sometimes have a _vector_ of them.  If you do, the `.emplace_back()`
    function accepts a raw number.  Not only is this unclear at the callsite, but it can cause
    long-range (and silent!) errors if you later refactor the vector to hold a different type. By
    contrast, omitting this constructor forces the user to name the unit explicitly at the callsite,
    every time.  This keeps callsites unambiguous, minimizes cognitive load for the reader, and
    enables safe refactoring.

    Overall, despite the initial surprise of the "missing" raw number constructor, experience shows
    that it's a net benefit.  Not only does its absence enhance safety, but thanks to the other
    construction methods, it also doesn't sacrifice usability!

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

This constructor performs a unit conversion.  The result will represent the _same quantity_ as the
input, but expressed in the _units_ of the newly constructed object's type.  It will also convert
the stored value to the _rep_ of the constructed object, if necessary.

Here is the signature.  We've simplified it slightly for illustration purposes, and enclosed it
inside the class definition for context.

```cpp
template <typename Unit, typename Rep>
class Quantity {
    template <typename OtherUnit, typename OtherRep>
    Quantity(Quantity<OtherUnit, OtherRep> other);
};
```

This constructor only exists when this unit-and-rep conversion is both meaningful and safe.  It can
fail to exist in several ways.

1. If the input quantity has a **different dimension**, then the operation is intrinsically
   meaningless and we forbid it.

2. If the unit conversion has what we consider to be **unacceptable risk**, then we also forbid it.
   Common examples include:

    a. When `OtherRep` is _floating point_, but `Rep` is _integral_, we forbid this conversion for
       excessive truncation risk.

    b. If the conversion factor is large, and `Rep` is small, such that _even very small values_ in
       `OtherRep` would exceed the bounds of `Rep`, we forbid this conversion for excessive overflow
       risk.  See the [overflow safety surface](../discussion/concepts/overflow.md#adapt) docs for
       more details.

These last two are examples of conversions that are physically meaningful, but forbidden due to the
risk of larger-than-usual errors.  The library can still perform these conversions, but not via this
constructor, and it must be "forced" to do so.  The next section explains how to do this.

### Constructing from another `Quantity`, with explicit risk policy

This constructor also performs a unit conversion, but the user selects which conversion risks are
actively checked for.  This is done by passing an explicit [conversion risk
policy](./conversion_risk_policies.md) as a second argument.

Here is the signature.  We've simplified it slightly for illustration purposes, and enclosed it
inside the class definition for context.

```cpp
template <typename Unit, typename Rep>
class Quantity {
    template <typename OtherUnit, typename OtherRep, typename Policy>
    Quantity(Quantity<OtherUnit, OtherRep> other, Policy policy);
};
```

A common example for `policy` is `ignore(TRUNCATION_RISK)`.  This can be used when truncation is
actually desired, or when you independently know that the specific input value will not truncate.

### Constructing from `Zero`

This constructs a `Quantity` with a value of 0.

Here is the signature, enclosed in the class definition for context.

```cpp
template <typename Unit, typename Rep>
class Quantity {
    Quantity(Zero);
};
```

!!! note
    `Zero` is the name of the type.  At the callsite, you would pass an _instance_ of that type,
    such as the constant `ZERO`.  For example:

    ```cpp
    QuantityD<Meters> height{ZERO};
    ```

For more information on the motivation and use of this type, read [our `Zero`
discussion](../discussion/concepts/zero.md).

### Default constructor

Here is the signature of the constructor, enclosed in the class template definition for context.

```cpp
template <typename Unit, typename Rep>
class Quantity {
    Quantity();
};
```

A default-constructed `Quantity` is always initialized (which helps avoid certain kinds of memory
safety bugs).  It will contain a default-constructed instance of the rep type.

!!! warning
    Avoid relying on the _specific value_ of a default-constructed `Quantity`, because it poorly
    communicates intent.  The only logically valid operation on a default-constructed `Quantity` is
    to assign to it later on.

    The default value for many rep types, including all fundamental arithmetic types, is `0`.
    Instead of relying on this behaviour, initialize your `Quantity` with [`au::ZERO`](./zero.md) to
    better communicate your intent.

### Constructing from corresponding quantity

Here is the signature.  We've simplified it slightly for illustration purposes, and enclosed it
inside the class definition for context.

```cpp
template <typename Unit, typename Rep>
class Quantity {
    template <typename T>
    Quantity(T &&x);
    // NOTE: only exists when `T` is an "exactly-equivalent" type.  One example:
    // `std::chrono::nanoseconds` can construct an `au::QuantityI64<Nano<Seconds>>`.
};
```

This constructor will only exist when `T` has a ["corresponding
quantity"](./corresponding_quantity.md), and this `Quantity` type is _implicitly constructible_ from
that "corresponding quantity" type by [the above mechanism](#implicit-from-quantity).

## Extracting the stored value

In order to access the raw numeric value stored inside of `Quantity`, you must explicitly name the
unit at the callsite.  There are two functions which can do this, depending on whether you want to
access by value or by reference.

### By value: `.in(unit)` {#extracting-with-in}

This function returns the underlying stored value, by value.  See the [unit
slots](../discussion/idioms/unit-slots.md) discussion for valid choices for `unit`.

??? example "Example: extracting `4.56` from `seconds(4.56)`"
    Consider this `Quantity<Seconds, double>`:

    ```cpp
    auto t = seconds(4.56);
    ```

    You can retrieve the underlying value by writing either `t.in(seconds)` (passing the
    `QuantityMaker`), or `t.in(Seconds{})` (passing an instance of the unit).

### By reference: `.data_in(unit)`

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

However, note that we may change this second property in the future.  The version with the template
arguments may be changed later so that it _does_ prevent lossy conversions.  If you want this
"forcing" semantic, prefer to use a policy argument, and add the explicit template parameter only if
you want to change the rep.  See [#122] to track progress on this change, and see the [policy
argument section](#policy-argument) for an example of the _preferred_ way to force a conversion.

!!! tip
    Prefer to **omit** the template argument if possible, because you will get more safety checks.
    The risks which the no-template-argument version warns about are real.

    As of [0.6.0] and [#122], however, this advice will no longer apply.  At that point, the
    explicit-rep versions will have the same safety checks as the implicit-rep versions.

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

However, note that we may change this second property in the future.  The version with the template
arguments may be changed later so that it _does_ prevent lossy conversions.  If you want this
"forcing" semantic, prefer to use a policy argument, and add the explicit template parameter only if
you want to change the rep.  See [#122] to track progress on this change, and see the [policy
argument section](#policy-argument) for an example of the _preferred_ way to force a conversion.

!!! tip
    Prefer to **omit** the template argument if possible, because you will get more safety checks.
    The risks which the no-template-argument version warns about are real.

    As of [0.6.0] and [#122], however, this advice will no longer apply.  At that point, the
    explicit-rep versions will have the same safety checks as the implicit-rep versions.

### Skipping risk checks: the `policy` argument {#policy-argument}

Some unit conversions have too much [conversion risk](../discussion/concepts/conversion_risks.md) to
permit by default.  When that happens, the compiler will tell you which risk (overflow or
truncation) it deemed too high.  You can still perform the conversion, but you must explicitly turn
off the corresponding safety check, by passing a [policy argument](./conversion_risk_policies.md).
Every variant of `as` and `in` mentioned above supports such an argument.  If you pass a policy, it
will control the set of conversion risks that we check for.

To be concrete, here are the signatures of the functions that support the policy argument:

| Usual form | With policy argument |
|------------|----------------------|
| `.as(unit)` | `.as(unit, policy)` |
| `.as<T>(unit)` | `.as<T>(unit, policy)` |
| `.in(unit)` | `.in(unit, policy)` |
| `.in<T>(unit)` | `.in<T>(unit, policy)` |

??? example "Example: forcing a conversion from inches to feet"
    `inches(24).as(feet)` is not allowed.  This conversion will divide the underlying value, `24`,
    by `12`.  While this particular value would produce an integer result, most other `int` values
    would not.  Because our result uses `int` for storage --- same as the input --- we forbid this.

    `inches(24).as(feet, ignore(TRUNCATION_RISK))` _is_ allowed.  This second argument turns off the
    truncation risk check.  The conversion would produce `feet(2)`.  However, note that this
    operation uses integer division, which truncates: so, for example,
    `inches(23).as(feet, ignore(TRUNCATION_RISK))` would produce `feet(1)`.

??? example "Example: simultaneous unit and type conversion"
    `inches(27.8).as<int>(feet, ignore(TRUNCATION_RISK))` will return `feet(2)`.

!!! tip
    In most cases, prefer **not** to use the policy versions if possible, because you will get more
    safety checks.  The risks which the "base" versions warn about are real.

    However, one place where it's _very safe_ to use the "coercing versions" is right after running
    a _runtime conversion checker_.  These provide _exact_ conversion checks, even more accurate than
    the default compile-time safety surface (although at the cost of runtime operations).  See the
    [subsequent section](#runtime-conversion-checkers) for more details.

### Forcing lossy conversions: `.coerce_as(unit)`, `.coerce_in(unit)` {#coerce}

!!! warning
    We are planning to deprecate these functions in the [0.6.0] release.  See [#481] to track the
    progress.

    In the meantime, here is how you convert.

    First, figure out which conversion risks you are trying to override: **overflow**, or
    **truncation**, or **both**.  (If you don't have an explicit `<Rep>` argument, you can simply
    delete the `"coerce_"` word and compile, and the error message will tell you which one is
    relevant.  Otherwise, you will need to use your knowledge of the types and units involved to
    figure this out.)

    Then, follow this table to rewrite your conversion, using the conversion risk you identified
    above.

    | "Coerce" version (dis-preferred; will be deprecated) | "Policy" version (preferred) |
    |------------------------------------------------------|------------------------------|
    | `q.coerce_as(unit)` | One of:<br>`q.as(unit, ignore(OVERFLOW_RISK))`<br>`q.as(unit, ignore(TRUNCATION_RISK))`<br>`q.as(unit, ignore(OVERFLOW_RISK | TRUNCATION_RISK))` |
    | `q.coerce_as<T>(unit)` | One of:<br>`q.as<T>(unit, ignore(OVERFLOW_RISK))`<br>`q.as<T>(unit, ignore(TRUNCATION_RISK))`<br>`q.as<T>(unit, ignore(OVERFLOW_RISK | TRUNCATION_RISK))` |
    | `q.coerce_in(unit)` | One of:<br>`q.in(unit, ignore(OVERFLOW_RISK))`<br>`q.in(unit, ignore(TRUNCATION_RISK))`<br>`q.in(unit, ignore(OVERFLOW_RISK | TRUNCATION_RISK))` |
    | `q.coerce_in<T>(unit)` | One of:<br>`q.in<T>(unit, ignore(OVERFLOW_RISK))`<br>`q.in<T>(unit, ignore(TRUNCATION_RISK))`<br>`q.in<T>(unit, ignore(OVERFLOW_RISK | TRUNCATION_RISK))` |

    These new versions are both more clear about their intent, and safer (because they only turn off
    the safety checks that they need to).

This function performs the exact same kind of unit conversion as if the string `coerce_` were
removed.  However, it will ignore any safety checks for overflow or truncation.

??? example "Example: forcing a conversion from inches to feet"
    `inches(24).as(feet)` is not allowed.  This conversion will divide the underlying value, `24`,
    by `12`.  While this particular value would produce an integer result, most other `int` values
    would not.  Because our result uses `int` for storage --- same as the input --- we forbid this.

    `inches(24).coerce_as(feet)` _is_ allowed.  The `coerce_` prefix has "forcing" semantics.  This
    would produce `feet(2)`.  However, note that this operation uses integer division, which
    truncates: so, for example, `inches(23).coerce_as(feet)` would produce `feet(1)`.

These functions also support an explicit template parameter: so, `.coerce_as<T>(unit)` and
`.coerce_in<T>(unit)`.  If you supply this parameter, it will be the rep of the result.

??? example "Example: simultaneous unit and type conversion"
    `inches(27.8).coerce_as<int>(feet)` will return `feet(2)`.

!!! tip
    In most cases, prefer **not** to use the "coercing versions" if possible, because you will get
    more safety checks.  The risks which the "base" versions warn about are real.

    However, one place where it's _very safe_ to use the "coercing versions" is right after running
    a _runtime conversion checker_.  These provde _exact_ conversion checks, even more accurate than
    the default compile-time safety surface (although at the cost of runtime operations).  See the
    next section for more details.

### Runtime conversion checkers {#runtime-conversion-checkers}

Au's default, compile-time conversion checks are only heuristics, based on the _general_ risk of
overflow or truncation.  They operate on the conversion as a whole, not on specific values.  This
means that some input values for forbidden conversions would actually be just fine, while some input
values for permitted conversions would be lossy.

This section documents a more exact alternative: the _runtime conversion checkers_, which can detect
overflow or truncation for specific runtime values.  The downside is that you will pay a runtime
penalty for these checks, as opposed to the compile-time checks which are basically free.  However,
unit conversions very rarely occur in the "hot loops" of well designed programs, so this performance
cost usually doesn't matter.

!!! tip
    A great way to use these functions is to write your own conversion utilities, using your
    preferred error handling mechanism (exceptions, optional, return codes, and so on).  See our
    [overflow guide](../discussion/concepts/overflow.md#check-at-runtime) for more details.

We provide individual checkers for overflow and truncation, as well as a checker for general
lossiness (which combines both).

#### `will_conversion_overflow`

`will_conversion_overflow` takes a `Quantity` value and a target unit, and returns whether the
conversion will overflow.  Users can also provide an "explicit rep" template parameter to check the
corresponding explicit-rep conversion.

We define "overflow" as a value that would either be lower than the lowest representable number in
the target type, or higher than the highest representable number.  The precise implementation will
depend on the types involved.  For example, if the input is an unsigned integral type, we won't
emit a runtime instruction to check the lower bound of the target.

Here are the usage patterns, and their corresponding signatures.

- `will_conversion_overflow(q, target_unit)` returns whether `q.as(target_unit)`, or
  `q.in(target_unit)`, would overflow.

    ```cpp
    template <typename U, typename R, typename TargetUnitSlot>
    constexpr bool will_conversion_overflow(Quantity<U, R> q, TargetUnitSlot target_unit);
    ```

- `will_conversion_overflow<T>(q, target_unit)` returns whether `q.as<T>(target_unit)`, or
  `q.in<T>(target_unit)`, would overflow.

    ```cpp
    template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
    constexpr bool will_conversion_overflow(Quantity<U, R> q, TargetUnitSlot target_unit);
    ```

#### `will_conversion_truncate`

`will_conversion_truncate` takes a `Quantity` value and a target unit, and returns whether the
conversion will truncate.  For example, if the target unit is `feet`, then `inches(13)` and
`inches(11)` _would_ truncate, but `inches(12)` would _not_ truncate. Users can also provide an
"explicit rep" template parameter to check the corresponding explicit-rep conversion.

!!! note "Note: floating point destination types are treated as non-truncating"
    See the discussion in the [floating point section](../discussion/concepts/truncation.md#float)
    of our truncation discussion for more detail.

Here are the usage patterns, and their corresponding signatures.

- `will_conversion_truncate(q, target_unit)` returns whether `q.as(target_unit)`, or
  `q.in(target_unit)`, would truncate.

    ```cpp
    template <typename U, typename R, typename TargetUnitSlot>
    constexpr bool will_conversion_truncate(Quantity<U, R> q, TargetUnitSlot target_unit);
    ```

- `will_conversion_truncate<T>(q, target_unit)` returns whether `q.as<T>(target_unit)`, or
  `q.in<T>(target_unit)`, would truncate.

    ```cpp
    template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
    constexpr bool will_conversion_truncate(Quantity<U, R> q, TargetUnitSlot target_unit);
    ```

#### `is_conversion_lossy`

`is_conversion_lossy` combines both of the previous two checks: it returns `true` whenever _either
or both_ of `will_conversion_overflow` or `will_conversion_truncate` would return `true`.  Like
these functions, it takes a `Quantity` value and a target unit.  Users can also provide an "explicit
rep" template parameter to check the corresponding explicit-rep conversion.

The reason the other two functions are publicly available (rather than only this one) is that often,
users may only care about either of overflow or truncation, not both.  For example, working with
integral quantities in the embedded domain, users may wish to decompose a nanosecond duration
quantity into separate parts for "seconds" and "nanoseconds", where the "seconds" part uses
a smaller integer type, and the leftover "nanoseconds" part amounts to less than one second.  In
this case, truncating the initial quantity when converting to "seconds" is explicitly desired, but
we still want to check for overflow.

Here are the usage patterns, and their corresponding signatures.

- `is_conversion_lossy(q, target_unit)` returns whether `q.as(target_unit)`, or `q.in(target_unit)`,
  would either overflow or truncate.

    ```cpp
    template <typename U, typename R, typename TargetUnitSlot>
    constexpr bool is_conversion_lossy(Quantity<U, R> q, TargetUnitSlot target_unit);
    ```

- `is_conversion_lossy<T>(q, target_unit)` returns whether `q.as<T>(target_unit)`, or
  `q.in<T>(target_unit)`, would either overflow or truncate.

    ```cpp
    template <typename TargetRep, typename U, typename R, typename TargetUnitSlot>
    constexpr bool is_conversion_lossy(Quantity<U, R> q, TargetUnitSlot target_unit);
    ```

### Dimensionless and unitless results: `as_raw_number` {#as-raw-number}

Users may expect that the product of quantities such as `seconds` and `hertz` would completely
cancel out, and produce a raw, simple C++ numeric type.  Currently, this is indeed the case, but we
have also found that it makes the library harder to reason about.  Instead, we hope in the future to
return a `Quantity` type _consistently_ from arithmetical operations on `Quantity` inputs (see
[#185]).

In order to obtain that raw number robustly, both now and in the future, you can use the
`as_raw_number` function, a callsite-readable way to "exit" the library.  This will also opt into
all mechanisms and safety features of the library.  In particular:

- We will automatically perform all necessary conversions.
- This will not compile unless the input is _dimensionless_.
- If the conversion is dangerous (say, from `Quantity<Percent, int>`, which cannot in general be
  represented exactly as a raw `int`), we will also fail to compile, unless users provide a second
  policy argument to override the safety check.

Users should get in the habit of using `as_raw_number` whenever they really want a raw number.  This
communicates intent, and also works both before and after [#185] is implemented.

Here are the available APIs:

- `as_raw_number(q)`
    - Converts the dimensionless quantity `q` to be unitless, and returns the underlying value.
    - For compatibility reasons, also accepts an argument that is already a raw number, simply
      returning it.
- `as_raw_number(q, policy)`
    - Same as above, but allows the user to override safety checks by passing a [policy
      argument](./conversion_risk_policies.md).

!!! example "Example: `as_raw_number` with a non-truncating conversion"
    ```cpp
    constexpr auto num_beats = as_raw_number(kilo(hertz)(7) * seconds(3));
    // Result: 21'000 (of type `int`)
    ```

!!! example "Example: overriding safety checks for a truncating conversion"
    The following will not compile:

    ```cpp
    constexpr auto factor = as_raw_number(percent(1234));
    ```

    The result would in principle be 12.34, but the rep is an `int`, which cannot hold this value.
    If truncation is truly desired, you can provide a second argument to override the safety check:

    ```cpp
    constexpr auto factor = as_raw_number(percent(1234), ignore(TRUNCATION_RISK));
    ```

    This produces `12` (of type `int`).

## Non-Type Template Parameters (NTTPs) {#nttp}

A _non-type template parameter_ (NTTP) is a template parameter that is not a _type_, but rather some
kind of _value_.  Common examples include `template<int N>`, or `template<bool B>`.  Before C++20,
only a small number of types could be used as NTTPs: very roughly, these were _integral_ types,
_pointer_ types, and _enumerations_.

Au provides a workaround for pre-C++20 users that lets you _effectively_ encode any `Quantity<U, R>`
as an NTTP, _as long as_ its rep `R` is an **integral** type.  To do this, use the
`Quantity<U, R>::NTTP` type as the template parameter.  You will be able to assign between
`Quantity<U, R>` and `Quantity<U, R>::NTTP`, _in either direction_, but only in the case of exact
match of both `U` and `R`.  For all other cases, you'll need to perform a conversion (using the
usual mechanisms for `Quantity` described elsewhere on this page).

!!! warning
    It is undefined behavior to invoke `Quantity<U, R>::NTTP` whenever `std::is_integral<R>::value`
    is `false`.

    We cannot strictly prevent users from doing this.  However, in practice, it is very unlikely for
    this to happen by accident.  Both conversion operators between `Quantity<U, R>` and
    `Quantity<U, R>::NTTP` would fail with a hard compiler error, based on a `static_assert` that
    explains this situation.  So users can name this type, but they cannot assign to it or from it
    without prohibitive difficulty.

??? example "Example: defining and using a template with a `Quantity` NTTP"
    ```cpp
    template <QuantityI<Hertz>::NTTP Frequency>
    struct TemplatedOnFrequency {
        QuantityI<Hertz> value = Frequency;      // Assigning `Quantity` from NTTP
    };

    using T = TemplatedOnFrequency<hertz(440)>;  // Setting template parameter from `Quantity`
    ```

### `from_nttp(Quantity<U, R>::NTTP)`

Calling `from_nttp` on a `Quantity<U, R>::NTTP` will convert it back into the corresponding
`Quantity<U, R>` that was encoded in the template parameter.  This lets it automatically participate
in all of the usual `Quantity` operations and conversions.

!!! note
    If you are simply _assigning_ a `Quantity<U, R>::NTTP` to a `Quantity<U, R>`, where `U` and `R`
    are identical, you do not need to call `from_nttp`.  We support implcit conversion in that case.

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

!!! warning
    If your two quantities have the same dimension, you may prefer the _common-unit_ form of
    division, rather than the _arbitrary-unit_ form that `/` provides.

    As a motivating example, note that `hours(8) / minutes(60)` is `(hours / minute)(0)` --- that
    is, the stored value is _zero_ when expressed in the dimensionless unit `hours / minute`
    (because we are using an integral rep, which truncates). This is the correct result for these
    input units, but is unlikely to be what the user wants.  (Happily, Au protects against this
    dangerous division; see [`unblock_int_div`](#unblock-int-div) below.)

    The _common-unit_ form of division, obtained via the
    [`divide_in_common_units()`](#divide-in-common-units) function, will convert both inputs to
    their common unit before dividing.  In this case, it will convert `hours(8)` to `minutes(480)`,
    and the division will produce the dimensionless result of `8`. This form of division also pairs
    properly with the `%` operator; see [`mod`](#mod) below for more details.

If either _input_ is a raw number, then it only affects the value, not the unit.  It's equivalent to
a `Quantity` whose unit is [a unitless unit](./unit.md#unitless-unit).

#### `unblock_int_div()` {#unblock-int-div}

Experience has shown that raw integer division can be dangerous in a units library context.  It
conflicts with intuitions, and can produce code that is silently and grossly incorrect: see the
[integer division section] of the troubleshooting guide for an example.

To use integer division, you must ask for it explicitly by name, by calling `unblock_int_div()` on
the denominator.

??? example "Using `unblock_int_div()` to explicitly opt in to integer division"

    This will not work:

    ```cpp
    miles(115) / hours(2);
    //         ^--- Forbidden!  Compiler error.
    ```

    However, this will work just fine:

    ```cpp
    miles(115) / unblock_int_div(hours(2));
    ```

    It produces `(miles / hour)(57)`.

!!! warning "`unblock_int_div` can be dangerous"
    This compiler error is limited to what experience has shown to be the most dangerous types of
    integer division: those where the _denominator_ has non-trivial units, and they are _different_
    from the numerator's units.

    Before using `unblock_int_div`, please carefully read the [integer division section] of the
    troubleshooting guide to understand the risks.  If you end up using it anyway, consider adding
    a brief comment to explain why it's OK in your use case.

!!! tip
    If your inputs have the _same dimension_, you most likely want to convert them to the same unit
    --- their _common unit_ --- before dividing.  This removes the need for `unblock_int_div` in the
    most common cases, and is generally much safer.  See
    [`divide_in_common_units()`](#divide-in-common-units) below for more details.

#### `divide_in_common_units()` {#divide-in-common-units}

The `divide_in_common_units()` utility takes two `Quantity` inputs, `a`, and `b`, whose dimension is
the same.  It first converts each input to their [common
unit](../discussion/concepts/common_unit.md), and then performs regular division.  The result
will be a dimensionless and unitless number (or, after [#185], a `Quantity`).

There is no need to wrap the denominator in a call to `unblock_int_div`, because same-unit division
is always allowed by Au.

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

### AreQuantityTypesEquivalent {#are-quantity-types-equivalent}

**Result:** Indicates whether two `Quantity` types are equivalent.  Equivalent types may be freely
converted to each other, and no arithmetic operations will be performed in doing so.

More precisely, `Quantity<U1, R1>` and `Quantity<U2, R2>` are equivalent if and only if **both** of
the following conditions hold.

1. The units `U1` and `U2` are [quantity-equivalent](./unit.md#quantity-equivalent).

2. The reps `R1` and `R2` are the same type.

**Syntax:**

- For _types_ `U1` and `U2`:
    - `AreQuantityTypesEquivalent<U1, U2>::value`

[#122]: https://github.com/aurora-opensource/au/issues/122
[#185]: https://github.com/aurora-opensource/au/issues/185
[#481]: https://github.com/aurora-opensource/au/issues/481
[0.6.0]: https://github.com/aurora-opensource/au/milestone/9
[integer division section]: ../troubleshooting.md#integer-division-forbidden
