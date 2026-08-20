# Au documentation

Welcome to the Au library!  Au (pronounced "ay yoo") is a C++14-compatible units library, by
[Aurora](https://aurora.tech/).  Its key strengths include safety, accessibility, performance, and
developer experience.

The library source is on GitHub, at
[aurora-opensource/au](https://github.com/aurora-opensource/au).

## What it looks like

Here's a wheel's road speed, converted to revolutions per minute (RPM).  Click between the tabs to
compare.

=== "⚠️ Before: raw C++"

    ⚠️ **Before** --- conversion factors are magic numbers, and units completely lack enforcement.
    { .ab-banner .ab-before }

    ```cpp
    --8<-- "examples/angular_velocity/raw.cc:headline"
    ```

=== "✅ After: with Au"

    ✅ **After** --- the types carry the units, and `rad / r` states the definition directly.
    { .ab-banner .ab-after }

    ```cpp
    --8<-- "examples/angular_velocity/au.cc:headline"
    ```

The `2π` and the `60` vanish: in their place, Au _automatically_ generates the [_single_
number](./discussion/implementation/applying_magnitudes/#summary-and-conclusion) that represents the
overall conversion --- _at compile time_, so there's no runtime penalty.  And the code now directly
states the physical truth: converting between linear and angular speed is just applying
a length/angle ratio, and `rad / r` --- "one radian per radius" --- is nothing more than the
_definition_ of a radian, applied as a formula.
<!-- The last sentence above, in particular, sounds very "Claude-y" to me.  But I actually wrote
this prose myself, from scratch!  I guess I have a robotic personality.  Beep boop. -->

See **[Code examples](./examples/index.md)** for this one [in full](./examples/angular-velocity.md),
and several more.

## Getting started

These pages will be most useful as you begin your Au journey:

- **[Alternatives](./alternatives/index.md).**  First off: is Au the right fit for _your_ needs?
  What else is out there?  This page gives a detailed comparison to some of the most prominent other
  C++ units libraries.

- **[Code examples](./examples/index.md).**  Short, complete programs showing what Au looks like on
  real problems --- each one paired with the plain C++ version, so you can see exactly what changes.

- **[Supported Compilers](./supported-compilers.md).**  Au aims to work with _any_ C++ compiler that
  fully supports C++14 or later.  That said, some platform/compiler combinations have more detailed
  data.  Learn about our tiers of support and what it takes to add a specific compiler.

- **[Installation](./install.md).**  Once you're ready to try it out, here's how.  You can be up and
  running in any project within minutes!

- **[Tutorials](./tutorial/index.md).**  Start with [Au 101: Quantity
  Makers](./tutorial/101-quantity-makers.md), and go forward from there.  You'll learn the basic
  library concepts, and get some hands on experience with the included exercises.

## Continuing your journey

Once you're up and running with the library, these pages will be handy tools to help you use it more
effectively:

- **[Troubleshooting](./troubleshooting.md).**  A guide to the most commonly encountered types of
  error, what they mean, and how to fix them.  Take key snippets from your compiler errors, and
  use in-page search to get help!

- **[How-to guides](./howto/index.md).**  Step-by-step instructions for accomplishing common tasks
  you may encounter in using the library.

- **[Reference](./reference/index.md).**  Detailed reference documentation on `Quantity`,
  `QuantityPoint`, units, magnitudes, and other core library abstractions.

We also have a [GitHub Issues](https://github.com/aurora-opensource/au/issues) page for tracking
problems and future work.  If you have a bug report or feature request, check the existing issues to
see if it's been posted, and file a new one if it hasn't.  While we can't promise timely
_resolution_, we will do our best to respond quickly so you know both that you've been heard and
where we stand on the issue.

!!! tip
    Feel free to vote for existing issues by reacting to the main post with the :+1: emoji: we'll
    take this into account in prioritizing what to work on!

## Shoring up foundations

When you're looking to understand the library better (as opposed to actively trying to accomplish
some task), these docs will help you strengthen your foundations.

- **[Discussion](./discussion/index.md).**  Philosophy and principles, deep dives on design choices,
  explanations of core concepts, and more.
