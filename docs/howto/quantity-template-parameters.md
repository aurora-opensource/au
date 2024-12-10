# Quantity template parameters

Sometimes, you may want to use a `Quantity` _value_ as a template parameter.  Generally, this isn't
possible prior to C++20: the set of types whose values can be template parameters is severely
limited.  However, if your `Quantity` has an _integral_ rep, we can provide
a [workaround](../reference/quantity.md#nttp).  This page explains how to use it.

??? info "How does it work, under the hood?"
    Before C++20, non-type template parameters had to be, very roughly, _integral_ types, _pointer_
    types (including references), or _enumerations_.  With integral and pointer types, there's no
    way to provide unit safety.  However, with enumerations, we _were_ able to find a way.

    The way it works is that every `Quantity<U, R>` type defines its own custom enumeration type,
    which is associated only with that `Quantity` type: we call it `Quantity<U, R>::NTTP`. The type
    system preserves the association between the two, so we don't get mixed up with the `NTTP` type
    for any other `Quantity`. This lets us safely convert back and forth between the `Quantity` type
    and its `NTTP` type, using the `to_nttp(Quantity)` and `from_nttp(Quantity::NTTP)` utilities.
    What's more, it's legal C++ to `static_cast` between an enumeration and its underlying type,
    even for values that aren't part of the enumeration --- therefore, we're able to hold any value
    that the `Quantity` type can.

    So, even though we can't _exactly_ use a `Quantity` _value_ as a template parameter, what we
    _can_ do is use a special type that has _low-friction conversion_ to and from that `Quantity`,
    while still providing the unit safety that we need.

## How to use

Let's say that you have a specific specialization of `Quantity<U, R>`, with some concrete unit `U`
and rep `R`.  (Remember that `std::is_integral<R>::value` **must** be true in order to use this
feature.)  Here's how to use this type as a template parameter:

1. Use `Quantity<U, R>::NTTP` as the _type_ of the template parameter.

2. When instantiating the template, pass any instance of `Quantity<U, R>` after calling
   `to_nttp(...)` on it.

    - Note that the value (call it `q`) must be **exactly** an instance of `Quantity<U, R>`, and not
      any other `Quantity` type.  If it's not, you'll get a compiler error.  You can fix this by
      converting to `Quantity<U, R>` via the usual library mechanisms --- namely, something like
      `q.as<R>(U{})`.

3. To use the value of the template parameter _as a `Quantity`_ in your code, you have two options.

    - You can assign it to a `Quantity<U, R>` variable.  If you do this, note again that the type
      must match **exactly**.  If it does not, your best bet will be the following alternative.

    - You can pass it to the `from_nttp()` function.  This converts it to a `Quantity<U, R>`
      variable, which means that all of the usual library conversion mechanisms will automatically
      work, with no further effort.

## Worked example

Suppose we have a `Processor` class that has some specific clock frequency.  We may want to use
a `QuantityU64<Hertz>` to represent this frequency, and we may want to _template the class_ on this
frequency as well.  When we specialize this template, we'd like to provide the value in megahertz,
which could be more convenient for our use case.  Finally, we want to make it easy for end users to
get the frequency as a variable, but as a twist, we'll provide it in gigahertz, with a `double` rep.

Here's how we'd define that template.

```cpp
// Step (1): use the NTTP type as the template parameter.
template <QuantityU64<Hertz>::NTTP ClockFreq>
class Processor {
 public:
    static constexpr QuantityD<Giga<Hertz>> clock_freq() {

        // Step (3): use `from_nttp()` to make it a `Quantity`.
        //
        // Note how we don't have to specify the conversion explicitly!
        // Now that it's a `Quantity`, the usual mechanisms suffice.
        return from_nttp(ClockFreq);
    }
};
```

And here's how we'd use it.

```cpp
// Step (2): when instantiating, convert to the exact right `Quantity` type, using `to_nttp`.
using Proc = Processor<to_nttp(mega(hertz)(1'200).as<uint64_t>(hertz))>;

std::cout << Proc::clock_freq() << std::endl;
```

This program prints out `"1.2 GHz"`.
