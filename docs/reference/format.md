# Format

Au supports string formatting for both `Quantity` and `QuantityPoint` using either the [{fmt}]
library, or else [std::format] (available in C++20 or later).

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
create a single file, and write a few lines of code.  Then, you'll include that file anywhere you
want to format a `Quantity` or `QuantityPoint`.

## Setup

The setup depends on whether you are using `std::format`, or the [{fmt}] library.

### Using `std::format`

Include `"au/std_format.hh"`.  This provides formatters for both `Quantity` and `QuantityPoint`.

Note that this will not work unless you are building on a toolchain that fully supports
`std::format` and the `<format>` header.

### Using `{fmt}`

There are two steps to support formatting in your project:

1. Create a file to hold the authoritative formatter definitions.

2. Include this file anywhere you want to format a `Quantity` or `QuantityPoint`.

Here are the contents of the file to create:

```cpp
#pragma once

#include "au/au.hh"
#include "fmt/format.h"

namespace fmt {
template <typename U, typename R>
struct formatter<::au::Quantity<U, R>> : ::au::QuantityFormatter<U, R, ::fmt::formatter> {};

template <typename U, typename R>
struct formatter<::au::QuantityPoint<U, R>>
    : ::au::QuantityPointFormatter<U, R, ::fmt::formatter> {};
}  // namespace fmt
```

!!! tip "Tip: finding the file in your project"
    It's very important to have _only one_ definition of `formatter<Quantity<U, R>>` in your
    project.  This means you need a way to tell whether someone has created this file already, and
    find it if it exists.

    Fortunately, the string `QuantityFormatter` is very greppable.  In most cases, it should only
    occur once in your project --- even if there are a few stray mentions, it should be easy to find
    the authoritative definition.  Start by grepping your project for this string.  If you find it,
    you can just use the file.  If not, you know you need to create it.

## Syntax

### `Quantity` formatting

Printing a `Quantity<U, R>` means printing its numeric value, followed by its unit label.  Au offers
some degree of formatting control for each of these.

- For the **numeric part**, the syntax is exactly the same as for formatting the underlying
  representation type `R`.  For example, if `R` is `double`, you can use any of the standard format
  specifiers for `double`.
- For the **unit label**, we offer very limited control: only a minimum label string width.  If the
  label is shorter than this, we will pad it on the right with spaces.  The syntax is a `U` (for
  "unit label") followed by digits representing the minimum width.
- If both are present, then the unit label format string must come first, and they must be separated
  by a `;`.

It may be easier to understand with several examples.

| Specialize format for | Pattern | Example | Output |
|-----------------------|---------|---------|--------|
| Neither | `{}` | `fmt::format("{}", meters(123.456))` | `"123.456 m"` |
| Numeric value | `{:~^10.2f}` | `fmt::format("{:~^10.2f}", meters(123.456))` | `"~~123.46~~ m"` |
| Unit label | `{:U5}` | `fmt::format("{:U5}", meters(123.456))` | `"123.456 m    "` |
| Both | `{:U5;~^10.2f}` | `fmt::format("{:U5;~^10.2f}", meters(123.456))` | `"~~123.46~~ m    "` |

### `QuantityPoint` formatting

`QuantityPoint` uses the same format syntax as `Quantity`, but wraps the output in `@(...)` to
visually distinguish points (absolute values) from quantities (differences).  The `@` can be read as
"at", indicating a position on a measurement scale rather than a difference between positions.

| Specialize format for | Pattern | Example | Output |
|-----------------------|---------|---------|--------|
| Neither | `{}` | `fmt::format("{}", meters_pt(123.456))` | `"@(123.456 m)"` |
| Numeric value | `{:~^10.2f}` | `fmt::format("{:~^10.2f}", meters_pt(123.456))` | `"@(~~123.46~~ m)"` |
| Unit label | `{:U5}` | `fmt::format("{:U5}", meters_pt(123.456))` | `"@(123.456 m    )"` |
| Both | `{:U5;~^10.2f}` | `fmt::format("{:U5;~^10.2f}", meters_pt(123.456))` | `"@(~~123.46~~ m    )"` |

### Controlling overall width

To target a specific and consistent overall width --- say, for aligning columnar output --- provide
formatters for both the unit label and the numeric value, and choose a specific width for each.  The
overall width will be the sum of the widths of the two pieces, plus one space in between.

### Specialized use cases

The core support provides a formatted numeric value, then a space `' '`, the unit label, and
optional padding.  There may be other use cases, but we don't support them directly, and we don't
plan to. Fortunately, these tend to have straightforward workarounds.

Here are some examples.

Besides the separate formatters for the numeric value and unit label, some users may wish to format
the "overall result".  For example, they may want the whole quantity (unit and label) to be centered
in a width of, say, 25 characters.  We cannot support this directly without excessive complexity ---
both in the implementation, and in the formatter syntax.  The simple workaround is to format your
quantity as desired using the existing syntax, and then format the result using simple `std::string`
formatting, like so:

```cpp
constexpr auto speed = (centi(meters) / second)(987.654321);
std::cout << fmt::format("{:.^31}", fmt::format("{:U10;,<8.2f}", speed));
// Output: "......987.65,, cm / s    ......"
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
