# Defining new units

This page explains how to define new units that aren't included in the library.

!!! tip
    If it's a common unit---one which _should_ be in the library, but _isn't_---go ahead and
    [file an issue](https://github.com/aurora-opensource/au/issues)!  We should be able to turn it
    around pretty quickly (either adding it to the library, or explaining why we won't).

## Definition features

Many libraries provide "convenience" macros for creating new units, but ours tries to avoid macros
completely.[^1]  Instead, you define new units by just writing regular C++ code.

[^1]:  Macros have long been considered contrary to C++ best practices.  If we're going to use one,
especially in user-facing code, it needs to meet a very high bar.  Unit definition macros don't meet
this bar.  They mostly exist to save typing.  But code is read far more often than written, and
macros actually make the definitions _harder_ to read and understand (because they use _positional_
arguments, so the meaning of the parameters is unclear at the callsite).

There are several pieces you can add, each of which provides some particular feature. Here is
a complete sample definition of a new Unit, with these features annotated and explained.

=== "C++14"

    ```cpp
    // Example custom unit definition below.
    //
    // Items labeled with `*` are _required_; everything else is optional.

    // In .hh file:
    struct Fathoms : decltype(Inches{} * mag<72>()) {           // *[1]
        static constexpr const char label[] = "ftm";            //  [2a]
    };
    constexpr auto fathom  = SingularNameFor<Fathoms>{};        //  [3]
    constexpr auto fathoms = QuantityMaker<Fathoms>{};          // *[4]
    constexpr auto fathoms_pt = QuantityPointMaker<Fathoms>{};  //  [5; less common]

    // In .cc file:
    constexpr const char Fathoms::label[];                      //  [2b]
    ```

=== "C++17 or later"

    ```cpp
    // Example custom unit definition below.
    //
    // Items labeled with `*` are _required_; everything else is optional.

    // In .hh file:
    struct Fathoms : decltype(Inches{} * mag<72>()) {           // *[1]
        static constexpr inline const char label[] = "ftm";     //  [2]
    };
    constexpr auto fathom  = SingularNameFor<Fathoms>{};        //  [3]
    constexpr auto fathoms = QuantityMaker<Fathoms>{};          // *[4]
    constexpr auto fathoms_pt = QuantityPointMaker<Fathoms>{};  //  [5; less common]
    ```

!!! note
    If you've seen the unit definitions included in our library, you may notice they look a little
    different from the above.  That's because the library has different goals and constraints than
    end user projects have.

    For example, the library needs to be both C++14-compatible and header-only.  This forces us to
    define our labels in a more complicated way.  By contrast, your project is unlikely to have both
    these constraints.

    Prefer the simpler approach outlined in this page, instead of treating our library's source code
    definitions as examples to follow.

Here are the features.

1. _Strong type definition_.
    - **Required.**  Make a `struct` with the name you want, and inherit from `decltype(u)`, where
      `u` is some _unit expression_ which gives it the right Dimension and Magnitude.  (We'll
      explain unit expressions in the next section.)

2. _Label_.
    - A `sizeof()`-compatible label which is useful for printing the Unit.
    - Note that _if_ your project needs C++14 compatibility, then besides the label itself (`[2a]`),
      you'll need to provide a _definition_ (`[2b]`) in the `.cc` file.  By contrast, if you use
      C++17 or later, you can just use an inline variable, and you won't need a `.cc` file.
    - **If omitted:**  Everything will still _work_; your Quantity will just be labeled as
      `[UNLABELED UNIT]` in printing contexts.

3. _Singular name_.
    - An object whose name is the singular name for your unit.  Useful in certain contexts: for
      example, the traditional unit for torque is "**newton** meters", _not_ "**newtons** meters".
    - **If omitted:** you'll sacrifice some readability flow: the grammar becomes strange.  You'll
      end up with constructs like `speed.in(miles / hours)`, rather than `speed.in(miles / hour)`.

4. _Quantity maker_.
    - **Required.**  This gives you a `snake_case` version of your unit which acts like a function.
      If you call this "function" and pass it any numeric type, it creates a `Quantity` of _your
      unit_, whose Rep is that type.  Of course, a quantity maker is much more than a function: it
      composes nicely with prefixes, and with other quantity makers.

5. _Quantity point maker_.
    - Just like the quantity maker, but conventionally with a `_pt` suffix to indicate that it makes
      `QuantityPoint` instead.  You can call this like a function on arbitrary numeric types.  You
      can also compose it with prefixes, or scale it with Magnitudes.
    - **If omitted:** _this is usually fine to omit:_ most Units are only used with `Quantity`, not
      `QuantityPoint`.

!!! note
    Not shown here: adding an `origin` member.  We skipped this because it is very rare.  It only
    has any effect at all for Units you plan to use with `QuantityPoint`, which is not the usual
    case.  Even among those units, only a small subset have a non-default origin.  The main examples
    are `Celsius` and `Fahrenheit`, and the library will provide those out of the box.

## Unit expressions

Above, we said to inherit your unit's strong type from the `decltype` of a "unit expression".
Recall the line from above:

```cpp
struct Fathoms : decltype(Inches{} * mag<72>()) {
//        Unit Expression ^^^^^^^^^^^^^^^^^^^^
```

This section explains what kinds of things can go inside of the `decltype(...)`.

Conceptually, units are defined by combining _other units_.  In general, given any set of units, you
can multiply them, divide them, raise them to powers, or scale them by real numbers ("magnitudes"):
the result of any of these operations defines a new unit.

In C++ code, the easiest way to do this is by working with _instances_ of the unit types.  (`Meters`
is the _type_; `Meters{}` is an _instance_ of the type.)  This lets us multiply them naturally by
writing `*`, rather than using cumbersome template traits such as `UnitProductT<...>`.  Here are
some examples:

- Newtons: `Kilo<Grams>{} * Meters{} / squared(Seconds{})`
- Miles: `Feet{} * mag<5280>()`
- Degrees: `Radians{} * PI / mag<180>()`

## Aliases vs. strong types: best practices

A shorter method of defining units is as _aliases_ for a compound unit.  For example:

```cpp
using MilesPerHour = decltype(Miles{} / Hours{});
constexpr auto miles_per_hour = miles / hour;
```

We can use the alias, `MilesPerHour`, anywhere we'd use a unit type.  And we can call the
QuantityMaker, `miles_per_hour`, just as we would call `miles`.[^3]    We even get an automatically
generated unit label: `mi / h`.

[^3]:  Note that we don't "need" to define this.  We could write `(miles / hour)(65)`, and get
exactly the same result as `miles_per_hour(65)`.  However, some users may prefer the latter syntax.

Despite this convenience, aliases aren't always the best choice.  Here's the best practices guidance
to follow.

1. Use **strong types** for **named** units.
    - _Example:_ `Newtons`; `Fathoms`
    - _Rationale:_ Strong types show up in compiler errors, making them easier to read.
        - _Counterpoint:_ as seen below, this will reduce the ability to cancel out units.  For
          example, `Meters{} * Hertz{}` will **not** be the same as `Meters{} / Seconds{}`; instead,
          it will be a different-but-equivalent Unit.  Given the way we handle quantity-equivalent
          Units, this will usually not be a problem, and we believe the value of seeing shorter,
          more familiar names in the compiler errors outweighs this cost.

2. Use **aliases** for **compound units** with no special name.
    - _Example:_ `NewtonMeters`; `MilesPerHour`.  Both of these are better implemented as _aliases_
      rather than _strong types_.
    - _Rationale:_ Keeping these as aliases increases support for cancellation: it enables the
      library to notice that `MetersPerSecond{} * Seconds{}` is _identical_ to `Meters{}`, not
      merely quantity-equivalent.  This doesn't _usually_ matter, but it can reduce exposure to
      compiler errors in the (rare) situations where exact-type-equality matters (e.g., initializer
      lists).
