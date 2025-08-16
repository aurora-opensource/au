# Defining new constants

This page explains how to define new constants that aren't [included in the
library](../reference/constant.md#built-in).

!!! tip
    If you think it _should_ be included in the library, feel free to [file an
    issue](https://github.com/aurora-opensource/au/issues): there's no harm in asking.

    In order for us to consider it, it should be relatively widely used.  It also needs to have an
    _exact_ value: we don't have a great way to deal with values that can change over time.

    If these conditions don't apply, then follow the directions in this guide to define a _custom_
    constant for your project.

## Methods of definition

There are two ways to define a new constant.  The only difference is _whether you need a label_ for
your constant.  In either case, you create a constant by calling `make_constant(...)`.  The argument
is a [unit slot](../discussion/idioms/unit-slots.md), so you can pass it anything that goes into
a unit slot.

### Ad hoc (no special label)

Pass any ad hoc unit expression to the unit slot.  The constant will produce correct results in
code, in every situation.  It will even have _a_ label, which will identify it exactly.  The label
will simply be cumbersome.

!!! example "Example: speed of light in an ad hoc manner"
    Here's how to create a constant for the speed of light, without giving it a special symbol.

    ```cpp
    constexpr auto c = au::make_constant(au::meters / au::second * au::mag<299'792'458>());
    ```

    Here's an example use case, in user code:

    ```cpp
    std::cout << (0.8 * c) << std::endl;
    ```

    The above prints `"0.8 [299792458 m/s]"`.  Notice how the unit label, `"[299792458 m/s]"`, is
    _correct_, but _cumbersome_.

### Full unit definition (includes custom label)

First, follow a stripped down version of the [new unit instructions](./new-units.md) to define
a _unit_ for the constant.  The only thing you need to give it is a _label_; you can omit the
instructions for quantity makers and so on.

Next, pass an instance of this custom unit to `make_constant`.

!!! example "Example: speed of light with full unit definition"
    Here's how to create a constant for the speed of light using a full custom unit, with label.

    === "C++14"
        ```cpp
        // In `.hh` file, in your project's namespace:
        struct SpeedOfLightUnit : decltype(au::Meters{} / au::Seconds{} * au::mag<299'792'458>()) {
            static constexpr const char label[] = "c";
        };
        constexpr auto c = au::make_constant(SpeedOfLightUnit{});

        // In `.cc` file, in your project's namespace:
        constexpr const char SpeedOfLightUnit::label[];
        ```

    === "C++17"
        ```cpp
        // In `.hh` file, in your project's namespace:
        struct SpeedOfLightUnit : decltype(au::Meters{} / au::Seconds{} * au::mag<299'792'458>()) {
            static constexpr inline const char label[] = "c";
        };
        constexpr auto c = au::make_constant(SpeedOfLightUnit{});
        ```

    Here's an example use case, in user code:

    ```cpp
    std::cout << (0.8 * c) << std::endl;
    ```

    The above prints `"0.8 c"`.
