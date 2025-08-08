# Conversion Risks

Unit conversions turn one `Quantity` into another, where both the unit and the representation type
(the "rep") might change.  Think of a "unit conversion" as being defined by three parameters:

1. The _initial_ **rep**.
2. The _target_ **rep**.
3. The ratio between the _initial_ and _target_ **units**.

Under the hood, we implement each conversion as a sequence of operations, including `static_cast`
for changing the rep, and mathematical operations (mainly multiplying and dividing) for changing the
value.  Each operation has an input type, and an output type (which might be the same as the input).

## What can go wrong?

On pen and paper, any unit conversion can be done exactly.  _In software_, this is not true.  Most
representation types can only represent a limited range of numbers, and with only finite precision
within that range.  This means that some conversions can produce grossly incorrect results!  In
general, there are two main failure modes for an operation:

- **[Overflow](./overflow.md)**: the result _exceeds the bounds_ of the output type.
- **[Truncation](./truncation.md)**: the output type has _insufficient precision_ to represent the
  result.

Au accounts for these risks in our APIs.  For each conversion, we assess the overall risk in
deciding whether to permit it _by default_.  We also let you opt out of individual named safety
checks, for situations where you're confident it's okay.  Finally, we have efficient functions to
check the lossiness of _individual values_ at runtime, for the most granular control possible.

## Default policies

Any conversion has a set of input values that are lossy, and a set that aren't.  For example,
imagine that we had `int x`:

- `inches(x).as(feet)` would be lossy for any number that isn't an exact multiple of 12 (due to
  truncation).
- `feet(x).as(inches)` would be lossy for any number greater than `357913941`, or less than
  `-357913941` (due to overflow).

Au's task is to decide which unit conversions to allow, and which to turn into compiler errors.
Because we must make this decision at compile time, _we don't know the actual input value_. All we
can do is to assess the risk across the values that we expect to see most often.

Notice how the above two examples present very different risk profiles.  For `inches(x).as(feet)`,
11 out of every 12 values will be significantly lossy.  For `feet(x).as(inches)`, it's another
story.  In practice, users are unlikely to use values greater than 357 million feet in most
applications, since that's over 1/4 of the way to the moon!  As you might expect, Au forbids the
first conversion but permits the second.

In more detail, Au has separate risk checks for overflow and truncation.

- The **overflow check** is an [adaptive check](./overflow.md#adapt) based on a single number: the
  smallest input that would exceed the output range.

- The **truncation check** is based on whether truncation is possible for _any_ input value, with
  the caveat that we treat [floating point as non-truncating](./truncation.md#float).

Au only permits conversions by default if they pass _both_ checks.

### Overall risk versus individual values

It's critically important to understand that these policies are based on _risk_, which takes into
account _all possible_ input values (and, crudely, their relative likelihoods).  This is different
from determining whether _a specific input value_ will be lossy.  For example:

| Conversion | Policy | Result (if allowed) | Actually lossy? |
|------------|--------|---------------------|-----------------|
| `inches(24).as(feet)` | :x: Forbidden (truncation risk) | `feet(2)` | :thumbs_up: No |
| `seconds(4u).as(nano(seconds))` | :x: Forbidden (overflow risk) | `nano(seconds(4'000'000'000u))` | :thumbs_up: No |
| `seconds(9000u).as(micro(seconds))` | :thumbs_up: Allowed | `micro(seconds(410'065'408u))` | :x: Yes (overflow) |

This shows that our default policy has two kinds of failure modes.

The first is that **allowed conversions can still be lossy** for some input values.  Granted, this
can only happen for cases that Au assesses to be low risk.  While practical experience has largely
validated these policies, some applications may not be able to tolerate _any_ risk of loss.  We'll
explore the solution --- namely, _runtime checking_ --- later on in more detail.

The second is that we forbid some conversions that are actually perfectly fine, at least for certain
values.  Or, certain use cases --- for example, many embedded applications _expect and desire_ the
truncating properties of integer division.  The solution for these cases is to opt out of individual
safety checks.  We'll dive into it now.

### Opting out of safety checks

When a conversion fails to compile, the error message will tell you what risk Au was concerned
about.  You can then pass `ignore(X)` as a _second parameter to the conversion function_, and it
will ignore the risk `X`.  There are two reasons to prefer this more granular opt-out mechanism to
a blanket approach that turns off all safety checks:

1. It's more _intent-based_: readers will better understand what risk you are ignoring.
2. It's _safer_: you still have coverage for the other risk.

Let's look at some more examples to make this clear.

| Conversion | Policy | Result | Actually lossy? |
|------------|--------|--------|-----------------|
| `inches(24).as(feet, ignore(TRUNCATION_RISK))` | :thumbs_up: Allowed (by policy override) | `feet(2)` | :thumbs_up: No |
| `seconds(4u).as(nano(seconds), ignore(OVERFLOW_RISK))` | :thumbs_up: Allowed (by policy override) | `nano(seconds(4'000'000'000))` | :thumbs_up: No |

Here, we have revisited the examples from the previous table, but thanks to our policy override,
they are no longer forbidden.  We see that they do produce the correct results.  However, we
emphasize that this is due to the _specific input values_ that we used.  Very similar input values
could easily produce grossly incorrect results:

| Conversion | Policy | Result | Actually lossy? |
|------------|--------|--------|-----------------|
| `inches(23).as(feet, ignore(TRUNCATION_RISK))` | :thumbs_up: Allowed (by policy override) | `feet(1)` | :x: Yes (truncation) |
| `seconds(5u).as(nano(seconds), ignore(OVERFLOW_RISK))` | :thumbs_up: Allowed (by policy override) | `nano(seconds(705'032'704u))` | :x: Yes (overflow) |

Use the opt-out tool wisely, and carefully consider whether it's the right solution for your use
case.

### Not all conversions are actually possible

Opting out of safety checks is not sufficient to guarantee that Au will perform a conversion.  The
opt-out removes one barrier.  If there are others, the conversion will still not happen.

One common example is when users try to convert between two units with different dimensions.  This
is a meaningless operation, and is always user error.  Au will not perform this conversion because
it does not exist.

Another example is when a conversion is meaningful in principle, but Au doesn't know how to
implement it in the desired rep.  For example, `radians(1).as(degrees)` would require us to support
irrational conversion factors in integral reps, which is very hard (although [#453] tracks one
possible future solution).  In this case, if your architecture can tolerate floating point types
such as `double`, you can write `radians(1.0).as<int>(degrees)` to obtain the answer.

## Runtime checks

Here, we address the second limitation: even conversions that Au considers low-risk can be lossy.
If we tried to stick to compile time solutions, we would end up only allowing conversions where
_no_ input value is lossy.  In practice, this is incredibly restrictive.

The alternative is to provide _runtime checkers_, which can efficiently and accurately assess
whether any individual value will be lossy.

Unlike the compile time approach, we will pay a runtime cost.  However, this cost can be extremely
low --- for example, the overflow checks simply compare to a single number that we compute for each
conversion.  What's more, this cost doesn't usually _matter_, because unit conversions almost never
occur in the performance-sensitive "hot loops" of well designed programs.

A more serious issue is that you have to decide what to do if the check _fails_.  In general, there
is no one-size-fits-all error handling mechanism that would be suitable for all clients of Au.  In
general, you should use whatever error handling mechanism is appropriate for your project.

### Provided checkers

Au provides runtime checkers for overflow, truncation, and combined lossiness.  For a `Quantity q`,
converting to a new unit `u`, we provide these signatures:

| Risk | Target rep | Function |
|------|------------|----------|
| Overflow | same | `will_conversion_overflow(q, u)` |
| Overflow | New rep `T` | `will_conversion_overflow<T>(q, u)` |
| Truncation | same | `will_conversion_truncate(q, u)` |
| Truncation | New rep `T` | `will_conversion_truncate<T>(q, u)` |
| Any lossiness | same | `is_conversion_lossy(q, u)` |
| Any lossiness | New rep `T` | `is_conversion_lossy<T>(q, u)` |

In general, `is_conversion_lossy` will return true if either `will_conversion_overflow` or
`will_conversion_truncate` returns true.

### Creating checked helpers

In general, effective runtime checker use has two steps.  First, call the checker you're interested
in. Then, if it passed, call the conversion function while _opting out of the risk check you already
validated_.

It's even better to combine these in a utility function.  For example, using C++17's
`std::optional`, here's a function that returns the correct result for non-truncating inputs, or
`std::nullopt` otherwise:

```cpp
template <typename Target, typename U, typename R>
std::optional<Quantity<TargetUnit, R>> checked_conversion(Quantity<U, R> q, Target u) {
    return will_conversion_truncate(q, u)
        ? std::nullopt
        : std::make_optional(q.as(u, ignore(TRUNCATION_RISK)));
}
```

`checked_conversion(inches(35), feet)` will produce `std::nullopt`, while
`checked_conversion(inches(36), feet)` gives `std::make_optional(feet(3))`.

## Takeaways

By default, Au only permits a conversion if the risk is low, for both overflow and truncation.
However, this is just a heuristic, based on _all inputs_ for a conversion.  Almost every conversion
has _some_ inputs that go against this policy.

If your conversion is forbidden, but you're confident it's OK for your inputs, you can _pass
a second argument_ to opt out of that safety check: usually, `ignore(OVERFLOW_RISK)` or
`ignore(TRUNCATION_RISK)`.  This tool does a good job of communicating your intent, but be wary of
using it too widely.  The risk checks are there for good reason, so make sure your use case really
is OK.

By contrast, if you can't tolerate any lossiness, Au provides _runtime checkers_ that can quickly
provide accurate answers for any individual value.  This adds runtime cost, but it's rarely
important.  A harder problem is deciding what to do if the check fails.  We recommend using the
runtime checkers as building blocks for a "checked conversion" utility, because this lets you use
the error handling mechanism that's best for your project.

[#453]: https://github.com/aurora-opensource/au/issues/453
