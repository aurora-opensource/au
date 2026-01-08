# Parameter packs

Products of base powers are the foundation for the Au library.  We use them for:

  - The [Dimension](./dimension.md) of a Unit.
  - The [Magnitude](../magnitude.md) of a Unit.
  - Making _compound_ Units (products of powers of units, e.g., $\text{m} \cdot \text{s}^{-2}$).

We represent them as variadic parameter packs.  Each pack element represents a "base power": this is
some "base", raised to some rational exponent.  For a base power `BP`, `Base<BP>` retrieves its
base, and `Exp<BP>` retrieves its exponent (as a `std::ratio`).

!!! note
    This approach, with products of base powers, is known as the [_vector space
    representation_](../../discussion/implementation/vector_space.md) for Dimensions, Magnitudes,
    and so on. The `//au:packs` target, which this page describes, is our tool for implementing
    these vector spaces robustly.

## Representing powers {#powers}

These packs show up in compiler errors, and we want those errors to be as friendly as possible.
Clutter is our enemy!  Thus, we canonicalize each base power to its simplest form.  Consider an
arbitrary base type, `B`; here is how it shows up in the pack:

  | This power of $B$...                | ...shows up in a pack as: |
  |-------------------------------------|---------------------------|
  | $B ^ 0$                             | (omitted)                 |
  | $B ^ 1$                             | `B`                       |
  | $B ^ N$, with $N$ any other integer | `Pow<B, N>`               |
  | $B^{ N / D }$, with $D > 1$         | `RatioPow<B, N, D>`       |

Canonicalizing in this way keeps our compiler errors more concise and readable.

## Strict total ordering

The above canonicalization tells us _what items_ to store.  We also need to be careful about _which
order_ to store them in.  We are modeling multiplication, and in our applications, $(A \times B)$ is
always the same as $(B \times A)$.  However, `Pack<A, B>` is _not_ the same **type** as `Pack<B, A>`!
Thus, we are going to need a way to define whether `A` or `B` should come first inside of a `Pack`.

What we need is a [_strict total ordering_](https://mathworld.wolfram.com/StrictOrder.html), which
applies to _all types_ which might represent a Base in a given kind of Pack.  This is a critical
foundational concept for the library, so we use explicit traits for each kind of pack.  There are
two main elements to this API:

- `InOrderFor<Pack, A, B>` is for _generic algorithms_.  It's how we **check** whether `A` and `B`
  are in the right order for `Pack`.

- `LexicographicTotalOrdering<A, B, Orderings...>` is for _implementing_ `InOrderFor` for a given
  `Pack`.  It's how we **define** whether `A` and `B` are in order for `Pack`.

The point in using `LexicographicTotalOrdering` is that it guards against the most common failure
mode in our application: namely, two _distinct_ types which compare as _equivalent_.
`LexicographicTotalOrdering` tries `A` and `B` against every comparator in `Orderings...`, in
sequence.  If any comparator knows how to order `A` and `B`, we use it.  _If we run out of
comparators, but `A` is not the same as `B`,_ then we produce a hard error.  The fix is to add a new
comparator to "break the tie".

??? example "Example: defining the ordering for a Pack"

    Suppose we have a particular pack, `Pack`, and our bases are `std::ratio` instances.  We need to
    define _some_ canonical ordering.  Let's say that we want to order first by
    denominator---integers first, then halves, thirds, etc---and then by numerator.  We can define
    traits for those orderings, and then combine those traits using `LexicographicTotalOrdering` to
    implement `InOrderFor<Pack, ...>`.  Specifically:

    ```cpp
    template <typename A, typename B>
    struct OrderByDenom : stdx::bool_constant<(A::den < B::den)> {};

    template <typename A, typename B>
    struct OrderByNum : stdx::bool_constant<(A::num < B::num)> {};

    template <typename A, typename B>
    struct InOrderFor<Pack, A, B> :
        LexicographicTotalOrdering<A, B, OrderByDenom, OrderByNum> {};
    ```

    With this definition, something like `Pack<std::ratio<-1>, std::ratio<8>, std::ratio<1, 2>>`
    would be _in-order_.

## Validation

We validate packs using type traits.  `IsValidPack<Pack, T>` is the "overall" validator.  It
verifies that `T` is an instance of `Pack<...>`, and that its parameters satisfy the necessary
conditions.  Specifically, those conditions are:

- `AreBasesInOrder<Pack, T>`: assuming `T` is `Pack<BPs...>`, verifies that all consecutive elements
  in `Base<BPs>...` are all properly ordered (according to `InOrderFor<Pack, ...>`, naturally).

- `AreAllPowersNonzero<Pack, T>`: assuming `T` is `Pack<BPs...>`, verifies that
  `Exp<BPs>::num` is nonzero for every element in `BPs`.

## Algebra on Packs

The whole reason we built `//au:packs` was to support exact symbolic algebra for two operations:
_products_, and _rational powers_.  This section explains how we do that.  Our strategy is:

- **The `//au:packs` target** provides _generic_ versions of these operations that are pre-built, but
  _cumbersome_.

    - (What makes them cumbersome?  They need an extra parameter to specify _which Pack_ they
      operate on.  This is much like `InOrderFor`, which defines the ordering _for a specific type
      of Pack_.)

- **Client targets** provide _aliases_ which "hide" the extra parameter (because they know what
  value it should take!).

Let's take the "pack product" operation as an example, using `Dimension` as our Pack:

```cpp
// The `//au:packs` library provides this:
template <template <class...> typename Pack, typename... Ts>
using PackProductT = /* (implementation; irrelevant here) */;

// A _particular_ Pack (say, `Dimension`) would expose it to their users like this:
template <typename... Dims>
using DimProductT = PackProductT<Dimension, Dims...>;

// End users would use the _latter_, e.g.:
using Length = DimProductT<Speed, Time>;
```

### Supported algebraic operations

Here are the operations we support:

- `PackProductT<Pack, Ps...>`: the product of arbitrarily many (0 or more) `Pack<...>` instances,
  `Ps...`.
- `PackQuotientT<Pack, P1, P2>`: the quotient `P1 / P2`.
- `PackPowerT<Pack, P, N, D=1>`: raise the Pack `P` to the rational power `N / D`.
- `PackInverseT<Pack, P>`: the Pack that gives the null pack when multiplied with the Pack `P`.
