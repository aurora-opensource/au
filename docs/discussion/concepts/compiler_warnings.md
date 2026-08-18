# Compiler Warnings

Every project has its own policy about compiler warnings: which ones to enable, and which of those
to turn into errors.  That policy is expressed in _compiler flags_, and it belongs to _you_, the
user --- not to your units library.  Au's job is to respect your choices and avoid interfering with
them.  This page explains how we do that.

## Au's core policy {#policy}

The core of our approach can be summarized in a simple policy statement:

> **Au neither _adds_ nor _subtracts_ compiler warnings.**

More precisely: for anything which is a property of the _underlying numeric types_ --- lost
precision, narrowing, sign conversion, and so on --- Au aims to give you _exactly_ the diagnostics
your compiler flags would give you for the equivalent raw numbers: no more, no less.

Here's what each half of our policy statement actually _means_.

- **Don't add.**  "Adding" warnings would mean that your basic (raw numeric) program compiles
  without any warnings, but you start seeing warnings when you migrate it to use Au.  This implies
  that Au's _library-internal code_ is not clean under that warning: an incredibly frustrating
  experience.

- **Don't subtract.**  "Subtracting" is more subtle: it means that your basic (raw numeric) program
  _would_ produce a warning in a certain situation, but when you migrate it to Au, the warning
  disappears.  This would be a silent leak in the safety net that those warnings provide.

Historically, Au has always paid careful attention to the first half ("don't add"), and strove to be
clean under all warnings.  Noticing the other side of this coin ("don't subtract") took a lot
longer.  It wasn't until [#528] was filed that we recognized you can be _too_ clean under warnings
--- that an absent warning can be just as dangerous as a present one.

This half matters because of what warning flags _are_.  Every project needs some way to say which
numerical operations it considers safe: whether narrowing a `double` to a `float` is routine or
alarming is a _policy_ choice, and different projects land in different places.  For raw numeric
code, the canonical mechanism for expressing that choice already exists --- it _is_ the set of
warning flags.  A library that subtracts warnings takes that mechanism away, and forces its users to
invent a replacement.  One that doesn't lets you keep the mechanism you already have, and get the
same answers for `Quantity` that you get for the numbers inside it.

!!! note "Unit-level safety checks are a separate matter"
    Au _does_ add errors of its own: mismatched dimensions, [forbidden integer
    division](../../troubleshooting.md#integer-division-forbidden), and [conversion
    risks](./conversion_risks.md) such as [truncation](./truncation.md) and
    [overflow](./overflow.md).

    These are _unit-level_ checks, and they're the entire point of the library: they catch mistakes
    that raw numbers can't even express.  The policy on this page is about the _numeric_ layer
    underneath, where we defer entirely to your compiler flags.

### Implementation approach

Au performs every unit conversion as a [sequence of operations](./conversion_risks.md), and one of
those operations is converting from one rep to another.  Which _kind_ of conversion we use depends
on how _you_ asked for it:

- The **implicit constructor** uses an **implicit conversion**, because that's what the equivalent
  raw-number code (`float f = d;`) would do.  Your `-Wconversion` flags therefore fire exactly as
  they would for raw numbers.

- **`.as<T>(unit)` and `.in<T>(unit)`** use a **`static_cast`**, because naming the destination type
  explicitly _is_ an explicit request to convert.  Just as with a hand-written `static_cast`, no
  warning is raised.

!!! example

    ```cpp
    QuantityD<Meters> d = meters(1.0);

    QuantityF<Meters> a = d;         // Warns under `-Wfloat-conversion`, as `float a = 1.0;` would.
    auto b = d.as<float>(meters);    // No warning, as `static_cast<float>(1.0)` wouldn't.
    float c = d.in<float>(meters);   // No warning, for the same reason.
    ```

## Required build configuration {#required-build-configuration}

Historically, very few libraries have lived up to the "don't _add_ warnings" half of our policy.
Surprisingly, this has created a serious obstacle for libraries trying to live up to the "don't
_subtract_ warnings" half!

The mechanism at work here began with the standard library and the operating system.  Those headers,
as the [GCC manual][gcc-system-headers] puts it, "often cannot be written in strictly conforming C",
so any warning they produce is a false positive that you can do nothing about.  The answer was to
suppress essentially all warnings while processing a _system header_.  Later on, via flags such as
`-isystem`, users could nominate _any_ directory for that same treatment.[^linters]

[^linters]: Linters follow the compiler's lead here.  clang-tidy checks --- including
`cppcoreguidelines-narrowing-conversions`, which flags exactly the kind of narrowing this page is
about --- don't report findings in system headers either.

That escape hatch proved irresistible for third party code, and understandably so: warnings from
dependencies you don't control add noise and obscure your own mistakes --- or, under `-Werror`,
break your build entirely.  The upshot is that many build configurations now treat _every_ external
dependency as a system include path by default.

Unfortunately, this situation hurts libraries that treat warnings more carefully.  For Au
specifically, the effect is sweeping.  Every rep conversion physically _happens_ inside Au's
headers, no matter which line of _your_ code requested it: that's simply where the library's code
lives.  Compilers judge a diagnostic by the file it points to, so if Au's headers are considered
"system" headers, then **every** numeric conversion warning in **every** unit conversion disappears
--- silently.

```cpp
QuantityF<Meters> narrowed = meters(1.0);  // `-Wfloat-conversion` warns...
                                           // ...unless Au is on a system include path.
```

That is precisely the "subtract" failure mode from [the policy above](#policy), and it's not
something we can fix from inside the library.  It's a property of your build configuration.

Therefore: **configure Au as a normal (non-system) dependency.**  See the
[installation docs](../../install.md#warning-flags) for details on how to do this for each supported
installation method, and how to check what your build is actually doing.

!!! warning "Au can't do this; only users can"
    A header can't "opt out" of warning suppression from inside itself.  This is the one part of our
    policy that we can't deliver on your behalf: as long as your build puts Au on a system include
    path, the "neither add nor subtract" promise simply does not apply, and warnings you rely on
    everywhere else in your project will silently fail to fire for quantities.

### Last resort: forcing warnings back on

If your build genuinely cannot avoid putting Au on a system include path, these flags can restore
the diagnostics.  Fixing the build configuration would be the better approach, but these options may
be better than nothing.

| Flag | Compilers | Effect |
|------|-----------|--------|
| `--no-system-header-prefix=au/` | clang | Treats headers included as `"au/..."` as non-system.  Nicely targeted, but clang-only. |
| `-Wsystem-headers` | gcc, clang | Restores warnings in _all_ system headers, including your standard library.  Usually far too noisy. |
| `/external:W4` (with `/external:I`) | MSVC | Sets the warning level applied to external headers. |
| `--system-headers` (or `SystemHeaders` in `.clang-tidy`) | clang-tidy | Reports findings from all system headers, with the same noise problem as `-Wsystem-headers`. |

## Violations are bugs

Our policy is aspirational, not a strict guarantee, because it's hard to test every possible
combination of compiler flags.  However, we take it very seriously, and we want to hear about it
when we fall short.

If you think you've found a violation, the first thing to check is your [build
configuration](#required-build-configuration): a system include path for Au explains most "missing"
warnings, and it's the one cause that only _you_ can fix.  Once you've ruled that out, a violation in
_either_ direction is a bug in Au.  Please file an [issue] if you find one!

It's possible, in principle, that someone could find a violation of this principle that is
_architecturally impossible_ for Au to fix.  If that happens, we'll add a section to this page that
lists all known exceptions, and explains why they can't be fixed.

[#528]: https://github.com/aurora-opensource/au/issues/528
[gcc-system-headers]: https://gcc.gnu.org/onlinedocs/cpp/System-Headers.html
[issue]: https://github.com/aurora-opensource/au/issues/new
