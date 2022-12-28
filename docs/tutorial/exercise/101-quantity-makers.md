# Au 101: Quantity Makers Exercise

!!! note
    This is the exercise for the concepts in the tutorial page [Au 101: Quantity
    Makers](../101-quantity-makers.md). We'll assume you're familiar with everything in that page,
    so if you're not, you'll want to read it first.

TODO: Make a "development setup" page; link to it from here.

## Introduction

The point of this exercise is to get some practice making and using quantities.  We'll get values
into and out of the units library, and we'll also perform a few operations (simple arithmetic, and
streaming output).

The file we'll be working in can be found here, relative to the repository root:

- `"tutorial/101_quantity_makers.cc"`

The following command executes the code:

```sh
bazel run //tutorial:101_quantity_makers
```

Run it now to make sure everything's working: it should run to completion, and all tests should
pass.

## Exercise 1: printing quantities

Open up the file, `"tutorial/101_quantity_makers.cc"`, and scroll down to the `EXERCISE 1(a)`
section.  This takes you to a function, `print_raw_number_and_quantity()`, where you'll begin the
exercise.

!!! example "Exercise 1(a)"
    === "Task"
        Read through the function `print_raw_number_and_quantity()`, and uncomment the final two
        lines.  Note that in the second line, we're streaming a quantity variable to output. What do
        you _expect_ to see?

        When you've formed an expectation, run the target:

        ```sh
        bazel run //tutorial:101_quantity_makers
        ```

        What _do_ you see?  Did it match your expectations?

    === "Solution and Discussion"
        Here's what will be printed:

        ```
        track_length_m: 100
        track_length: 100 m
        ```

        Note how the unit information has effectively migrated.  With the raw `double`, it was
        a suffix on the variable name, which means that _human readers_ are responsible for keeping
        track of it.  With the quantity, it becomes a part of the type itself, so the _compiler_ is
        responsible for keeping track of it.

        The Au library has a compile-time label for every unit.  When we stream the quantity, we
        first stream its underlying value, and then stream the unit label.

Now we have a more interactive example.  We'll create several quantities, and for each one, you need
to write how you expect it to be printed.

!!! example "Exercise 1(b)"
    === "Task"
        Scroll down to the `PrintsAsExpected` test case (just below the `EXERCISE 1(b)` comment
        block), and uncomment each test assertion one at a time.  Replace the empty string
        placeholder, `""`, with the actual string you expect when streaming this quantity.

        For example, if you see this:

        ```cpp
        EXPECT_EQ(stream_to_string(squared(meters)(100)), "");
        ```

        then you would replace it with this:

        ```cpp
        EXPECT_EQ(stream_to_string(squared(meters)(100)), "100 m^2");
        ```

    === "Solution and Discussion"
        When you're done, your assertions should look something like this:

        ```cpp
        EXPECT_EQ(stream_to_string(meters(100)), "100 m");

        EXPECT_EQ(stream_to_string(meters(100.0) / seconds(8.0)), "12.5 m / s");
        EXPECT_EQ(stream_to_string((meters / second)(12.5)), "12.5 m / s");

        EXPECT_EQ(stream_to_string((meters / second)(10.0) / seconds(8.0)), "1.25 m / s^2");
        EXPECT_EQ(stream_to_string((meters / second)(10.0) * seconds(8.0)), "80 m");
        ```

        The first is a warm-up problem: a checkpoint to make sure you're doing the exercise
        correctly.

        The second gives an example of a general principle: when we multiply or divide quantities,
        we can reason _independently_ about the units, and the underlying values.  In this case, we
        know that the result's underlying value will be `100.0 / 8.0`, that is, `12.5`.  And the
        unit will be meters ($\text{m}$) divided by seconds ($\text{s}$), that is, $\text{m}
        / \text{s}$.  (Notice how the library automatically generates a label for compound units:
        from the input labels `m` and `s`, the compound label `m / s` is generated at compile time.)

        The third example represents the same quantity as the second, except that it's constructed
        directly via the _compound quantity maker_, `(meters / second)`.  Again, as in the tutorial
        page, note the grammar: we write `(meters / second)`, _not_ <s>`(meters / seconds)`</s>.

        The fourth example shows that when we accumulate powers of the same unit, the automatically
        generated label knows how to represent these.  "Meters per second", divided by "seconds",
        gives "meters per squared second", labeled as `m / s^2`.

        The fifth example shows that when any unit cancels out completely, it gets dropped from the
        label.  "Meters per second", times "seconds", gives simply "meters".


## Exercise 2: implementing with quantities

!!! example "Exercise 2"
    === "Task"
        Replace the implementation of the function `stopping_accel_mpss()` with quantities, instead
        of raw `double`s. You will probably want to do this in three stages.

           1. Create a new quantity variable for each parameter.  It should have the same name,
              minus the unit suffix.  For example, for a parameter `double duration_s`, you would
              write:

              ```cpp
              const auto duration = seconds(duration_s);
              ```

              (Note the unit-safe handoff, between the quantity-maker `seconds` and the unit suffix
              `_s`.)

           2. Replace the raw `double`s in the core computation with their corresponding quantity
              variables.  Note that you'll need to change both the type (to `auto`), and the name
              (to eliminate the suffix).

           3. Use `.in(...)` to extract the raw double to return.  You'll need to form the correct
              quantity maker to pass to it.

    === "Solution and Discussion"
        When you're done, your implementation should look something like this:

        ```cpp
        // Example updated solution:
        double stopping_accel_mpss(double initial_speed_mps, double stopping_distance_m) {
            const auto initial_speed = (meters / second)(initial_speed_mps);
            const auto stopping_distance = meters(stopping_distance_m);

            const auto accel = -(initial_speed * initial_speed) / (2.0 * stopping_distance);

            return accel.in(meters / squared(second));
        }
        ```

        Let's evaluate the code change relative to the original, which we'll reproduce here for
        convenience.

        ```cpp
        // Original:
        double stopping_accel_mpss(double initial_speed_mps, double stopping_distance_m) {
            const double accel_mpss =
                -(initial_speed_mps * initial_speed_mps) / (2.0 * stopping_distance_m);

            return accel_mpss;
        }
        ```

        - **PRO:**
            - :heavy_check_mark: The core computation is now protected from unit errors.
            - :heavy_check_mark: The core computation is also less cluttered without the unit
              suffixes.
            - :heavy_check_mark: The runtime performance is identical, with even the slightest
              optimization levels enabled.
        - **CON:**
            - :x: We've added extra lines to turn our `double` variables into quantities.
            - :x: We haven't improved unit safety at the callsites at all.

        Admittedly, this change is pretty marginal for such a short function, but keep in mind that
        this example is just a baby step.  The real power of the library will come when we learn how
        to use quantities in our _API types_: our function parameters, return values, and member
        variables.  Then we'll gain far-reaching improvements that can make development faster and
        safer across an entire codebase.

## Summary

This exercise showed the basics of how to work with quantities.  Here are some skills we practiced:

- Turning a raw numeric value into a quantity, by calling a quantity maker (for example,
  `meters(5.0)`).
- Extracting raw numeric values from quantities, by calling `.in(...)` (for example,
  `accel.in(meters / squared(second))`).
- Forming and using compound quantity makers, such as `(meters / second)(12.5)`.
- Performing basic operations with quantities, including:
    - Multiplying and dividing them.
    - Streaming them to output (but see the note below).

!!! note
    I/O streaming support isn't included in _every_ [installation method](../../install.md).

    - For single-file installations, it's included by default, but can be omitted by choice.
    - For full library installations, one needs to include `"au/io.hh"` to get this
      functionality.

    The gist is that it will tend to be either already included, or easily accessible, unless
    somebody made a conscious decision during the installation to exclude it.

Return to the [main tutorial page](../101-quantity-makers.md).
