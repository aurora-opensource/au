# Analog-to-digital converter counts to millivolts

A 12-bit analog-to-digital converter (ADC) reports voltages from 0 to 3300 millivolts (mV) as
integer counts from 0 to 4095.  For example, a reading of 2048 "ADC counts" corresponds to a voltage
of roughly 1650 mV: about half of the 3300 mV maximum.

Let's see how we would safely handle taking a reading from this device, and expressing it in more
familiar units.

=== "⚠️ Before: raw C++"

    ⚠️ **Before** --- the scale factor is a pair of magic numbers, and truncation is silent.
    { .ab-banner .ab-before }

    ??? note "Includes and usings"

        ```cpp
        --8<-- "examples/adc_millivolts/raw.cc:frontmatter"
        ```

    ```cpp
    --8<-- "examples/adc_millivolts/raw.cc:example"
    ```

=== "✅ After: with Au"

    ✅ **After** --- the hardware's count *is* a unit, and truncation must be opted into.
    { .ab-banner .ab-after }

    ??? note "Includes and usings"

        ```cpp
        --8<-- "examples/adc_millivolts/au.cc:frontmatter"
        ```

    ```cpp
    --8<-- "examples/adc_millivolts/au.cc:example"
    ```

Both programs print `1650 mV`.

## What's happening

Handling quantities on embedded devices, such as ADCs, has historically been a pain.  Each device
maps its integer output to its own bespoke range of values.  You would need to either convert to SI
units immediately (even if that might be less efficient for your use case), or risk passing a number
around your program with obscure or confusing units.

This is an ideal use case for Au!  The trick is to turn this range into a full-fledged _custom
unit_.  You get the safety and ergonomics of Au, from the moment the value leaves the board. You can
even _safely keep the value in its natural ADC units_ as long as you want, confident that any
conversions you _do_ need will be tracked and checked by the library.

One reason people are often reluctant to use units libraries with embedded applications is the need
for exact integer arithmetic.  Most units libraries use floating point as a strong default; even if
they technically support integer types, it's rare for them to be battle-tested enough to trust.  Not
so with Au: Aurora's embedded teams have been first-class customers since the library's inception,
so integer arithmetic is our bread and butter.  With Au, the conversion here compiles to the same
integer multiply-and-divide the raw version performs by hand.  Everything else follows from that:

- **The "magic numbers" move into the unit definition.**  Read them right off the spec sheet, and
  put them in one single place in your codebase.  Au will automatically generate the most efficient
  conversion factor to _any_ target unit, and apply it with a [tailor made
  strategy](../discussion/implementation/applying_magnitudes.md) for your specific situation.

- **A reading is now a quantity, from the moment it arrives.**  `adc_counts(2048)` is a voltage,
  and can be passed anywhere a voltage belongs, compared against other voltages, or converted to
  any other voltage unit.  In the raw version it is an `int` until someone remembers to call the
  conversion function.

- **You're in control of conversion risks.**  Au catches the truncating integer division here.  By
  default, it won't compile, but the `ignore(TRUNCATION_RISK)` [risk
  policy](../reference/conversion_risk_policies.md) argument overrides that.  Notice the
  readability: any risky conversions you accept _stand out visually_, and clearly communicate your
  intent to the reader.

- **The label comes along.**  Because `AdcCounts` declares one, a count prints as `counts`, and the
  converted value prints as `1650 mV` without anyone typing `"mV"` into a format string.

- **None of this costs anything at runtime.**  The rational factor is a compile-time constant, and
  the rep stays `int`: Au does not silently promote you to `double` to make a conversion work.  In
  fact, Au reduces 3300/4095 to 220/273 for you, so the same instruction sequence runs with a
  15-times-smaller intermediate product than the hand-written version: a significant reduction in
  overflow risk!

### A note on `<iostream>`

These examples print with `std::cout`, because it lets the value carry its own unit label and keeps
the whole example inside the library.  Plenty of embedded projects avoid `<iostream>` for code
size, and Au is entirely indifferent: nothing in the library depends on it.  Drop
[`au/io.hh`](https://github.com/aurora-opensource/au/blob/main/au/io.hh), reach for `.in()` instead
of `.as()`, and print however you like:

```cpp
std::printf("%d mV\n", v.in(milli(volts), ignore(TRUNCATION_RISK)));
```

## Related reading

- [Quantity: `.in()` and `.as()`](../reference/quantity.md), and which conversions are allowed.
- [Prefixes](../reference/prefix.md), for `milli(volts)`.
- [Rounding](../reference/math.md), when you *do* want the truncating conversion.
- [Namespaces and includes](../discussion/idioms/namespaces-and-includes.md), including why a `.cc`
  imports names individually and a `.hh` does not.
