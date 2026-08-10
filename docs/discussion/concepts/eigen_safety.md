# Using Eigen safely with Au

[Eigen] is a popular C++ library for vectors and matrices.  Using a technique called "expression
templates", Eigen gets blazing fast performance --- comparable to hand-crafted loops!  The cost of
this speed is a vastly increased risk of dangling references and undefined behavior.

Au supports Eigen.  You can use Eigen types as the underlying storage type, or
["rep"](../../reference/rep.md), of a `Quantity`.  If you do, you can even get the full performance characteristics of "raw" Eigen.[^1]
Unfortunately, this brings the same _cost_ as "raw" Eigen: heightened risk of dangling references.

[^1]: We are the first open source units library to provide this feature, as far as we know!

This page explains what expression templates are, why they're so performant _and_ so dangerous, and
how to adapt the standard Eigen best practices to keep your Au code safe.  **If you use Au with
Eigen, we strongly recommend that you read and understand the concepts here.**

## Understanding expression templates

Naive linear algebra code can be inefficient compared to hand written code.  For example, consider
adding three vectors:

```cpp
Eigen::Vector3d a, b, c;

Eigen::Vector3d result = a + b + c;
```

Based on the rules of the C++ language, this line computes `(a + b)` first, and then `(temp + c)`,
where `temp` is what we'll call the result of `(a + b)`.  Under the natural implementation for
`operator+`, `temp` is a full fledged temporary vector, so we pay for writing it to memory, and then
reading it back to add it to `c`.  However, this wouldn't be necessary if we wrote the loop by hand.
We could simply iterate once over all the inputs, and store each element directly in the result:

```cpp
Eigen::Vector3d a, b, c;

Eigen::Vector3d result;
for (auto i = 0; i < 3; ++i) {
    result[i] = a[i] + b[i] + c[i];
}
```

The first code sample is more natural, but the second is more efficient.  The point of expression
templates is to let you write code that _looks like_ the first sample but actually _performs like_
the second one.

### Under the hood: proxy types

The key to this magic is `operator+`.  For `Eigen::Vector3d + Eigen::Vector3d`, it does **not**
return another `Eigen::Vector3d`.  Instead, it returns an `Eigen::CwiseBinaryOp<...>`: a tiny object
that holds _references_ to the two operands, and a description of the operation.  Importantly,
constructing this object performs _no actual computation_.  Instead, it just keeps track of what
computation _needs to be performed_.

By itself, this is useless: whether you do the work now, or later, it's still the same work.  The
real value comes when you chain multiple operations together.  By waiting to perform the computation
until we know _all_ the ingredients, it gives the library a chance to produce the most efficient
_overall_ recipe.

Let's elaborate, using our three-vector addition example (and omitting some namespaces to make the
structure more clear).  The first operation, `a + b`, produces this type:

```
CwiseBinaryOp<scalar_sum_op<double, double>,
              const Vector3d,    // a
              const Vector3d>    // b
```

The pieces make sense.

- `CwiseBinaryOp` tells us we have two inputs to keep track of.
- `scalar_sum_op` indicates addition.
- `const Vector3d` tells us the type of each input.

So, we're adding two `Vector3d` objects.  And this type based reasoning **composes**: when we add
this result to `c`, we find:

```
CwiseBinaryOp<scalar_sum_op<double, double>,

              const CwiseBinaryOp<scalar_sum_op<double, double>, //
                                  const Vector3d,                // (a + b)
                                  const Vector3d>,               //

              const Vector3d>                                    // c
```

Notice that the type we already saw for `(a + b)` is nested within this new type.

!!! note
    It's worth emphasizing that Eigen does _not_ store references to the `(a + b)` temporary object.
    Instead, it recognizes this as an expression template, so it stores it by _value_.  The upshot
    is that we get our own, fresh references to `a` and `b`.  Along with the new reference to `c`,
    this means the only references we store overall are to the original inputs.

Next, we _assign_ this expression to `result`.  Assigning a `CwiseBinaryOp` to a `Vector3d` is what
triggers the actual computation.  Eigen is responsible for generating the code.  In this case, it
has enough information to see that we are ultimately adding three vectors. Therefore, it generates
a single loop that looks very much like our `a[i] + b[i] + c[i]` example above[^2].

[^2]: In practice, it's likely to be even more efficient than that, because Eigen can use SIMD
instructions to perform multiple additions in parallel.

### The trap

The previous section has two **critically important takeaways**:

1. **Operations** in Eigen **do not compute** results; they **store references** to their original
   inputs.

2. **Computation** is deferred until you **ask for it**.

This means that your original inputs _must stay alive (and unaltered) until you request that
computation_.

For most of Eigen's early existence, this risk was very small, because C++ forced users to _name the
type_ when assigning a variable.  Computation is deferred until you ask for it, but assigning to
a named type _is_ "asking for it", so all is well.

For example, suppose we have a function `displacement(t)`, which returns an `Eigen::Vector3d`, and
we want to compute a vector difference. Before C++11, you had to name the type of the result
explicitly:

```cpp
// ✅ `ds` is `Vector3d`, so assigning triggers the actual computation.
Eigen::Vector3d ds = displacement(1.0) - displacement(0.0);
```

Assigning the `CwiseBinaryOp` to an `Eigen::Vector3d` triggers the computation, and all is well.

All of that changed when C++11 introduced `auto`.[^3]  Now, you can assign to a variable whose type is
"whatever is to the right of the equals sign".  If you replace `Vector3d` with `auto` to save
typing, then the `CwiseBinaryOp<...>` that would have auto-converted to a `Vector3d` remains
a `CwiseBinaryOp<...>` instead:

```cpp
// ⛔ `ds` is `CwiseBinaryOp<...>`, which holds references to inputs.
auto ds = displacement(1.0) - displacement(0.0);
```

The later the computation gets deferred, the more opportunity the original inputs have to change or
die, and thus, to cause undefined behavior.  In fact, in this example, the inputs are already dead
once we reach the semicolon, and this is a guaranteed dangle!

[^3]: This `auto` pitfall is so notorious that Eigen documents it prominently in [their own list of
    common pitfalls](https://eigen.tuxfamily.org/dox/TopicPitfalls.html).

## Dangling is easy

Let's look at another example of how easy it is to dangle, this time in the context of the Au units
library.

With a scalar rep, an expression like this would be totally unremarkable, and indeed quite safe:

```cpp
// ✅ Normal; safe.
const auto displacement = meters(1.0) * 2.0;
```

However, the same exact code with an Eigen rep is _not_ safe, and has undefined behavior:

```cpp
// ⛔ Looks normal, but dangles!
const auto displacement = meters(Eigen::Vector3d{1.0, 2.0, 3.0}) * 2.0;
```

The `Eigen::Vector3d{1.0, 2.0, 3.0}` inside the `Quantity` that `meters` creates is a _temporary_,
which won't live past the end of the line.  Multiplying by 2.0 creates an expression template
(stored in `displacement`) that refers to the soon-to-be-dead temporary: instant dangle!

If you've never seen this before, it probably looks quite shocking.  But the failure mode actually
has nothing to do with Au; you could replace `meters` with _any_ function that returns an Eigen
vector.

```cpp
Eigen::Vector3d foo(const Eigen::Vector3d &v);

// ⛔ Looks normal, but dangles!
const auto displacement = foo(Eigen::Vector3d{1.0, 2.0, 3.0}) * 2.0;
```

This example dangles just as much as the previous one.  In fact, we don't even need the function
call to get dangling!

```cpp
// ⛔ Even this dangles!
const auto displacement = Eigen::Vector3d{1.0, 2.0, 3.0} * 2.0;
```

If you weren't already taking object lifetime seriously when using Eigen, I hope these sobering
examples will help you reconsider.  But remember, too, that it isn't just arbitrary; there are two
necessary ingredients for this undefined behavior.  Once more:

1. You need an **operation**.  (This could be arithmetic operators like `*` or `+`, Eigen's
   built-in "view" functions such as `.transpose()` or `.row()`, or Au's unit conversion operators
   such as `.as()` or `.in()`.)

2. You need to **defer computation** --- more specifically, defer it until after an _original_ input
   has _changed or died_.

You can't generally attack the first ingredient; after all, the operations are the things you want
to do.  So your best approach is to focus on the second ingredient: either keep those original
inputs alive (and unchanged) for longer, or _stop deferring the computation_.

!!! note
    Ending an input's lifetime isn't the only way to trigger the second ingredient.  An input that
    merely _changes_ is just as dangerous: `auto sum = a + b;` followed by
    `a = Eigen::Vector3d::Zero();` leaves `sum` silently reading the _new_ value of `a`.

## How to ask for computation

Eigen provides two ways to ask for computation.

1. Assign the expression to a **concrete** type (i.e., a vector or matrix type).
2. Call `.eval()` on the expression.

The first approach is the one we've already seen, and it works unchanged in Au.

The second approach _can't_ work the same, because `au::Quantity` doesn't have an `.eval()` member
function.  So we provide [a free function](../../reference/eigen.md#eval) instead: if your
expression is `x`, you would write
`eval(x)` instead of `x.eval()`.

Interestingly, combining Eigen with a units library changes how often you reach for each, and makes
the second method far more common than the first.  The reason is that the type names get another
level of nesting, and become that much more complicated: instead of assigning to `Eigen::Vector3d`,
you would need to assign to something like `Quantity<Meters, Eigen::Vector3d>`.  This is already
a little unwieldy --- and many unit type names are far more complicated than `Meters`!  In many
cases, it's simpler to just follow the `auto x = eval(...)` pattern.

The bottom line is that _the `eval()` free function should be at the forefront of your toolbox when
using Eigen with Au_.  Wrap it around an expression to force it to materialize the results,
automatically picking the most appropriate concrete type for the expression.  When in doubt,
`eval(...)` is a safe choice.

## You need sanitizers, too

Sadly, experience has conclusively shown that even the best human review is not enough to mitigate
Eigen's lifetime issues.  In fact, no single approach is: you will always do better with _defense in
depth_, using multiple approaches with independent failure modes.  With Eigen, the most effective
second line of defense is _sanitizers_, especially the Address Sanitizer (ASAN).

A sanitizer takes your same source code as input, but produces a modified program as output when you
compile.  It adds extra instructions that execute at runtime.  They keep track of each object's
memory and lifetime, and guard against common memory errors --- such as trying to access memory
outside an object's bounds, or reading or writing to its memory after its lifetime ends.

Of course, all this bookkeeping is extra work, so it slows down your program --- often, very
significantly.  Many projects cannot tolerate this slowness, so it doesn't always make sense to
build the _main program_ with sanitizers.  What _does_ always make sense is _unit tests_.  This
brings us to our guidance for using sanitizers to mitigate Eigen's lifetime issues:

- Every line of code that uses Eigen types **must** be covered by a unit test.
- Every one of those unit tests **must** be run under sanitizers.

In fact, this guidance _always_ applies to Eigen, completely independently of whether or not you
also use Au.

## Risk parity

While these risks, and the multifaceted tools needed to fight them, might seem daunting, it's
important to realize they're nothing new.  Everything we've seen so far is something that users of
Eigen _have already opted into_.  Therefore, they are no cause for concern from Au's point of view.

What _would_ be a serious concern is if Au added _new_ lifetime risks on top of Eigen's.  In writing
this feature, we found that it was surprisingly hard for a units library to avoid doing this.
However, it _is_ possible, and we believe it is the right standard to hold ourselves to.

We call this philosophy **risk parity**:

- Any lifetime risks _already present_ in raw Eigen are not problematic.
- Any _new_ lifetime risks _introduced by_ Au are **unacceptable**.

Loosely, this means that if you take a program using raw Eigen, and migrate it to Au, you should
find that the same risks are present, they manifest in similar ways, and you use similar tools and
techniques to mitigate them.  They may not be _exactly_ the same --- for example, `eval` is a free
function in Au, but a member function in Eigen --- but they should be similar enough to be familiar.

### Beyond lifetimes: element access {#element-access}

We've heavily emphasized lifetime issues because they're the most prevalent, and because they
require manual work to avoid (just as in raw Eigen).  However, this is not the only safety risk that
Au must mitigate in order to achieve risk parity.

In a raw Eigen vector `v`, `v[i]` returns a reference to the `i`th element.  A quantity vector,
`qv`, is necessarily different: `qv[i]` [returns a _brand new
object_](../../reference/quantity.md#element-access): a _`Quantity`_ that adds unit safety to the
`i`th element.

This is mostly very natural, but it brings one serious pitfall.  In raw Eigen, `v[i] = 3.0` will
update the `i`th value as expected.  However, `qv[i] = meters(3.0)` would be updating the _temporary
object `qv[i]`_: so the change gets immediately discarded, while the original
`qv` remains unchanged!  This behavior would seriously violate our "risk
parity" philosophy.

Fortunately, we are able to prevent it at compile time.  The above assignment will _not compile_: Au
will not let you assign to an rvalue `Quantity` in most cases.  Instead, you can explicitly request
a [`mutable_view()`](../../reference/quantity.md#mutable-view), which _will_ let you write:
`qv.mutable_view()[i] = meters(3.0)` _does_ work, and does what would be expected.

In practical, day to day Au use, this error category carries less risk than lifetime issues, because
the compiler detects and prevents it.  You merely need to be aware that this `mutable_view()` option
exists.  We mention it here to illustrate another way that the risk parity principle guides our
design decisions.

## Bottom line takeaways

Taking advantage of Eigen's terrific performance requires understanding its pitfalls, and how to
mitigate them.  Here are our rules of thumb for using it with Au specifically.

1. **Understand** the danger zone: having _both_ of (a) Eigen operations, _and_ (b) deferring
   computation until after an _original_ input has changed or died.
    - _TIP:_ by far the most common way this manifests is "using `auto` with Eigen expressions".
      That was true without Au, and it's still true with it.
2. Know how to **mitigate** it: [`eval(...)`](../../reference/eigen.md#eval) forces computation, and
   is generally the most flexible tool.
3. Use **defense in depth**: sanitizers complement human review.
    - _TIP:_ ensure _every_ line of Eigen code is covered by unit tests, and that those tests run
      automatically under sanitizers.

Overall, Au's unique approach to Eigen support means you can add unit safety to your existing Eigen
program, while retaining the existing levels of both performance and safety.

[Eigen]: https://eigen.tuxfamily.org/
