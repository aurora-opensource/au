# Troubleshooting Guide

This page is a guide to the most commonly encountered types of error, what they mean, and how to fix
them.

The intended use case is to help you interpret an _actual error in your code_, at the point where
you encounter it.  To use this page, copy some relevant snippets from your compiler error, and then
search the text of this page using your browser's Find function.

!!! tip
    To improve your chances of finding what you're looking for, we include full compiler errors from
    both gcc and clang, inline with the text.  Naturally, this makes this page very long, so it's
    not meant to be read straight through.  Stick with your browser's Find function.

Each section below lists one category of compiler error you might encounter when using the library.
It explains what it means and how to solve it, and gives specific snippets of erroneous code, along
with the compiler errors that each would yield.

## Private constructor

**Meaning:**  This means you passed a raw numeric value to an interface that expected a Quantity.
It's the "classic" error the units library aims to prevent.

**Solution:** Call the appropriate Quantity maker: instead of passing `x`, pass `meters(x)`,
`(kilo(meters) / hour)(x)`, etc.

??? note "A note on quantity makers vs. constructors"
    Every other major units library lets you construct its Quantity types from raw numeric values;
    it just makes that constructor explicit.  Au goes further, and makes this constructor private.
    The reason is to preserve unit safety at all callsites.  We can't know whether you made an alias
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
    au/error_examples.cc:33:17: error: calling a private constructor of class 'au::Quantity<au::Seconds, double>'
        set_timeout(0.5);
                    ^
    au/code/au/quantity.hh:400:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    au/error_examples.cc:36:33: error: calling a private constructor of class 'au::Quantity<au::Meters, double>'
        constexpr QuantityD<Meters> length{5.5};
                                    ^
    au/code/au/quantity.hh:400:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    ```

    **Compiler error (clang 11)**
    ```
    au/error_examples.cc:33:17: error: calling a private constructor of class 'au::Quantity<au::Seconds, double>'
        set_timeout(0.5);
                    ^
    au/code/au/quantity.hh:400:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    au/error_examples.cc:36:33: error: calling a private constructor of class 'au::Quantity<au::Meters, double>'
        constexpr QuantityD<Meters> length{5.5};
                                    ^
    au/code/au/quantity.hh:400:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    ```

    **Compiler error (gcc 10)**
    ```
    au/error_examples.cc: In function 'void au::example_private_constructor()':
    au/error_examples.cc:33:20: error: 'constexpr au::Quantity<UnitT, RepT>::Quantity(au::Quantity<UnitT, RepT>::Rep) [with UnitT = au::Seconds; RepT = double; au::Quantity<UnitT, RepT>::Rep = double]' is private within this context
       33 |     set_timeout(0.5);
          |                    ^
    In file included from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/quantity.hh:400:15: note: declared private here
      400 |     constexpr Quantity(Rep value) : value_{value} {}
          |               ^~~~~~~~
    au/error_examples.cc:36:43: error: 'constexpr au::Quantity<UnitT, RepT>::Quantity(au::Quantity<UnitT, RepT>::Rep) [with UnitT = au::Meters; RepT = double; au::Quantity<UnitT, RepT>::Rep = double]' is private within this context
       36 |     constexpr QuantityD<Meters> length{5.5};
          |                                           ^
    In file included from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/quantity.hh:400:15: note: declared private here
      400 |     constexpr Quantity(Rep value) : value_{value} {}
          |               ^~~~~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    error_examples.cc(32): error C2248: 'au::Quantity<au::Seconds,double>::Quantity': cannot access private member declared in class 'au::Quantity<au::Seconds,double>'
    D:\a\au\au\au.hh(6759): note: see declaration of 'au::Quantity<au::Seconds,double>::Quantity'
    D:\a\au\au\au.hh(6403): note: see declaration of 'au::Quantity<au::Seconds,double>'
    error_examples.cc(35): error C2248: 'au::Quantity<au::Meters,double>::Quantity': cannot access private member declared in class 'au::Quantity<au::Meters,double>'
    D:\a\au\au\au.hh(6759): note: see declaration of 'au::Quantity<au::Meters,double>::Quantity'
    D:\a\au\au\au.hh(6403): note: see declaration of 'au::Quantity<au::Meters,double>'
    ```

## Input to Maker

**Meaning:**  This happens when you try to pass something to a "maker" (quantity maker, or quantity
point maker), but it's _already_ a `Quantity` or `QuantityPoint`.

**Solution:** Generally, this is pretty easy: just remove the redundant call.

!!! example
    **Code**

    === "Broken"
        ```cpp
        constexpr auto x = meters(1);
        constexpr auto x_pt = meters_pt(1);

        // A (BROKEN): passing something that is already a quantity to a quantity maker.
        meters(x);

        // B (BROKEN): same as above, but with quantity _points_.
        meters_pt(x_pt);
        ```

    === "Fixed"
        ```cpp
        constexpr auto x = meters(1);
        constexpr auto x_pt = meters_pt(1);

        // A (FIXED): just use the quantity directly.
        x;

        // B (FIXED): just use the quantity point directly.
        x_pt;
        ```

    **Compiler error (clang 14)**
    ```
    au/code/au/quantity.hh:523:9: error: static_assert failed due to requirement 'is_not_already_a_quantity' "Input to QuantityMaker is already a Quantity"
            static_assert(is_not_already_a_quantity, "Input to QuantityMaker is already a Quantity");
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~
    au/error_examples.cc:47:11: note: in instantiation of function template specialization 'au::QuantityMaker<au::Meters>::operator()<au::Meters, int>' requested here
        meters(x);
              ^
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:19:
    au/code/au/quantity_point.hh:295:9: error: static_assert failed due to requirement 'is_not_already_a_quantity_point' "Input to QuantityPointMaker is already a QuantityPoint"
            static_assert(is_not_already_a_quantity_point,
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    au/error_examples.cc:50:14: note: in instantiation of function template specialization 'au::QuantityPointMaker<au::Meters>::operator()<au::Meters, int>' requested here
        meters_pt(x_pt);
                 ^
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    ```

    **Compiler error (clang 11)**
    ```
    au/code/au/quantity.hh:523:9: error: static_assert failed due to requirement 'is_not_already_a_quantity' "Input to QuantityMaker is already a Quantity"
            static_assert(is_not_already_a_quantity, "Input to QuantityMaker is already a Quantity");
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~
    au/error_examples.cc:47:11: note: in instantiation of function template specialization 'au::QuantityMaker<au::Meters>::operator()<au::Meters, int>' requested here
        meters(x);
              ^
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:19:
    au/code/au/quantity_point.hh:295:9: error: static_assert failed due to requirement 'is_not_already_a_quantity_point' "Input to QuantityPointMaker is already a QuantityPoint"
            static_assert(is_not_already_a_quantity_point,
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    au/error_examples.cc:50:14: note: in instantiation of function template specialization 'au::QuantityPointMaker<au::Meters>::operator()<au::Meters, int>' requested here
        meters_pt(x_pt);
                 ^
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/quantity.hh: In instantiation of 'constexpr void au::QuantityMaker<UnitT>::operator()(au::Quantity<OtherUnit, OtherRep>) const [with U = au::Meters; R = int; UnitT = au::Meters]':
    au/error_examples.cc:47:13:   required from here
    au/code/au/quantity.hh:523:23: error: static assertion failed: Input to QuantityMaker is already a Quantity
      523 |         static_assert(is_not_already_a_quantity, "Input to QuantityMaker is already a Quantity");
          |                       ^~~~~~~~~~~~~~~~~~~~~~~~~
    In file included from au/code/au/prefix.hh:19,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/quantity_point.hh: In instantiation of 'constexpr void au::QuantityPointMaker<UnitT>::operator()(au::QuantityPoint<U, R>) const [with U = au::Meters; R = int; Unit = au::Meters]':
    au/error_examples.cc:50:19:   required from here
    au/code/au/quantity_point.hh:295:23: error: static assertion failed: Input to QuantityPointMaker is already a QuantityPoint
      295 |         static_assert(is_not_already_a_quantity_point,
          |                       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(6871): error C2338: static_assert failed: 'Input to QuantityMaker is already a Quantity'
    D:\a\au\au\au.hh(6871): note: the template instantiation context (the oldest one first) is
    error_examples.cc(50): note: see reference to function template instantiation 'void au::QuantityMaker<au::Meters>::operator ()<au::Meters,int>(au::Quantity<au::Meters,int>) const' being compiled
    error_examples.cc(50): note: see the first reference to 'au::QuantityMaker<au::Meters>::operator ()' in 'au::example_input_to_maker'
    D:\a\au\au\au.hh(7880): error C2338: static_assert failed: 'Input to QuantityPointMaker is already a QuantityPoint'
    D:\a\au\au\au.hh(7880): note: the template instantiation context (the oldest one first) is
    error_examples.cc(53): note: see reference to function template instantiation 'void au::QuantityPointMaker<au::Meters>::operator ()<Unit,T>(au::QuantityPoint<Unit,T>) const' being compiled
            with
            [
                Unit=au::Meters,
                T=int
            ]
    error_examples.cc(53): note: see the first reference to 'au::QuantityPointMaker<au::Meters>::operator ()' in 'au::example_input_to_maker'
    ```

## Dangerous conversion

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

2. **Use the "coercing" version**.  `inches(24).coerce_as(feet)` produces `feet(2)`.

!!! warning
    Stop and think before using the coercing version.  If you're reviewing code that uses it, ask
    about it.  The library is trying to protect you from an error prone operation.  The mechanism
    exists because sometimes you can know that it's OK, but remember to stop and check first!

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

    === "Fixed (2. Coercing version)"
        ```cpp
        // A (FIXED): 2. use coercing version.
        inches(24).coerce_as(feet);

        // B (FIXED): 2. use coercing version.
        giga(hertz)(1).coerce_as(hertz);
        ```


    **Compiler error (clang 14)**
    ```
    au/code/au/quantity.hh:163:9: error: static_assert failed due to requirement 'IMPLICIT_OK' "Dangerous conversion for integer Rep!  See: https://aurora-opensource.github.io/au/main/troubleshooting/#dangerous-conversion"
            static_assert(
            ^
    au/error_examples.cc:58:16: note: in instantiation of function template specialization 'au::Quantity<au::Inches, int>::as<au::QuantityMaker<au::Feet>, void>' requested here
        inches(24).as(feet);
                   ^
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    au/code/au/quantity.hh:163:9: error: static_assert failed due to requirement 'IMPLICIT_OK' "Dangerous conversion for integer Rep!  See: https://aurora-opensource.github.io/au/main/troubleshooting/#dangerous-conversion"
            static_assert(
            ^
    au/error_examples.cc:61:20: note: in instantiation of function template specialization 'au::Quantity<au::Giga<au::Hertz>, int>::as<au::QuantityMaker<au::Hertz>, void>' requested here
        giga(hertz)(1).as(hertz);
                       ^
    ```

    **Compiler error (clang 11)**
    ```
    au/code/au/quantity.hh:163:9: error: static_assert failed due to requirement 'IMPLICIT_OK' "Dangerous conversion for integer Rep!  See: https://aurora-opensource.github.io/au/main/troubleshooting/#dangerous-conversion"
            static_assert(
            ^
    au/error_examples.cc:58:16: note: in instantiation of function template specialization 'au::Quantity<au::Inches, int>::as<au::QuantityMaker<au::Feet>, void>' requested here
        inches(24).as(feet);
                   ^
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    au/code/au/quantity.hh:163:9: error: static_assert failed due to requirement 'IMPLICIT_OK' "Dangerous conversion for integer Rep!  See: https://aurora-opensource.github.io/au/main/troubleshooting/#dangerous-conversion"
            static_assert(
            ^
    au/error_examples.cc:61:20: note: in instantiation of function template specialization 'au::Quantity<au::Giga<au::Hertz>, int>::as<au::QuantityMaker<au::Hertz>, void>' requested here
        giga(hertz)(1).as(hertz);
                       ^
    ```

    **Compiler error (gcc 10)**
    ```
    ./au/quantity.hh: In instantiation of 'constexpr auto au::Quantity<UnitT, RepT>::as(NewUnit) const [with NewUnit = au::Feet; <template-parameter-2-2> = void; UnitT = au::Inches; RepT = int]':
    ./au/quantity.hh:206:18:   required from 'constexpr auto au::Quantity<UnitT, RepT>::as(au::QuantityMaker<NewUnit>) const [with NewUnit = au::Feet; UnitT = au::Inches; RepT = int]'
    au/error_examples.cc:44:23:   required from here
    ./au/quantity.hh:169:13: error: static assertion failed: Dangerous conversion for integer Rep!  See: https://aurora-opensource.github.io/au/main/troubleshooting/#dangerous-conversion
      169 |             IMPLICIT_OK,
          |             ^~~~~~~~~~~
    ./au/quantity.hh: In instantiation of 'constexpr auto au::Quantity<UnitT, RepT>::as(NewUnit) const [with NewUnit = au::Hertz; <template-parameter-2-2> = void; UnitT = au::Giga<au::Hertz>; RepT = int]':
    ./au/quantity.hh:206:18:   required from 'constexpr auto au::Quantity<UnitT, RepT>::as(au::QuantityMaker<NewUnit>) const [with NewUnit = au::Hertz; UnitT = au::Giga<au::Hertz>; RepT = int]'
    au/error_examples.cc:47:28:   required from here
    ./au/quantity.hh:169:13: error: static assertion failed: Dangerous conversion for integer Rep!  See: https://aurora-opensource.github.io/au/main/troubleshooting/#dangerous-conversion
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(6742): error C2338: static_assert failed: 'Truncation risk too high.  Can silence by passing `ignore(TRUNCATION_RISK)` as second argument, but first CAREFULLY CONSIDER whether this is really what you mean to do.'
    D:\a\au\au\au.hh(6742): note: the template instantiation context (the oldest one first) is
    error_examples.cc(61): note: see reference to function template instantiation 'au::Quantity<au::Feet,T> au::Quantity<au::Inches,int>::as<au::QuantityMaker<au::Feet>,au::detail::CheckTheseRisks<au::detail::RiskSet<3>>>(NewUnitSlot,RiskPolicyT) const' being compiled
            with
            [
                T=int,
                NewUnitSlot=au::QuantityMaker<au::Feet>,
                RiskPolicyT=au::detail::CheckTheseRisks<au::detail::RiskSet<3>>
            ]
    D:\a\au\au\au.hh(6469): note: see reference to function template instantiation 'OtherRep au::Quantity<au::Inches,int>::in_impl<int,NewUnitSlot,RiskPolicyT>(OtherUnitSlot,RiskPolicyT) const' being compiled
            with
            [
                OtherRep=int,
                NewUnitSlot=au::QuantityMaker<au::Feet>,
                RiskPolicyT=au::detail::CheckTheseRisks<au::detail::RiskSet<3>>,
                OtherUnitSlot=au::QuantityMaker<au::Feet>
            ]
    D:\a\au\au\au.hh(6735): error C2338: static_assert failed: 'Overflow risk too high.  Can silence by passing `ignore(OVERFLOW_RISK)` as second argument, but first CAREFULLY CONSIDER whether this is really what you mean to do.'
    D:\a\au\au\au.hh(6735): note: the template instantiation context (the oldest one first) is
    error_examples.cc(64): note: see reference to function template instantiation 'au::Quantity<au::Hertz,T> au::Quantity<au::Giga<U>,int>::as<au::QuantityMaker<au::Hertz>,au::detail::CheckTheseRisks<au::detail::RiskSet<3>>>(NewUnitSlot,RiskPolicyT) const' being compiled
            with
            [
                T=int,
                U=au::Hertz,
                NewUnitSlot=au::QuantityMaker<au::Hertz>,
                RiskPolicyT=au::detail::CheckTheseRisks<au::detail::RiskSet<3>>
            ]
    D:\a\au\au\au.hh(6469): note: see reference to function template instantiation 'OtherRep au::Quantity<au::Giga<U>,int>::in_impl<int,NewUnitSlot,RiskPolicyT>(OtherUnitSlot,RiskPolicyT) const' being compiled
            with
            [
                OtherRep=int,
                U=au::Hertz,
                NewUnitSlot=au::QuantityMaker<au::Hertz>,
                RiskPolicyT=au::detail::CheckTheseRisks<au::detail::RiskSet<3>>,
                OtherUnitSlot=au::QuantityMaker<au::Hertz>
            ]
    ```

## No type named 'type' in 'std::common_type'

**Meaning:**  You probably tried to perform a ["common-unit
operation"](./discussion/concepts/arithmetic.md#common-unit) (addition, subtraction, comparison)
with two incompatible Quantities.  Typically, this means they have different _dimensions_, which
makes this an intrinsically meaningless operation.

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
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:17:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/chrono:697:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/__chrono/calendar.h:13:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/__chrono/duration.h:14:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/limits:105:
    external/llvm_14_toolchain_llvm/bin/../include/c++/v1/type_traits:2388:25: error: no type named 'type' in 'std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>>'
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^~~~~
    au/code/au/quantity.hh:625:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    au/code/au/quantity.hh:663:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>, au::detail::Plus>' requested here
        return detail::using_common_type(q1, q2, detail::plus);
                       ^
    au/error_examples.cc:69:15: note: in instantiation of function template specialization 'au::operator+<au::Meters, au::Seconds, int, int>' requested here
        meters(1) + seconds(1);
                  ^
    ```

    **Compiler error (clang 11)**
    ```
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:17:
    In file included from external/llvm_11_toolchain_llvm/bin/../include/c++/v1/chrono:828:
    external/llvm_11_toolchain_llvm/bin/../include/c++/v1/type_traits:2462:25: error: no type named 'type' in 'std::__1::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>>'
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^~~~~
    au/code/au/quantity.hh:625:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    au/code/au/quantity.hh:663:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>, au::detail::Plus>' requested here
        return detail::using_common_type(q1, q2, detail::plus);
                       ^
    au/error_examples.cc:69:15: note: in instantiation of function template specialization 'au::operator+<au::Meters, au::Seconds, int, int>' requested here
        meters(1) + seconds(1);
                  ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from external/sysroot_x86_64//include/c++/10.3.0/ratio:39,
                     from external/sysroot_x86_64//include/c++/10.3.0/chrono:39,
                     from au/code/au/chrono_interop.hh:17,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    external/sysroot_x86_64//include/c++/10.3.0/type_traits: In substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>}]':
    au/code/au/quantity.hh:625:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Meters, int>; U = au::Quantity<au::Seconds, int>; Func = au::detail::Plus]'
    au/code/au/quantity.hh:663:37:   required from 'constexpr auto au::operator+(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Meters; U2 = au::Seconds; R1 = int; R2 = int]'
    au/error_examples.cc:69:26:   required from here
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11: error: no type named 'type' in 'struct std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int> >'
     2562 |     using common_type_t = typename common_type<_Tp...>::type;
          |           ^~~~~~~~~~~~~
    In file included from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/quantity.hh: In instantiation of 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Meters, int>; U = au::Quantity<au::Seconds, int>; Func = au::detail::Plus]':
    au/code/au/quantity.hh:663:37:   required from 'constexpr auto au::operator+(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Meters; U2 = au::Seconds; R1 = int; R2 = int]'
    au/error_examples.cc:69:26:   required from here
    au/code/au/quantity.hh:627:94: error: no type named 'type' in 'struct std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int> >'
      627 |         std::is_same<typename C::Rep, std::common_type_t<typename T::Rep, typename U::Rep>>::value,
          |                                                                                              ^~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.41.34120\include\type_traits(1334): error C2794: 'type': is not a member of any direct or indirect base class of 'std::common_type<T,U>'
            with
            [
                T=au::Quantity<au::Meters,int>,
                U=au::Quantity<au::Seconds,int>
            ]
    C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.41.34120\include\type_traits(1334): note: the template instantiation context (the oldest one first) is
    error_examples.cc(70): note: see reference to function template instantiation 'auto au::operator +<au::Meters,au::Seconds,int,int>(au::Quantity<au::Meters,int>,au::Quantity<au::Seconds,int>)' being compiled
    D:\a\au\au\au.hh(4531): note: see reference to function template instantiation 'auto au::detail::using_common_type<au::Quantity<au::Meters,int>,au::Quantity<au::Seconds,int>,au::detail::Plus>(T,U,Func)' being compiled
            with
            [
                T=au::Quantity<au::Meters,int>,
                U=au::Quantity<au::Seconds,int>,
                Func=au::detail::Plus
            ]
    D:\a\au\au\au.hh(4493): note: see reference to alias template instantiation 'std::common_type_t<T,U>' being compiled
            with
            [
                T=au::Quantity<au::Meters,int>,
                U=au::Quantity<au::Seconds,int>
            ]
    D:\a\au\au\au.hh(4493): error C2938: 'std::common_type_t' : Failed to specialize alias template
    D:\a\au\au\au.hh(4495): error C2057: expected constant expression
    D:\a\au\au\au.hh(4367): error C2668: 'au::Quantity<au::Meters,int>::as': ambiguous call to overloaded function
    D:\a\au\au\au.hh(4023): note: could be 'auto au::Quantity<au::Meters,int>::as<NewRep,enable_if<au::IsUnit<AssociatedUnit<NewUnit>::type>::value,void>::type>(NewUnit) const'
            with
            [
                NewRep=TargetUnit::Rep,
                NewUnit=TargetUnit::Rep
            ]
    D:\a\au\au\au.hh(4013): note: or       'auto au::Quantity<au::Meters,int>::as<NewRep,Unit,void>(NewUnit) const'
            with
            [
                NewRep=TargetUnit::Rep,
                Unit=au::Meters,
                NewUnit=au::Meters
            ]
    D:\a\au\au\au.hh(4367): note: while trying to match the argument list '(Unit)'
            with
            [
                Unit=au::Meters
            ]
    D:\a\au\au\au.hh(4367): note: the template instantiation context (the oldest one first) is
    D:\a\au\au\au.hh(4498): note: see reference to function template instantiation 'auto au::detail::cast_to_common_type<au::detail::using_common_type::C,au::Meters,int>(au::Quantity<au::Meters,int>)' being compiled
    D:\a\au\au\au.hh(4488): note: see reference to function template instantiation 'auto au::rep_cast<TargetUnit::Rep,au::Meters,int>(au::Quantity<au::Meters,int>)' being compiled
    D:\a\au\au\au.hh(4367): error C2668: 'au::Quantity<au::Seconds,int>::as': ambiguous call to overloaded function
    D:\a\au\au\au.hh(4023): note: could be 'auto au::Quantity<au::Seconds,int>::as<NewRep,enable_if<au::IsUnit<AssociatedUnit<NewUnit>::type>::value,void>::type>(NewUnit) const'
            with
            [
                NewRep=TargetUnit::Rep,
                NewUnit=TargetUnit::Rep
            ]
    D:\a\au\au\au.hh(4013): note: or       'auto au::Quantity<au::Seconds,int>::as<NewRep,Unit,void>(NewUnit) const'
            with
            [
                NewRep=TargetUnit::Rep,
                Unit=au::Seconds,
                NewUnit=au::Seconds
            ]
    D:\a\au\au\au.hh(4367): note: while trying to match the argument list '(Unit)'
            with
            [
                Unit=au::Seconds
            ]
    D:\a\au\au\au.hh(4367): note: the template instantiation context (the oldest one first) is
    D:\a\au\au\au.hh(4498): note: see reference to function template instantiation 'auto au::detail::cast_to_common_type<au::detail::using_common_type::C,au::Seconds,int>(au::Quantity<au::Seconds,int>)' being compiled
    D:\a\au\au\au.hh(4488): note: see reference to function template instantiation 'auto au::rep_cast<TargetUnit::Rep,au::Seconds,int>(au::Quantity<au::Seconds,int>)' being compiled
    D:\a\au\au\au.hh(4498): error C3889: call to object of class type 'au::detail::Plus': no matching call operator found
    D:\a\au\au\au.hh(954): note: could be 'auto au::detail::Plus::operator ()(const T &,const U &) const'
    D:\a\au\au\au.hh(4498): note: Failed to specialize function template 'auto au::detail::Plus::operator ()(const T &,const U &) const'
    D:\a\au\au\au.hh(4498): note: With the following template arguments:
    D:\a\au\au\au.hh(4498): note: 'T=void'
    D:\a\au\au\au.hh(4498): note: 'U=void'
    D:\a\au\au\au.hh(4498): note: you cannot create a reference to 'void'
    ```

## Can't pass `Quantity` to a unit slot {#quantity-to-unit-slot}

**Other variants:**

- "Can't pass `QuantityPoint` to a unit slot"
- "Can't pass `Quantity` to a unit slot for points"
- "Can't pass `QuantityPoint` to a unit slot for points"

**Meaning:**  A [unit slot](./discussion/concepts/unit-slot.md) is an API that takes _any unit-named
type in the library_, and treats it as the associated unit.  Besides simple unit types themselves,
these can include quantity makers (such as `meters`), unit symbols (such as `symbols::m`), constants
(such as `SPEED_OF_LIGHT`), and so on.

Notably, what it _cannot_ include is a `Quantity` or `QuantityPoint`.  Notice that all of the types
we mentioned above have a _completely unambiguous value_, known at compile time from the _type
alone_.  This is not the case for something like `Quantity`, which holds an underlying runtime
numeric value, to represent the quantity in its specific unit.

**Solution:**  If you're attempting to use the `Quantity` as an ad hoc unit, simply replace it with
a unit that you scale by a magnitude, `mag<N>()`.

!!! example

    **Code**

    Let's try to round a quantity of bytes to the nearest 10-byte amount.

    === "Broken"
        ```cpp
        // (BROKEN): can't pass Quantity to unit slot.
        auto size = bytes(1234);
        size = round_as<int>(bytes(10), size);
        //         unit slot ^^^^^^^^^
        ```

    === "Fixed"
        ```cpp
        // (FIXED): use an ad hoc scaled unit.
        auto size = bytes(1234);
        size = round_as<int>(bytes * mag<10>(), size);
        //         unit slot ^^^^^^^^^^^^^^^^^
        ```

    **Compiler error (clang 14)**

    ```
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    au/code/au/quantity.hh:430:5: error: static_assert failed due to requirement 'detail::AlwaysFalse<au::Bytes, int>::value' "Can't pass `Quantity` to a unit slot"
        static_assert(detail::AlwaysFalse<U, R>::value, "Can't pass `Quantity` to a unit slot");
        ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    au/code/au/unit_of_measure.hh:147:1: note: in instantiation of template class 'au::AssociatedUnit<au::Quantity<au::Bytes, int>>' requested here
    using AssociatedUnitT = typename AssociatedUnit<U>::type;
    ^
    au/code/au/math.hh:434:26: note: in instantiation of template type alias 'AssociatedUnitT' requested here
        return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in<OutputRep>(rounding_units, q));
                             ^
    au/error_examples.cc:78:12: note: in instantiation of function template specialization 'au::round_as<int, au::Quantity<au::Bytes, int>, au::Bytes, int>' requested here
        size = round_as<int>(bytes(10), size);
               ^
    ```

    **Compiler error (clang 11)**

    ```
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    au/code/au/quantity.hh:430:5: error: static_assert failed due to requirement 'detail::AlwaysFalse<au::Bytes, int>::value' "Can't pass `Quantity` to a unit slot"
        static_assert(detail::AlwaysFalse<U, R>::value, "Can't pass `Quantity` to a unit slot");
        ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    au/code/au/unit_of_measure.hh:147:1: note: in instantiation of template class 'au::AssociatedUnit<au::Quantity<au::Bytes, int>>' requested here
    using AssociatedUnitT = typename AssociatedUnit<U>::type;
    ^
    au/code/au/math.hh:434:26: note: in instantiation of template type alias 'AssociatedUnitT' requested here
        return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in<OutputRep>(rounding_units, q));
                             ^
    au/error_examples.cc:78:12: note: in instantiation of function template specialization 'au::round_as<int, au::Quantity<au::Bytes, int>, au::Bytes, int>' requested here
        size = round_as<int>(bytes(10), size);
               ^
    ```

    **Compiler error (gcc 10)**

    ```
    au/code/au/quantity.hh: In instantiation of 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >':
    au/code/au/unit_of_measure.hh:147:7:   required by substitution of 'template<class U> using AssociatedUnitT = typename au::AssociatedUnit::type [with U = au::Quantity<au::Bytes, int>]'
    au/code/au/math.hh:434:12:   required from 'auto au::round_as(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/error_examples.cc:78:41:   required from here
    au/code/au/quantity.hh:430:46: error: static assertion failed: Can't pass `Quantity` to a unit slot
      430 |     static_assert(detail::AlwaysFalse<U, R>::value, "Can't pass `Quantity` to a unit slot");
          |                                              ^~~~~
    In file included from au/code/au/conversion_policy.hh:22,
                     from au/code/au/quantity.hh:20,
                     from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/unit_of_measure.hh: In substitution of 'template<class U> using AssociatedUnitT = typename au::AssociatedUnit::type [with U = au::Quantity<au::Bytes, int>]':
    au/code/au/math.hh:434:12:   required from 'auto au::round_as(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/error_examples.cc:78:41:   required from here
    au/code/au/unit_of_measure.hh:147:7: error: no type named 'type' in 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >'
      147 | using AssociatedUnitT = typename AssociatedUnit<U>::type;
          |       ^~~~~~~~~~~~~~~
    In file included from au/code/au/au.hh:19,
                     from au/error_examples.cc:15:
    au/code/au/math.hh: In instantiation of 'auto au::round_in(RoundingUnits, au::Quantity<U, R>) [with RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]':
    au/code/au/math.hh:400:43:   required from 'auto au::round_in(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/code/au/math.hh:434:77:   required from 'auto au::round_as(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/error_examples.cc:78:41:   required from here
    au/code/au/math.hh:382:52: error: no matching function for call to 'au::Quantity<au::Bytes, int>::in<OurRoundingRep>(au::Quantity<au::Bytes, int>&)'
      382 |     return std::round(q.template in<OurRoundingRep>(rounding_units));
          |                       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~
    In file included from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/quantity.hh:174:22: note: candidate: 'template<class NewRep, class NewUnit, class> constexpr NewRep au::Quantity<UnitT, RepT>::in(NewUnit) const [with NewRep = NewRep; NewUnit = NewUnit; <template-parameter-2-3> = <template-parameter-1-3>; UnitT = au::Bytes; RepT = int]'
      174 |     constexpr NewRep in(NewUnit u) const {
          |                      ^~
    au/code/au/quantity.hh:174:22: note:   template argument deduction/substitution failed:
    au/code/au/quantity.hh:173:15: error: no type named 'type' in 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >'
      173 |               typename = std::enable_if_t<IsUnit<AssociatedUnitT<NewUnit>>::value>>
          |               ^~~~~~~~
    au/code/au/quantity.hh:184:19: note: candidate: 'template<class NewUnit, class> constexpr au::Quantity<UnitT, RepT>::Rep au::Quantity<UnitT, RepT>::in(NewUnit) const [with NewUnit = NewUnit; <template-parameter-2-2> = <template-parameter-1-2>; UnitT = au::Bytes; RepT = int]'
      184 |     constexpr Rep in(NewUnit u) const {
          |                   ^~
    au/code/au/quantity.hh:184:19: note:   template argument deduction/substitution failed:
    In file included from external/sysroot_x86_64//include/c++/10.3.0/ratio:39,
                     from external/sysroot_x86_64//include/c++/10.3.0/chrono:39,
                     from au/code/au/chrono_interop.hh:17,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    external/sysroot_x86_64//include/c++/10.3.0/type_traits: In substitution of 'template<bool _Cond, class _Tp> using enable_if_t = typename std::enable_if::type [with bool _Cond = false; _Tp = void]':
    au/code/au/quantity.hh:183:15:   required from 'auto au::round_in(RoundingUnits, au::Quantity<U, R>) [with RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/code/au/math.hh:400:43:   required from 'auto au::round_in(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/code/au/math.hh:434:77:   required from 'auto au::round_as(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/error_examples.cc:78:41:   required from here
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2554:11: error: no type named 'type' in 'struct std::enable_if<false, void>'
     2554 |     using enable_if_t = typename enable_if<_Cond, _Tp>::type;
          |           ^~~~~~~~~~~
    In file included from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    ```

    **Compiler error (MSVC 2022 x64)**

    ```
    ```

## Integer division forbidden {#integer-division-forbidden}

**Meaning:**  Although Au generally tries to act just like the underlying raw numeric types, we also
try to prevent wrong code that _looks_ correct from compiling.  It turns out to be just too easy to
use integral Reps without noticing, and thus to get integer division without noticing.  This can
lead to very large errors.

**Solution:**  If you _really wanted_ integer division, wrap the denominator in `unblock_int_div()`.
Otherwise, use floating point types.

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

    === "Fixed (2. `unblock_int_div()`)"
        ```cpp
        // (FIXED): 2. Integer result == (meter * hours / mile)(0)
        auto t = meters(60) / unblock_int_div((miles / hour)(65));
        ```


    **Compiler error (clang 14)**
    ```
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    au/code/au/quantity.hh:395:9: error: static_assert failed due to requirement 'are_units_quantity_equivalent || !uses_integer_division' "Integer division forbidden: wrap denominator in `unblock_int_div()` if you really want it"
            static_assert(are_units_quantity_equivalent || !uses_integer_division,
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    au/code/au/quantity.hh:325:9: note: in instantiation of function template specialization 'au::Quantity<au::Meters, int>::warn_if_integer_division<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1>>, int>' requested here
            warn_if_integer_division<OtherUnit, OtherRep>();
            ^
    au/error_examples.cc:77:39: note: in instantiation of function template specialization 'au::Quantity<au::Meters, int>::operator/<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1>>, int>' requested here
        QuantityD<Seconds> t = meters(60) / (miles / hour)(65);
                                          ^
    ```

    **Compiler error (clang 11)**
    ```
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    au/code/au/quantity.hh:395:9: error: static_assert failed due to requirement 'are_units_quantity_equivalent || !uses_integer_division' "Integer division forbidden: wrap denominator in `unblock_int_div()` if you really want it"
            static_assert(are_units_quantity_equivalent || !uses_integer_division,
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    au/code/au/quantity.hh:325:9: note: in instantiation of function template specialization 'au::Quantity<au::Meters, int>::warn_if_integer_division<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1>>, int>' requested here
            warn_if_integer_division<OtherUnit, OtherRep>();
            ^
    au/error_examples.cc:77:39: note: in instantiation of function template specialization 'au::Quantity<au::Meters, int>::operator/<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1>>, int>' requested here
        QuantityD<Seconds> t = meters(60) / (miles / hour)(65);
                                          ^
    ```

    **Compiler error (gcc 10)**
    ```
    au/code/au/quantity.hh: In instantiation of 'static constexpr void au::Quantity<UnitT, RepT>::warn_if_integer_division() [with OtherUnit = au::UnitProduct<au::Miles, au::Pow<au::Hours, -1> >; OtherRep = int; UnitT = au::Meters; RepT = int]':
    au/code/au/quantity.hh:325:54:   required from here
    au/error_examples.cc:77:58:   in 'constexpr' expansion of 'au::meters.au::QuantityMaker<au::Meters>::operator()<int>(60).au::Quantity<au::Meters, int>::operator/<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1> >, int>(au::miles.au::QuantityMaker<au::Miles>::operator/<au::Hours>((au::hour, const au::SingularNameFor<au::Hours>())).au::QuantityMaker<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1> > >::operator()<int>(65))'
    au/code/au/quantity.hh:395:53: error: static assertion failed: Integer division forbidden: wrap denominator in `unblock_int_div()` if you really want it
      395 |         static_assert(are_units_quantity_equivalent || !uses_integer_division,
          |                       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(4263): error C2338: static_assert failed: 'Integer division forbidden: wrap denominator in `unblock_int_div()` if you really want it'
    D:\a\au\au\au.hh(4263): note: the template instantiation context (the oldest one first) is
    error_examples.cc(78): note: see reference to function template instantiation 'au::Quantity<au::UnitProduct<T,au::Pow<B,-1>,au::Hours>,int> au::Quantity<au::Meters,int>::operator /<au::UnitProduct<au::Miles,au::Pow<au::Hours,-1>>,int>(au::Quantity<au::UnitProduct<au::Miles,au::Pow<au::Hours,-1>>,int>) const' being compiled
            with
            [
                T=au::Meters,
                B=au::Miles
            ]
    D:\a\au\au\au.hh(4193): note: see reference to function template instantiation 'void au::Quantity<au::Meters,int>::warn_if_integer_division<OtherUnit,OtherRep>(void)' being compiled
            with
            [
                OtherUnit=au::UnitProduct<au::Miles,au::Pow<au::Hours,-1>>,
                OtherRep=int
            ]
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
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:19:
    au/code/au/math.hh:278:5: error: static_assert failed due to requirement 'UNITY.in<int>(associated_unit(target_units) * au::Hertz{}) >= threshold || std::is_floating_point<int>::value' "Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired"
        static_assert(
        ^
    au/code/au/math.hh:294:56: note: in instantiation of function template specialization 'au::inverse_in<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in(target_units, q));
                                                           ^
    au/error_examples.cc:85:5: note: in instantiation of function template specialization 'au::inverse_as<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        inverse_as(seconds, hertz(5));
        ^
    ```

    **Compiler error (clang 11)**
    ```
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:19:
    au/code/au/math.hh:278:5: error: static_assert failed due to requirement 'UNITY.in<int>(associated_unit(target_units) * au::Hertz{}) >= threshold || std::is_floating_point<int>::value' "Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired"
        static_assert(
        ^
    au/code/au/math.hh:294:56: note: in instantiation of function template specialization 'au::inverse_in<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in(target_units, q));
                                                           ^
    au/error_examples.cc:85:5: note: in instantiation of function template specialization 'au::inverse_as<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        inverse_as(seconds, hertz(5));
        ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from au/code/au/au.hh:19,
                     from au/error_examples.cc:15:
    au/code/au/math.hh: In instantiation of 'constexpr auto au::inverse_in(TargetUnits, au::Quantity<U, R>) [with TargetUnits = au::QuantityMaker<au::Seconds>; U = au::Hertz; R = int]':
    au/code/au/math.hh:294:66:   required from 'constexpr auto au::inverse_as(TargetUnits, au::Quantity<U, R>) [with TargetUnits = au::QuantityMaker<au::Seconds>; U = au::Hertz; R = int]'
    au/error_examples.cc:85:33:   required from here
    au/code/au/math.hh:279:71: error: static assertion failed: Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired
      279 |         UNITY.in<R>(associated_unit(target_units) * U{}) >= threshold ||
          |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~
      280 |             std::is_floating_point<R>::value,
          |             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                           
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(6120): error C2338: static_assert failed: 'Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired'
    D:\a\au\au\au.hh(6120): note: the template instantiation context (the oldest one first) is
    error_examples.cc(86): note: see reference to function template instantiation 'auto au::inverse_as<au::QuantityMaker<au::Seconds>,au::Hertz,int>(TargetUnits,au::Quantity<au::Hertz,int>)' being compiled
            with
            [
                TargetUnits=au::QuantityMaker<au::Seconds>
            ]
    D:\a\au\au\au.hh(6135): note: see reference to function template instantiation 'auto au::inverse_in<TargetUnits,au::Hertz,int>(TargetUnits,au::Quantity<au::Hertz,int>)' being compiled
            with
            [
                TargetUnits=au::QuantityMaker<au::Seconds>
            ]
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
    au/error_examples.cc:93:34: error: deduced conflicting types ('Quantity<au::QuantityMaker<au::Hertz>::Unit, [...]>' vs 'Quantity<au::QuantityMaker<au::Pow<au::Seconds, -1>>::Unit, [...]>') for initializer list element type
        for (const auto &frequency : {
                                     ^
    ```

    **Compiler error (clang 11)**
    ```
    au/error_examples.cc:93:34: error: deduced conflicting types ('Quantity<au::Hertz, [...]>' vs 'Quantity<au::Pow<au::Seconds, -1>, [...]>') for initializer list element type
        for (const auto &frequency : {
                                     ^
    ```

    **Compiler error (gcc 10)**
    ```
    au/error_examples.cc: In function 'void au::example_deduced_conflicting_types()':
    au/error_examples.cc:96:10: error: unable to deduce 'std::initializer_list<auto>&&' from '{au::hertz.au::QuantityMaker<au::Hertz>::operator()<double>(1.0e+0), au::operator/<int>(1, au::seconds.au::QuantityMaker<au::Seconds>::operator()<double>(2.0e+0))}'
       96 |          }) {
          |          ^
    au/error_examples.cc:96:10: note:   deduced conflicting types for parameter 'auto' ('au::Quantity<au::Hertz, double>' and 'au::Quantity<au::Pow<au::Seconds, -1>, double>')
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    error_examples.cc(94): error C3535: cannot deduce type for 'auto &&' from 'initializer list'
    error_examples.cc(94): error C2440: 'initializing': cannot convert from 'initializer list' to 'std::initializer_list<int> &&'
    error_examples.cc(94): note: Reason: cannot convert from 'initializer list' to 'std::initializer_list<int>'
    error_examples.cc(94): note: Element '1': no conversion from 'au::Quantity<au::Hertz,double>' to 'int'
    error_examples.cc(94): note: Element '2': no conversion from 'au::Quantity<au::Pow<B,-1>,T>' to 'int'
            with
            [
                B=au::Seconds,
                T=double
            ]
    ```

## Broken strict total ordering

**Meaning:**  This means you performed an operation that needs to put unit types into a parameter
pack --- say, a common unit, or a unit product --- but the library couldn't figure out how to order
the units inside that pack.

If that sounds obscure, it is: ordering units inside packs is a deep library implementation detail,
and we try to avoid letting end users encounter this.  To reach this error, you need two _distinct_
units that have the _same_ Dimension, Magnitude, _and_ Origin.  That's a necessary but not
sufficient condition: for example, even `UnitInverseT<Seconds>` and `Hertz` won't trigger this!

??? info "More background info on why this error exists"
    In case you want to understand more, here is the gist.

    Au is _heavily_ based on [parameter packs](./reference/detail/packs.md).  Some of these packs,
    such as `UnitProduct<...>` and `CommonUnit<...>`, take _units_ as their arguments.

    Every parameter pack needs an unambiguous canonical ordering for any possible set of input
    arguments.  Therefore, we need to create a _strict total ordering_ for the (infinitely many!)
    unit types that could appear in these packs.  This ordering needs to be known _at compile time_.
    The ordering itself doesn't matter so much, but if we don't strictly adhere to _some_ ordering,
    it's undefined behaviour.

    Our strategy is to construct a "gauntlet" of properties which we can measure for any unit (e.g.,
    Dimension, Magnitude, ...), and define some arbitrary ordering for each property.  We then
    compare the units on each property in turn.  The first one where they differ "wins".  If we get
    through _all_ the properties, and they're _still_ tied, then we have two _distinct_ unit types
    which _compare_ as equal.  This would be undefined behaviour!  Rather than silently ignoring
    this, we manifest this as a compiler error.

    That is what "broken strict total ordering" means.

**Solution:**  If you have two distinct units, and the library can't figure out how to order them,
you can _force_ a particular ordering.  Choose one of the units and give it a high "unit avoidance"
score (see example below).  This will break the tie.

Again, this is pretty unusual.  For most normal ways of forming units, the library should
automatically be able to define an ordering for them.  If you do hit this error, it may be worth
pausing to double-check that you're using the library correctly.  If you've checked, and it still
seems like something the library should be able to handle, feel free to file an
[issue](https://github.com/aurora-opensource/au/issues) --- maybe there's a way we can improve our
ordering!

!!! tip
    If you hit this error, you might be annoyed by its obscurity.  Instead, try feeling relieved!
    After all, the alternative is not "correctly working program", but "silent undefined behaviour".
    A compiler error with a searchable error message is infinitely preferable to the latter.

!!! example

    **Code**

    Note that this example is somewhat convoluted, but again, that's to be expected because this
    error is pretty hard to hit in practice.

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
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    In file included from au/code/au/quantity.hh:19:
    In file included from au/code/au/apply_magnitude.hh:17:
    In file included from au/code/au/apply_rational_magnitude_to_integral.hh:19:
    In file included from au/code/au/magnitude.hh:21:
    au/code/au/packs.hh:287:5: error: static_assert failed due to requirement 'std::is_same<au::Quarterfeet, au::Trinches>::value' "Broken strict total ordering: distinct input types compare equal"
        static_assert(std::is_same<A, B>::value,
        ^             ~~~~~~~~~~~~~~~~~~~~~~~~~
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches>' requested here
        std::conditional_t<
        ^
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderAsUnitProduct>' requested here
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>' requested here
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>' requested here
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>' requested here
    au/code/au/unit_of_measure.hh:920:40: note: (skipping 8 contexts in backtrace; use -ftemplate-backtrace-limit=0 to see all)
    struct InOrderFor<UnitProduct, A, B> : LexicographicTotalOrdering<A,
                                           ^
    au/code/au/quantity.hh:762:7: note: in instantiation of template class 'au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>>' requested here
        : au::CommonQuantity<au::Quantity<U1, R1>, au::Quantity<U2, R2>> {};
          ^
    external/llvm_14_toolchain_llvm/bin/../include/c++/v1/type_traits:2388:25: note: in instantiation of template class 'std::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>>' requested here
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^
    au/code/au/quantity.hh:625:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    au/code/au/quantity.hh:637:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, au::detail::Equal>' requested here
        return detail::using_common_type(q1, q2, detail::equal);
                       ^
    au/error_examples.cc:112:25: note: in instantiation of function template specialization 'au::operator==<au::Quarterfeet, au::Trinches, int, int>' requested here
        if (quarterfeet(10) == trinches(10)) {
                            ^
    ```

    **Compiler error (clang 11)**
    ```
    In file included from au/error_examples.cc:15:
    In file included from au/code/au/au.hh:17:
    In file included from au/code/au/chrono_interop.hh:20:
    In file included from au/code/au/prefix.hh:18:
    In file included from au/code/au/quantity.hh:19:
    In file included from au/code/au/apply_magnitude.hh:17:
    In file included from au/code/au/apply_rational_magnitude_to_integral.hh:19:
    In file included from au/code/au/magnitude.hh:21:
    au/code/au/packs.hh:287:5: error: static_assert failed due to requirement 'std::is_same<au::Quarterfeet, au::Trinches>::value' "Broken strict total ordering: distinct input types compare equal"
        static_assert(std::is_same<A, B>::value,
        ^             ~~~~~~~~~~~~~~~~~~~~~~~~~
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches>' requested here
        std::conditional_t<
        ^
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, OrderAsUnitProduct>' requested here
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, OrderByOrigin, OrderAsUnitProduct>' requested here
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, OrderByMag, OrderByOrigin, OrderAsUnitProduct>' requested here
    au/code/au/packs.hh:303:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, OrderByDim, OrderByMag, OrderByOrigin, OrderAsUnitProduct>' requested here
    au/code/au/unit_of_measure.hh:920:40: note: (skipping 8 contexts in backtrace; use -ftemplate-backtrace-limit=0 to see all)
    struct InOrderFor<UnitProduct, A, B> : LexicographicTotalOrdering<A,
                                           ^
    au/code/au/quantity.hh:762:7: note: in instantiation of template class 'au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, void>' requested here
        : au::CommonQuantity<au::Quantity<U1, R1>, au::Quantity<U2, R2>> {};
          ^
    external/llvm_11_toolchain_llvm/bin/../include/c++/v1/type_traits:2462:25: note: in instantiation of template class 'std::__1::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>>' requested here
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^
    au/code/au/quantity.hh:625:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    au/code/au/quantity.hh:637:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, au::detail::Equal>' requested here
        return detail::using_common_type(q1, q2, detail::equal);
                       ^
    au/error_examples.cc:112:25: note: in instantiation of function template specialization 'au::operator==<au::Quarterfeet, au::Trinches, int, int>' requested here
        if (quarterfeet(10) == trinches(10)) {
                            ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from au/code/au/magnitude.hh:21,
                     from au/code/au/apply_rational_magnitude_to_integral.hh:19,
                     from au/code/au/apply_magnitude.hh:17,
                     from au/code/au/quantity.hh:19,
                     from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/packs.hh: In instantiation of 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches>':
    au/code/au/packs.hh:298:8:   recursively required from 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>'
    au/code/au/packs.hh:298:8:   required from 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByUnitAvoidance, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct>'
    au/code/au/unit_of_measure.hh:920:8:   required from 'struct au::InOrderFor<au::UnitProduct, au::Quarterfeet, au::Trinches>'
    au/code/au/unit_of_measure.hh:505:8:   required from 'struct au::InOrderFor<au::CommonUnit, au::Quarterfeet, au::Trinches>'
    au/code/au/packs.hh:383:8:   required from 'struct au::FlatDedupedTypeList<au::CommonUnit, au::CommonUnit<au::Quarterfeet>, au::CommonUnit<au::Trinches> >'
    au/code/au/unit_of_measure.hh:592:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    au/code/au/quantity.hh:748:8:   required from 'struct au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, void>'
    au/code/au/quantity.hh:761:8:   required from 'struct std::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int> >'
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11:   required by substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>}]'
    au/code/au/quantity.hh:625:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Quarterfeet, int>; U = au::Quantity<au::Trinches, int>; Func = au::detail::Equal]'
    au/code/au/quantity.hh:637:37:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:112:39:   required from here
    au/code/au/packs.hh:287:39: error: static assertion failed: Broken strict total ordering: distinct input types compare equal
      287 |     static_assert(std::is_same<A, B>::value,
          |                                       ^~~~~
    au/code/au/packs.hh: In instantiation of 'struct au::LexicographicTotalOrdering<au::Trinches, au::Quarterfeet>':
    au/code/au/packs.hh:298:8:   [ skipping 2 instantiation contexts, use -ftemplate-backtrace-limit=0 to disable ]
    au/code/au/unit_of_measure.hh:505:8:   required from 'struct au::InOrderFor<au::CommonUnit, au::Trinches, au::Quarterfeet>'
    au/code/au/unit_of_measure.hh:554:8:   required from 'struct au::detail::IsFirstUnitRedundant<au::CommonUnit, au::Quarterfeet, au::Trinches>'
    au/code/au/unit_of_measure.hh:564:8:   required from 'struct au::detail::EliminateRedundantUnitsImpl<au::CommonUnit<au::Trinches, au::Quarterfeet> >'
    au/code/au/unit_of_measure.hh:592:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    au/code/au/quantity.hh:748:8:   required from 'struct au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, void>'
    au/code/au/quantity.hh:761:8:   required from 'struct std::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int> >'
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11:   required by substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>}]'
    au/code/au/quantity.hh:625:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Quarterfeet, int>; U = au::Quantity<au::Trinches, int>; Func = au::detail::Equal]'
    au/code/au/quantity.hh:637:37:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:112:39:   required from here
    au/code/au/packs.hh:287:39: error: static assertion failed: Broken strict total ordering: distinct input types compare equal
    In file included from au/code/au/conversion_policy.hh:22,
                     from au/code/au/quantity.hh:20,
                     from au/code/au/prefix.hh:18,
                     from au/code/au/chrono_interop.hh:20,
                     from au/code/au/au.hh:17,
                     from au/error_examples.cc:15:
    au/code/au/unit_of_measure.hh: In instantiation of 'struct au::CommonUnit<au::Trinches, au::Quarterfeet>':
    au/code/au/packs.hh:203:7:   required by substitution of 'template<class U> using DimMemberT = typename U::Dim [with U = au::CommonUnit<au::Trinches, au::Quarterfeet>]'
    au/code/au/packs.hh:205:8:   required from 'struct au::detail::DimImpl<au::CommonUnit<au::Trinches, au::Quarterfeet> >'
    au/code/au/unit_of_measure.hh:455:8:   required from 'struct au::HasSameDimension<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>'
    au/code/au/stdx/type_traits.hh:38:59:   required from 'struct au::stdx::conjunction<au::HasSameDimension<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>, au::detail::HasSameMagnitude<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches> >'
    au/code/au/unit_of_measure.hh:470:8:   [ skipping 2 instantiation contexts, use -ftemplate-backtrace-limit=0 to disable ]
    au/code/au/unit_of_measure.hh:592:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    au/code/au/quantity.hh:748:8:   required from 'struct au::CommonQuantity<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>, void>'
    au/code/au/quantity.hh:761:8:   required from 'struct std::common_type<au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int> >'
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11:   required by substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Quarterfeet, int>, au::Quantity<au::Trinches, int>}]'
    au/code/au/quantity.hh:625:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Quarterfeet, int>; U = au::Quantity<au::Trinches, int>; Func = au::detail::Equal]'
    au/code/au/quantity.hh:637:37:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:112:39:   required from here
    au/code/au/unit_of_measure.hh:495:70: error: static assertion failed: Elements must be listed in ascending order
      495 |     static_assert(AreElementsInOrder<CommonUnit, CommonUnit<Us...>>::value,
          |                                                                      ^~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(1392): error C2338: static_assert failed: 'Broken strict total ordering: distinct input types compare equal'
    D:\a\au\au\au.hh(1392): note: the template instantiation context (the oldest one first) is
    error_examples.cc(113): note: see reference to function template instantiation 'bool au::operator ==<au::Quarterfeet,au::Trinches,int,int>(au::Quantity<au::Quarterfeet,int>,au::Quantity<au::Trinches,int>)' being compiled
    D:\a\au\au\au.hh(4505): note: see reference to function template instantiation 'auto au::detail::using_common_type<au::Quantity<au::Quarterfeet,int>,au::Quantity<au::Trinches,int>,au::detail::Equal>(T,U,Func)' being compiled
            with
            [
                T=au::Quantity<au::Quarterfeet,int>,
                U=au::Quantity<au::Trinches,int>,
                Func=au::detail::Equal
            ]
    D:\a\au\au\au.hh(4493): note: see reference to alias template instantiation 'std::common_type_t<T,U>' being compiled
            with
            [
                T=au::Quantity<au::Quarterfeet,int>,
                U=au::Quantity<au::Trinches,int>
            ]
    C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.41.34120\include\type_traits(1334): note: see reference to class template instantiation 'std::common_type<T,U>' being compiled
            with
            [
                T=au::Quantity<au::Quarterfeet,int>,
                U=au::Quantity<au::Trinches,int>
            ]
    D:\a\au\au\au.hh(4630): note: see reference to class template instantiation 'au::CommonQuantity<au::Quantity<au::Quarterfeet,int>,au::Quantity<au::Trinches,int>,void>' being compiled
    D:\a\au\au\au.hh(4619): note: see reference to alias template instantiation 'au::CommonUnitT<U1,U2>' being compiled
            with
            [
                U1=au::Quarterfeet,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(2806): note: see reference to class template instantiation 'au::ComputeCommonUnit<U1,U2>' being compiled
            with
            [
                U1=au::Quarterfeet,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(3230): note: see reference to alias template instantiation 'au::ComputeCommonUnitImpl<U1,U2>' being compiled
            with
            [
                U1=au::Quarterfeet,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(3226): note: see reference to alias template instantiation 'au::FlatDedupedTypeListT<au::CommonUnit,U1,U2>' being compiled
            with
            [
                U1=au::Quarterfeet,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(1213): note: see reference to class template instantiation 'au::FlatDedupedTypeList<au::CommonUnit,au::CommonUnit<T>,au::CommonUnit<au::Trinches>>' being compiled
            with
            [
                T=au::Quarterfeet
            ]
    D:\a\au\au\au.hh(1496): note: see reference to class template instantiation 'au::InOrderFor<List,T,H>' being compiled
            with
            [
                List=au::CommonUnit,
                T=au::Quarterfeet,
                H=au::Trinches
            ]
    D:\a\au\au\au.hh(3142): note: see reference to class template instantiation 'au::InOrderFor<au::UnitProduct,A,B>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(3557): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByUnitAvoidance,au::detail::OrderByDim,au::detail::OrderByMag,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(1408): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByDim,au::detail::OrderByMag,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(1408): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByMag,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(1408): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(1408): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderAsUnitProduct>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(1408): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(3132): error C2338: static_assert failed: 'Elements must be listed in ascending order'
    D:\a\au\au\au.hh(3132): note: the template instantiation context (the oldest one first) is
    D:\a\au\au\au.hh(3230): note: see reference to class template instantiation 'au::detail::FirstMatchingUnit<au::AreUnitsQuantityEquivalent,au::CommonUnit<T,au::Quarterfeet>,TargetUnit>' being compiled
            with
            [
                T=au::Trinches,
                TargetUnit=au::CommonUnit<au::Trinches,au::Quarterfeet>
            ]
    D:\a\au\au\au.hh(3170): note: see reference to class template instantiation 'au::AreUnitsQuantityEquivalent<TargetUnit,H>' being compiled
            with
            [
                TargetUnit=au::CommonUnit<au::Trinches,au::Quarterfeet>,
                H=au::Trinches
            ]
    D:\a\au\au\au.hh(3108): note: see reference to class template instantiation 'au::stdx::conjunction<au::HasSameDimension<U1,U2>,au::detail::HasSameMagnitude<U1,U2>>' being compiled
            with
            [
                U1=au::CommonUnit<au::Trinches,au::Quarterfeet>,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(210): note: see reference to class template instantiation 'au::HasSameDimension<U1,U2>' being compiled
            with
            [
                U1=au::CommonUnit<au::Trinches,au::Quarterfeet>,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(3093): note: see reference to alias template instantiation 'au::detail::DimT<U1>' being compiled
            with
            [
                U1=au::CommonUnit<au::Trinches,au::Quarterfeet>
            ]
    D:\a\au\au\au.hh(1312): note: see reference to class template instantiation 'au::detail::DimImpl<U1>' being compiled
            with
            [
                U1=au::CommonUnit<au::Trinches,au::Quarterfeet>
            ]
    D:\a\au\au\au.hh(1310): note: see reference to alias template instantiation 'au::detail::DimMemberT<U>' being compiled
            with
            [
                U=au::CommonUnit<au::Trinches,au::Quarterfeet>
            ]
    D:\a\au\au\au.hh(1308): note: see reference to class template instantiation 'au::CommonUnit<T,au::Quarterfeet>' being compiled
            with
            [
                T=au::Trinches
            ]
    ```



