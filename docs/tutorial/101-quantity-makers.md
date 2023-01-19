# Au 101: Quantity Makers

This tutorial gives a gentle introduction to the Au library.

- **Time:** TBD.
- **Prerequisites:** Experience writing C++ code.
- **You will learn:**
    - The concept and importance of "unit safety".
    - How to store a numeric value in a _quantity_.
    - How to retrieve the stored numeric value.
    - Some basic operations you can perform with a quantity.

## Status quo: no units library

Suppose you have a variable that represents a _physical quantity_.  That variable has some _value_,
but that value is meaningless unless you also know the _unit of measurement_. We usually indicate
the unit with a suffix on the variable name.  Here's a concrete example:

```cpp
const double track_length_m = 100.0;
//          Unit suffix--^^   ^^^^^--Value

const double best_time_s = 10.34;
//       Unit suffix--^^   ^^^^^--Value

```

The first value is `100.0`.  Since there's no such thing as a "length of 100", we add a `_m` suffix
on the end of our variable name to make it clear that the value is the length _in meters_.  We take
a similar approach for our time _in seconds_.

This strategy _works_, in the sense that it can prevent unit errors, but it's labor intensive and
error prone.  The naming suffixes provide hints, but enforcement is basically on the honor system.
Consider a function we might want to call:

```cpp
double average_speed_mps(double length_m, double time_s);
```

With the above variables, our callsite might look like this:

```cpp
const auto speed_mps = average_speed_mps(track_length_m, best_time_s);
```

It's time to consider a very important property:

!!! info "Definition"
    **Unit correctness:** a program is _unit-correct_ when every variable associated with physical
    units is used consistently with those units.

So: is this unit-correct?  Yes:

- `track_length_m` gets passed as the parameter `length_m`: meters to meters :heavy_check_mark:
- `best_time_s` gets passed as the parameter `time_s`: seconds to seconds :heavy_check_mark:

However, it's quite fragile.  We could just as easily have written the following.

```cpp
const auto speed_mps = average_speed_mps(best_time_s, track_length_m);
```

By itself, this line looks correct: we're asking for an average speed, given a time and a length.
We can even see that we're passing in values in seconds and meters to get a result in
meters-per-second, increasing our confidence!

Of course, the line is wrong, but the only way to _know_ that it's wrong is to go read the
declaration of `average_speed_mps`.  This could easily be in some other file.  In a big project, it
might be hard to even figure out _which_ file it's in.

That's a lot of cognitive load!

## Our goal: unit safety

To write code quickly _and_ robustly, unit-correctness is **not enough**.  We need more: we need
_unit safety_.

!!! info "Definition"
    **Unit safety**: We call a program _unit-safe_ when the _unit-correctness_ of _each line_ of
    code can be checked by _inspection_, in _isolation_.

_This_ is the way to reduce cognitive load for code readers, when it comes to physical units.  If
you inspect a unit-safe line, and see that it's correct, then you're done with that line.  You can
move on; you don't have to hold it in your head.

!!! tip
    A unit-safe line doesn't guarantee that the _program_ has no unit errors.  It _does_ guarantee
    that _if there are_ unit errors, then they're in some _other_ line (which you can also
    inspect!).

Unit-safety is not something you could ever get from the standard numeric types, but you _can_ get
it from the Au library.  Let's learn how!

## Storing values: the "quantity maker"

The way to achieve unit-safety is by turning our raw numeric values into _quantities_.  We do this
with quantity _makers_.  These are callables---things that act like functions---which have the _name
of some unit_, and accept _any numeric type_.

For example, let's make our variable `track_length_m` _unit-safe_ by using the quantity maker,
`meters`:

```cpp
const auto track_length = meters(track_length_m);
//                        ^^^^^^             ^^
//     Quantity maker of *meters*      Takes value in *meters*
```

This is an example of a _unit-safe handoff_.  We take a raw number whose _name_ tells us it was in
meters, and we pass it to the _quantity maker_ for that **same unit**.  We can see this line is
unit-correct simply by inspection---our first example of a unit-safe line.

In fact, we have already achieved unit safety everywhere we use the quantity `track_length` instead
of the raw number `track_length_m`!  Think of the quantity as a _container_, which holds its value
securely together with information about its unit.  We'll see that the quantity prevents us from
using that value in ways that are incompatible with its unit.

## Retrieving values: you must name the unit

Ideally, every interface that takes physical quantities would use unit-safe quantity _types_.  In
practice, you can't upgrade your entire codebase at once.  Even if you could, there will always be
third-party libraries which don't know about these quantity types.  One way or another, it's
important to be able to get the value out.

Let's imagine we have this example third-party API, which needs a raw `double`.  How can we call it
if we have a quantity?

```cpp
// Example third-party API.

class Racetrack;

class RacetrackBuilder {
 public:

    // Main function we'll call:
    void set_length_m(double length_m);

    Racetrack build_track();
};
```

Most units libraries provide a function that retrieves a quantity's value "in whatever units it
happens to be stored".  (Think of
[`std::chrono::duration::count()`](https://en.cppreference.com/w/cpp/chrono/duration/count) as a
very common example.)  These kinds of functions may be convenient, but they're _not_ unit-safe.[^1]

[^1]: To take the example from `std::chrono::duration`, note that the system clock [has different
resolutions on different widely used toolchains](https://stackoverflow.com/a/51539549/15777264). gcc
uses $1\,\text{ns}$, MSVC uses $100\,\text{ns}$, and clang uses $1000\,\text{ns}$.  So if you
subtracted two calls to `std::chrono::system_clock::now()` and called `.count()`, your answers would
vary by 3 orders of magnitude on different compilers!  This is not to say that doing so would be a
good use of the chrono library.  It's not, and that's the point: a bare call to `.count()` gives the
reader no idea how to interpret its result.

Au takes a different approach.  To retrieve the value from a quantity `q`, you call `q.in(units)`,
where `units` is the quantity maker you used to _store_ the value.  Continuing with our earlier
example, we could call that API like so:

```cpp
RacetrackBuilder builder;
builder.set_length_m(track_length.in(meters));
//                ^^             ^^^^^^^^^^^
// API wants length in *meters*      Get value in *meters*
```

Here, we have another _unit-safe handoff_.  Our first one showed how we _enter_ the library by
naming the unit.  This one shows how we _exit_ the library by naming that _same_ unit.

!!! tip
    Think of the quantity maker's name as a kind of "password" which you set when you create the
    quantity.  The quantity will hold its underlying value securely.  To retrieve that value, you
    must speak the same "password" (that is, name the same unit).

Of course, this API is a best-case scenario for raw numeric APIs, since it names the units at the
callsite (via the `_m` suffix on `set_length_m()`).  Our other API, `average_speed_mps()`, can't do
this, because we can't see the parameter names at the callsite.  In fact, although we'll see some
coping strategies in later lessons, _there is no unit-safe way to call `average_speed_mps()`
directly_.

## Basic quantity operations

Quantity types do much more than simply hold their values securely: they support a variety of
operations.  In fact, we strive to support _every meaningful_ operation, because operation
implementations _for quantity types_ can faithfully maintain unit safety.

!!! tip
    Treat any instance of retrieving the value as "code smell".  Stop and check whether there's some
    way to perform the operation within the quantity type.  If there's not, stop and consider
    whether there _should_ be.

    By "code smell", we don't mean that it's _definitely_ wrong; in fact, it's often necessary.  We
    just mean it's worth checking to see if there's a better pattern.

The first and most basic operations which we'll cover here are _arithmetic_ operations.

- You can _add, subtract, and compare_ quantities of the **same** units.[^2]
- You can _multiply and divide_ quantities of **any** units.

[^2]: What about adding, subtracting, and comparing quantities of _different_ units, but the _same_
dimensions---like comparing `seconds(100)` to `minutes(1)`, or adding `inches(1)` to `feet(6)`?  In
most cases, we _do_ support this as well, but it's a more advanced usage which we'll discuss further
in future lessons.

!!! example "Example: same-unit operations"
    Here are a couple examples of operations among quantities with the _same unit_.

    ```cpp
    constexpr auto distance = meters(1.0) + meters(2.0);
    // distance -> meters(3.0)

    constexpr auto is_duration_greater = (seconds(60) > seconds(55));
    // is_duration_greater -> true
    ```

    Admittedly, these examples are _very_ basic for now.  Future lessons will explore more
    interesting examples---like, what happens when you compare a length in inches, to a length in
    centimeters?  But for now, the takeaway is simply that we neither need nor want to extract
    underlying values to perform basic operations.

### Multiplying and dividing quantities

The product of two quantities is another quantity.

Recall that a quantity variable has two parts: the _unit_, and the _value_. These parts compose
nicely with multiplication.

- The unit of the product is the product of the units.
- The value of the product is the product of the values.

All of these same considerations apply to division.

So for example: `(meters / second)` is a quantity maker.  You can call it and pass any numerical
type, just as with the quantity makers `meters` or `seconds`.  In particular,

```cpp
meters(50.0) / seconds(10.0) == (meters / second)(5.0);
```

!!! tip
    To form a compound quantity maker, use the grammatically correct name of the unit.  Examples:

    - `meters / second`, _not_ <s>`meters / seconds`</s>
    - `newton * meters`, _not_ <s>`newtons * meters`</s>

    Empirically, we have found that this pattern works: `(s * ...) * p / (s * ...)`.  That is:

    - pluralize _only one_ token
    - for singular tokens: put those which _multiply_ on the _left_, and those which _divide_ on the
      _right_.

## Exercise: computing with quantities

To get some practice with quantities, we've included an exercise where you can make and print some
quantities, and then upgrade an existing function implementation from raw numbers to quantities.

Check out the [Au 101: API Types Exercise](./exercise/101-quantity-makers.md)!

## Takeaways

1. We strive for **unit safety**.  If we can check the unit-correctness of every individual line of
   code, by inspection, in isolation, we can reduce cognitive load, and write code faster and more
   robustly.

2. To **store** a raw numeric value safely inside of a quantity object, **call the quantity maker**
   whose name is the unit of interest.

    - For example, `meters(3)` is quantity representing $3\,\text{m}$, stored as `int`.

3. To **retrieve** a stored numeric value from a quantity `q`, **call `q.in(units)`**, where `units` was the
   quantity maker used in the first place.

    - For example, `meters(3).in(meters)` is simply `3`.

4. **Quantity makers compose**: you can multiply, divide, and raise them to powers to get a new quantity
   maker.

    - For example, `(meters / second)` is a quantity maker which you can call like any other.<br>
      `(meters / second)(5)` represents the quantity $5\,\text{m/s}$.

!!! tip
    Au **only** contains unit-safe interfaces.  That's why simply storing the value in a quantity is
    enough to achieve unit-safety!
