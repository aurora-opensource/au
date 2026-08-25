# Namespaces and includes

Two questions come up as soon as you start writing Au code in earnest: which headers to include, and
how to bring Au's names into scope.  This page gives our recommendations and the reasoning behind
them.  Our [code examples](../../examples/index.md) follow these conventions throughout, so you can
see them applied to real programs.

## `#include` lines

Every file that uses Au will include `"au/au.hh"`, plus a collection of individual files from one or
more of:

- `"au/units/..."`
- `"au/units/literals/..."`
- `"au/constants/..."`

Sometimes, this can seem like a large number of files.  However, we believe experience shows this to
be a strongly justified tradeoff.  Consider the following:

1. Giving each unit, literal, and constant its own file empowers users to include _only_ what they
   need.  Since accumulating these objects is the main cause of slow compilation in units libraries,
   this _maximizes_ compile time performance.

2. Many other libraries try to "group" these items into related units: whether by system of
   measurement (e.g., "SI"), or by dimension (e.g., "length").  We consider this a clear design
   mistake.  Not only does this harm compile time performance, but _it makes the headers harder to
   guess_ --- and, therefore, it makes the library harder to use.

The pattern should quickly become clear: `"au/au.hh"` for the core, and one _easily guessable_
header for each specific unit, literal, or constant that you actually use.

### Optional feature headers

A few pieces of functionality live behind their own header, because not every project wants to pay
for them:

| Header | Provides |
|--------|----------|
| `"au/io.hh"` | `operator<<`, for printing quantities to a stream |
| `"au/std_format.hh"` | `std::format` support |
| `"au/testing.hh"` | Utilities for writing googletest tests |

This is the same principle as above, applied to _features_ rather than units.  `"au/io.hh"` is the
one you'll meet first, and it makes the point well: it pulls in the C++ streams machinery, which is
famously expensive, and which many embedded projects avoid entirely.  Charging that cost to every
Au user --- including those who never print a quantity --- would be a poor trade.  So we don't:
you include it when you want it, and otherwise you never pay.  (We take this seriously enough
that our [single-file packages](../../install.md#single-file) come in `noio` variants with the
streaming support stripped out.)

These headers are _additions_ to `"au/au.hh"`, not replacements for it: `"au/io.hh"` teaches the
library how to print, but it doesn't bring in the core.  Our examples that print a result include
both.

!!! note
    Under Bazel, each of these is a separate dependency as well as a separate header ---
    `@au//au:io`, `@au//au:std_format`, `@au//au:testing` --- so you'll need to add it to your
    `deps`.  Under CMake, the single `Au::au` target provides all of them, and you only need the
    `#include`.  See the [installation instructions](../../install.md) for the full table.

## Individual `using` statements

When code is within an implementation file (`.cc`, `.cpp`, etc.), we recommend giving every Au
object a "using declaration", making its name visible within that file.  This is the same convention
that [Googletest recommends](https://google.github.io/googletest/gmock_cook_book.html).

The downside is that this section of declarations can become lengthy.  If you'd rather avoid this,
it's fine; we've designed Au so that even the fully qualified names are as concise as possible (just
an `au::` prefix: four characters).  However, we believe the benefits win out on balance.

1. The prefix may be _small_, but Au is designed for _composability_, so it can be repeated many
   times.  Compare `speed.in(au::kilo(au::meters) / au::hour)` to `speed.in(kilo(meters) / hour)`:
   the latter has notably better readability and flow.

2. The cost is small: it only affects one place in the file, and the location is predictable.

3. The more times you use a name, the more the `using` declaration pays off.

What you will _not_ find in our examples is `using namespace au;`.  We don't recommend it, so we
don't model it:[^1] `au::` is deliberately two characters, and a directive that drags in every unit,
maker, and symbol in the library is a poor trade for saving them.  (It's the same instinct that
keeps `using namespace std;` out of most codebases.  `std::` is short for the same reason `au::` is,
and in both cases the directive gives up a great deal of clarity to save very little typing.)

[^1]: We _do_ use `using namespace au;` in the canonical "quickstart" [godbolt link] from our
README, because the goal of that page is to make it as frictionless as possible to write Au code.
It's not intended to showcase _landable_ code; it's intended to make it as easy as possible to see
how the library behaves in some situation, or to use it to get some result.  To further illustrate
this point, note that our godbolt link also includes every unit, constant, and literal in the whole
library, even though we would _never_ recommend this in production code, because the compile time
cost doesn't scale.

[godbolt link]: https://godbolt.org/z/o91nfh5Wc

### Header files cannot take this approach {#headers}

The above advice applies to implementation files (`.cc`, `.cpp`, etc.) _only_.  _Header_ files
(`.hh`, `.hpp`, etc.) are a different story altogether.

The core reason is that `#include` is simple textual inclusion in C++, and it works transitively.
This means any names we expose in a header file will _silently leak_ into every translation unit
that includes it: an unbounded set that grows over time.  This problem is known as "namespace
pollution", and experience has shown that it causes a litany of problems.

- Dependencies become entangled and hard to extract.

- It becomes hard to understand where names come from.

- The build can break due to name collisions.

So we can't use `using` declarations at namespace scope in a header file.  Instead, there are two
alternatives that avoid this problem.

1. **Qualify at the point of use** --- `au::QuantityD<au::Meters>`.  This is the default.

2. **Scope the `using` down to a block** where a name would otherwise repeat awkwardly.  A `using`
  declaration _inside a function body_ (or any other block) expires at the closing brace and leaks
  nowhere.  This is the right tool for [unit symbols](../../reference/unit.md#symbols) in a header's
  inline function, where `au::symbols::rad` at every mention would drown the formula:

    ```cpp
    inline au::QuantityF<au::Radians> turn_angle(au::QuantityF<au::Meters> arc,
                                                 au::QuantityF<au::Meters> r) {
        using au::symbols::rad;  // Confined to this function.
        return arc * rad / r;
    }
    ```

    [Unit literals](../../reference/constant.md#unit-literals) follow the same rule.  Their suffixes
    are too arcane to spell out individually, so the conventional `using namespace
    ::au::au_literals;` is the right tool --- but in a header, it belongs _inside the function
    body_, not at namespace scope:

    ```cpp
    constexpr au::QuantityD<au::Seconds> default_timeout() {
        using namespace ::au::au_literals;  // Confined to this function.
        return (1.5_s).as<double>(au::seconds);
    }
    ```

## Unit aliases

Au has excellent composability for _objects_.  You can write `(kilo(meters) / hour)(100)`, for
example, and it will automatically create the unit for "km/h" on the fly.  Unfortunately, this
composability _can't_ extend to _type names_.  For this same example unit, if we want to declare
a quantity variable, all of our options are a little bit awkward.  They either need type traits, or
`decltype`:

```cpp
QuantityF<decltype(Kilo<Meters>{} / Hours{})> speed;

QuantityF<UnitQuotient<Kilo<Meters>, Hours>> speed;
```

We can make this easier to read by just defining a _type alias_ for the unit --- and here, too,
either spelling will do:

```cpp
// Either of these will do; pick one.
using KiloMetersPerHour = decltype(Kilo<Meters>{} / Hours{});
using KiloMetersPerHour = UnitQuotient<Kilo<Meters>, Hours>;

QuantityF<KiloMetersPerHour> speed;
```

All three of these `QuantityF` declarations refer to the _exact same type_, just with different
spelling.  Which one to reach for is a matter of taste.  `decltype` lets you reuse the same
arithmetic operators you already know from the object world, at the cost of the keyword;
`UnitQuotient` names the operation outright, at the cost of learning and using a visually clunky
trait.

A type alias is a different thing from a `using` _declaration_, and the rules are correspondingly
looser: it introduces one name you chose, rather than importing a set of names you did not.  That
makes it fine at namespace scope even in a header.  That said, we recommend the same instincts as
above:

- If you're in an implementation file (`.cc`, `.cpp`, etc.), use them without worrying about it.

- If you're in a header file (`.hh`, `.hpp`, etc.), a unit alias at namespace scope is safe, but
  keep in mind that it becomes part of your _public API_.  Every consumer of the header will see it,
  and may come to depend on it.  The [Google C++ Style Guide][google-aliases] puts it well:

    > Don't put an alias in your public API just to save typing in the implementation; do so only if
    > you intend it to be used by your clients.

    So: if the alias is for your consumers, give it a descriptive name and treat it as the interface
    it is.  If it's only for your own convenience, hide it inside a function or class.

[google-aliases]: https://google.github.io/styleguide/cppguide.html#Aliases

## Summary

Every project has its own style, conventions, and rules.  We've provided the conventions here as
a good _default_ style for using Au, and explained the reasons that we recommend them.  These aren't
hard and fast rules that we intend to force on all projects; they simply convey our vision for how
to use the library ergonomically.  We hope users will find them useful.

Much of this guidance comes from experience with large C++ projects, where the costs of namespace
pollution compound most visibly.  That doesn't mean it's for large projects only.  Remember that
"small" projects have a well known habit of becoming larger than anyone originally envisioned!
These habits are far cheaper to adopt early than to retrofit later.
