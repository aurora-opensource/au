# Zero

Au contains a special type, `Zero`, which represents the number `0`.  We also provide a built-in
constant, `ZERO`, which is an instance of that type.

But why?  What value does it provide?

To answer that question, we'll walk through a common pain point people encounter when switching to
units libraries.

## Motivation

Let's imagine we have some computation which produces a squared speed.  Perhaps it's based on this
elementary kinematic equation:

$$v^2 = v_0^2 + 2a\Delta x$$

Now, it's possible to provide values for $v_0$, $a$, and $\Delta x$ that give a negative result.  If
this happens, we know we've been provided erroneous values, and we'll have to handle that condition.
Let's look at how we would do that at different stages of our project.

### Pre-units library

If we wrote the first version of our code without a units library, we probably stored our variables
in raw `double`, and used variable name suffixes to keep track of the units.  For example, our code
might look like this:

```cpp
const auto v_squared_mmpss = (v_mps * v_mps) + 2 * a_mpss * delta_x_m;

if (v_squared_mmpss < 0.0) {
    // Handle error condition
}
```

### Pitfalls in switching to a units library

Now suppose we want to switch to a units library.  The first line is great --- the library really
cleans it up!  But the comparison in the second line presents a problem.  Let's look at both.

```cpp
const auto v_squared = (v * v) + 2 * a * delta_x;

if (v_squared < 0.0) {  // <--- Compiler error!
    // Handle error condition
}
```

Getting rid of the unit suffixes is nice, but now our comparison won't compile.  And for good
reason: we know we can't compare a dimensioned quantity to a raw number!  So, we roll up our
sleeves, and make sure to use the _right kind_ of `0`:

```cpp
const auto v_squared = (v * v) + 2 * a * delta_x;

if (v_squared < squared(meters / second)(0.0)) {
    // Handle error condition
}
```

This _works_, but we couldn't really call it satisfying.  Specifying these units adds a lot of
clutter.  Shouldn't there be a better way?

### `ZERO` to the rescue

Fortunately, there is!  To see why, notice that `0` is the _one and only number_ where the results
of this comparison are _completely independent_ of the choice of units.  Simply put, zero of
anything is just zero!

This fact is the key to reducing friction.  We created a type, [`Zero`](../../reference/zero.md),
which always represents the value `0`.  We also made a built-in constant of that type, `ZERO`, for
convenience. Our `Quantity` types are _implicitly constructible_ from `ZERO`.  That means when we
compare to it, we always get zero _in the same units as the variable we're comparing to_.

Armed with this new tool, our code becomes:

```cpp
const auto v_squared = (v * v) + 2 * a * delta_x;

if (v_squared < ZERO) {
    // Handle error condition
}
```

Now we have the best of both worlds!

## Example use cases

Use `ZERO` liberally whenever you need a `Quantity` of `0`!  Key use cases include:

- initialization
- assignment
- sign comparison

Here's a code comparison for a couple examples.  (Once you click on a tab below, you can use the
left and right arrow keys to flip back and forth.)

=== "Without `ZERO`"
    ```cpp
    // Initialization
    QuantityD<UnitQuotientT<Radians, Meters>> curvature = (radians / meter)(0);

    // Checking for negative numbers
    if (v_squared < squared(meters / second)(0)) { /* ... */ }
    ```

=== "With `ZERO`"
    ```cpp
    // Initialization
    QuantityD<UnitQuotientT<Radians, Meters>> curvature = ZERO;

    // Checking for negative numbers
    if (v_squared < ZERO) { /* ... */ }
    ```

### One non-use case

It's tempting to enable `ZERO` to construct `QuantityPoint` too, not just `Quantity`.  Wouldn't it
be nice to be able to write something like this?

```cpp
QuantityPointD<Celsius> freezing_temp = ZERO;
// Warning: will not compile!
```

We forbid this, because it would do more harm than good.  The reason `ZERO` works so well for
`Quantity` is that its meaning is completely unambiguous, independent of any units.  But [the whole
point of `QuantityPoint`](./quantity_point.md) is that different scales can apply the label of
"zero" to different points!  Imagine we refactored our codebase to use `Fahrenheit` or `Kelvins`
instead of `Celsius`.  It would be easy to miss this line (and many others like it).  If we did, it
would completely change the meaning of our program!

## Conclusion

With a quantity of `0`, the units don't matter!  So reach for `ZERO`, and make your code easier to
write and read.

And if you have another type where `0` is similarly completely unambiguous --- say, a linear algebra
vector class --- feel free to add a constructor that takes `Zero` to give it that same
expressibility!
