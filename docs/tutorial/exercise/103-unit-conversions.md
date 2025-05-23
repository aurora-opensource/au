# Au 103: Unit Conversions Exercise

!!! note
    This is the exercise for the concepts in the tutorial page [Au 103: Unit
    Conversions](../103-unit-conversions.md). We'll assume you're familiar with everything in that
    page, so if you're not, you'll want to read it first.

!!! tip
    Before you start, make sure you've followed the [developer's guide](../../develop.md) so you're
    able to build and test the code!

## Introduction

This exercise is a chance to practice unit conversions.

The file we'll be working in can be found here, relative to the repository root:

- `"tutorial/103_unit_conversions_test.cc"`

The following command executes the tests.

```sh
bazel test --test_output=all //tutorial:103_unit_conversions_test
```

## Exercise 1: ad hoc conversions

We'll practice writing some ad hoc conversions that can replace their more labor-intensive,
error-prone manual counterparts.

Since these both start and end with raw numeric types, we'll use `.in` rather than `.as`, even
though the latter is generally preferred whenever we have a choice.  (Perhaps a future refactoring
could replace these raw numeric types with _quantities_ in the rest of this hypothetical
codebase. :grin:)

Open up the file, `"tutorial/103_unit_conversions_test.cc"`, and scroll down to the `EXERCISE 1`
section.  This takes you to two unit tests in the `AdHocConversions` group: `DegreesToRadians`, and
`MilesPerHourToMetersPerSecond`.

In each test, find the unit conversion, which is currently done with manual conversion factors.
Replace it with an ad hoc inline conversion based on Au.

!!! example "Exercise 1(a)"
    === "Task"
        Convert the `DegreesToRadians` test:

        ```cpp
        TEST(AdHocConversions, DegreesToRadians) {
            constexpr double angle_deg = 135.0;

            constexpr double RAD_PER_DEG = M_PI / 180.0;

            // TODO: replace `angle_rad` computation with an ad hoc conversion, using Au.
            constexpr double angle_rad = angle_deg * RAD_PER_DEG;

            EXPECT_THAT(angle_rad, DoubleEq(3.0 * M_PI / 4.0));
        }
        ```

    === "Solution and Discussion"
        A possible solution:

        ```cpp
        TEST(AdHocConversions, DegreesToRadians) {
            constexpr double angle_deg = 135.0;




            constexpr double angle_rad = degrees(angle_deg).in(radians);

            EXPECT_THAT(angle_rad, DoubleEq(3.0 * M_PI / 4.0));
        }
        ```

        !!! tip
            Instead of _deleting_ the obsolete lines, we replaced them with
            blank lines.  This lets you go back and forth, using the left and
            right arrow keys, to see exactly what changed.

        We start with a unit-safe handoff, `degrees(angle_deg)`, from the `_deg` suffix to the
        `degrees()` function.  Then we end with another unit-safe handoff: from `.in(radians)` to
        the `_rad` "suffix" in the variable name.

        Overall, we can see that this conversion is correct by just reading this single line of
        code.  That's unit safety!

!!! example "Exercise 1(b)"
    === "Task"
        Convert the `MilesPerHourToMetersPerSecond` test:

        ```cpp
        TEST(AdHocConversions, MilesPerHourToMetersPerSecond) {
            constexpr double speed_mph = 65.0;

            // Carefully compute conversion factor manually.
            constexpr double M_PER_CM = 0.01;
            constexpr double CM_PER_INCH = 2.54;
            constexpr double INCHES_PER_FEET = 12.0;
            constexpr double FEET_PER_MILE = 5280.0;
            constexpr double M_PER_MILE = M_PER_CM * CM_PER_INCH * INCHES_PER_FEET * FEET_PER_MILE;

            constexpr double S_PER_H = 3600.0;

            constexpr double MPS_PER_MPH = M_PER_MILE / S_PER_H;

            // TODO: replace `speed_mps` computation with an ad hoc conversion, using Au.
            constexpr double speed_mps = speed_mph * MPS_PER_MPH;

            EXPECT_THAT(speed_mps, DoubleEq(29.0576));
        }
        ```

    === "Solution and Discussion"
        A possible solution:

        ```cpp
        TEST(AdHocConversions, MilesPerHourToMetersPerSecond) {
            constexpr double speed_mph = 65.0;













            constexpr double speed_mps = (miles / hour)(speed_mph).in(meters / second);

            EXPECT_THAT(speed_mps, DoubleEq(29.0576));
        }
        ```

        !!! tip
            Instead of _deleting_ the obsolete lines, we replaced them with
            blank lines.  This lets you go back and forth, using the left and
            right arrow keys, to see exactly what changed.

        As before, we start and end our conversion with _unit-safe handoffs_.  The units themselves
        are a little more complicated than before because they're compound units, but overall this
        Au-based conversion has similar readability to the one from the previous example.

        We can't say the same for the code it replaced, however --- it's _much_ more complicated
        than the previous example!  In order to write it in a form which is clearly correct, we need
        to chain together a string of carefully named elementary conversion factors.  Even once we
        do, it probably takes some squinting to convince yourself that `MPS_PER_MPH` really is
        "meters-per-mile" divided by "seconds-per-hour".

        The Au-based conversion is a huge improvement in readability and simplicity, as evidenced by
        all the whitespace above (which we can now remove).

## Exercise 2: decomposing inches onto feet-and-inches

Now we want to take a height in inches, and decompose it so we can represent it as some integer
number of feet, plus some integer number of inches which measures less than a foot.  Scroll down to
the function `decompose_height()`, and fill in the `PLACEHOLDER` instances with correct expressions
that will make the test below pass.

You may find it useful to use the explicit-Rep overload to force a conversion that is _generally_
risky, but in _this_ instance is known to be OK.  You should not need to use floating point
numbers at all.

!!! example "Exercise 1(b)"
    === "Task"
        Replace `PLACEHOLDER` instances with correct expressions.

        ```cpp
        // Decompose a height, given in inches, into the largest whole number of feet, plus the leftover
        // inches.  For example, `inches(17)` would be decomposed into `Height{feet(1), inches(5)}`.
        Height decompose_height(QuantityU32<Inches> total_height) {
            Height h;
            h.feet = PLACEHOLDER;
            h.inches = PLACEHOLDER;
            return h;
        }
        ```

    === "Solution and Discussion"
        A possible solution:

        ```cpp
        // Decompose a height, given in inches, into the largest whole number of feet, plus the leftover
        // inches.  For example, `inches(17)` would be decomposed into `Height{feet(1), inches(5)}`.
        Height decompose_height(QuantityU32<Inches> total_height) {
            Height h;
            h.feet = total_height.coerce_as(feet);  // NOTE: truncation is intended.
            h.inches = total_height - h.feet.as(inches);
            return h;
        }
        ```

        If we convert the total height from inches to feet, and force it to remain an integer, it
        will truncate.  In this case, that's actually the desired behaviour!  (Of course, it's nice
        to leave a comment to reassure the reader.)  We can then reduce the _total_ height by this
        integral number of feet, and we obtain the leftover inches.

        !!! tip
            As it happens, the `.as(inches)` is unnecessary.  You can subtract a `QuantityU32<Feet>`
            from a `QuantityU32<Inches>` directly!  If you do, the quantity of feet will be
            _automatically converted_ to inches, because inches is a "common unit" in which we can
            express both quantities.  However, common units will be the topic of a later, 200-level
            tutorial, so we wrote the example solution using only constructs you've already seen.

## Summary

This exercise gave a few worked examples of unit conversions.  We saw how Au can immediately replace
crufty, error-prone manual conversions, making code more readable.  We also saw a practical example
of quantity-to-quantity conversion using `.as(...)`, including one example where the forcing
"explicit-Rep" form is clearly safe and correct to use.

Return to the [main tutorial page](../103-unit-conversions.md).
