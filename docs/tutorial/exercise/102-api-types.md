# Au 102: API Types Exercise

!!! note
    This is the exercise for the concepts in the tutorial page [Au 102: API
    Types](../102-api-types.md). We'll assume you're familiar with everything in that page, so if
    you're not, you'll want to read it first.

!!! tip
    Before you start, make sure you've followed the [developer's guide](../../develop.md) so you're
    able to build and test the code!

## Introduction: the legacy function

Here's a simple example of a function that deals with physical quantities, but lacks _unit safety_
because it's using raw numeric types.

```cpp
// The distance (in meters) it would take to stop, with a given starting speed
// and constant (negative) acceleration.
//
// Parameters:
// - speed_mps:  The starting speed, in meters per second.
// - acceleration_mpss:  The braking acceleration, in meters per second squared.
//
// Preconditions:
// - speed_mps >= 0.0
// - acceleration_mpss < 0.0
constexpr double stopping_distance_m(
    double speed_mps, double acceleration_mpss);
```

We're going to provide a unit-safe interface for any clients that want to use it.  We'll leave the
old interface in place, so we won't be forced to update our codebase all at once!

To make sure you're set up and ready to go, run `bazel test //tutorial:102_api_types_test`.  All
tests should pass.

The three files we'll be working in can be found here, relative to the repository root:

- `"tutorial/102_api_types.hh"`
- `"tutorial/102_api_types.cc"`
- `"tutorial/102_api_types_test.cc"`

## Exercise 1: new interface shim

Open up the test file, `"tutorial/102_api_types_test.cc"`, and scroll down to the `EXERCISE 1`
section.  You'll find two test cases below, which are commented out.  Uncomment them, and rerun the
tests.  As before:

```sh
bazel test //tutorial:102_api_types_test
```

They'll fail.

!!! example "Exercise 1(a)"
    === "Task"
        Make these new tests compile by declaring a new function, in `"tutorial/102_api_types.hh"`,
        with this signature:

        ```cpp
        AAA stopping_distance(BBB speed, CCC acceleration);
        ```

        The types `AAA`, `BBB`, and `CCC` are _placeholders_ for quantity types; it's your job to
        figure out the actual types which should go there.  The `Rep` for each quantity should be
        `double`, since that's what the function we're replacing uses.  As for the `Unit`, you may
        find it useful to define appropriate aliases for, say, `MetersPerSecond` and
        `MetersPerSecondSquared`.

    === "Solution and Discussion"
        Something like this inside of your `"tutorial/102_api_types.hh"` file should work:

        ```cpp
        using MetersPerSecond = decltype(Meters{} / Seconds{});
        using MetersPerSecondSquared = decltype(MetersPerSecond{} / Seconds{});

        QuantityD<Meters> stopping_distance(QuantityD<MetersPerSecond> speed,
                                            QuantityD<MetersPerSecondSquared> acceleration);
        ```

        First, we declared the aliases for the compound units.  Then, we made a new signature with
        quantity types instead of raw `double`.  We used the `QuantityD<U>` form, rather than
        `Quantity<U, double>`, for conciseness.

        !!! warning
            When composing unit type instances (like `Meters{}`), we don't get the same fluency as
            when we compose quantity makers (like `meters`).  In particular, we can't offer the same
            grammatical fluency.  With quantity makers, we would write `meters / second`.  However,
            with unit type instances, we end up needing to write `Meters{} / Seconds{}`, rather than
            `Meters{} / Second{}`.

        Note that we also removed the suffixes (such as `_m`, `_mps`, and `_mpss`) from the variable
        and function names.  Those suffixes were there to help _humans_ keep track of units.  Now
        that's the _compiler's_ job!

If we rerun the tests now, we'll find we get a _linker_ error rather than a compiler error.  This
makes sense: our function hasn't been implemented yet.

!!! example "Exercise 1(a)"
    === "Task"
        Go to `102_api_types.cc`, and implement the function you had just declared.

        Your function implementation should amount to a thin wrapper on the existing function.
        First, we'll "unwrap" the input quantity parameters, giving us raw `double` values we can
        pass to the old version.  Then, we'll "wrap" the answer with the appropriate quantity maker.

    === "Solution and Discussion"
        Something like this inside of your `"tutorial/102_api_types.cc"` file should work:

        ```cpp
        QuantityD<Meters> stopping_distance(QuantityD<MetersPerSecond> speed,
                                            QuantityD<MetersPerSecondSquared> acceleration) {
            return meters(stopping_distance_m(speed.in(meters / second),
                                              acceleration.in(meters / squared(second))));
        }
        ```

        Notice how we have a _unit-safe handoff_ on the output.  We're passing the result of
        `stopping_distance_m(...)` to `meters(...)`, and we can see that the units match.

        We _don't_ have a unit-safe handoff on the _inputs_.  The only way to check they're correct
        is by carefully inspecting the order and comparing to the signature of
        `stopping_distance_m()`.  Fortunately, that function lives in the same file, so it's not too
        onerous to verify the correctness.

        ??? info "Reminder about `.in(...)` syntax"
            The argument you pass needs to be an _instance_, not a _type_.  It may be tempting to
            call `speed.in(MetersPerSecond)`, but that's not valid C++ (you can't "pass a type" to
            a function).  That's why we passed the quantity maker instead: `speed.in(meters
            / second)`.

## Exercise 2: reverse the roles

At this point, we're in pretty decent shape.  We've provided a unit-safe interface which anyone can
take advantage of.  And we've left the old interface in place, so we haven't broken any existing
clients!

However, we can do a lot better.

First, if you read the commentary in the previous section's solution, you'll know that the input
parameters don't have a unit-safe handoff.  That's a small problem.  A bigger problem is that we're
not taking full advantage of the units library in our implementation itself!  To see why this
matters, introduce a unit error by deleting the `* t_s` from the end of the implementation:

```cpp
    return speed_mps * t_s + 0.5 * acceleration_mpss * t_s * t_s;
    //                                      Delete this---^^^^^^
```

Save the file and rerun all the tests.  Even though the implementation is wrong, the tests all pass!

Note that we're adding a distance and a speed here. If we were using _quantities_, the wrong code
wouldn't even _compile_.  (That said: in a real project, this would also be a good signal to add
more test cases, too.  :sweat_smile:)

Undo the error you introduced.  Now you're ready to fix both these problems at once.

!!! example "Exercise 2"
    === "Task"
        Reverse the roles of the functions.

        - Use **quantity** types for the **core logic**.
        - Turn the **raw numeric** version into the **thin wrapper**.

        All existing tests should pass without modification.

    === "Solution and Discussion"
        Something like this inside of your `"tutorial/102_api_types.cc"` file should work:

        ```cpp
        double stopping_distance_m(double speed_mps, double acceleration_mpss) {
            // Convenient ad hoc quantity makers, for readability.
            constexpr auto mps = meters / second;
            constexpr auto mpss = meters / squared(second);

            return stopping_distance(mps(speed_mps), mpss(acceleration_mpss)).in(meters);
        }

        QuantityD<Meters> stopping_distance(QuantityD<MetersPerSecond> speed,
                                            QuantityD<MetersPerSecondSquared> acceleration) {
            // This first implementation uses only the most basic kinematic equations.
            // We could refactor it later to be more efficient.

            // t = (v - v0) / a
            const auto t = -speed / acceleration;

            // (x - x0) = (v0 * t) + (1/2)(a * t^2)
            return speed * t + 0.5 * acceleration * t * t;
        }
        ```

        Here, we chose to separate out the construction of our quantity makers, so we could make the
        "real" line more concise and readable.  This has no impact on performance either way, but in
        this case it makes the unit-safe handoffs easier to see at a glance.  Do whatever makes your
        code the most readable!

        As for the "core logic" implementation, note how it turns out to be much cleaner without the
        unit suffixes clogging up the variable name.  It's also more robust.  If you try introducing
        the same unit error we did up above, you should find that it gets caught at compile time.

## Summary

This exercise showed how you can upgrade the unit safety of your codebase **incrementally**, without
forcing huge changes across your project.  You can provide a new, unit-safe API which coexists with
the old one, which becomes a thin wrapper.  You can migrate your clients independently in small
batches.  And when they're all migrated, simply delete the old one!

!!! tip
    As you become more proficient with the library, you may prefer to skip straight to the second
    step, and turn the old function into the "shim" right away.  This is generally a better
    approach.

Return to the [main tutorial page](../102-api-types.md).
