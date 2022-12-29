# Troubleshooting Guide

How do you interpret compiler errors from Au?  This page is a guide to the most commonly encountered
types of error, what they mean, and how to fix them.

The intended use case is to help you interpret an _actual error in your code_, at the point where
you encounter it.  To use this page, copy some relevant snippets from your compiler error, and then
search the text of this page using `Ctrl-F`.

!!! tip
    To help your chances in finding what you're looking for, we include full compiler errors from
    both gcc and clang, inline with the text.  Naturally, this makes this page very long, so it's
    not meant to be read straight through.  Stick with the `Ctrl-F` route.

## Private constructor

**Meaning:**  This is the "classic" error the units library aims to prevent.  It means you passed
a raw numeric value to an interface that expected a Quantity.

**Solution:** Call the appropriate Quantity maker: instead of passing `x`, pass `meters(x)`,
`(kilo(meters) / hour)(x)`, etc.

!!! note
    Every other units library lets you construct its Quantity types from raw numeric values; it just
    makes that constructor explicit.  Au goes further, and makes this constructor private.  The
    reason is to preserve unit safety at all callsites.  We can't know whether you made an alias
    that doesn't name the unit.  For example, if you want everybody to measure lengths in `Meters`
    in your codebase, you might provide a common alias like this:

    ```cpp
    using Length = QuantityD<Meters>;
    ```

    If you did, then end users could write the following:

    ```cpp
    constexpr Length MAX_LENGTH{5.5};  // Unsafe!  Units unclear.  :(
    ```

    A core principle of the Au library is that the only way to enter or exit the library boundaries
    is to name the unit of measure, explicitly, at the callsite, like this:

    ```cpp
    constexpr Length MAX_LENGTH = meters(5.5);  // Usable!  Units unambiguous.  :)
    ```

    This enables users who want to use this kind of "dimension-named alias" in their codebase to do
    so safely.

!!! example
    **Code**

    === "Broken"
        ```cpp
        void set_timeout(QuantityD<Seconds> dt);

        // A (BROKEN): passing raw number where duration expected.
        set_timeout(0.5);

        // B (BROKEN): calling Quantity constructor directly.
        constexpr QuantityD<Meters> length{5.5};
        ```


    === "Fixed"
        ```cpp
        void set_timeout(QuantityD<Seconds> dt);

        // A (FIXED): name the unit.
        set_timeout(seconds(0.5));

        // B (FIXED): calling Quantity constructor directly.
        constexpr QuantityD<Meters> length = meters(5.5);
        ```

    **Compiler error (clang 14)**
    ```
    au/error_examples.cc:19:17: error: calling a private constructor of class 'au::Quantity<au::Seconds, double>'
        set_timeout(0.5);
                    ^
    ./au/quantity.hh:369:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    au/error_examples.cc:22:33: error: calling a private constructor of class 'au::Quantity<au::Meters, double>'
        constexpr QuantityD<Meters> length{5.5};
                                    ^
    ./au/quantity.hh:369:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    ```

    **Compiler error (clang 11)**
    ```
    au/error_examples.cc:19:17: error: calling a private constructor of class 'au::Quantity<au::Seconds, double>'
        set_timeout(0.5);
                    ^
    ./au/quantity.hh:369:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    au/error_examples.cc:22:33: error: calling a private constructor of class 'au::Quantity<au::Meters, double>'
        constexpr QuantityD<Meters> length{5.5};
                                    ^
    ./au/quantity.hh:369:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    ```

    **Compiler error (gcc 10)**
    ```
    au/error_examples.cc: In function 'void au::example_private_constructor()':
    au/error_examples.cc:19:20: error: 'constexpr au::Quantity<UnitT, RepT>::Quantity(au::Quantity<UnitT, RepT>::Rep) [with UnitT = au::Seconds; RepT = double; au::Quantity<UnitT, RepT>::Rep = double]' is private within this context
       19 |     set_timeout(0.5);
          |                    ^
    In file included from ./au/math.hh:8,
                     from ./au/au.hh:7,
                     from au/error_examples.cc:1:
    ./au/quantity.hh:369:15: note: declared private here
      369 |     constexpr Quantity(Rep value) : value_{value} {}
          |               ^~~~~~~~
    au/error_examples.cc:22:43: error: 'constexpr au::Quantity<UnitT, RepT>::Quantity(au::Quantity<UnitT, RepT>::Rep) [with UnitT = au::Meters; RepT = double; au::Quantity<UnitT, RepT>::Rep = double]' is private within this context
       22 |     constexpr QuantityD<Meters> length{5.5};
          |                                           ^
    In file included from ./au/math.hh:8,
                     from ./au/au.hh:7,
                     from au/error_examples.cc:1:
    ./au/quantity.hh:369:15: note: declared private here
      369 |     constexpr Quantity(Rep value) : value_{value} {}
          |               ^~~~~~~~
    ```

## Dangerous Conversion

**Meaning:**  This is a _physically_ meaningful conversion, but we think the risk of a grossly
incorrect answer is too high, so we forbid it.  There are two main sources for this risk, both
having to do with integral storage types.

1. **Inexact conversion**.  Example: `inches(24).as(feet)`.

2. **Overflow**.  Example: `giga(hertz)(1).as(hertz)`.

Both of these examples would in fact produce the correct answer with the specific values given (`24`
and `1`).  However, many (most!) other values would not.  Thus, we disallow the entire conversion
operation (at least in this format).

**Solution:**  There are different strategies to solve this, depending on your use case.

1. **Use floating point**.  As mentioned above, these risks only apply to integer values.  If
   floating point is what you want anyway, just use it.  `giga(hertz)(1.0).as(hertz)` produces
   `hertz(1'000'000'000.0)`.

2. **Use "explicit Rep" form**.  The "Rep" is the storage type for the Quantity.  If you pass it as
   a template parameter, it is "morally equivalent" to a `static_cast`, and has the same "forcing"
   semantics.  `inches(24).as<int>(feet)` produces `feet(2)`.

!!! warning
    Stop and think before using the explicit Rep version.  If you're reviewing code that uses it,
    ask about it.  The library is trying to protect you from an error prone operation.  The
    mechanism exists because sometimes you can know that it's OK, but remember to stop and check
    first!

!!! example
    **Code**

    === "Broken"
        ```cpp
        // A (BROKEN): inexact conversion.
        inches(24).as(feet);

        // B (BROKEN): overflow risk.
        giga(hertz)(1).as(hertz);
        ```

    === "Fixed (1. Floating Point)"
        ```cpp
        // A (FIXED): 1. use floating point.
        inches(24.0).as(feet);

        // B (FIXED): 1. use floating point.
        giga(hertz)(1.0).as(hertz);
        ```

    === "Fixed (2. Explicit Rep)"
        ```cpp
        // A (FIXED): 2. provide explicit Rep.
        inches(24).as<int>(feet);

        // B (FIXED): 2. provide explicit Rep.
        giga(hertz)(1).as<int>(hertz);
        ```


    **Compiler error (clang 14)**
    ```
    ./au/quantity.hh:147:9: error: static_assert failed due to requirement 'implicit_rep_permitted_from_source_to_target<int>(unit, u)' "Dangerous conversion: use .as<Rep>(NewUnit) instead"
            static_assert(implicit_rep_permitted_from_source_to_target<Rep>(unit, u),
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/quantity.hh:183:16: note: in instantiation of function template specialization 'au::Quantity<au::Inches, int>::as<au::Feet, void>' requested here
            return as(NewUnit{});
                   ^
    au/error_examples.cc:30:16: note: in instantiation of function template specialization 'au::Quantity<au::Inches, int>::as<au::Feet>' requested here
        inches(24).as(feet);
                   ^
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:7:
    In file included from ./au/math.hh:8:
    ./au/quantity.hh:147:9: error: static_assert failed due to requirement 'implicit_rep_permitted_from_source_to_target<int>(unit, u)' "Dangerous conversion: use .as<Rep>(NewUnit) instead"
            static_assert(implicit_rep_permitted_from_source_to_target<Rep>(unit, u),
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/quantity.hh:183:16: note: in instantiation of function template specialization 'au::Quantity<au::Giga<au::Hertz>, int>::as<au::Hertz, void>' requested here
            return as(NewUnit{});
                   ^
    au/error_examples.cc:33:20: note: in instantiation of function template specialization 'au::Quantity<au::Giga<au::Hertz>, int>::as<au::Hertz>' requested here
        giga(hertz)(1).as(hertz);
                       ^
    ```

    **Compiler error (clang 11)**
    ```
    ./au/quantity.hh:147:9: error: static_assert failed due to requirement 'implicit_rep_permitted_from_source_to_target<int>(unit, u)' "Dangerous conversion: use .as<Rep>(NewUnit) instead"
            static_assert(implicit_rep_permitted_from_source_to_target<Rep>(unit, u),
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/quantity.hh:183:16: note: in instantiation of function template specialization 'au::Quantity<au::Inches, int>::as<au::Feet, void>' requested here
            return as(NewUnit{});
                   ^
    au/error_examples.cc:30:16: note: in instantiation of function template specialization 'au::Quantity<au::Inches, int>::as<au::Feet>' requested here
        inches(24).as(feet);
                   ^
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:7:
    In file included from ./au/math.hh:8:
    ./au/quantity.hh:147:9: error: static_assert failed due to requirement 'implicit_rep_permitted_from_source_to_target<int>(unit, u)' "Dangerous conversion: use .as<Rep>(NewUnit) instead"
            static_assert(implicit_rep_permitted_from_source_to_target<Rep>(unit, u),
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/quantity.hh:183:16: note: in instantiation of function template specialization 'au::Quantity<au::Giga<au::Hertz>, int>::as<au::Hertz, void>' requested here
            return as(NewUnit{});
                   ^
    au/error_examples.cc:33:20: note: in instantiation of function template specialization 'au::Quantity<au::Giga<au::Hertz>, int>::as<au::Hertz>' requested here
        giga(hertz)(1).as(hertz);
                       ^
    ```

    **Compiler error (gcc 10)**
    ```
    ./au/quantity.hh: In instantiation of 'constexpr auto au::Quantity<UnitT, RepT>::as(NewUnit) const [with NewUnit = au::Feet; <template-parameter-2-2> = void; UnitT = au::Inches; RepT = int]':
    ./au/quantity.hh:183:18:   required from 'constexpr auto au::Quantity<UnitT, RepT>::as(au::QuantityMaker<NewUnit>) const [with NewUnit = au::Feet; UnitT = au::Inches; RepT = int]'
    au/error_examples.cc:30:23:   required from here
    ./au/quantity.hh:147:72: error: static assertion failed: Dangerous conversion: use .as<Rep>(NewUnit) instead
      147 |         static_assert(implicit_rep_permitted_from_source_to_target<Rep>(unit, u),
          |                       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~
    ./au/quantity.hh: In instantiation of 'constexpr auto au::Quantity<UnitT, RepT>::as(NewUnit) const [with NewUnit = au::Hertz; <template-parameter-2-2> = void; UnitT = au::Giga<au::Hertz>; RepT = int]':
    ./au/quantity.hh:183:18:   required from 'constexpr auto au::Quantity<UnitT, RepT>::as(au::QuantityMaker<NewUnit>) const [with NewUnit = au::Hertz; UnitT = au::Giga<au::Hertz>; RepT = int]'
    au/error_examples.cc:33:28:   required from here
    ./au/quantity.hh:147:72: error: static assertion failed: Dangerous conversion: use .as<Rep>(NewUnit) instead
    ```

## No type named 'type' in 'std::common_type'

**Meaning:**  You probably tried to perform a "common-unit operation" (addition, subtraction,
comparison) with two incompatible Quantities.  Typically, this means they have different
_dimensions_, which makes this an intrinsically meaningless operation.

!!! warning "TODO"
    Make a page which explains common-unit operations, and link to it.

**Solution:**  Figure out what dimension you expected them to have, and which value had the wrong
dimension.  Then, figure out how to fix your expression so it has the right dimension.

!!! example

    **Code**

    === "Broken"
        ```cpp
        // (BROKEN): different dimensions.
        meters(1) + seconds(1);
        ```

    === "Fixed"
        ```cpp
        // (FIXED): fix coding mistake.
        meters(1) + seconds(1) * (meters / second)(10);
        ```


    **Compiler error (clang 14)**
    ```
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:5:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/chrono:697:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/__chrono/calendar.h:13:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/__chrono/duration.h:14:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/limits:105:
    external/llvm_14_toolchain_llvm/bin/../include/c++/v1/type_traits:2388:25: error: no type named 'type' in 'std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>>'
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^~~~~
    ./au/quantity.hh:494:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    ./au/quantity.hh:532:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>, au::detail::Plus>' requested here
        return detail::using_common_type(q1, q2, detail::plus);
                       ^
    au/error_examples.cc:41:15: note: in instantiation of function template specialization 'au::operator+<au::Meters, au::Seconds, int, int>' requested here
        meters(1) + seconds(1);
                  ^
    ```

    **Compiler error (clang 11)**
    ```
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:5:
    In file included from external/llvm_11_toolchain_llvm/bin/../include/c++/v1/chrono:828:
    external/llvm_11_toolchain_llvm/bin/../include/c++/v1/type_traits:2462:25: error: no type named 'type' in 'std::__1::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>>'
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^~~~~
    ./au/quantity.hh:494:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    ./au/quantity.hh:532:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>, au::detail::Plus>' requested here
        return detail::using_common_type(q1, q2, detail::plus);
                       ^
    au/error_examples.cc:41:15: note: in instantiation of function template specialization 'au::operator+<au::Meters, au::Seconds, int, int>' requested here
        meters(1) + seconds(1);
                  ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from external/sysroot_x86_64//include/c++/10.3.0/ratio:39,
                     from external/sysroot_x86_64//include/c++/10.3.0/chrono:39,
                     from ./au/au.hh:5,
                     from au/error_examples.cc:1:
    external/sysroot_x86_64//include/c++/10.3.0/type_traits: In substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>}]':
    ./au/quantity.hh:494:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Meters, int>; U = au::Quantity<au::Seconds, int>; Func = au::detail::Plus]'
    ./au/quantity.hh:532:37:   required from 'constexpr auto au::operator+(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Meters; U2 = au::Seconds; R1 = int; R2 = int]'
    au/error_examples.cc:41:26:   required from here
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11: error: no type named 'type' in 'struct std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int> >'
     2562 |     using common_type_t = typename common_type<_Tp...>::type;
          |           ^~~~~~~~~~~~~
    In file included from ./au/math.hh:8,
                     from ./au/au.hh:7,
                     from au/error_examples.cc:1:
    ./au/quantity.hh: In instantiation of 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Meters, int>; U = au::Quantity<au::Seconds, int>; Func = au::detail::Plus]':
    ./au/quantity.hh:532:37:   required from 'constexpr auto au::operator+(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Meters; U2 = au::Seconds; R1 = int; R2 = int]'
    au/error_examples.cc:41:26:   required from here
    ```

## Integer division forbidden

**Meaning:**  Although Au generally tries to act just like the underlying raw numeric types, we also
try to prevent wrong code that _looks_ correct from compiling.  It turns out to be just too easy to
use integral Reps without noticing, and thus to get integer division without noticing.  This can
lead to very large errors.

**Solution:**  If you _really wanted_ integer division, call `integer_quotient()`.  Otherwise, use
floating point types.

!!! example

    **Code**

    How long does it take to travel 60 m at a speed of 65 MPH?

    === "Broken"
        ```cpp
        // (BROKEN): gives (60 / 65) == 0 before conversion!
        QuantityD<Seconds> t = meters(60) / (miles / hour)(65);
        ```

    === "Fixed (1. Floating point)"
        ```cpp
        // (FIXED): 1. Using floating point, we get ~= seconds(2.06486)
        QuantityD<Seconds> t = meters(60.0) / (miles / hour)(65.0);
        ```

    === "Fixed (2. `integer_quotient()`)"
        ```cpp
        // (FIXED): 2. Integer result == (meter * hours / mile)(0)
        auto t = integer_quotient(meters(60), (miles / hour)(65));
        ```


    **Compiler error (clang 14)**
    ```
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:7:
    In file included from ./au/math.hh:8:
    ./au/quantity.hh:304:9: error: static_assert failed due to requirement '!uses_integer_division' "Integer division forbidden: use integer_quotient() if you really want it"
            static_assert(!uses_integer_division,
            ^             ~~~~~~~~~~~~~~~~~~~~~~
    au/error_examples.cc:49:39: note: in instantiation of function template specialization 'au::Quantity<au::Meters, int>::operator/<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1>>, int>' requested here
        QuantityD<Seconds> t = meters(60) / (miles / hour)(65);
                                          ^
    ```

    **Compiler error (clang 11)**
    ```
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:7:
    In file included from ./au/math.hh:8:
    ./au/quantity.hh:304:9: error: static_assert failed due to requirement '!uses_integer_division' "Integer division forbidden: use integer_quotient() if you really want it"
            static_assert(!uses_integer_division,
            ^             ~~~~~~~~~~~~~~~~~~~~~~
    au/error_examples.cc:49:39: note: in instantiation of function template specialization 'au::Quantity<au::Meters, int>::operator/<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1>>, int>' requested here
        QuantityD<Seconds> t = meters(60) / (miles / hour)(65);
                                          ^
    ```

    **Compiler error (gcc 10)**
    ```
    ./au/quantity.hh:496:94: error: no type named 'type' in 'struct std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int> >'
      496 |         std::is_same<typename C::Rep, std::common_type_t<typename T::Rep, typename U::Rep>>::value,
          |                                                                                              ^~~~~
    ./au/quantity.hh: In instantiation of 'constexpr auto au::Quantity<UnitT, RepT>::operator/(au::Quantity<OtherUnit, OtherRep>) const [with OtherUnit = au::UnitProduct<au::Miles, au::Pow<au::Hours, -1> >; OtherRep = int; UnitT = au::Meters; RepT = int]':
    au/error_examples.cc:49:58:   required from here
    ./au/quantity.hh:304:23: error: static assertion failed: Integer division forbidden: use integer_quotient() if you really want it
      304 |         static_assert(!uses_integer_division,
          |                       ^~~~~~~~~~~~~~~~~~~~~~
    ```

## Dangerous inversion

**Meaning:**  This is analogous to our overflow safety surface.  When computing the inverse of
an integral quantity in a given target unit, there is some smallest value that will get truncated
down to zero (a tremendous error!).  If that value is "small enough to be scary" (currently 1,000),
we forbid the conversion.

**Solution:**  Consider using floating point; you'll always get a precise answer.  Alternatively,
use a smaller target unit.

!!! example

    **Code**

    === "Broken"
        ```cpp
        // (BROKEN): excessive truncation risk.
        inverse_as(seconds, hertz(5));
        ```

    === "Fixed (1. Floating point)"
        ```cpp
        // (FIXED): 1. Floating point result ~= seconds(0.2)
        inverse_as(seconds, hertz(5.0));
        ```

    === "Fixed (2. Smaller target unit)"
        ```cpp
        // (FIXED): 2. Integer result == milli(seconds)(200)
        inverse_as(milli(seconds), hertz(5));
        ```

    !!! note
        If you're _really_ sure it's OK, you can use the explicit-Rep version of `inverse_as`, which is
        forcing like a `static_cast`.  This is rarely the right choice, though.  Consider:

        ```cpp
        inverse_as<int>(seconds, hertz(5));
        ```

        This yields `seconds(0)`, due to the gross truncation error which the check was designed to
        prevent in the first place.



    **Compiler error (clang 14)**
    ```
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:7:
    ./au/math.hh:205:5: error: static_assert failed due to requirement 'make_quantity<au::UnitProduct<>>(int{1}).in(associated_unit(target_units) * au::Hertz{}) >= 1000 || std::is_floating_point<int>::value' "Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired"
        static_assert(
        ^
    ./au/math.hh:221:56: note: in instantiation of function template specialization 'au::inverse_in<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in(target_units, q));
                                                           ^
    au/error_examples.cc:57:5: note: in instantiation of function template specialization 'au::inverse_as<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        inverse_as(seconds, hertz(5));
        ^
    ```

    **Compiler error (clang 11)**
    ```
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:7:
    ./au/math.hh:205:5: error: static_assert failed due to requirement 'make_quantity<au::UnitProduct<>>(int{1}).in(associated_unit(target_units) * au::Hertz{}) >= 1000 || std::is_floating_point<int>::value' "Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired"
        static_assert(
        ^
    ./au/math.hh:221:56: note: in instantiation of function template specialization 'au::inverse_in<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in(target_units, q));
                                                           ^
    au/error_examples.cc:57:5: note: in instantiation of function template specialization 'au::inverse_as<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        inverse_as(seconds, hertz(5));
        ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from ./au/au.hh:7,
                     from au/error_examples.cc:1:
    ./au/math.hh: In instantiation of 'constexpr auto au::inverse_in(TargetUnits, au::Quantity<Unit, Rep>) [with TargetUnits = au::QuantityMaker<au::Seconds>; U = au::Hertz; R = int]':
    ./au/math.hh:221:66:   required from 'constexpr auto au::inverse_as(TargetUnits, au::Quantity<Unit, Rep>) [with TargetUnits = au::QuantityMaker<au::Seconds>; U = au::Hertz; R = int]'
    au/error_examples.cc:57:33:   required from here
    ./au/math.hh:206:93: error: static assertion failed: Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired
      206 |         make_quantity<UnitProductT<>>(R{1}).in(associated_unit(target_units) * U{}) >= 1000 ||
          |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~
      207 |             std::is_floating_point<R>::value,
          |             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ```

## Deduced conflicting types

**Meaning:**  In some contexts, it's not enough to have Quantity types that can easily convert to
each other.  (Common examples include the ternary operator `?:`, and initializer lists.)  You need
types that are identical, or very nearly so.  Even fully quantity-equivalent types, such as
$\text{Hz}$ and $\text{s}^{-1}$, often won't work in these contexts!

**Solution:**  You can always cast non-conforming instances to your favored unit, using `.as()`.
For the initializer list case, you can also make an explicit container, which will handle the
casting automatically when possible.

!!! example

    **Code**

    === "Broken"
        ```cpp
        // (BROKEN): Initializer list confused by Hz and s^(-1).
        for (const auto &frequency : {
                 hertz(1.0),
                 (1 / seconds(2.0)),
             }) {
            // ...
        }
        ```

    === "Fixed (1. Cast to explicit unit)"
        ```cpp
        // (FIXED): 1. Cast individual elements to desired unit.
        for (const auto &frequency : {
                 hertz(1.0),
                 (1 / seconds(2.0)).as(hertz),
             }) {
            // ...
        }
        ```

    === "Fixed (2. Use explicit container)"
        ```cpp
        // (FIXED): 2. Use container with explicit type.
        for (const auto &frequency : std::vector<QuantityD<Hertz>>{
                 hertz(1.0),
                 (1 / seconds(2.0)),
             }) {
            // ...
        }
        ```


    **Compiler error (clang 14)**
    ```
    au/error_examples.cc:65:34: error: deduced conflicting types ('Quantity<au::QuantityMaker<au::Hertz>::Unit, [...]>' vs 'Quantity<au::QuantityMaker<au::Pow<au::Seconds, -1>>::Unit, [...]>') for initializer list element type
        for (const auto &frequency : {
                                     ^
    ```

    **Compiler error (clang 11)**
    ```
    au/error_examples.cc:65:34: error: deduced conflicting types ('Quantity<au::Hertz, [...]>' vs 'Quantity<au::Pow<au::Seconds, -1>, [...]>') for initializer list element type
        for (const auto &frequency : {
                                     ^
    ```

    **Compiler error (gcc 10)**
    ```
    au/error_examples.cc: In function 'void au::example_deduced_conflicting_types()':
    au/error_examples.cc:68:10: error: unable to deduce 'std::initializer_list<auto>&&' from '{au::hertz.au::QuantityMaker<au::Hertz>::operator()<double>(1.0e+0), au::operator/<int>(1, au::seconds.au::QuantityMaker<au::Seconds>::operator()<double>(2.0e+0))}'
       68 |          }) {
          |          ^
    au/error_examples.cc:68:10: note:   deduced conflicting types for parameter 'auto' ('au::Quantity<au::Hertz, double>' and 'au::Quantity<au::Pow<au::Seconds, -1>, double>')
    ```

## Broken strict total ordering

**Meaning:**  This one is very deep in the weeds and is generally pretty hard to hit.  You need two
_distinct_ Units that have the _same_ Dimension, Magnitude, _and_ Origin.  That's a necessary but
not sufficient condition: for example, even `UnitInverseT<Seconds>` and `Hertz` won't trigger this!
Even once you have such Units, you need to perform some operation that puts them together in the
same parameter pack (say, comparing them, adding them, or taking a product).

!!! info
    In case you want to understand more, here is the gist.

    Au is _heavily_ based on parameter packs.  Some of these packs, such as `UnitProduct<...>` and
    `CommonUnit<...>`, take _Units_ as their arguments.

    !!! warning "TODO"
        Make a doc page for parameter packs, and link to it here.

    Every parameter pack needs an unambiguous canonical ordering for any possible set of input
    arguments.  Therefore, we need to create a _strict total ordering_ for the (infinitely many!)
    Unit types that could appear in these packs.  This ordering needs to be known _at compile time_.
    The ordering itself doesn't matter so much, but if we don't strictly adhere to _some_ ordering,
    it's undefined behaviour.

    Our strategy is to construct a "gauntlet" of properties which we can measure for any Unit (e.g.,
    Dimension, Magnitude, ...), and define some arbitrary ordering for each property.  We then
    compare the Units on each property in turn.  The first one where they differ "wins".  If we get
    through _all_ the properties, and they're _still_ tied, then we have two _distinct_ Unit types
    which _compare_ as equal.  This would be undefined behaviour!  Rather than silently ignoring
    this, we manifest this as a compiler error.

    That is what "broken strict total ordering" means.

**Solution:**  If you have two distinct Units, and the library can't figure out how to order them,
you can _force_ a particular ordering.  Choose one of the Units and give it a high "unit avoidance"
score (see example below).  This will break the tie.

Again, this is pretty unusual.  For most normal ways of forming units, the library should
automatically be able to define an ordering for them.  If you do hit this error, it may be worth
pausing to double-check that you're using the library correctly.

!!! tip
    If you hit this error, you might be annoyed by its obscurity.  Instead, try feeling relieved!
    After all, the alternative is not "correctly working program", but "silent undefined behaviour".
    A compiler error with a searchable error message is infinitely preferable to the latter.

!!! example

    **Code**

    Note that this example is somewhat convoluted, but again, that's to be expected because this error
    is pretty hard to hit in practice.

    === "Broken"
        ```cpp
        struct Quarterfeet : decltype(Feet{} / mag<4>()) {};
        constexpr auto quarterfeet = QuantityMaker<Quarterfeet>{};

        struct Trinches : decltype(Inches{} * mag<3>()) {};
        constexpr auto trinches = QuantityMaker<Trinches>{};

        // (BROKEN): Can't tell how to order Quarterfeet and Trinches when forming common type
        if (quarterfeet(10) == trinches(10)) {
            // ...
        }
        ```

    === "Fixed"
        ```cpp
        struct Quarterfeet : decltype(Feet{} / mag<4>()) {};
        constexpr auto quarterfeet = QuantityMaker<Quarterfeet>{};

        struct Trinches : decltype(Inches{} * mag<3>()) {};
        constexpr auto trinches = QuantityMaker<Trinches>{};

        namespace au { namespace detail {
        template <>
        struct UnitAvoidance<::Trinches> : std::integral_constant<int, 100> {};
        }}

        // (FIXED): Trinches has high "unit avoidance", so it goes after Quarterfeet
        if (quarterfeet(10) == trinches(10)) {
            // ...
        }
        ```


    **Compiler error (clang 14)**
    ```
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:7:
    In file included from ./au/math.hh:8:
    In file included from ./au/quantity.hh:7:
    In file included from ./au/conversion_policy.hh:7:
    In file included from ./au/magnitude.hh:7:
    ./au/packs.hh:276:5: error: static_assert failed due to requirement 'std::is_same<au::Quarterfeet, au::Trinches>::value' "Broken strict total ordering: distinct input types compare equal"
        static_assert(std::is_same<A, B>::value,
        ^             ~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches>' requested here
        std::conditional_t<
        ^
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderAsUnitProduct>' requested here
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>' requested here
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>' requested here
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>' requested here
    ./au/unit_of_measure.hh:868:40: note: (skipping 8 contexts in backtrace; use -ftemplate-backtrace-limit=0 to see all)
    struct InOrderFor<UnitProduct, A, B> : LexicographicTotalOrdering<A,
                                           ^
    ./au/quantity.hh:631:7: note: in instantiation of template class 'au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>>' requested here
        : au::CommonQuantity<au::Quantity<U1, R1>, au::Quantity<U2, R2>> {};
          ^
    external/llvm_14_toolchain_llvm/bin/../include/c++/v1/type_traits:2388:25: note: in instantiation of template class 'std::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>>' requested here
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^
    ./au/quantity.hh:494:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    ./au/quantity.hh:506:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, au::detail::Equal>' requested here
        return detail::using_common_type(q1, q2, detail::equal);
                       ^
    au/error_examples.cc:84:25: note: in instantiation of function template specialization 'au::operator==<au::Quarterfeet, au::Trinches, int, int>' requested here
        if (quarterfeet(10) == trinches(10)) {
                            ^
    ```

    **Compiler error (clang 11)**
    ```
    In file included from au/error_examples.cc:1:
    In file included from ./au/au.hh:7:
    In file included from ./au/math.hh:8:
    In file included from ./au/quantity.hh:7:
    In file included from ./au/conversion_policy.hh:7:
    In file included from ./au/magnitude.hh:7:
    ./au/packs.hh:276:5: error: static_assert failed due to requirement 'std::is_same<au::Quarterfeet, au::Trinches>::value' "Broken strict total ordering: distinct input types compare equal"
        static_assert(std::is_same<A, B>::value,
        ^             ~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches>' requested here
        std::conditional_t<
        ^
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, OrderAsUnitProduct>' requested here
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, OrderByOrigin, OrderAsUnitProduct>' requested here
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, OrderByMag, OrderByOrigin, OrderAsUnitProduct>' requested here
    ./au/packs.hh:292:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, OrderByDim, OrderByMag, OrderByOrigin, OrderAsUnitProduct>' requested here
    ./au/unit_of_measure.hh:868:40: note: (skipping 8 contexts in backtrace; use -ftemplate-backtrace-limit=0 to see all)
    struct InOrderFor<UnitProduct, A, B> : LexicographicTotalOrdering<A,
                                           ^
    ./au/quantity.hh:631:7: note: in instantiation of template class 'au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, void>' requested here
        : au::CommonQuantity<au::Quantity<U1, R1>, au::Quantity<U2, R2>> {};
          ^
    external/llvm_11_toolchain_llvm/bin/../include/c++/v1/type_traits:2462:25: note: in instantiation of template class 'std::__1::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>>' requested here
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^
    ./au/quantity.hh:494:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    ./au/quantity.hh:506:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, au::detail::Equal>' requested here
        return detail::using_common_type(q1, q2, detail::equal);
                       ^
    au/error_examples.cc:84:25: note: in instantiation of function template specialization 'au::operator==<au::Quarterfeet, au::Trinches, int, int>' requested here
        if (quarterfeet(10) == trinches(10)) {
                            ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from ./au/magnitude.hh:7,
                     from ./au/conversion_policy.hh:7,
                     from ./au/quantity.hh:7,
                     from ./au/math.hh:8,
                     from ./au/au.hh:7,
                     from au/error_examples.cc:1:
    ./au/packs.hh: In instantiation of 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches>':
    ./au/packs.hh:287:8:   recursively required from 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>'
    ./au/packs.hh:287:8:   required from 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByUnitAvoidance, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>'
    ./au/unit_of_measure.hh:868:8:   required from 'struct au::InOrderFor<au::UnitProduct, au::Quarterfeet, au::Trinches>'
    ./au/unit_of_measure.hh:502:8:   required from 'struct au::InOrderFor<au::CommonUnit, au::Quarterfeet, au::Trinches>'
    ./au/packs.hh:372:8:   required from 'struct au::FlatDedupedTypeList<au::CommonUnit, au::CommonUnit<au::Quarterfeet>, au::CommonUnit<au::Trinches> >'
    ./au/unit_of_measure.hh:540:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    ./au/quantity.hh:617:8:   required from 'struct au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, void>'
    ./au/quantity.hh:630:8:   required from 'struct std::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int> >'
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11:   required by substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>}]'
    ./au/quantity.hh:494:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Quarterfeet, int>; U = au::Quantity<au::Trinches, int>; Func = au::detail::Equal]'
    ./au/quantity.hh:506:37:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:84:39:   required from here
    ./au/packs.hh:276:39: error: static assertion failed: Broken strict total ordering: distinct input types compare equal
      276 |     static_assert(std::is_same<A, B>::value,
          |                                       ^~~~~
    ./au/packs.hh: In instantiation of 'struct au::LexicographicTotalOrdering<au::Trinches, au::Quarterfeet>':
    ./au/packs.hh:287:8:   required from 'struct au::LexicographicTotalOrdering<au::Trinches, au::Quarterfeet, au::detail::OrderByUnitAvoidance, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>'
    ./au/unit_of_measure.hh:868:8:   [ skipping 8 instantiation contexts, use -ftemplate-backtrace-limit=0 to disable ]
    ./au/unit_of_measure.hh:452:8:   required from 'struct au::HasSameDimension<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>'
    ./au/stdx/type_traits.hh:26:61:   required from 'struct au::stdx::conjunction<au::HasSameDimension<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>, au::detail::HasSameMagnitude<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches> >'
    ./au/unit_of_measure.hh:467:8:   required from 'struct au::AreUnitsQuantityEquivalent<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>'
    ./au/unit_of_measure.hh:529:8:   required from 'struct au::detail::FirstMatchingUnit<au::AreUnitsQuantityEquivalent, au::CommonUnit<au::Trinches, au::Quarterfeet>, au::CommonUnit<au::Trinches, au::Quarterfeet> >'
    ./au/unit_of_measure.hh:540:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    ./au/quantity.hh:617:8:   required from 'struct au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, void>'
    ./au/quantity.hh:630:8:   required from 'struct std::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int> >'
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11:   required by substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>}]'
    ./au/quantity.hh:494:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Quarterfeet, int>; U = au::Quantity<au::Trinches, int>; Func = au::detail::Equal]'
    ./au/quantity.hh:506:37:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:84:39:   required from here
    ./au/packs.hh:276:39: error: static assertion failed: Broken strict total ordering: distinct input types compare equal
    In file included from ./au/conversion_policy.hh:10,
                     from ./au/quantity.hh:7,
                     from ./au/math.hh:8,
                     from ./au/au.hh:7,
                     from au/error_examples.cc:1:
    ./au/unit_of_measure.hh: In instantiation of 'struct au::CommonUnit<au::Trinches, au::Quarterfeet>':
    ./au/packs.hh:192:7:   required by substitution of 'template<class U> using DimMemberT = typename U::Dim [with U = au::CommonUnit<au::Trinches, au::Quarterfeet>]'
    ./au/packs.hh:194:8:   required from 'struct au::detail::DimImpl<au::CommonUnit<au::Trinches, au::Quarterfeet> >'
    ./au/unit_of_measure.hh:452:8:   required from 'struct au::HasSameDimension<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>'
    ./au/stdx/type_traits.hh:26:61:   required from 'struct au::stdx::conjunction<au::HasSameDimension<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>, au::detail::HasSameMagnitude<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches> >'
    ./au/unit_of_measure.hh:467:8:   [ skipping 2 instantiation contexts, use -ftemplate-backtrace-limit=0 to disable ]
    ./au/unit_of_measure.hh:540:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    ./au/quantity.hh:617:8:   required from 'struct au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, void>'
    ./au/quantity.hh:630:8:   required from 'struct std::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int> >'
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11:   required by substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>}]'
    ./au/quantity.hh:494:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Quarterfeet, int>; U = au::Quantity<au::Trinches, int>; Func = au::detail::Equal]'
    ./au/quantity.hh:506:37:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:84:39:   required from here
    ./au/unit_of_measure.hh:492:70: error: static assertion failed: Elements must be listed in ascending order
      492 |     static_assert(AreElementsInOrder<CommonUnit, CommonUnit<Us...>>::value,
          |                                                                      ^~~~~
    ```
