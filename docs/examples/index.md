# Code examples

Short, complete programs that show what using Au actually looks like.

Most of these come in two versions: the way the problem is usually solved in plain C++, and the way
it looks with Au.  They are presented as tabs so you can flip between them and compare corresponding
lines in place --- the two versions are kept line-aligned on purpose.

## The examples

- **[Linear speed to revolutions per minute](./angular-velocity.md).**  Converting a wheel's road
  speed into revolutions per minute (RPM), without fussing with manual conversion factors like `2π`
  or `60`.

## How these examples are written {#front-matter}

Sharing code examples brings an intrinsic tension.  On the one hand, we want to minimize clutter,
and emphasize the core ideas and the way the APIs flow together.  On the other hand, the examples
are how new users will begin using the library, so we can't compromise on including _everything_
that's needed to get them running!

Our solution is to distinguish _"front matter"_ from the _core_ of the example.  Front matter
includes:

- `#include` lines
- Individual `using` statements
- Any unit aliases we define for convenience

We'll always provide this front matter, but we'll collapse it by default.  Look for the
**"Includes and usings"** block inside each tab: expand it and you have the whole file.  Nothing on
these pages depends on context we have hidden from you, and no snippet is a sketch with the awkward
parts elided.

These are also the same conventions we recommend for _your_ code.  See **[Namespaces and
includes](../discussion/idioms/namespaces-and-includes.md)** for our detailed reasoning.

## About these examples

Every example here is real source code from the
[`examples/`](https://github.com/aurora-opensource/au/tree/main/examples) directory of the
repository, inlined into these pages directly.  CI compiles and runs both versions of each one, and
checks that they produce identical output --- so the "before" and "after" really do solve the same
problem, and neither can drift out of date with the library.

<!-- ADDING AN EXAMPLE: see `examples/README.md` in the repository.  It covers the invariants these
pages depend on (line-aligned regions, one-line banners), the design decisions behind them, and what
is still to do.  Deliberately kept in one place so the two copies cannot drift.

  Quality over quantity.  A good example here is a real problem someone actually had, small enough
  to read in one screenful, where the Au version is better in a way the reader can clearly see.
-->
