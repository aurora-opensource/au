# Vector Space Representations

To understand why _vector space representations_ are so important for units libraries, we'll dig
into the most fundamental example.  After that, we'll look at some other instances of vector space
representations in our library.

Consider [Dimensions](../../reference/detail/dimension.md).  How can we teach the library to recognize
that, say, the product of the Dimensions `Speed` and `Time` is the Dimension `Length`?  If these
three Dimensions are all primitive, irreducible objects, this is very challenging.  However, if
`Speed` is just an _alias_ for `(Length / Time)`, then it's easy to see that `(Length / Time)
* Time` reduces to `Length`.

This is what we do: we single out certain Dimensions and call them "Base Dimensions".  Any valid
choice must fulfill these conditions:

1. **Independence:** no product of rational powers of Base Dimensions is _dimensionless_, unless all
   exponents are 0.

2. **Completeness:** every Dimension of interest can be represented as some product of rational
   powers of Base Dimensions.

The reader may recognize these properties of _Base Dimensions_ as analogous to the defining
properties of [_Basis **Vectors**_](https://en.wikipedia.org/wiki/Basis_(linear_algebra)) in
a vector space, but with these differences:

- instead of _adding_ vectors, we _multiply_ Base Dimensions
- instead of _multiplying_ vectors by a scalar, we _raise them to a power_

In fact, we can bridge this gap if we consider the **exponents** of the dimensions to be the
**scalars** of the vector space.  In a sense, the "logarithms" of the dimensions form a vector
space; in this case, over the rationals.

This is the "vector space representation" for Dimensions.

## Fleshing out the analogy

How do the [defining properties of vector
spaces](https://en.wikipedia.org/wiki/Vector_space#Definition_and_basic_properties) manifest
themselves here?  Let the space of all Dimensions be $\mathscr{D}$.  For any dimensions $D, D_1,
D_2, D_3 \in \mathscr{D}$, and any rational numbers $a, b \in \mathbb{Q}$, we have:

- **Associativity:** $D_1 \cdot (D_2 \cdot D_3) = (D_1 \cdot D_2) \cdot D_3$
- **Commutativity:** $D_1 \cdot D_2 = D_2 \cdot D_1$
- **Identity (Vector):** $\exists \pmb1 \in \mathscr{D}: \,\, \pmb1 \cdot D = D \cdot \pmb1 = D,
  \,\, \forall D$
- **Inverse:** $\forall D \in \mathscr{D}, \exists D^{-1} \in \mathscr{D}: \,\, D \cdot D^{-1}
  = D^{-1} \cdot D = 1$
- **Scalar/Field Multiplication Compatibility:** $(D^a)^b = D^{(ab)}$
      - (Recall that "scalar multiplication" in "ordinary" vector spaces corresponds to
        _exponentiation_ in _our_ vector space.)
- **Identity (Scalar):**  $\exists 1 \in \mathbb{Q}: \,\, D^1 = D$
- **Distributivity (Vectors):** $(D_1 \cdot D_2)^a = D_1^a \cdot D_2^a$
- **Distributivity (Scalars):** $D^{(a + b)} = D^a \cdot D^b$

## C++ Implementation Strategies

The abstract concepts above form the core of basically every C++ units library.  When it comes to
_implementation_, there are a variety of choices.

### Naive approach: positional arguments

The simplest implementation of a vector space is to use positional template parameters to represent
the coefficients (exponents) of each basis vector (base dimension).  For example:

```cpp
// Basic approach (not used in this library)
template<typename LengthExp, typename TimeExp>
struct Dimension;

using Length = Dimension<std::ratio<1>, std::ratio<0>>;
using Time   = Dimension<std::ratio<0>, std::ratio<1>>;
using Speed  = DimQuotientT<Length, Time>;
```

This approach is easy to implement, but its simplicity comes at a cost.

- Compiler errors are inscrutable.  (What exactly does
  `Dimension<std::ratio<1, 1>, std::ratio<-1, 1>>` represent?)

- If we need to add a new basis vector, it will affect an immense number of callsites.

- Some applications need infinitely many basis vectors!  This approach is a complete non-starter.

### Advanced approach: variadic templates

We can solve all of these problems by making `Dimension` a _variadic_ template.

```cpp
// Advanced approach (the one we use, although simplified here)
template<typename... BaseDimPowers>
struct Dimension;

using Length = Dimension<base_dim::Length>;
using Time   = Dimension<base_dim::Time>;
using Speed  = DimQuotientT<Length, Time>;  // As before.
```

Compiler errors are now easy (or at least possible) to read: when we see something like
`Dimension<base_dim::Length, Pow<base_dim::Time, -1>>`, we can recognize it as "Speed".  And adding
new basis vectors---even _arbitrarily many_ new ones---doesn't affect any existing callsites.

The downside is that the added complexity incurs new risk.  Now we have to care about the **order**
of the template parameters; otherwise, we could have _different types_ representing the _same
conceptual Dimension_.  Fortunately, that's exactly why we built the [`//au:packs`
target](../../reference/detail/packs.md): to handle these subtleties robustly.

## Other vector space representations

Above, we focused on Dimensions as an example use case for the vector space representation.  Though
by far the most common in units libraries, it's not the only one that adds value.  Here are some
others worth recognizing.

### Magnitude

The ratio between two Units of the same Dimension is a nonzero real number: a "Magnitude".  We use
a vector space representation for Magnitudes, because then it will naturally support all the same
operations which Dimensions support.  But then, what are the basis vectors?  What numbers can we use
that are "independent", in the sense that every Magnitude gets a **unique** representation?

Prime numbers are a great start!  Given any collection of primes, $\{p_1, \ldots, p_N\}$, and
corresponding rational exponents $\{a_1, \ldots, a_N\}$, the product
$p_1^{a_1} \cdot (\ldots) \cdot p_N^{a_N}$ is _unique_: no other collection $\{a_1, \ldots, a_N\}$
can produce the same number[^1].

[^1]: Technically, this is only true for a _finite_ collection of primes, though the collection can
be arbitrarily large.  If we took an _infinite_ collection of primes, it wouldn't _just_ give some
numbers multiple representations --- it would give _every_ number _uncountably infinitely many_
distinct representations!  In practice, this distinction is largely academic, because our library
currently only targets computers with finite amounts of memory.

This already lets us represent _anything_ we could get with `std::ratio`.  And, unlike a
`(num, denom)` representation, we're always automatically in lowest terms: any common factors cancel
out automatically when we represent it via its prime factorization!

In fact, we have _surpassed_ `std::ratio`'s functionality, too.  We can handle very large numbers
with negligible risk of overflow: `yotta` ($10^{24}$) doesn't even fit in `std::intmax_t`, but
`pow<24>(mag<10>())`[^2] handles it with ease.  We can even handle radicals: something unthinkable for
`std::ratio`, like $\sqrt{2}$, is as easy as `root<2>(mag<2>())`[^3].

[^2]: `pow<24>(mag<10>())` expands to `Magnitude<Pow<Prime<2>, 24>, Pow<Prime<5>, 24>>`.

[^3]: `root<2>(mag<2>())` expands to `Magnitude<RatioPow<Prime<2>, 1, 2>>`.

Finally, we can incorporate other irrational numbers, too.  No units library is complete without
robust support for $\pi$, but `std::ratio` isn't up to the task.  For vector space magnitude
representations, though, its difficulty becomes a strength.  We know there is no collection of
exponents $\{a_i\}$ such that $\pi = \prod\limits_{i=1}^N p_i^{a_i}$, for any collection of primes
$\{p_i\}$.  This means that $\pi$ is **independent**, and we can add it as a new basis vector.  Then
the ratio of, say, `Degrees` to `Radians` (i.e., $\pi / 180$) could be expressed as
`Magnitude<Pi>{} / mag<180>()`[^4].

[^4]: `Magnitude<Pi>{} / mag<180>()` expands to `Magnitude<Pow<Prime<2>, -2>, Pow<Prime<3>, -2>, Pi,
Pow<Prime<5>, -1>>`.

### Units

If we form a Unit by combining other units---say, `Miles{} / Hours{}`---it's useful to retain the
identities of the units that went into it.  There are several reasons to prefer this to, say,
converting everything to a coherent combination of preferred "base units", and some Magnitude for
scaling.

- It will be easy to generate a compound _label_, by combining the primitive labels for `Miles` and
  `Hours`.

- Compiler errors will mention only familiar, recognizable Units.

- It promotes cancellation where appropriate: `Miles{} / Hours{}` times `Hours{}` will give simply
  `Miles{}`.

Our treatment of Units differs from other vector space instances, because we prefer not to use the
container type (in this case, `UnitProduct<...>`) unless we have to: after all, `Meters` is more
user-friendly than `UnitProduct<Meters>`, let alone something awful like
`UnitProduct<RatioPow<Meters, 1, 0>>`!  We support this use case with the following strategy:

- **wrap-if-necessary** on the way **in** (via `AsPack`)

- **unwrap-if-possible** on the way **out** (via `UnpackIfSolo`)

This was the only machinery we needed to add: apart from that, we were able to leverage our
pre-existing [packs](../../reference/detail/packs.md) support to provide a fluent experience for
compound units.
