# Eigen Interoperation

Au supports using [Eigen](https://eigen.tuxfamily.org/) vector and matrix types as the underlying
storage type --- the ["rep"](../../reference/rep.md) --- of a `Quantity`.  This gives you
a vector or matrix where every element carries the same unit, with all of Au's usual safety and
conversion features, and with the same runtime performance as raw Eigen.

Once you're finished with this guide, you'll be able to create Eigen-backed quantities, do
unit-aware arithmetic and conversions with them, read and write individual elements, and avoid the
lifetime pitfalls that Eigen is famous for.

!!! warning
    Eigen carries a seriously elevated object lifetime risk, compared to most other libraries.  We
    strongly recommend reading and understanding our [safety guide] so that you can recognize and
    avoid the pitfalls.

## Creating Eigen-backed quantities

Pass an Eigen object to any quantity maker, just as you would a raw number:

```cpp
auto displacement = meters(Eigen::Vector3d{1.0, 2.0, 3.0});
```

This produces a `Quantity<Meters, Eigen::Vector3d>`: a single quantity whose value is a vector, and
whose every element is a length in meters.

??? note "`auto` and safety"
    The above snippet uses `auto` with Eigen, which means we should carefully check it for object
    lifetime risk.  Using the principles in our [safety guide], we can see that this instance _is_
    safe, because there are no _operations_ --- and, therefore, no expression templates with
    references that could dangle.

## Arithmetic and conversions

The usual `Quantity` operations work as expected, including mixed-unit operations:

```cpp
auto a = meters(Eigen::Vector3d{1.0, 2.0, 3.0});
auto b = centi(meters)(Eigen::Vector3d{400.0, 500.0, 600.0});

a.in<Eigen::Vector3d>(centi(meters));   // Eigen::Vector3d{100.0, 200.0, 300.0}

eval((a + b).in(centi(meters)));        // Eigen::Vector3d{500.0, 700.0, 900.0}
```

Note that the above examples carefully avoid lifetime issues by following the principles in our
[safety guide], which also explains how to spot these issues in the first place.

## Element access {#element-access}

Reading an element produces an ordinary `Quantity` whose rep is the element type:

```cpp
auto displacement = meters(Eigen::Vector3d{1.0, 2.0, 3.0});

displacement[0];     // meters(1.0)
displacement(2);     // meters(3.0)
```

Writing an element requires an explicit call to `.mutable_view()`:

```cpp
displacement.mutable_view()[0] = meters(10.0);
displacement.mutable_view()[1] = centi(meters)(2000.0);  // Unit-safe: stores 20.0
```

`displacement[0] = meters(10.0)` will not compile, on purpose: `displacement[0]` is a temporary, so
that assignment would be a silent no-op.  See the [element access reference
docs](../../reference/quantity.md#element-access) for more detail.

## Eigen member functions {#member-functions}

Eigen provides much of its API as member functions --- `v.norm()`, `v.transpose()`, `v.dot(w)`, and
so on.

Au cannot provide these as _member functions_ for an Eigen-backed `Quantity`, because it would be
inappropriately invasive, and would massively increase the scope of the `Quantity` API.  Instead,
there is a simple rule: every Eigen _member function_ becomes a _free function_ with the same name
for an Eigen-backed `Quantity`. These are all available in Au's Eigen compatibility header:

```cpp
#include "au/compatibility/eigen.hh"
```

This header has no physical dependency on Eigen, so including it doesn't add Eigen to your project,
and include order doesn't matter.  Here are some usage examples.

```cpp
auto v = meters(Eigen::Vector3d{3.0, 4.0, 0.0});

norm(v);           // meters(5.0)
squaredNorm(v);    // squared(meters)(25.0)
dot(v, v);         // squared(meters)(25.0)
eval(transpose(v));  // Transposed vector, still in meters.
```

See the [Eigen compatibility reference](../../reference/eigen.md) for the full list of functions.

### Changing the scalar type {#cast}

To change the _scalar_ type of an Eigen-backed `Quantity`, use
[`cast<NewScalar>`](../../reference/eigen.md#cast), the free function form of Eigen's
`.cast<NewScalar>()`.

```cpp
auto v_f = meters(Eigen::Vector3f{1.5f, 2.5f, 3.5f});

eval(cast<double>(v_f));  // Quantity<Meters, Eigen::Vector3d>
```

Note that Au's usual tools for changing the rep do **not** work here.  Neither the preferred,
risk-checked `.as<NewRep>(unit)` --- which can't form a common rep for two Eigen types with different
scalars --- nor the forcing `rep_cast<NewRep>`, which performs a `static_cast` that Eigen
deliberately rejects between types with different scalars.

## Run your tests under sanitizers

Lifetime bugs with expression templates are easy to write and hard to spot in review --- again,
exactly as with raw Eigen.  We strongly recommend making sure that every line of Eigen code is
covered by a unit test, and that those unit tests are run under the address sanitizer.  This
"defense in depth" approach complements human review.

## Outcome and limitations

You can now use Eigen types as first-class quantity reps, with unit safety on top and no performance
penalty underneath.  Here are the main known limitations.

1. Every element of a vector or matrix quantity has the _same unit_.  "Heterogeneous" matrices,
   with distinct units per row or column, are planned future work (see [#707]).

2. `prod()` and `determinant()` require fixed-size operands, because the result's _unit_ depends on
   the operand's size, which must therefore be known at compile time.

3. `mutable_view()` is more verbose than raw Eigen's `v[i] = ...`.  This is a deliberate safety
   tradeoff; the safety guide [explains
   why](../../discussion/concepts/eigen_safety.md#element-access).

[#707]: https://github.com/aurora-opensource/au/issues/707
[safety guide]: ../../discussion/concepts/eigen_safety.md
