# Abbreviated Quantity Construction

Ever since C++11 brought [user-defined literals] (UDLs) to the language, most C++ units libraries
have provided them out of the box for a wide variety of units.  It's a very popular feature ---
after all, who _wouldn't_ rather write `65_mph` than `miles_per_hour(65)`?  Checking off the
"user-defined literals" box makes a library seem more complete, more appealing.

We'd like to reframe that box in terms of the need it's actually meeting.  Instead of "user-defined
literals", think of it as "abbreviated quantity construction".  Users want concise, readable ways to
make quantity objects.  UDLs are one way to do that, but not the only way.  In fact, for quantities
in particular, UDLs have serious flaws that competing approaches don't suffer from.

This article will explain how UDLs generally work in a quantity context, and the downsides that
turned Au's _reluctance_ into outright _rejection_.  We'll explain an alternative approach, _unit
symbols_, that solves virtually all of UDLs' problems.  Finally, we'll share the surprising
discovery that put UDLs back on the table: by giving them _non-quantity_ return types, they turned
out to handle the _quantity_ use cases better than any other library on the market!  We'll close
with guidance on which tool to reach for, when.

## UDLs for quantities

The basic definition of a UDL is simple.  Although the _spelling_ of the function name is somewhat
arcane (for example, the `_m` literal is spelled `operator""_m`), it is, ultimately, just a
function.  As the [docs][user-defined literals] explain, the set of permissible parameter types is
heavily restricted. For quantity literals, the most relevant options are `long double` for floating
point literals, and `unsigned long long` for integer literals.

Let's define one now.  We'll write what we'd get if Au were to follow the common style in other
libraries, using `meters` as an example:

```cpp
constexpr auto operator""_m(long double v) {
    return meters(static_cast<double>(v));
}
```

With this, we can write `3.5_m`, and we'll get a `QuantityD<Meters>` object with a value of `3.5`.
This is a huge benefit for migrating callsites, especially when functions have many parameters.
Having many literals in a callsite is common in unit test files.  You can migrate those tests by
just adding a little unit suffix on each number.  This kind of change will be a lot easier to review
at a glance, and much less likely to overflow the line width than spelling out all the unit names in
full.

### Small nuisances

Already, we might notice some aspects that feel a little off.  For one thing, we've got a
`static_cast` to `double` for the return type.  That's because raw floating point literals are
`double` by default (`decltype(3.5)` is `double`), and we want _our_ literals to match the _original_
literals they're replacing.  So, we cast.  (Note that this is fine: although the _parameter_ type is
restricted, the _return_ type can be whatever you want.)

In fact, it gets worse, because something like `3_m` doesn't even compile!  The `3` cannot match the
`long double` parameter type; instead, it looks for an `unsigned long long` overload.  So if we want
this to cover all numeric literals, we need _two overloads for every unit_, with the second looking
something like this:

```cpp
// NOLINTNEXTLINE(runtime/int)
constexpr auto operator""_m(unsigned long long v) {
    // What is `T`?  Let's discuss...
    return meters(static_cast<T>(v));
}
```

The `NOLINTNEXTLINE` is just something you may need to add[^1] depending on your linter
configuration.  It's not aesthetic, but it's livable.

[^1]: We needed this comment in our Aurora-internal implementations, for instance.

The real question here is what type we should return.  On the one hand, `decltype(3)` is `int`,
which argues for an `int` return type.  On the other hand, `3_m` and `3._m` look very similar, and
some libraries want to avoid the `.` changing the type in a meaningful way --- _especially_ when
integer-backed quantities are typically far less fleshed out than floating point ones.[^2]  Thus,
`double` is also a common choice for `T` in this context (for example, the [nholthaus units] library
makes this choice).

[^2]: Note that this is _not_ true for Au.  Our embedded teams had a seat at the table from the very
    beginning, and robust handling for integer-backed quantities is a core strength of Au.

### The case against quantity UDLs

These oddities turn out to be just the tip of the iceberg.  Mateusz Pusz (lead author of the
[mp-units] library) was the first person we know of to make a _systematic case_ against using UDLs
for quantities.  Some of the key [problems] he identifies include:

- **UDLs compose poorly.**  If you have `_m` and `_s`, you can't make one for `_mps` (meters per
  second) without writing an entirely new overload.  Actually, make that _two_ new overloads --- one
  for floating point, and one for integral.  That means...

- **UDLs are expensive.**  We need two copies for every unit, so the _maintenance_ cost is high.
  Additionally, within Aurora, we found that the _compile time cost_ of having many UDLs adds up to
  a significant amount.

- **UDLs don't let you pick the rep[^3].**  The real code smell with the `static_cast<T>` is that _the
  UDL author_ is forced to choose `T`.  That choice should belong to the user!  With UDLs, if we
  support `double`, we cannot support `float` or `long double` at the same time.

[^3]: The "rep" is a common shorthand term in units libraries for "representation type".  It's the
    underlying numeric type that stores the value of a particular quantity.

- **UDLs only work for literals.**  If you have a raw number in a _variable_, you're out of luck:
  your only option is to spell out the full unit name.

As a historical point, Aurora's internal units library (the precursor to Au) always had UDLs, the
same as every other units library at the time.  When it came time to open source Au, something about
the feature didn't feel right.  Although we couldn't quite put our finger on the problem, we
deliberately omitted UDLs from the public library.  It wasn't until our collaborations with
[mp-units] that we realized we had dodged a bullet.  Fortunately, that wasn't the end of it ---
[mp-units] _also_ showed us an alternative approach that fixed _all_ of the problems we listed
above!

## Unit symbols

A "[unit symbol]" is a simple idea.  It's an object that can multiply or divide with both raw
numeric variables and quantity objects.  The result is always a quantity object, whether or not the
input already was.  And the _effect_ is just to change the units of the variable.

Here's a brief example.  If `m` is a unit symbol for meters, and `s` is a unit symbol for seconds,
then:

- `3.5f` is a raw number.
- `3.5f * m` is a quantity of meters: we changed a raw numeric type into a quantity type.
- `3.5f * m / s` is a quantity of meters per second: we changed one quantity type (`3.5f * m`) into
  another, with different units.

Unit symbols are [_monovalue types_][monovalue types].  They're empty, so the "multiplication" or "division" _never
has any runtime cost_.  Instead, it just tells the compiler how to change the units of some _other_
variable that _does_ have a runtime value.

Let's see how all of our UDL problems melt away with unit symbols:

- **Unit symbols compose naturally.**  We've already seen how `m / s` is a symbol for meters per
  second, composed on the fly.  We can also apply prefixes inline: `kilo(m)` does just what it looks
  like, and we can even make it into a new _named_ symbol locally in a file for even better
  readability.[^4]

[^4]: This would be a line like `constexpr auto km = kilo(m);`, in the same section at the top of
    the file where we import the symbols we use.  Expand the "Includes and usings" section in the
    [Au tab of our Eigen example](../../examples/eigen-kinematics.md#__tabbed_1_2) for an example of
    this.

- **Unit symbols are cheap.**  Just one definition covers every rep, so we'd expect it to be twice
  as fast as the two-definition UDL approach.  In practice, we've found it to be noticeably faster
  than even a single overload.  Better maintenance, lower compile time cost.

- **Unit symbols work with any rep.**  It's perfectly natural.  `3.5 * m` gives us a rep of
  `double`, and `3.5f * m` gives us a rep of `float`.  In fact, `Eigen::Vector3d{1.0, 2.0, 3.0} * m`
  even gives us a rep of `Eigen::Vector3d`!

- **Unit symbols support variables.**  You can write `my_legacy_raw_numeric_variable_m * m`, and see
  at a glance (`..._m * m`) that we've gotten the units right.

It's not as if UDLs have _no_ advantages.  `3.5_mps` has a more concise, unified appearance than
`3.5 * m / s`.  (Even if we defined an ad hoc `mps` symbol, to be as brief as possible, `3.5_mps` is
still more concise than `3.5 * mps` would be.)  Additionally, the multiplicative syntax might make
new users (wrongly) fearful of runtime costs.  However, on adding up all of the costs and benefits,
it's not even a close call: unit symbols win by a country mile.[^5]

[^5]: Technically, we have not been able to _strictly_ verify this claim, because Au does not
    include a definition for the "country mile" unit. 😁

## Au's Constants

This next section might seem like an odd detour.  And it is, in many ways, but it's what led us to
reconsider UDLs, and discover some surprising strengths.

One of Au's key hidden strengths is the [`Constant`][Constant] template type.  We originally
introduced it to support fundamental physical constants such as the speed of light, or Planck's
constant.  Like unit symbols, a `Constant` is a [_monovalue type_][monovalue types]: it can only
ever hold _one_ value, so we always _know_ that value.  This gives it a versatile superpower: the
**perfect conversion policy**.

To understand what this means, contrast `Constant` with `Quantity`.  A quantity is _not_ a monovalue
type.  It wraps some ordinary numeric type, which means we can't reason _at compile time_ about what
the value _is_.  All we can do is reason about what it _might be_.  When dealing with [conversion
risks], we think about _how many_ values are lossy, and _which_ values are lossy, and then we make
the best blanket decision we can: allow the conversion, or forbid it by default.  But these are
_heuristics_, and they have false positives _and_ false negatives.

Let's take a concrete example: a CPU has a word size of 64 bits, and we want to pass that constant
to a function expecting bytes.  If we reach for a `Quantity`, we have:

```cpp
constexpr auto word_size = bits(64);

// ⚠️ Doesn't compile: truncation risk too high!
constexpr auto word_size_bytes = word_size.as(bytes);
```

This is annoying, but also perfectly reasonable: a quantity of bits generally _won't_ be exactly
representable in bytes.  We can circumvent this by passing a [conversion risk policy] parameter,
telling the compiler (and the reader!) that _this particular_ truncation risk is not a concern:
`word_size.as(bytes, ignore(TRUNCATION_RISK))` unblocks the conversion.

While this _works_, it's less than perfectly satisfying.  After all, we know that _this particular_
value will not _actually_ truncate, even though most integer values would.  If only we could make
use of that knowledge directly!  This motivates us to reach for _ad hoc_ `Constant`s (as opposed to
constants of nature), which means we need to know how to _construct_ them.

### Making `Constant`

The main way to make an ad hoc `Constant` is with the `make_constant()` function.  Its single
parameter is a [unit slot], so it supports a wide variety of inputs.  Here's a natural solution for
our current example:

```cpp
constexpr auto word_size = make_constant(bits * mag<64>());

// ✅ Perfect conversion policy verifies: lossless conversion!
constexpr auto word_size_bytes = word_size.as<int>(bytes);
```

The one change we had to make was to add the `<int>` template parameter to `as()`.  This makes
sense: `Constant` has _no associated rep at all_, so we need to tell it what type to store it in.
Once we do, `Constant` has everything it needs for its perfect conversion policy.  If the value fits
in the target unit-and-rep, it allows it; if not, it doesn't.  If we replaced `64` with `65`, it
would --- _correctly_ --- fail to compile!

Au 0.6.0 put the flexibility of `Constant` into overdrive.  Besides multiplying and dividing them,
we can add, subtract, compare, and even `%` them, and Au will generate _brand new types on the fly_
to represent the results.  This means we're going to be making `Constant` a _lot_ more often in
modern Au --- which means every flaw and nuisance will be magnified.

### Making `Constant` _better_

The perfect conversion policy is amazing, but the readability took a real hit.  Instead of just
`bits(64)`, we now find ourselves writing `make_constant(bits * mag<64>())`.  And it gets worse for
fractional numbers --- a `Quantity` like `1.5 * s` turns into `make_constant(seconds * mag<3>() /
mag<2>())` if we upgrade it to a `Constant`.  We don't want to force users to do math in their heads
just to understand our intent!

Fortunately, there were still more powerful varieties of UDLs that we hadn't yet considered.
Instead of simple functions taking `long double` or `unsigned long long`, we can move the literal
into a _variadic template parameter pack_.  Note that the template parameters will be different for
_every distinct sequence of characters_.  This means each individual literal can have a **different
return type**, even with the _same_ unit suffix!

One type per number is just what we need to support a [monovalue type][monovalue types] like
`Constant`.  Here's what the signature looks like:

```cpp
template <char... Cs>
constexpr auto operator""_s() {
    // Figure out how to parse digits, exponents, decimal, separators, etc. 🤔

    // Return an appropriate `Constant` type.
}
```

The details aren't very enlightening, so we've omitted them here.  The key takeaway is what you can
_do_ with this tool: instead of `make_constant(seconds * mag<3>() / mag<2>())`, or even `1.5 * s`,
we can now simply write `1.5_s`.  We get all the superpowers of the first construct (it's a
`Constant`, _not_ a `Quantity`), with _even more_ conciseness than the second!

Remember: even when these values _look like_ floating point numbers, they are not.  Instead, **every
`Constant` literal is an exact rational number**.  When you see `1.2e-3_s`, it means _exactly_ $(12
/ 10,\!000)$ seconds, _even though no floating point number can represent this value_.

Let's drive this point home further.  Most units libraries will forbid assigning a floating point
value to an integer-backed quantity, because this is usually lossy.  But with `Constant` literals,
we _can_ assign `1.2e-3_s` to a `Quantity<Micro<Seconds>, int>`, and get _exactly_
`micro(seconds)(1'200)`!  Au's new UDLs give you the freedom to specify your `Constant` values with
a level of readability we could hardly have imagined before.

## Non-`Quantity` quantity UDLs

Now that we know the power of `Constant`, let's take a fresh look at the ingredients we ended up
with.  We have UDLs that flexibly and concisely construct `Constant` objects.  And we know those
`Constant` objects have _perfect conversion policies_ with any `Quantity` types of their same
dimension.  If we put them together, we see that `Constant` UDLs can _act like_ the old quantity
UDLs when we pass them to APIs expecting a `Quantity`!

This raises the obvious question: what about all those UDL downsides we listed earlier?  Let's
revisit them.

- **Poor composition:**  partial improvement.
    - Remember that UDLs produce a `Constant`, so we have full access to `Constant` APIs.  For
      _compound_ units, we can just multiply or divide at the end: `3.5_m / s` is a workable
      substitute for `3.5_mps`.  And for _prefixed_ units, we can apply the prefix _to the whole
      constant_: `kilo(54.3_g)` certainly _looks_ unusual, but it's obvious that it means the same
      thing that `54.3_kg` would have meant, and there are key use cases where the UDLs' flexibility
      outweighs the strange syntax (see the [atomic units example], for instance).

- **Expensive:**  partial improvement.
    - We're down to a single copy, so the maintenance cost is halved.  The compile time cost _per
      overload_ is likely much higher, but we have a strategy to mitigate it: every UDL gets its
      _own_ header file, so every translation unit includes _only_ the UDLs it actually needs.

- **Rep choice:**  solved.
    - The UDLs return `Constant`: a monovalue type.  Ironically, by having _no_ rep, we
      automatically support _every_ rep!  (At least, every rep that we can check at compile time for
      lossiness --- this is currently limited to the arithmetic reps, but see [#52] for future
      plans.)

- **Literals only:** no change, but no longer a problem.
    - UDLs are still, unsurprisingly, only for literals.  That's fine, though, since this is the
      only use case we _have_ when `Constant` is involved: every `Constant` must be known directly
      at compile time.

## Overall guidance

Au provides _two_ methods for abbreviated construction --- [unit symbols][unit symbol], and [unit
literals] (UDLs) --- with different strengths and weaknesses.  Here's how they compare, criterion by
criterion.  (The colors reinforce the text; they use the same colorblind-friendly scheme as our
[library comparison matrices].)

<table>
    <thead>
        <tr>
            <th></th>
            <th>Unit literals (UDLs)</th>
            <th>Unit symbols</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td><b>Conciseness</b></td>
            <td class="best"><code>3.5_m</code>: nothing shorter is possible</td>
            <td class="good"><code>3.5 * m</code>: still short, but more visually spread out</td>
        </tr>
        <tr>
            <td><b>Numeric notation</b></td>
            <td class="best">Any digits, decimal point, or exponent you like --- and the result is
                an <i>exact</i> rational number</td>
            <td class="good">An ordinary rep value --- the same one your program would use
                anyway --- with the usual rounding and overflow rules</td>
        </tr>
        <tr>
            <td><b>Convertibility</b></td>
            <td class="best">Perfect: converts to any unit and rep that can hold <i>this exact
                value</i>, and refuses every one that can't</td>
            <td class="good">The usual <code>Quantity</code> rules: based on what the <i>type</i>
                could hold, so some individually safe conversions are still blocked</td>
        </tr>
        <tr>
            <td><b>Non-literal values</b></td>
            <td class="poor">Not supported: UDLs are for literals only</td>
            <td class="best"><code>legacy_duration_s * s</code> works fine</td>
        </tr>
        <tr>
            <td><b>Generic <code>Quantity</code> APIs</b></td>
            <td class="poor">Not supported: a <code>Constant</code> can't know <i>which</i>
                <code>Quantity</code> to convert to</td>
            <td class="best">Produces a <code>Quantity</code> directly, so there's nothing to
                deduce</td>
        </tr>
        <tr>
            <td><b>Composability</b></td>
            <td class="fair">Prefixed and compound units need help: <code>kilo(54.3_g)</code>,
                <code>55_mi / h</code></td>
            <td class="best">Composes on the fly: <code>kilo(m)</code>, <code>m / s</code></td>
        </tr>
        <tr>
            <td><b>Rep support</b></td>
            <td class="good">Every rep we can check for lossiness at compile time (currently the
                arithmetic reps; see <a
                href="https://github.com/aurora-opensource/au/issues/52">#52</a>)</td>
            <td class="best">Any rep at all, including custom ones such as
                <code>Eigen::Vector3d</code></td>
        </tr>
        <tr>
            <td><b>Availability</b></td>
            <td class="fair">Opt-in per unit: needs an extra <code>au/units/literals/...</code>
                header</td>
            <td class="best">Ships with the unit's own header; nothing extra to include</td>
        </tr>
        <tr>
            <td><b>Namespace granularity</b></td>
            <td class="fair">All-or-nothing in practice: the conventional
                <code>using namespace ::au::au_literals;</code> brings in every literal you've
                included</td>
            <td class="best">Name exactly the symbols you want:
                <code>using ::au::symbols::m;</code></td>
        </tr>
    </tbody>
</table>

!!! warning "Both tools bring _very_ short names into scope"
    `m`, `s`, `_m`, `_s`: these are the shortest names in your program, and the most likely to
    collide.  A file-scope `using ::au::symbols::m;` or `using namespace ::au::au_literals;` is
    fine in an implementation file (`.cc`, `.cpp`), where it affects a single translation unit.  In
    a **header**, never do either at namespace scope: the names would leak into every translation
    unit that includes it.  See [Namespaces and includes] for the full treatment.

The literals in particular are still quite new, and our best practices could evolve as we learn more
in practice.  Nevertheless, here's our best current understanding.

- **Definitely reach for literals (UDLs)** when...
    - ...you are making a `Constant`.
    - ...you want to specify an exact value (such as a measured physical constant from [CODATA]) in
      a readable way.

- **Definitely reach for unit symbols** when...
    - ...your value is not a literal.
    - ...you are passing to a _generic_ (template) API that expects a `Quantity`: in these cases,
      `Constant` will not know _which_ `Quantity` to try converting to!

Otherwise --- when you have a literal, and you're passing it to a concrete `Quantity` API --- either
one will work, and the choice is a matter of taste and local context.  Consider the feature
comparison matrix above, and experiment and find out what works best for _your_ codebase!

As always, if you have any feedback for us or encounter any problems, feel free to [file an issue].

[user-defined literals]: https://en.cppreference.com/w/cpp/language/user_literal
[mp-units]: https://github.com/mpusz/mp-units
[problems]: https://mpusz.github.io/mp-units/2.5/getting_started/faq/#why-dont-we-use-udls-to-create-quantities
[unit symbol]: ../../reference/unit.md#symbols
[monovalue types]: ../../reference/detail/monovalue_types.md
[Constant]: ../../reference/constant.md
[conversion risks]: ../concepts/conversion_risks.md
[conversion risk policy]: ../../reference/conversion_risk_policies.md
[unit slot]: ./unit-slots.md
[atomic units example]: ../../examples/atomic-units.md
[#52]: https://github.com/aurora-opensource/au/issues/52
[nholthaus units]: https://github.com/nholthaus/units
[unit literals]: ../../reference/constant.md#unit-literals
[CODATA]: https://physics.nist.gov/cuu/Constants/
[Namespaces and includes]: ./namespaces-and-includes.md#headers
[library comparison matrices]: ../../alternatives/index.md#detailed-comparison-matrices
[file an issue]: https://github.com/aurora-opensource/au/issues
