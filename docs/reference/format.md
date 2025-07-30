# Format

Au supports string formatting using either the [{fmt}] library, or else [std::format] (available
in C++20 or later).

??? warning "Warning: if using {fmt}, ensure version is at least 9.0"
    If you are using the [{fmt}] library, **make sure that it is at least [version 9.0]** (July 5,
    2022).

    All earlier versions provide automatic specializations for `formatter<T>` for any type `T` that
    supports streaming output.  While this is convenient, it also makes it extremely easy to violate
    the One Definition Rule (ODR): if anyone calls `format` in a file without Au's specialization,
    then both definitions will exist.  This makes the program "ill-formed, no diagnostic required"
    (IFNDR), which means that it has **silently** failed to be a valid C++ program, and all
    guarantees are out the window.

    Version 9.0 was the first release to make the default specialization cause a hard compiler
    error.  This mitigates the risk of having two definitions, undetected.

    As for `std::format`, it has always required explicit specializations, so it is not subject to
    this vulnerability.

We can't provide fully seamless support out of the box without adding an external dependency --- and
Au is committed to being a zero-dependency library.  What we _can_ do is to define the format string
specification, and take the hard work out of implementing it.  To use it in your project, you'll
create a single file, and write a single line of code.  Then, you'll include that file anywhere you
want to format a `Quantity`.

## Setup

There are two steps to support formatting in your project:

1. Create a file to hold the authoritative formatter definition.

2. Include this file anywhere you want to format a `Quantity`.

Here are the contents of the file to create:

=== "Using {fmt}"

    ```cpp
    #pragma once

    #include "au/au.hh"
    #include "fmt/format.h"

    namespace fmt {
    template <typename U, typename R>
    struct formatter<::au::Quantity<U, R>> : ::au::QuantityFormatter<U, R, ::fmt::formatter> {};
    }  // namespace fmt
    ```

=== "Using std::format"

    !!! warning
        These instructions have not yet been tested, because the Au repo doesn't yet have an
        official toolchain that supports `std::format`.

    ```cpp
    #pragma once

    #include <format>

    #include "au/au.hh"

    template <typename U, typename R>
    struct std::formatter<::au::Quantity<U, R>> : ::au::QuantityFormatter<U, R, std::formatter> {};
    ```

!!! tip "Tip: finding the file in your project"
    It's very important to have _only one_ definition of `formatter<Quantity<U, R>>` in your
    project.  This means you need a way to tell whether someone has created this file already, and
    find it if it exists.

    Fortunately, the string `QuantityFormatter` is very greppable.  In most cases, it should only
    occur once in your project --- even if there are a few stray mentions, it should be easy to find
    the authoritative definition.  Start by grepping your project for this string.  If you find it,
    you can just use the file, and if not, you know you need to create it.

## Syntax

Printing a quantity means printing its numeric value, followed by its unit label.  Au lets you
provide format strings for either or both of these.  If you add a format string for the unit label,
you need to prefix it with a `U`.  If you add format strings for both, separate them with a `;`.  In
both cases, the syntax delegates to the [standard format syntax] for each piece.

It may be easier to understand with several examples.

| Specialize format for | Pattern | Example | Output |
|-----------------------|---------|---------|--------|
| Neither | `{}` | `std::format("{}", meters(123.456))` | `"123.456 m"` |
| Numeric value | `{:~^10.2f}` | `std::format("{:~^10.2f}", meters(123.456))` | `"~~123.46~~ m"` |
| Unit label | `{:U.>5}` | `std::format("{:U.>5}", meters(123.456))` | `"123.456 ....m"` |
| Both | `{:~^10.2f;U.>5}` | `std::format("{:~^10.2f;U.>5}", meters(123.456))` | `"~~123.46~~ ....m"` |

To target a specific and consistent overall width --- say, for aligning columnar output --- provide
formatters for both the numeric value and the unit label, and choose a specific width for each.  The
overall width will be the sum of the widths of the two pieces, plus one space in between.

### Specialized use cases

The core support provides a formatted numeric value, then a space `' '`, then a formatted unit
label.  There may be other use cases, but we don't support them directly, and we don't plan to.
Fortunately, these tend to have straightforward workarounds.

Here are some examples.

Besides the separate formatters for the numeric value and unit label, some users may wish to format
the "overall result".  For example, they may want the whole quantity (unit and label) to be centered
in a width of, say, 25 characters.  We cannot support this directly without excessive complexity, in
both the implementation, and the formatter syntax.  The simple workaround is to format your quantity
as desired using the existing syntax, and then format the result using simple `std::string`
formatting, like so:

```cpp
constexpr auto speed = (centi(meters) / second)(987.654321);
std::cout << fmt::format("{:.^31}", fmt::format("{:,<8.2f;U*>10}", speed));
// Output: "......987.65,, ****cm / s......"
```

Some users may want a format that prints _only_ the numeric value, or _only_ the unit label.  We
don't provide a direct way to do this, because neither of these pieces is meaningful on its own.  If
you do have a use case for formatting only one of these, the workaround is straightforward.  Simply
retrieve the numeric value or unit label by the library's standard methods, and pass the result to
a formatter for that numeric type, like so:

```cpp
constexpr auto c = 299'792.458 * km / s;
std::cout << fmt::format("{:,<12.2f}", c.data_in(km / s));
// Output: "299792.46,,,"
```

[{fmt}]: https://github.com/fmtlib/fmt
[std::format]: https://en.cppreference.com/w/cpp/utility/format/format.html
[version 9.0]: https://github.com/fmtlib/fmt/releases/tag/9.0.0
[standard format syntax]: https://hackingcpp.com/cpp/libs/fmt.html
