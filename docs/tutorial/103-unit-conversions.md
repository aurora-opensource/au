# Au 103: Unit Conversions

This tutorial explains how to perform unit conversions, both implicitly and explicitly.

- **Time:** 30 minutes.
- **Prerequisites:**
    - [Au 101: Quantity Makers](./101-quantity-makers.md).
    - [Au 102: API types](./102-api-types.md)
- **You will learn:**
    - How `.in(...)`, the "value retrieving" function, actually performs unit conversions.
    - `.as(...)`: the safer cousin of `.in(...)`, which gives a quantity instead of a raw number.
    - How to "force" a physically meaningful conversion which Au thinks is dangerous.
    - Which conversions work automatically, which work only when forced, and which are prevented.
    - Which _implicit_ conversions are allowed.

## New units for `.in(...)`

Once we store a value in a `Quantity`, we know we need to name its unit explicitly to get the value
back out --- that's [Au 101](./101-quantity-makers.md).

```cpp
constexpr auto q = feet(6);
q.in(feet);  // <-- produces `6`
```

That API is awfully suggestive, though.  What happens if we pass some _other_ unit?

Answer: it does just what it looks like it does.

```cpp
constexpr auto q = feet(6);
q.in(inches); // <-- produces `72`
```

We introduced `.in(...)` as, essentially, "the function that gets the value back out".  That was
true, but incomplete.  Now we see its true role: it's the _quantity value conversion function_.  The
role we saw earlier is just a special case: when we pass the same unit we used to create it, that
"conversion" is simply the identity.

Already, this opens up a new simple, self-contained use case for Au: it's very easy to conjure up
highly readable unit conversions on the fly, even if you both start and end with raw numbers.
Consider this example:

```cpp
// Starting with a raw numeric type:
double angle_deg = 90.0;

{ // ⚠️ Old, manual method for doing conversions:
    constexpr auto RAD_PER_DEG = M_PI / 180.0;
    double angle_rad = angle_deg * RAD_PER_DEG;
}

{ // ✔️ Easy, readable ad hoc conversion with Au:
    double angle_rad = degrees(angle_deg).in(radians);
}
```

With the old method, we needed to manually craft a carefully named conversion constant.  And because
this was an _angular_ conversion, we also needed to worry about how to get a good value for $\pi$
(here, we chose the `M_PI` macro).  By contrast, the Au-based alternative gives you a readable,
clearly correct one-liner out of the box --- and it doesn't trouble the author or reader with the
details of correctly obtaining (and using) $\pi$.

## `.as(...)`: like `.in(...)`, but makes a quantity

Using `.in(...)` works very well when you want a raw number --- typically, when you're interacting
with some legacy interface.  However, sometimes what you want is a _quantity_ that's expressed in a
specific unit.  For example, you might be comparing quantities in a hot loop, and you'd rather avoid
repeated conversions.  Or, you might want to print your quantity in some specific unit.

We _could_ satisfy these use cases with `.in(...)`, but it's a little clunky:

```cpp
// Not recommended!  See below.
auto angle = radians(degrees(angle_deg).in(radians));
//           ^^^^^^^        Raw number--^^ ^^^^^^^
//                 |                       |
//                 \--Repeated identifier--/
```

This approach wouldn't just be repetitive; it would also create a (small!) opportunity for error,
because you temporarily leave the safety of the units library before re-entering it.

Fortunately, there's a better choice.  `Quantity` has another member function, `.as(...)`, for
exactly this use case.  You can pass it any unit you would pass to `.in(...)`, but `.as(...)`
_returns a quantity_, not a raw number.  Building on our earlier example:

```cpp
auto angle = degrees(angle_deg).as(radians);
//      👍 Don't Repeat Yourself---^^^^^^^
```

Use `.as(...)` when you want easy, inline, fine-grained control over the units in which your
quantities are expressed.

## Conversion categories

The examples so far have been pretty straightforward.  To convert from `feet` to `inches`, we simply
multiply the underlying value by 12.  That seems pretty safe for just about any Rep[^1], whether
floating point or integral.  However, other conversions can be more subtle.

[^1]: Recall that the "Rep" is shorthand for the underlying storage type of the quantity.

Let's look at a bunch of example unit conversions.  We'll show how each conversion works with both
`int` and `double` Rep, because the rules can differ significantly for integral and floating point
types.

!!! note "Instructions"
    _For each example:_ stop and think about what you would expect the library to produce in each
    case.  When you're ready, click over to the "Results and Discussion" tab to check your
    intuition.

!!! example "Example: `feet` to `yards`"

    === "Conversion"
        ```cpp
        feet(6).as(yards);

        feet(6.0).as(yards);
        ```

    === "Results and Discussion"
        ```cpp
        feet(6).as(yards);  // Compiler error!

        feet(6.0).as(yards);  // yards(2.0)
        ```

        Converting from `feet` to `yards` means dividing the underlying value by 3.

        For an **integral** Rep, this actually yields a **compiler error**, because we can't
        guarantee that the result will be an integer.  True, with `feet(6)`, it so happens that it
        would --- but if we had `feet(5)`, this wouldn't be the case!

        **Floating point** Rep is simpler.  When we divide any value by `3`, we won't exceed typical
        floating point error.  Because this level of uncertainty simply goes with the territory when
        using floating point types, Au **allows** this operation with no complaint.

!!! example "Example: `feet` to `nano(meters)`"

    === "Conversion"
        ```cpp
        feet(6).as(nano(meters));

        feet(6.0).as(nano(meters));
        ```

    === "Results and Discussion"
        ```cpp
        feet(6).as(nano(meters));  // Compiler error!

        feet(6.0).as(nano(meters));  // nano(meters)(1'828'800'000.0)
        ```

        Converting from `feet` to `nano(meters)` means multiplying the underlying value by
        304,800,000.

        Unlike the last example, this is guaranteed to produce an integer result.  Yet, the
        **integral** Rep _again_ gives us a **compiler error**!  This time, we're guarding against
        a different risk: **overflow**.  It turns out that _any underlying value larger than
        `feet(7)`_ would overflow in this conversion.  That's pretty scary, so we forbid this
        conversion.

        Of course, that's just because `int` is typically only 32 bits.  Au _adapts_ to the specific
        level of overflow risk, based on both the conversion and the range of the type.  For
        example, _this_ integral-type conversion _would_ work:

        ```cpp
        feet(6LL).as(nano(meters));  // nano(meters)(1'828'800'000LL)
        ```

        Since `long long` is at least 64 bits, we could handle values into the tens of billions of
        feet before overflowing!

        ??? info "In more detail: the \"Overflow Safety Surface\""
            Here is how to reason about which integral-Rep conversions the library supports.

            For every conversion operation, there is _some smallest value which would overflow_.
            This depends on both the size of the conversion factor, and the range of values which
            the type can hold.  If that smallest value is small enough to be "scary", we forbid the
            conversion.

            How small is "scary"?  Here are some considerations.

              - Once our values get over 1,000, we can consider switching to a larger SI-prefixed
                version of the unit.  (For example, lengths over $1000\,\text{m}$ can be
                approximated in $\text{km}$.)  This means that if a value as small as 1,000 would
                overflow --- so small that we haven't even _reached_ the next unit --- we should
                _definitely_ forbid the conversion.

              - On the other hand, we've found it useful to initialize, say, `QuantityI32<Hertz>`
                variables with something like `mega(hertz)(500)`.  Thus, we'd like this operation
                to succeed (although it should probably be near the border of what's allowed).

            Putting it all together, we settled on [a value threshold of 2'147][threshold].  If we
            can convert this value without overflow, then we permit the operation; otherwise, we
            don't.  We picked this value because it satisfies our above criteria nicely.  It will
            prevent operations that can't handle values of 1,000, but it still lets us use
            $\text{MHz}$ freely when storing $\text{Hz}$ quantities in `int32_t`.

            We can picture this relationship in terms of the _biggest allowable conversion factor_,
            as a function of the _max value of the type_.  This function separates the allowed
            conversions from the forbidden ones, permitting bigger conversions for bigger types.
            We call this abstract boundary the **"overflow safety surface"**, and it's the secret
            ingredient that lets us use a wide variety of integral types with confidence.

        As for the **floating point** value, this is again very safe, so we **allow** it without
        complaint.


!!! example "Example: `feet` to `kelvins`"

    === "Conversion"
        ```cpp
        feet(6).as(kelvins);

        feet(6.0).as(kelvins);
        ```

    === "Results and Discussion"
        ```cpp
        feet(6).as(kelvins);  // Compiler error!

        feet(6.0).as(kelvins);  // Compiler error!
        ```

        Converting from `feet` to `kelvins` is an intrinsically meaningless operation, because they
        have _different dimensions_ (namely, _length_ and _temperature_).  For both **integral** and
        **floating point** Rep, we **forbid** this operation.

## Forcing lossy conversions: `.coerce_as(...)` and `.coerce_in(...)`

Sometimes, you may want to perform a conversion even though you know it's usually lossy.  For
example, maybe you know that _your particular_ value will give an exact result (like converting
6 feet into yards).  Or perhaps the truncation is desired.

Whatever the reason, you can simply add the word "coerce" before your conversion function to make it
"forcing".  Consider this example.

```cpp
feet(6.0).as(yards);       // yards(2.0)

// Compiler error!
// feet(6).as(yards);

feet(6).coerce_as(yards);  // yards(2)
```

These "coercing" versions work similarly to `static_cast`, in that they will truncate if necessary.
For example:

```cpp
feet(5).coerce_as(yards);  // yards(1) --- a truncation of (5/3 = 1.6666...) yards
```

You can use this to "overrule" Au when we prevent a _physically meaningful_ conversion because we
think it's too risky.

!!! tip
    Prefer **not** to use the coercing versions unless you have a good reason.  If you do, consider
    adding a comment to explain why your specific use case is OK.

    As a code reviewer, if you see a coercing version that doesn't seem necessary or justified, ask
    about it!

At this point, we've seen several examples of conversions which Au forbids.  We've also seen how
some of them can be forced anyway.  Here's a chance to test your understanding: what will happen if
you try to force that _final_ example --- the one where the _dimensions_ differ?

!!! example "Example: forcing different-dimension conversions?"

    As before, stop and think about what you would expect the library to produce.  When you're
    ready, click over to the "Results and Discussion" tab to check your intuition.

    === "Conversion"
        ```cpp
        feet(6).coerce_as(kelvins);
        ```

    === "Results and Discussion"
        ```cpp
        feet(6).coerce_as(kelvins);  // Compiler error!
        ```

        Converting units with different dimensions isn't merely "unsafe"; it's completely
        meaningless.  We can't "force" the answer because there isn't even an answer to force.

## Conversion summary

This table gives a visual summary of how different kinds of risks impact conversions with different
storage types.

<table>
  <tr>
    <th>Conversion</th>
    <th>Result (<code>int</code> Rep):<br><code>length = feet(6)</code></th>
    <th>Result (<code>double</code> Rep):<br><code>length = feet(6.0)</code></th>
  </tr>
  <tr>
    <td><code>length.as(inches)</code></td>
    <td class="good"><code>inches(72)</code></td>
    <td class="good"><code>inches(72.0)</code></td>
  </tr>
  <tr>
    <td><code>length.as(yards)</code></td>
    <td class="fair">
        <b>Forbidden</b>: not guaranteed to be integral<br>(can be forced with coercing version)
    </td>
    <td class="good"><code>yards(2.0)</code></td>
  </tr>
  <tr>
    <td><code>length.as(nano(meters))</code></td>
    <td class="fair">
        <b>Forbidden</b>: excessive overflow risk<br>(can be forced with coercing version)
    </td>
    <td class="good"><code>nano(meters)(1'828'800'000.0)</code></td>
  </tr>
  <tr>
    <td><code>length.as(kelvins)</code></td>
    <td class="poor"><b>Forbidden</b>: meaningless</td>
    <td class="poor"><b>Forbidden</b>: meaningless</td>
  </tr>
</table>

## Implicit conversions

Au emphasizes developer experience.  We strive to provide the same ergonomics which developers have
come to expect from the venerable `std::chrono` library.  This means that any meaningful conversion
which we consider "safe enough" (based on the above criteria), we permit _implicitly_.  This lets
you fluently initialize a quantity parameter with _any convertible expression_.  For example:

```cpp
// API accepting a quantity parameter.
void rotate(QuantityD<Radians> angle);

// This works!
// We'll automatically convert the integral quantity of degrees to `QuantityD<Radians>`.
rotate(degrees(45));
```

Our conversion policy is a _refinement_ of [the policy for `std::chrono::duration`][duration].  Here
is their policy (paraphrased and simplified):

- Implicit conversions are permitted if _either_:
    - The destination is floating point;
    - Or, the _source_ type is integer, and the conversion _multiplies_ by an integer.

And here is our refinement (the _overflow safety surface_):

- If an integral-Rep conversion would overflow the _destination type_ for a _source value_ [as small
  as `2'147`][threshold], we _forbid_ the conversion.

??? info "Deeper dive: comparing overflow strategies for Au and `chrono`"
    The `std::chrono` library doesn't consider overflow in its conversion policy, because they
    handle the problem in a different way.  Instead of encouraging users to use `duration` directly,
    they provide pre-defined helper types such as `std::chrono::nanoseconds`,
    `std::chrono::milliseconds`, etc.  The Rep for each of these types is chosen to guarantee
    covering at least 292 years in either direction.

    This is a good and practical solution, which is effective at preventing overflow for users who
    stick to these helper types.  The downside is that it forces users to change their underlying
    storage types --- changing the assembly code produced --- in the process of acquiring unit
    safety.

    A key design goal of Au is to avoid forcing users to change their underyling numeric types.  We
    want to empower people to get the same assembly they would have had without Au, just more
    safely. Because smaller numeric types bring this extra overflow risk (and in a way that's often
    non-obvious to developers), we designed this adaptive policy which prevents the biggest risks.

(Lastly, of course we also forbid conversions between units of different dimensions.  This
consideration wasn't part of the `std::chrono` library, because that library only has a single
dimension.)

## Exercise: practicing conversions

Check out the [Au 103: Unit Conversions Exercise](./exercise/103-unit-conversions.md)!

## Takeaways

1. **To convert** a quantity to a particular unit, **pass that unit's quantity maker** to the
   appropriate member function, `.in(...)` or `.as(...)`.

    - For example, `minutes(3)` is a quantity, and `minutes(3).as(seconds)` produces `seconds(180)`.

2. **`.in(...)`** gives a **raw number**, while **`.as(...)`** gives a **quantity**.

    - These names are used consistently in this way throughout the library.  For example, we'll
      learn in the next tutorial that `round_in(...)` produces a _raw number_, while `round_as(...)`
      produces a _quantity_.

    - **Prefer `.as(...)`** when you have a choice, because it stays within the safety of the
      library.

3. Both conversion functions **include safety checks**.  This means you can generally _just use
   them_, and rely on the library to guard against the biggest risks.  Here are the details to
   remember about these safety checks.

    - We **forbid** conversions with **mismatched dimensions** (as with any units library).

    - We **forbid** conversions for **integer destinations** _unless_ we're sure we'll **always have an
      integer** (as [with the `chrono`
      library](https://en.cppreference.com/w/cpp/chrono/duration/duration)), _and_ we **don't have
      excessive overflow risk** (an Au-original feature!).

4. You can **force** a conversion that is **meaningful, but considered dangerous** by preceding your
   function name with **`coerce_`**.

    - For example: `seconds(200).as(minutes)` won't compile, but `seconds(200).coerce_as(minutes)`
      gives `minutes(3)`, because we forced this lossy conversion by using the word "coerce".

    - **Use this sparingly,** since the safety checks are there for a reason, and consider **adding
      a comment** to explain why your usage is safe.

    - You can never force a conversion to a different dimension, because this is not meaningful.

5. Any conversion **allowed by `.as(...)`** will also work as an **implicit conversion**.

    - For example, if you have an API which takes `QuantityD<Radians>`, you can pass `degrees(45)`
      directly to it.

[threshold]: https://github.com/aurora-opensource/au/blob/dbd79b2/au/conversion_policy.hh#L27-L28
[duration]: https://en.cppreference.com/w/cpp/chrono/duration/duration
