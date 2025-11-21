# Troubleshooting Guide

This page is a guide to the most commonly encountered types of error, what they mean, and how to fix
them.

The intended use case is to help you interpret an _actual error in your code_, at the point where
you encounter it.  To use this page, copy some relevant snippets from your compiler error, and then
search the text of this page using your browser's Find function.

!!! tip
    To improve your chances of finding what you're looking for, we include full compiler errors from
    gcc, clang, and MSVC, inline with the text.  Naturally, this makes this page very long, so it's
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
    au/error_examples.cc:34:17: error: calling a private constructor of class 'au::Quantity<au::Seconds, double>'
        set_timeout(0.5);
                    ^
    ./au/quantity.hh:483:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    au/error_examples.cc:37:33: error: calling a private constructor of class 'au::Quantity<au::Meters, double>'
        constexpr QuantityD<Meters> length{5.5};
                                    ^
    ./au/quantity.hh:483:15: note: declared private here
        constexpr Quantity(Rep value) : value_{value} {}
                  ^
    ```

    **Compiler error (gcc 10)**
    ```
    au/error_examples.cc: In function 'void au::example_private_constructor()':
    au/error_examples.cc:34:20: error: 'constexpr au::Quantity<UnitT, RepT>::Quantity(au::Quantity<UnitT, RepT>::Rep) [with UnitT = au::Seconds; RepT = double; au::Quantity<UnitT, RepT>::Rep = double]' is private within this context
       34 |     set_timeout(0.5);
          |                    ^
    In file included from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity.hh:483:15: note: declared private here
      483 |     constexpr Quantity(Rep value) : value_{value} {}
          |               ^~~~~~~~
    au/error_examples.cc:37:43: error: 'constexpr au::Quantity<UnitT, RepT>::Quantity(au::Quantity<UnitT, RepT>::Rep) [with UnitT = au::Meters; RepT = double; au::Quantity<UnitT, RepT>::Rep = double]' is private within this context
       37 |     constexpr QuantityD<Meters> length{5.5};
          |                                           ^
    In file included from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity.hh:483:15: note: declared private here
      483 |     constexpr Quantity(Rep value) : value_{value} {}
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
    ./au/quantity.hh:595:9: error: static_assert failed due to requirement 'is_not_already_a_quantity' "Input to QuantityMaker is already a Quantity"
            static_assert(is_not_already_a_quantity, "Input to QuantityMaker is already a Quantity");
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~
    au/error_examples.cc:48:11: note: in instantiation of function template specialization 'au::QuantityMaker<au::Meters>::operator()<au::Meters, int>' requested here
        meters(x);
              ^
    In file included from au/error_examples.cc:15:
    In file included from ./au/au.hh:17:
    In file included from ./au/chrono_interop.hh:20:
    In file included from ./au/prefix.hh:19:
    ./au/quantity_point.hh:273:9: error: static_assert failed due to requirement 'is_not_already_a_quantity_point' "Input to QuantityPointMaker is already a QuantityPoint"
            static_assert(is_not_already_a_quantity_point,
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    au/error_examples.cc:51:14: note: in instantiation of function template specialization 'au::QuantityPointMaker<au::Meters>::operator()<au::Meters, int>' requested here
        meters_pt(x_pt);
                 ^
    In file included from au/error_examples.cc:15:
    In file included from ./au/au.hh:17:
    In file included from ./au/chrono_interop.hh:20:
    In file included from ./au/prefix.hh:18:
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity.hh: In instantiation of 'constexpr void au::QuantityMaker<UnitT>::operator()(au::Quantity<OtherUnit, OtherRep>) const [with U = au::Meters; R = int; UnitT = au::Meters]':
    au/error_examples.cc:48:13:   required from here
    ./au/quantity.hh:595:23: error: static assertion failed: Input to QuantityMaker is already a Quantity
      595 |         static_assert(is_not_already_a_quantity, "Input to QuantityMaker is already a Quantity");
          |                       ^~~~~~~~~~~~~~~~~~~~~~~~~
    In file included from ./au/prefix.hh:19,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity_point.hh: In instantiation of 'constexpr void au::QuantityPointMaker<UnitT>::operator()(au::QuantityPoint<U, R>) const [with U = au::Meters; R = int; Unit = au::Meters]':
    au/error_examples.cc:51:19:   required from here
    ./au/quantity_point.hh:273:23: error: static assertion failed: Input to QuantityPointMaker is already a QuantityPoint
      273 |         static_assert(is_not_already_a_quantity_point,
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

## Conversion risk too high {#risk-too-high}

**Meaning:**  This is a _physically_ meaningful conversion, but we think the risk of a grossly
incorrect answer is too high, so we forbid it.  There are two main types of [conversion risk], both
most prominently associated with integral storage types.

1. **[Truncation risk]**.  Example: `inches(24).as(feet)`.

2. **[Overflow risk]**.  Example: `giga(hertz)(1).as(hertz)`.

Both of these examples would in fact produce the correct answer with the specific values given (`24`
and `1`).  However, many (most!) other values would not:

- In `inches(x).as(feet)`, for `int x`, any value not exactly divisible by 12 would truncate.
- In `giga(hertz)(x).as(hertz)`, for `int x`, any value greater than 2 would overflow.

Thus, we disallow the entire conversion operation --- at least, by default.

**Solution:**  There are different strategies to solve this, depending on your use case.

1. **Use floating point**.  As mentioned above, these risks only apply to integer values.  If
   floating point is what you want anyway, just use it.  `giga(hertz)(1.0).as(hertz)` produces
   `hertz(1'000'000'000.0)`.

2. **Turn off the safety checks**.  `inches(24).as(feet, ignore(TRUNCATION_RISK))` produces
   `feet(2)`.

The syntax for the latter is to pass `ignore(X)` as a second argument to your conversion function,
where `X` is that conversion's "risk set" --- that is, the set of risks that caused Au to prevent
that conversion from compiling.  The "risk set" will be one of these:

- `OVERFLOW_RISK`
- `TRUNCATION_RISK`
- `OVERFLOW_RISK | TRUNCATION_RISK` (as in, "both risks")

The same compiler error that links directly to this section of the troubleshooting guide will tell
you the risk set for your conversion, so you don't have to guess.

!!! warning
    Stop and think before overriding the safety checks.  If you're reviewing code that uses it, ask
    about it.  The library is trying to protect you from an error prone operation.  The mechanism
    exists because sometimes you can know that it's OK, but remember to stop and check first!

!!! example
    **Code**

    === "Broken"
        ```cpp
        // A (BROKEN): truncation risk.
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

    === "Fixed (2. Overriding risk checks)"
        ```cpp
        // A (FIXED): 2. turn off truncation risk check.
        inches(24).as(feet, ignore(TRUNCATION_RISK));

        // B (FIXED): 2. turn off overflow risk check.
        giga(hertz)(1).as(hertz, ignore(OVERFLOW_RISK));
        ```


    **Compiler error (clang 14)**
    ```
    ./au/quantity.hh:467:9: error: static_assert failed due to requirement '!is_truncation_only_unacceptable_risk' "Truncation risk too high.  See <https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>.  Your \"risk set\" is `TRUNCATION_RISK`."
            static_assert(!is_truncation_only_unacceptable_risk,
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/quantity.hh:192:60: note: in instantiation of function template specialization 'au::Quantity<au::Inches, int>::in_impl<int, au::QuantityMaker<au::Feet>, au::detail::CheckTheseRisks<au::detail::RiskSet<'\x03'>>>' requested here
            return make_quantity<AssociatedUnitT<NewUnitSlot>>(in_impl<Rep>(u, policy));
                                                               ^
    au/error_examples.cc:59:16: note: in instantiation of function template specialization 'au::Quantity<au::Inches, int>::as<au::QuantityMaker<au::Feet>, au::detail::CheckTheseRisks<au::detail::RiskSet<'\x03'>>>' requested here
        inches(24).as(feet);
                   ^
    In file included from au/error_examples.cc:15:
    In file included from ./au/au.hh:17:
    In file included from ./au/chrono_interop.hh:20:
    In file included from ./au/prefix.hh:18:
    ./au/quantity.hh:460:9: error: static_assert failed due to requirement '!is_overflow_only_unacceptable_risk' "Overflow risk too high.  See <https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>.  Your \"risk set\" is `OVERFLOW_RISK`."
            static_assert(!is_overflow_only_unacceptable_risk,
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/quantity.hh:192:60: note: in instantiation of function template specialization 'au::Quantity<au::Giga<au::Hertz>, int>::in_impl<int, au::QuantityMaker<au::Hertz>, au::detail::CheckTheseRisks<au::detail::RiskSet<'\x03'>>>' requested here
            return make_quantity<AssociatedUnitT<NewUnitSlot>>(in_impl<Rep>(u, policy));
                                                               ^
    au/error_examples.cc:62:20: note: in instantiation of function template specialization 'au::Quantity<au::Giga<au::Hertz>, int>::as<au::QuantityMaker<au::Hertz>, au::detail::CheckTheseRisks<au::detail::RiskSet<'\x03'>>>' requested here
        giga(hertz)(1).as(hertz);
                       ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity.hh: In instantiation of 'constexpr OtherRep au::Quantity<UnitT, RepT>::in_impl(OtherUnitSlot, RiskPolicyT) const [with OtherRep = int; OtherUnitSlot = au::QuantityMaker<au::Feet>; RiskPolicyT = au::detail::CheckTheseRisks<au::detail::RiskSet<3> >; UnitT = au::Inches; RepT = int]':
    ./au/quantity.hh:192:72:   required from 'constexpr auto au::Quantity<UnitT, RepT>::as(NewUnitSlot, RiskPolicyT) const [with NewUnitSlot = au::QuantityMaker<au::Feet>; RiskPolicyT = au::detail::CheckTheseRisks<au::detail::RiskSet<3> >; UnitT = au::Inches; RepT = int]'
    au/error_examples.cc:59:23:   required from here
    ./au/quantity.hh:467:23: error: static assertion failed: Truncation risk too high.  See <https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>.  Your "risk set" is `TRUNCATION_RISK`.
      467 |         static_assert(!is_truncation_only_unacceptable_risk,
          |                       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/quantity.hh: In instantiation of 'constexpr OtherRep au::Quantity<UnitT, RepT>::in_impl(OtherUnitSlot, RiskPolicyT) const [with OtherRep = int; OtherUnitSlot = au::QuantityMaker<au::Hertz>; RiskPolicyT = au::detail::CheckTheseRisks<au::detail::RiskSet<3> >; UnitT = au::Giga<au::Hertz>; RepT = int]':
    ./au/quantity.hh:192:72:   required from 'constexpr auto au::Quantity<UnitT, RepT>::as(NewUnitSlot, RiskPolicyT) const [with NewUnitSlot = au::QuantityMaker<au::Hertz>; RiskPolicyT = au::detail::CheckTheseRisks<au::detail::RiskSet<3> >; UnitT = au::Giga<au::Hertz>; RepT = int]'
    au/error_examples.cc:62:28:   required from here
    ./au/quantity.hh:460:23: error: static assertion failed: Overflow risk too high.  See <https://aurora-opensource.github.io/au/main/troubleshooting/#risk-too-high>.  Your "risk set" is `OVERFLOW_RISK`.
      460 |         static_assert(!is_overflow_only_unacceptable_risk,
          |                       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
    In file included from ./au/au.hh:17:
    In file included from ./au/chrono_interop.hh:17:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/chrono:697:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/__chrono/calendar.h:13:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/__chrono/duration.h:14:
    In file included from external/llvm_14_toolchain_llvm/bin/../include/c++/v1/limits:105:
    external/llvm_14_toolchain_llvm/bin/../include/c++/v1/type_traits:2388:25: error: no type named 'type' in 'std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>>'
    template <class ..._Tp> using common_type_t = typename common_type<_Tp...>::type;
                            ^~~~~
    ./au/quantity.hh:722:20: note: in instantiation of template type alias 'common_type_t' requested here
        using C = std::common_type_t<T, U>;
                       ^
    ./au/quantity.hh:769:20: note: in instantiation of function template specialization 'au::detail::using_common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>, au::detail::Plus>' requested here
        return detail::using_common_type(q1, q2, detail::plus);
                       ^
    au/error_examples.cc:70:15: note: in instantiation of function template specialization 'au::operator+<au::Meters, au::Seconds, int, int>' requested here
        meters(1) + seconds(1);
                  ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from external/sysroot_x86_64//include/c++/10.3.0/ratio:39,
                     from external/sysroot_x86_64//include/c++/10.3.0/chrono:39,
                     from ./au/chrono_interop.hh:17,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    external/sysroot_x86_64//include/c++/10.3.0/type_traits: In substitution of 'template<class ... _Tp> using common_type_t = typename std::common_type::type [with _Tp = {au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int>}]':
    ./au/quantity.hh:722:11:   required from 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Meters, int>; U = au::Quantity<au::Seconds, int>; Func = au::detail::Plus]'
    ./au/quantity.hh:769:37:   required from 'constexpr auto au::operator+(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Meters; U2 = au::Seconds; R1 = int; R2 = int]'
    au/error_examples.cc:70:26:   required from here
    external/sysroot_x86_64//include/c++/10.3.0/type_traits:2562:11: error: no type named 'type' in 'struct std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int> >'
     2562 |     using common_type_t = typename common_type<_Tp...>::type;
          |           ^~~~~~~~~~~~~
    In file included from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity.hh: In instantiation of 'constexpr auto au::detail::using_common_type(T, U, Func) [with T = au::Quantity<au::Meters, int>; U = au::Quantity<au::Seconds, int>; Func = au::detail::Plus]':
    ./au/quantity.hh:769:37:   required from 'constexpr auto au::operator+(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Meters; U2 = au::Seconds; R1 = int; R2 = int]'
    au/error_examples.cc:70:26:   required from here
    ./au/quantity.hh:724:94: error: no type named 'type' in 'struct std::common_type<au::Quantity<au::Meters, int>, au::Quantity<au::Seconds, int> >'
      724 |         std::is_same<typename C::Rep, std::common_type_t<typename T::Rep, typename U::Rep>>::value,
          |                                                                                              ^~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.44.35207\include\type_traits(1300): error C2794: 'type': is not a member of any direct or indirect base class of 'std::common_type<T,U>'
            with
            [
                T=au::Quantity<au::Meters,int>,
                U=au::Quantity<au::Seconds,int>
            ]
    C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC\14.44.35207\include\type_traits(1300): note: the template instantiation context (the oldest one first) is
    error_examples.cc(72): note: see reference to function template instantiation 'auto au::operator +<au::Meters,au::Seconds,int,int>(au::Quantity<au::Meters,int>,au::Quantity<au::Seconds,int>)' being compiled
    D:\a\au\au\au.hh(7045): note: see reference to function template instantiation 'auto au::detail::using_common_type<au::Quantity<au::Meters,int>,au::Quantity<au::Seconds,int>,au::detail::Plus>(T,U,Func)' being compiled
            with
            [
                T=au::Quantity<au::Meters,int>,
                U=au::Quantity<au::Seconds,int>,
                Func=au::detail::Plus
            ]
    D:\a\au\au\au.hh(6998): note: see reference to alias template instantiation 'std::common_type_t<T,U>' being compiled
            with
            [
                T=au::Quantity<au::Meters,int>,
                U=au::Quantity<au::Seconds,int>
            ]
    D:\a\au\au\au.hh(6998): error C2938: 'std::common_type_t' : Failed to specialize alias template
    D:\a\au\au\au.hh(7000): error C2057: expected constant expression
    D:\a\au\au\au.hh(6847): error C2668: 'au::Quantity<au::Meters,int>::as': ambiguous call to overloaded function
    D:\a\au\au\au.hh(6468): note: could be 'auto au::Quantity<au::Meters,int>::as<NewRep,au::detail::CheckTheseRisks<au::detail::RiskSet<3>>>(NewUnitSlot,RiskPolicyT) const'
            with
            [
                NewRep=TargetUnit::Rep,
                NewUnitSlot=TargetUnit::Rep,
                RiskPolicyT=au::detail::CheckTheseRisks<au::detail::RiskSet<3>>
            ]
    D:\a\au\au\au.hh(6462): note: or       'auto au::Quantity<au::Meters,int>::as<NewRep,Unit,au::detail::CheckTheseRisks<au::detail::RiskSet<0>>>(NewUnitSlot,RiskPolicyT) const'
            with
            [
                NewRep=TargetUnit::Rep,
                Unit=au::Meters,
                NewUnitSlot=au::Meters,
                RiskPolicyT=au::detail::CheckTheseRisks<au::detail::RiskSet<0>>
            ]
    D:\a\au\au\au.hh(6847): note: while trying to match the argument list '(Unit)'
            with
            [
                Unit=au::Meters
            ]
    D:\a\au\au\au.hh(6847): note: the template instantiation context (the oldest one first) is
    D:\a\au\au\au.hh(7003): note: see reference to function template instantiation 'auto au::detail::cast_to_common_type<au::detail::using_common_type::C,au::Meters,int>(au::Quantity<au::Meters,int>)' being compiled
    D:\a\au\au\au.hh(6993): note: see reference to function template instantiation 'auto au::rep_cast<TargetUnit::Rep,au::Meters,int>(au::Quantity<au::Meters,int>)' being compiled
    D:\a\au\au\au.hh(6847): error C2668: 'au::Quantity<au::Seconds,int>::as': ambiguous call to overloaded function
    D:\a\au\au\au.hh(6468): note: could be 'auto au::Quantity<au::Seconds,int>::as<NewRep,au::detail::CheckTheseRisks<au::detail::RiskSet<3>>>(NewUnitSlot,RiskPolicyT) const'
            with
            [
                NewRep=TargetUnit::Rep,
                NewUnitSlot=TargetUnit::Rep,
                RiskPolicyT=au::detail::CheckTheseRisks<au::detail::RiskSet<3>>
            ]
    D:\a\au\au\au.hh(6462): note: or       'auto au::Quantity<au::Seconds,int>::as<NewRep,Unit,au::detail::CheckTheseRisks<au::detail::RiskSet<0>>>(NewUnitSlot,RiskPolicyT) const'
            with
            [
                NewRep=TargetUnit::Rep,
                Unit=au::Seconds,
                NewUnitSlot=au::Seconds,
                RiskPolicyT=au::detail::CheckTheseRisks<au::detail::RiskSet<0>>
            ]
    D:\a\au\au\au.hh(6847): note: while trying to match the argument list '(Unit)'
            with
            [
                Unit=au::Seconds
            ]
    D:\a\au\au\au.hh(6847): note: the template instantiation context (the oldest one first) is
    D:\a\au\au\au.hh(7003): note: see reference to function template instantiation 'auto au::detail::cast_to_common_type<au::detail::using_common_type::C,au::Seconds,int>(au::Quantity<au::Seconds,int>)' being compiled
    D:\a\au\au\au.hh(6993): note: see reference to function template instantiation 'auto au::rep_cast<TargetUnit::Rep,au::Seconds,int>(au::Quantity<au::Seconds,int>)' being compiled
    D:\a\au\au\au.hh(7003): error C3889: call to object of class type 'au::detail::Plus': no matching call operator found
    D:\a\au\au\au.hh(1361): note: could be 'auto au::detail::Plus::operator ()(const T &,const U &) const'
    D:\a\au\au\au.hh(7003): note: Failed to specialize function template 'auto au::detail::Plus::operator ()(const T &,const U &) const'
    D:\a\au\au\au.hh(7003): note: With the following template arguments:
    D:\a\au\au\au.hh(7003): note: 'T=void'
    D:\a\au\au\au.hh(7003): note: 'U=void'
    D:\a\au\au\au.hh(7003): note: you cannot create a reference to 'void'
    ```

## Can't pass `Quantity` to a unit slot {#quantity-to-unit-slot}

**Other variants:**

- "Can't pass `QuantityPoint` to a unit slot"
- "Can't pass `Quantity` to a unit slot for points"
- "Can't pass `QuantityPoint` to a unit slot for points"

**Meaning:**  A [unit slot](./discussion/idioms/unit-slots.md) is an API that takes _any unit-named
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
        //         unit slot ^^^^^^^^^ passing Quantity: no good.
        ```

    === "Fixed"
        ```cpp
        // (FIXED): use an ad hoc scaled unit.
        auto size = bytes(1234);
        size = round_as<int>(bytes * mag<10>(), size);
        //         unit slot ^^^^^^^^^^^^^^^^^ passing scaled unit: good!
        ```

    **Compiler error (clang 14)**
    ```
    In file included from au/error_examples.cc:15:
    In file included from ./au/au.hh:17:
    In file included from ./au/chrono_interop.hh:20:
    In file included from ./au/prefix.hh:18:
    ./au/quantity.hh:491:5: error: static_assert failed due to requirement 'detail::AlwaysFalse<au::Bytes, int>::value' "Can't pass `Quantity` to a unit slot (see: https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)"
        static_assert(
        ^
    ./au/unit_of_measure.hh:145:1: note: in instantiation of template class 'au::AssociatedUnit<au::Quantity<au::Bytes, int>>' requested here
    using AssociatedUnitT = typename AssociatedUnit<U>::type;
    ^
    ./au/math.hh:497:26: note: in instantiation of template type alias 'AssociatedUnitT' requested here
        return make_quantity<AssociatedUnitT<RoundingUnits>>(round_in<OutputRep>(rounding_units, q));
                             ^
    au/error_examples.cc:78:12: note: in instantiation of function template specialization 'au::round_as<int, au::Quantity<au::Bytes, int>, au::Bytes, int>' requested here
        size = round_as<int>(bytes(10), size);
               ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity.hh: In instantiation of 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >':
    ./au/unit_of_measure.hh:145:7:   required by substitution of 'template<class U> using AssociatedUnitT = typename au::AssociatedUnit::type [with U = au::Quantity<au::Bytes, int>]'
    ./au/math.hh:497:12:   required from 'auto au::round_as(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/error_examples.cc:78:41:   required from here
    ./au/quantity.hh:492:36: error: static assertion failed: Can't pass `Quantity` to a unit slot (see: https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)
      492 |         detail::AlwaysFalse<U, R>::value,
          |                                    ^~~~~
    In file included from ./au/conversion_policy.hh:26,
                     from ./au/quantity.hh:19,
                     from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/unit_of_measure.hh: In substitution of 'template<class U> using AssociatedUnitT = typename au::AssociatedUnit::type [with U = au::Quantity<au::Bytes, int>]':
    ./au/math.hh:497:12:   required from 'auto au::round_as(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/error_examples.cc:78:41:   required from here
    ./au/unit_of_measure.hh:145:7: error: no type named 'type' in 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >'
      145 | using AssociatedUnitT = typename AssociatedUnit<U>::type;
          |       ^~~~~~~~~~~~~~~
    In file included from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity.hh: In instantiation of 'constexpr OtherRep au::Quantity<UnitT, RepT>::in_impl(OtherUnitSlot, RiskPolicyT) const [with OtherRep = double; OtherUnitSlot = au::Quantity<au::Bytes, int>; RiskPolicyT = au::detail::CheckTheseRisks<au::detail::RiskSet<0> >; UnitT = au::Bytes; RepT = int]':
    ./au/quantity.hh:200:31:   required from 'constexpr auto au::Quantity<UnitT, RepT>::in(NewUnitSlot, RiskPolicyT) const [with NewRep = double; NewUnitSlot = au::Quantity<au::Bytes, int>; RiskPolicyT = au::detail::CheckTheseRisks<au::detail::RiskSet<0> >; UnitT = au::Bytes; RepT = int]'
    ./au/math.hh:445:52:   required from 'auto au::round_in(RoundingUnits, au::Quantity<Unit, Rep>) [with RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    ./au/math.hh:463:43:   required from 'auto au::round_in(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    ./au/math.hh:497:77:   required from 'auto au::round_as(RoundingUnits, au::Quantity<U2, R2>) [with OutputRep = int; RoundingUnits = au::Quantity<au::Bytes, int>; U = au::Bytes; R = int]'
    au/error_examples.cc:78:41:   required from here
    ./au/quantity.hh:446:42: error: no type named 'type' in 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >'
      446 |         static_assert(IsUnit<OtherUnit>::value, "Invalid type passed to unit slot");
          |                                          ^~~~~
    ./au/quantity.hh:448:15: error: no type named 'type' in 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >'
      448 |         using Op = detail::ConversionForRepsAndFactor<Rep, OtherRep, UnitRatioT<Unit, OtherUnit>>;
          |               ^~
    ./au/quantity.hh:452:85: error: no type named 'type' in 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >'
      452 |         constexpr bool is_overflow_risk_ok = detail::OverflowRiskAcceptablyLow<Op>::value;
          |                                                                                     ^~~~~
    ./au/quantity.hh:456:89: error: no type named 'type' in 'struct au::AssociatedUnit<au::Quantity<au::Bytes, int> >'
      456 |         constexpr bool is_truncation_risk_ok = detail::TruncationRiskAcceptablyLow<Op>::value;
          |                                                                                         ^~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(6768): error C2338: static_assert failed: 'Can't pass `Quantity` to a unit slot (see: https://aurora-opensource.github.io/au/main/troubleshooting/#quantity-to-unit-slot)'
    D:\a\au\au\au.hh(6768): note: the template instantiation context (the oldest one first) is
    error_examples.cc(80): note: see reference to function template instantiation 'auto au::round_as<int,au::Quantity<au::Bytes,int>,au::Bytes,int>(RoundingUnits,au::Quantity<au::Bytes,int>)' being compiled
            with
            [
                RoundingUnits=au::Quantity<au::Bytes,int>
            ]
    D:\a\au\au\au.hh(3638): note: see reference to class template instantiation 'au::AssociatedUnit<RoundingUnits>' being compiled
            with
            [
                RoundingUnits=au::Quantity<au::Bytes,int>
            ]
    D:\a\au\au\au.hh(3638): error C2794: 'type': is not a member of any direct or indirect base class of 'au::AssociatedUnit<RoundingUnits>'
            with
            [
                RoundingUnits=au::Quantity<au::Bytes,int>
            ]
    D:\a\au\au\au.hh(7394): error C2938: 'au::AssociatedUnitT' : Failed to specialize alias template
    D:\a\au\au\au.hh(7342): error C2672: 'au::Quantity<au::Bytes,int>::in': no matching overloaded function found
    D:\a\au\au\au.hh(5020): note: could be 'int au::Quantity<au::Bytes,int>::in(NewUnit) const'
    D:\a\au\au\au.hh(7342): note: 'int au::Quantity<au::Bytes,int>::in(NewUnit) const': could not deduce template argument for '<unnamed-symbol>'
    D:\a\au\au\au.hh(5019): note: 'std::enable_if_t<false,void>' : Failed to specialize alias template
    D:\a\au\au\au.hh(5010): note: or       'NewRep au::Quantity<au::Bytes,int>::in(NewUnit) const'
    D:\a\au\au\au.hh(7342): note: 'NewRep au::Quantity<au::Bytes,int>::in(NewUnit) const': could not deduce template argument for '<unnamed-symbol>'
    D:\a\au\au\au.hh(5009): note: 'au::AssociatedUnitT' : Failed to specialize alias template
    D:\a\au\au\au.hh(3638): note: 'type': is not a member of any direct or indirect base class of 'au::AssociatedUnit<RoundingUnits>'
            with
            [
                RoundingUnits=au::Quantity<au::Bytes,int>
            ]
    D:\a\au\au\au.hh(5009): note: syntax error: missing '>' before identifier '<missingId>'
    D:\a\au\au\au.hh(7342): note: the template instantiation context (the oldest one first) is
    D:\a\au\au\au.hh(7394): note: see reference to function template instantiation 'auto au::round_in<OutputRep,RoundingUnits,au::Bytes,int>(RoundingUnits,au::Quantity<au::Bytes,int>)' being compiled
            with
            [
                OutputRep=int,
                RoundingUnits=au::Quantity<au::Bytes,int>
            ]
    D:\a\au\au\au.hh(7360): note: see reference to function template instantiation 'auto au::round_in<RoundingUnits,au::Bytes,int>(RoundingUnits,au::Quantity<au::Bytes,int>)' being compiled
            with
            [
                RoundingUnits=au::Quantity<au::Bytes,int>
            ]
    D:\a\au\au\au.hh(7360): error C2440: 'static_cast': cannot convert from 'void' to 'OutputRep'
            with
            [
                OutputRep=int
            ]
    D:\a\au\au\au.hh(7360): note: Expressions of type void cannot be converted to other types
    D:\a\au\au\au.hh(7394): error C2672: 'au::make_quantity': no matching overloaded function found
    D:\a\au\au\au.hh(4872): note: could be 'auto au::make_quantity(T)'
    D:\a\au\au\au.hh(7394): note: Failed to specialize function template 'auto au::make_quantity(T)'
    D:\a\au\au\au.hh(7394): note: With the following template arguments:
    D:\a\au\au\au.hh(7394): note: 'UnitT=unknown-type'
    D:\a\au\au\au.hh(7394): note: 'T=void'
    D:\a\au\au\au.hh(7394): note: 'void' cannot be used as a function parameter except for '(void)'
    error_examples.cc(79): error C2679: binary '=': no operator found which takes a right-hand operand of type 'void' (or there is no acceptable conversion)
    D:\a\au\au\au.hh(5261): note: could be 'au::Quantity<au::Bytes,int> &au::Quantity<au::Bytes,int>::operator =(au::Quantity<au::Bytes,int> &&)'
    error_examples.cc(79): note: 'au::Quantity<au::Bytes,int> &au::Quantity<au::Bytes,int>::operator =(au::Quantity<au::Bytes,int> &&)': cannot convert argument 2 from 'void' to 'au::Quantity<au::Bytes,int> &&'
    error_examples.cc(79): note: Expressions of type void cannot be converted to other types
    D:\a\au\au\au.hh(5261): note: or       'au::Quantity<au::Bytes,int> &au::Quantity<au::Bytes,int>::operator =(const au::Quantity<au::Bytes,int> &)'
    error_examples.cc(79): note: 'au::Quantity<au::Bytes,int> &au::Quantity<au::Bytes,int>::operator =(const au::Quantity<au::Bytes,int> &)': cannot convert argument 2 from 'void' to 'const au::Quantity<au::Bytes,int> &'
    error_examples.cc(79): note: Expressions of type void cannot be converted to other types
    error_examples.cc(79): note: while trying to match the argument list '(au::Quantity<au::Bytes,int>, void)'
    ```

## Integer division forbidden {#integer-division-forbidden}

**Meaning:**  Although Au generally tries to act just like the underlying raw numeric types, we also
try to prevent wrong code that _looks_ correct from compiling.  Over time, we have found that
certain instances of integer division --- namely, cases where the _denominator_ has units, and those
units are _different_ from the numerator --- are extremely pernicious.  The code looks correct at
first glance _even to most experts_, but the risk for truncating (_even down to zero!_) can be much
higher than usual.

**Solution:**  Floating point types do not have this problem, so you can change either of the
variables to floating point.

Alternatively, if the dimensions are the same, you may want to perform the division in their common
unit, using the `divide_using_common_unit()` utility.

If you cannot use floating point, and the dimensions are different, then wrapping the denominator in
`unblock_int_div()` will overrule the compiler error.  However, please be very careful about this
approach: read the warning below first.

!!! warning "Carefully consider your situation before using `unblock_int_div()`"
    Au already accepts many common integer division use cases without complaint, including:

    <ul>
        <li class="check">Dividing by a raw numeric integer, not a <code>Quantity</code></li>
        <li class="check">
            Dividing by a <code>Quantity</code> whose units are the <em>same</em> as the numerator
        </li>
    </ul>

    Thus, if you see this error, **it is probably preventing a serious mistake**.

    Let's revisit the example from the [`unblock_int_div`
    docs](./reference/quantity.md#unblock-int-div), and consider the effect of different units in
    the denominator.  (We will assign to a quantity of miles per hour for illustration purposes.)

    ```cpp
    // `v1`: Original example above.
    Quantity<UnitQuotientT<Miles, Hours>> v1
        = miles(115) / unblock_int_div(hours(2));

    // `v2`: Changing the denominator's units.
    Quantity<UnitQuotientT<Miles, Hours>> v2
        = miles(115) / unblock_int_div(minutes(120));
    ```

    `v1` and `v2` represent the same quantity, in principle.  However, `v1` has a value of
    `(miles / hour)(57)`, while `v2` has a value of `(miles / hour)(0)`!

    We hope this sobering example helps to communicate the risk, and encourages you to think
    carefully before using `unblock_int_div`.  If you still find that it's appropriate for your use
    case, we suggest _including a brief comment to explain why_ for the benefit of future readers.

!!! example

    **Code**

    How long does it take to travel 60 m at a speed of 65 MPH?

    And how many 40-minute periods are in 8 hours?

    === "Broken"
        ```cpp
        // (BROKEN): gives (60 / 65) == 0 before conversion!
        QuantityD<Seconds> t = meters(60) / (miles / hour)(65);

        // (BROKEN): gives (8 / 40) == 0 before conversion!
        const auto n = hours(8) / minutes(40);
        ```

    === "Fixed (1. Floating point)"
        ```cpp
        // (FIXED): 1. Using floating point, we get ~= seconds(2.06486)
        QuantityD<Seconds> t = meters(60.0) / (miles / hour)(65.0);

        // (FIXED): 1. Using floating point, we get ~= (hours / minute)(0.2)
        //
        // To get more familiar units, we can call `.as(unos)`, obtaining `unos(12)`.
        // This corresponds to the expected result of 12.
        const auto n = hours(8.0) / minutes(40.0);
        ```

    === "Fixed (2. Divide in common units)"
        ```cpp
        // (FIX NOT APPLICABLE): 2. Divide-in-common-units needs same-dimension inputs
        // QuantityD<Seconds> t = divide_using_common_unit(meters(60), (miles / hour)(65));
        // (Will produce a compiler error.)

        // (FIXED): 2. Divide-in-common-units produces 480 min / 40 min == 12
        const auto n = divide_using_common_unit(hours(8), minutes(40));
        ```



    === "\"Fixed\" (3. `unblock_int_div()`)"
        ```cpp
        // ("FIXED"): 3. Integer result == `(meter * hours / mile)(0)`:
        auto               t = meters(60) / unblock_int_div((miles / hour)(65));

        // ("FIXED"): 3. Integer result == `(hours / minute)(0)`:
        const auto         n = hours(8) / unblock_int_div(minutes(40));

        // Now it compiles, but note that the answer is very inaccurate: it has truncated to 0!
        ```


    **Compiler error (clang 14)**
    ```
    In file included from au/error_examples.cc:15:
    In file included from ./au/au.hh:17:
    In file included from ./au/chrono_interop.hh:20:
    In file included from ./au/prefix.hh:18:
    ./au/quantity.hh:436:9: error: static_assert failed due to requirement 'are_units_quantity_equivalent || !uses_integer_division' "Dangerous integer division forbidden.  See <https://aurora-opensource.github.io/au/main/troubleshooting/#integer-division-forbidden> for more details about the risks, and your options to resolve this error."
            static_assert(are_units_quantity_equivalent || !uses_integer_division,
            ^             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/quantity.hh:309:9: note: in instantiation of function template specialization 'au::Quantity<au::Meters, int>::warn_if_integer_division<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1>>, int>' requested here
            warn_if_integer_division<OtherUnit, OtherRep>();
            ^
    au/error_examples.cc:86:39: note: in instantiation of function template specialization 'au::Quantity<au::Meters, int>::operator/<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1>>, int>' requested here
        QuantityD<Seconds> t = meters(60) / (miles / hour)(65);
                                          ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/quantity.hh: In instantiation of 'static constexpr void au::Quantity<UnitT, RepT>::warn_if_integer_division() [with OtherUnit = au::UnitProduct<au::Miles, au::Pow<au::Hours, -1> >; OtherRep = int; UnitT = au::Meters; RepT = int]':
    ./au/quantity.hh:309:54:   required from here
    au/error_examples.cc:86:58:   in 'constexpr' expansion of 'au::meters.au::QuantityMaker<au::Meters>::operator()<int>(60).au::Quantity<au::Meters, int>::operator/<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1> >, int>(au::miles.au::QuantityMaker<au::Miles>::operator/<au::Hours>((au::hour, const au::SingularNameFor<au::Hours>())).au::QuantityMaker<au::UnitProduct<au::Miles, au::Pow<au::Hours, -1> > >::operator()<int>(65))'
    ./au/quantity.hh:436:53: error: static assertion failed: Dangerous integer division forbidden.  See <https://aurora-opensource.github.io/au/main/troubleshooting/#integer-division-forbidden> for more details about the risks, and your options to resolve this error.
      436 |         static_assert(are_units_quantity_equivalent || !uses_integer_division,
          |                       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(6713): error C2338: static_assert failed: 'Integer division forbidden: wrap denominator in `unblock_int_div()` if you really want it'
    D:\a\au\au\au.hh(6713): note: the template instantiation context (the oldest one first) is
    error_examples.cc(88): note: see reference to function template instantiation 'au::Quantity<au::UnitProduct<T,au::Pow<B,-1>,au::Hours>,int> au::Quantity<au::Meters,int>::operator /<au::UnitProduct<au::Miles,au::Pow<au::Hours,-1>>,int>(au::Quantity<au::UnitProduct<au::Miles,au::Pow<au::Hours,-1>>,int>) const' being compiled
            with
            [
                T=au::Meters,
                B=au::Miles
            ]
    D:\a\au\au\au.hh(6586): note: see reference to function template instantiation 'void au::Quantity<au::Meters,int>::warn_if_integer_division<OtherUnit,OtherRep>(void)' being compiled
            with
            [
                OtherUnit=au::UnitProduct<au::Miles,au::Pow<au::Hours,-1>>,
                OtherRep=int
            ]
    ```

## Dangerous inversion

**Meaning:**  This is analogous to our [overflow safety surface].  When computing the inverse of
an integral quantity in a given target unit, there is some smallest value that will be "lossy": that
is, where converting _back_ to the original unit will _not be guaranteed_ to produce the original
value.  Au's policy is that all values up to _at least_ 1,000 must be free from this kind of loss:
otherwise, we forbid this operation.

**Solution:**  Consider using floating point; you'll always get a precise answer.  Alternatively,
use a smaller target unit: one whose product with the input unit is smaller than
$\left(\frac{1}{1,000}\right)^2 = \frac{1}{1,000,000}$.

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
        // (FIXED): 2. Integer result == micro(seconds)(200'000)
        inverse_as(micro(seconds), hertz(5));
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
    In file included from ./au/au.hh:19:
    ./au/math.hh:272:5: error: static_assert failed due to requirement 'UNITY.in<int>(associated_unit(au::QuantityMaker<au::Seconds>{}) * au::Hertz{}) >= threshold || std::is_floating_point<int>::value' "Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired"
        static_assert(
        ^
    ./au/math.hh:288:56: note: in instantiation of function template specialization 'au::inverse_in<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        return make_quantity<AssociatedUnitT<TargetUnits>>(inverse_in(target_units, q));
                                                           ^
    au/error_examples.cc:94:5: note: in instantiation of function template specialization 'au::inverse_as<au::QuantityMaker<au::Seconds>, au::Hertz, int>' requested here
        inverse_as(seconds, hertz(5));
        ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from ./au/au.hh:19,
                     from au/error_examples.cc:15:
    ./au/math.hh: In instantiation of 'constexpr auto au::inverse_in(TargetUnits, au::Quantity<Unit, Rep>) [with TargetUnits = au::QuantityMaker<au::Seconds>; U = au::Hertz; R = int]':
    ./au/math.hh:288:66:   required from 'constexpr auto au::inverse_as(TargetUnits, au::Quantity<Unit, Rep>) [with TargetUnits = au::QuantityMaker<au::Seconds>; U = au::Hertz; R = int]'
    au/error_examples.cc:94:33:   required from here
    ./au/math.hh:273:72: error: static assertion failed: Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired
      273 |         UNITY.in<R>(associated_unit(TargetUnits{}) * U{}) >= threshold ||
          |         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~
      274 |             std::is_floating_point<R>::value,
          |             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(8856): error C2338: static_assert failed: 'Dangerous inversion risking truncation to 0; must supply explicit Rep if truly desired'
    D:\a\au\au\au.hh(8856): note: the template instantiation context (the oldest one first) is
    error_examples.cc(96): note: see reference to function template instantiation 'auto au::inverse_as<au::QuantityMaker<au::Seconds>,au::Hertz,int>(TargetUnits,au::Quantity<au::Hertz,int>)' being compiled
            with
            [
                TargetUnits=au::QuantityMaker<au::Seconds>
            ]
    D:\a\au\au\au.hh(8871): note: see reference to function template instantiation 'auto au::inverse_in<TargetUnits,au::Hertz,int>(TargetUnits,au::Quantity<au::Hertz,int>)' being compiled
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
    au/error_examples.cc:102:34: error: deduced conflicting types ('Quantity<au::QuantityMaker<au::Hertz>::Unit, [...]>' vs 'Quantity<au::QuantityMaker<au::Pow<au::Seconds, -1>>::Unit, [...]>') for initializer list element type
        for (const auto &frequency : {
                                     ^
    ```

    **Compiler error (gcc 10)**
    ```
    au/error_examples.cc: In function 'void au::example_deduced_conflicting_types()':
    au/error_examples.cc:105:10: error: unable to deduce 'std::initializer_list<auto>&&' from '{au::hertz.au::QuantityMaker<au::Hertz>::operator()<double>(1.0e+0), au::operator/<int>(1, au::seconds.au::QuantityMaker<au::Seconds>::operator()<double>(2.0e+0))}'
      105 |          }) {
          |          ^
    au/error_examples.cc:105:10: note:   deduced conflicting types for parameter 'auto' ('au::Quantity<au::Hertz, double>' and 'au::Quantity<au::Pow<au::Seconds, -1>, double>')
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    error_examples.cc(104): error C3535: cannot deduce type for 'auto &&' from 'initializer list'
    error_examples.cc(104): error C2440: 'initializing': cannot convert from 'initializer list' to 'std::initializer_list<<error type>> &&'
    error_examples.cc(104): note: Reason: cannot convert from 'initializer list' to 'std::initializer_list<<error type>>'
    error_examples.cc(104): note: Element '1': no conversion from 'au::Quantity<au::Hertz,double>' to '<error type>'
    error_examples.cc(104): note: Element '2': no conversion from 'au::Quantity<au::Pow<B,-1>,T>' to '<error type>'
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
you can _force_ a particular ordering.  Choose one of the units `U`, and give it a "tiebreaker" by
creating a specialization of `::au::UnitOrderTiebreaker<U>`.  As the name suggests, this will break
the tie, and your program will compile.

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

        namespace au {
        template <>
        struct UnitOrderTiebreaker<::Trinches> : std::integral_constant<int, 100> {};
        }

        // (FIXED): Trinches is no longer "tied" with Quarterfeet
        if (quarterfeet(10) == trinches(10)) {
            // ...
        }
        ```


    **Compiler error (clang 14)**
    ```
    In file included from au/error_examples.cc:15:
    In file included from ./au/au.hh:17:
    In file included from ./au/chrono_interop.hh:20:
    In file included from ./au/prefix.hh:18:
    In file included from ./au/quantity.hh:19:
    In file included from ./au/conversion_policy.hh:19:
    In file included from ./au/conversion_strategy.hh:17:
    In file included from ./au/abstract_operations.hh:17:
    In file included from ./au/magnitude.hh:21:
    ./au/packs.hh:302:5: error: static_assert failed due to requirement 'std::is_same<au::Quarterfeet, au::Trinches>::value' "Broken strict total ordering: distinct input types compare equal"
        static_assert(std::is_same<A, B>::value,
        ^             ~~~~~~~~~~~~~~~~~~~~~~~~~
    ./au/packs.hh:318:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches>' requested here
        std::conditional_t<
        ^
    ./au/packs.hh:318:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByUnitOrderTiebreaker>' requested here
    ./au/packs.hh:318:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderAsOriginDisplacementUnit, au::detail::OrderByUnitOrderTiebreaker>' requested here
    ./au/packs.hh:318:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderAsUnitProduct, au::detail::OrderAsOriginDisplacementUnit, au::detail::OrderByUnitOrderTiebreaker>' requested here
    ./au/packs.hh:318:5: note: in instantiation of template class 'au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct, au::detail::OrderAsOriginDisplacementUnit, au::detail::OrderByUnitOrderTiebreaker>' requested here
    ./au/packs.hh:318:5: note: (skipping 8 contexts in backtrace; use -ftemplate-backtrace-limit=0 to see all)
    ./au/unit_of_measure.hh:759:28: note: in instantiation of template class 'au::ComputeCommonUnitImpl<au::Quarterfeet, au::Trinches>' requested here
              typename detail::IncludeInPackIf<IsNonzero, ComputeCommonUnitImpl, Us...>::type>::type>> {
                               ^
    ./au/unit_of_measure.hh:167:1: note: in instantiation of template class 'au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>' requested here
    using CommonUnitT = typename ComputeCommonUnit<Us...>::type;
    ^
    ./au/quantity.hh:732:15: note: in instantiation of template type alias 'CommonUnitT' requested here
        using U = CommonUnitT<U1, U2>;
                  ^
    ./au/quantity.hh:743:20: note: in instantiation of function template specialization 'au::detail::convert_and_compare<au::detail::Equal, au::Quarterfeet, au::Trinches, int, int>' requested here
        return detail::convert_and_compare<detail::Equal>(q1, q2);
                       ^
    au/error_examples.cc:121:25: note: in instantiation of function template specialization 'au::operator==<au::Quarterfeet, au::Trinches, int, int>' requested here
        if (quarterfeet(10) == trinches(10)) {
                            ^
    In file included from au/error_examples.cc:15:
    In file included from ./au/au.hh:17:
    In file included from ./au/chrono_interop.hh:20:
    In file included from ./au/prefix.hh:18:
    In file included from ./au/quantity.hh:19:
    In file included from ./au/conversion_policy.hh:19:
    In file included from ./au/conversion_strategy.hh:17:
    In file included from ./au/abstract_operations.hh:17:
    ./au/magnitude.hh:163:1: error: implicit instantiation of undefined template 'au::SignImpl<void>'
    using Sign = typename SignImpl<MagT>::type;
    ^
    ./au/unit_of_measure.hh:140:1: note: in instantiation of template type alias 'Sign' requested here
    using UnitSign = Sign<detail::MagT<U>>;
    ^
    ./au/quantity.hh:735:40: note: in instantiation of template type alias 'UnitSign' requested here
        return detail::SignAwareComparison<UnitSign<U>, Op>{}(q1.template in<ComRep1>(U{}),
                                           ^
    ./au/quantity.hh:743:20: note: in instantiation of function template specialization 'au::detail::convert_and_compare<au::detail::Equal, au::Quarterfeet, au::Trinches, int, int>' requested here
        return detail::convert_and_compare<detail::Equal>(q1, q2);
                       ^
    au/error_examples.cc:121:25: note: in instantiation of function template specialization 'au::operator==<au::Quarterfeet, au::Trinches, int, int>' requested here
        if (quarterfeet(10) == trinches(10)) {
                            ^
    ./au/magnitude.hh:161:8: note: template is declared here
    struct SignImpl;
           ^
    ```

    **Compiler error (gcc 10)**
    ```
    In file included from ./au/magnitude.hh:21,
                     from ./au/abstract_operations.hh:17,
                     from ./au/conversion_strategy.hh:17,
                     from ./au/conversion_policy.hh:19,
                     from ./au/quantity.hh:19,
                     from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/packs.hh: In instantiation of 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches>':
    ./au/packs.hh:313:8:   recursively required from 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByScaleFactor, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct, au::detail::OrderAsOriginDisplacementUnit, au::detail::OrderByUnitOrderTiebreaker>'
    ./au/packs.hh:313:8:   required from 'struct au::LexicographicTotalOrdering<au::Quarterfeet, au::Trinches, au::detail::OrderByCoarseUnitOrdering, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByScaleFactor, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct, au::detail::OrderAsOriginDisplacementUnit, au::detail::OrderByUnitOrderTiebreaker>'
    ./au/unit_of_measure.hh:1184:8:   required from 'struct au::InOrderFor<au::UnitProduct, au::Quarterfeet, au::Trinches>'
    ./au/unit_of_measure.hh:595:8:   required from 'struct au::InOrderFor<au::CommonUnit, au::Quarterfeet, au::Trinches>'
    ./au/packs.hh:436:8:   required from 'struct au::FlatDedupedTypeList<au::CommonUnit, au::CommonUnit<au::Quarterfeet>, au::CommonUnit<au::Trinches> >'
    ./au/unit_of_measure.hh:745:8:   required from 'struct au::ComputeCommonUnitImpl<au::Quarterfeet, au::Trinches>'
    ./au/unit_of_measure.hh:756:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    ./au/unit_of_measure.hh:167:7:   required by substitution of 'template<class ... Us> using CommonUnitT = typename au::ComputeCommonUnit::type [with Us = {au::Quarterfeet, au::Trinches}]'
    ./au/quantity.hh:732:11:   required from 'constexpr auto au::detail::convert_and_compare(au::Quantity<U2, R2>, au::Quantity<U2, R2>) [with Op = au::detail::Equal; U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    ./au/quantity.hh:743:54:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:121:39:   required from here
    ./au/packs.hh:302:39: error: static assertion failed: Broken strict total ordering: distinct input types compare equal
      302 |     static_assert(std::is_same<A, B>::value,
          |                                       ^~~~~
    ./au/packs.hh: In instantiation of 'struct au::LexicographicTotalOrdering<au::Trinches, au::Quarterfeet>':
    ./au/packs.hh:313:8:   recursively required from 'struct au::LexicographicTotalOrdering<au::Trinches, au::Quarterfeet, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByScaleFactor, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct, au::detail::OrderAsOriginDisplacementUnit, au::detail::OrderByUnitOrderTiebreaker>'
    ./au/packs.hh:313:8:   required from 'struct au::LexicographicTotalOrdering<au::Trinches, au::Quarterfeet, au::detail::OrderByCoarseUnitOrdering, au::detail::OrderByDim, au::detail::OrderByMag, au::detail::OrderByScaleFactor, au::detail::OrderByOrigin, au::detail::OrderAsUnitProduct, au::detail::OrderAsOriginDisplacementUnit, au::detail::OrderByUnitOrderTiebreaker>'
    ./au/unit_of_measure.hh:1184:8:   required from 'struct au::InOrderFor<au::UnitProduct, au::Trinches, au::Quarterfeet>'
    ./au/unit_of_measure.hh:595:8:   required from 'struct au::InOrderFor<au::CommonUnit, au::Trinches, au::Quarterfeet>'
    ./au/unit_of_measure.hh:649:8:   required from 'struct au::detail::IsFirstUnitRedundant<au::CommonUnit, au::Quarterfeet, au::Trinches>'
    ./au/unit_of_measure.hh:660:8:   required from 'struct au::detail::EliminateRedundantUnitsImpl<au::CommonUnit<au::Trinches, au::Quarterfeet> >'
    ./au/unit_of_measure.hh:745:8:   required from 'struct au::ComputeCommonUnitImpl<au::Quarterfeet, au::Trinches>'
    ./au/unit_of_measure.hh:756:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    ./au/unit_of_measure.hh:167:7:   required by substitution of 'template<class ... Us> using CommonUnitT = typename au::ComputeCommonUnit::type [with Us = {au::Quarterfeet, au::Trinches}]'
    ./au/quantity.hh:732:11:   required from 'constexpr auto au::detail::convert_and_compare(au::Quantity<U2, R2>, au::Quantity<U2, R2>) [with Op = au::detail::Equal; U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    ./au/quantity.hh:743:54:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:121:39:   required from here
    ./au/packs.hh:302:39: error: static assertion failed: Broken strict total ordering: distinct input types compare equal
    In file included from ./au/conversion_policy.hh:26,
                     from ./au/quantity.hh:19,
                     from ./au/prefix.hh:18,
                     from ./au/chrono_interop.hh:20,
                     from ./au/au.hh:17,
                     from au/error_examples.cc:15:
    ./au/unit_of_measure.hh: In instantiation of 'struct au::CommonUnit<au::Trinches, au::Quarterfeet>':
    ./au/packs.hh:220:8:   recursively required by substitution of 'template<class Default, template<class ...> class Op, class ... Args> struct au::stdx::experimental::detail::detector<Default, au::stdx::void_t<Op<Args ...> >, Op, Args ...> [with Default = void; Op = au::detail::DimMemberT; Args = {au::CommonUnit<au::Trinches, au::Quarterfeet>}]'
    ./au/packs.hh:220:8:   required from 'struct au::detail::DimImpl<au::CommonUnit<au::Trinches, au::Quarterfeet> >'
    ./au/unit_of_measure.hh:545:8:   required from 'struct au::HasSameDimension<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>'
    ./au/stdx/type_traits.hh:38:59:   required from 'struct au::stdx::conjunction<au::HasSameDimension<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>, au::detail::HasSameMagnitude<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches> >'
    ./au/unit_of_measure.hh:560:8:   required from 'struct au::AreUnitsQuantityEquivalent<au::CommonUnit<au::Trinches, au::Quarterfeet>, au::Trinches>'
    ./au/unit_of_measure.hh:627:8:   required from 'struct au::detail::FirstMatchingUnit<au::AreUnitsQuantityEquivalent, au::CommonUnit<au::Trinches, au::Quarterfeet>, au::CommonUnit<au::Trinches, au::Quarterfeet> >'
    ./au/unit_of_measure.hh:756:8:   required from 'struct au::ComputeCommonUnit<au::Quarterfeet, au::Trinches>'
    ./au/unit_of_measure.hh:167:7:   required by substitution of 'template<class ... Us> using CommonUnitT = typename au::ComputeCommonUnit::type [with Us = {au::Quarterfeet, au::Trinches}]'
    ./au/quantity.hh:732:11:   required from 'constexpr auto au::detail::convert_and_compare(au::Quantity<U2, R2>, au::Quantity<U2, R2>) [with Op = au::detail::Equal; U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    ./au/quantity.hh:743:54:   required from 'constexpr bool au::operator==(au::Quantity<U1, R1>, au::Quantity<U2, R2>) [with U1 = au::Quarterfeet; U2 = au::Trinches; R1 = int; R2 = int]'
    au/error_examples.cc:121:39:   required from here
    ./au/unit_of_measure.hh:585:70: error: static assertion failed: Elements must be listed in ascending order
      585 |     static_assert(AreElementsInOrder<CommonUnit, CommonUnit<Us...>>::value,
          |                                                                      ^~~~~
    ```

    **Compiler error (MSVC 2022 x64)**
    ```
    D:\a\au\au\au.hh(2109): error C2338: static_assert failed: 'Broken strict total ordering: distinct input types compare equal'
    D:\a\au\au\au.hh(2109): note: the template instantiation context (the oldest one first) is
    error_examples.cc(123): note: see reference to function template instantiation 'bool au::operator ==<au::Quarterfeet,au::Trinches,int,int>(au::Quantity<au::Quarterfeet,int>,au::Quantity<au::Trinches,int>)' being compiled
    D:\a\au\au\au.hh(7019): note: see reference to function template instantiation 'auto au::detail::convert_and_compare<au::detail::Equal,au::Quarterfeet,au::Trinches,int,int>(au::Quantity<au::Quarterfeet,int>,au::Quantity<au::Trinches,int>)' being compiled
    D:\a\au\au\au.hh(7008): note: see reference to alias template instantiation 'au::CommonUnitT<U1,U2>' being compiled
            with
            [
                U1=au::Quarterfeet,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(4800): note: see reference to class template instantiation 'au::ComputeCommonUnit<U1,U2>' being compiled
            with
            [
                U1=au::Quarterfeet,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(5392): note: see reference to class template instantiation 'au::ComputeCommonUnitImpl<U1,U2>' being compiled
            with
            [
                U1=au::Quarterfeet,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(5380): note: see reference to alias template instantiation 'au::FlatDedupedTypeListT<au::CommonUnit,au::Quarterfeet,au::Trinches>' being compiled
    D:\a\au\au\au.hh(1930): note: see reference to class template instantiation 'au::FlatDedupedTypeList<au::CommonUnit,au::CommonUnit<T>,au::CommonUnit<au::Trinches>>' being compiled
            with
            [
                T=au::Quarterfeet
            ]
    D:\a\au\au\au.hh(2251): note: see reference to class template instantiation 'au::InOrderFor<List,T,H>' being compiled
            with
            [
                List=au::CommonUnit,
                T=au::Quarterfeet,
                H=au::Trinches
            ]
    D:\a\au\au\au.hh(5228): note: see reference to class template instantiation 'au::InOrderFor<au::UnitProduct,A,B>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(5818): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByCoarseUnitOrdering,au::detail::OrderByDim,au::detail::OrderByMag,au::detail::OrderByScaleFactor,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct,au::detail::OrderAsOriginDisplacementUnit,au::detail::OrderByUnitOrderTiebreaker>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(2125): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByDim,au::detail::OrderByMag,au::detail::OrderByScaleFactor,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct,au::detail::OrderAsOriginDisplacementUnit,au::detail::OrderByUnitOrderTiebreaker>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(2125): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByMag,au::detail::OrderByScaleFactor,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct,au::detail::OrderAsOriginDisplacementUnit,au::detail::OrderByUnitOrderTiebreaker>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(2125): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByScaleFactor,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct,au::detail::OrderAsOriginDisplacementUnit,au::detail::OrderByUnitOrderTiebreaker>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(2125): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByOrigin,au::detail::OrderAsUnitProduct,au::detail::OrderAsOriginDisplacementUnit,au::detail::OrderByUnitOrderTiebreaker>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(2125): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderAsUnitProduct,au::detail::OrderAsOriginDisplacementUnit,au::detail::OrderByUnitOrderTiebreaker>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(2125): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderAsOriginDisplacementUnit,au::detail::OrderByUnitOrderTiebreaker>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(2125): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B,au::detail::OrderByUnitOrderTiebreaker>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(2125): note: see reference to class template instantiation 'au::LexicographicTotalOrdering<A,B>' being compiled
            with
            [
                A=au::Quarterfeet,
                B=au::Trinches
            ]
    D:\a\au\au\au.hh(5218): error C2338: static_assert failed: 'Elements must be listed in ascending order'
    D:\a\au\au\au.hh(5218): note: the template instantiation context (the oldest one first) is
    D:\a\au\au\au.hh(5392): note: see reference to class template instantiation 'au::detail::FirstMatchingUnit<au::AreUnitsQuantityEquivalent,T,TargetUnit>' being compiled
            with
            [
                T=au::CommonUnit<au::Trinches,au::Quarterfeet>,
                TargetUnit=au::CommonUnit<au::Trinches,au::Quarterfeet>
            ]
    D:\a\au\au\au.hh(5261): note: see reference to class template instantiation 'au::AreUnitsQuantityEquivalent<TargetUnit,H>' being compiled
            with
            [
                TargetUnit=au::CommonUnit<au::Trinches,au::Quarterfeet>,
                H=au::Trinches
            ]
    D:\a\au\au\au.hh(5194): note: see reference to class template instantiation 'au::stdx::conjunction<au::HasSameDimension<U1,U2>,au::detail::HasSameMagnitude<U1,U2>>' being compiled
            with
            [
                U1=au::CommonUnit<au::Trinches,au::Quarterfeet>,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(252): note: see reference to class template instantiation 'au::HasSameDimension<U1,U2>' being compiled
            with
            [
                U1=au::CommonUnit<au::Trinches,au::Quarterfeet>,
                U2=au::Trinches
            ]
    D:\a\au\au\au.hh(5179): note: see reference to alias template instantiation 'au::detail::DimT<U1>' being compiled
            with
            [
                U1=au::CommonUnit<au::Trinches,au::Quarterfeet>
            ]
    D:\a\au\au\au.hh(2029): note: see reference to class template instantiation 'au::detail::DimImpl<U1>' being compiled
            with
            [
                U1=au::CommonUnit<au::Trinches,au::Quarterfeet>
            ]
    D:\a\au\au\au.hh(2027): note: see reference to alias template instantiation 'au::detail::DimMemberT<U>' being compiled
            with
            [
                U=au::CommonUnit<au::Trinches,au::Quarterfeet>
            ]
    D:\a\au\au\au.hh(2025): note: see reference to class template instantiation 'au::CommonUnit<T,au::Quarterfeet>' being compiled
            with
            [
                T=au::Trinches
            ]
    ```



[conversion risk]: ./discussion/concepts/conversion_risks.md
[Truncation risk]: ./discussion/concepts/truncation.md
[Overflow risk]: ./discussion/concepts/overflow.md
[overflow safety surface]: ./discussion/concepts/overflow.md#adapt
