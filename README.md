![Au library logo](docs/assets/au-logo-color.png)

# Au: A C++14-compatible units library, by Aurora

Au (pronounced "ay yoo") is a C++ units library, by
[Aurora](https://aurora.tech/).  What the `<chrono>` library did for time
variables, _Au_ does for _all physical quantities_. Namely:

- Catch unit errors at compile time, with **no runtime penalty**.
- Make unit conversions effortless to get right.
- _Accelerate and improve your general developer experience._

In short: if your C++ programs handle physical quantities, Au will make you faster and more
effective at your job.  You'll find everything you need in our [full documentation
website](https://aurora-opensource.github.io/au).

## As seen at CppCon 2021

We gave a talk at CppCon 2021 about how to use units libraries effectively, and what properties to
look for.  Since Au was designed from the ground up with these best practices in mind, it
exemplifies them.  Check out the video below, and follow along with [the slide
deck](https://chogg.name/cppcon-2021-units/) if you like.

[![Chip Hogg's CppCon 2021 Aurora units talk](https://user-images.githubusercontent.com/10720055/203602853-9437f26a-9b1f-4b54-8a4d-2fb242ed9953.png)](https://www.youtube.com/watch?v=5dhFtSu3wCo)

> NOTE: this open-source version has been significantly improved from what was presented in the
talk: both in its user interfaces, _and_ under the hood!  The one downside is that matrix and vector
support is still pending; see [#70](https://github.com/aurora-opensource/au/issues/70) for details.

## Getting started

Our [installation instructions](https://aurora-opensource.github.io/au/install) can have you up and
running in minutes, in any project that supports C++14 or newer.

To use the library effectively, we recommend working through the tutorials, starting with the first:
[Au 101: Quantity Makers](https://aurora-opensource.github.io/au/tutorial/101-quantity-makers).  To
get set up with the tutorials — or, to contribute to the library — check out our [development
guide](https://aurora-opensource.github.io/au/develop).

## Why Au?

There are many other C++ units libraries, several quite well established.  Each of them offers
_some_ of the following properties, but **only Au** offers _all_ of them:

- Wide compatibility with C++ versions (anything C++14 or newer).
- Easy installation in any project (including a customizable single-header option).
- Quick to compile.
- Concise, readable typenames in compiler errors.

We also provide several totally new features, including fully unit-safe APIs, an adaptive "safety
surface" that protects conversions against overflow, unit-aware rounding and inverse functions, and
many more.

Forged in the crucible of Aurora's diverse, demanding use cases, Au has a proven track record of
usability and reliability. This **includes embedded support**: Aurora's embedded teams have been
first class customers since the library's inception.

To learn more about our place in the C++ units library ecosystem, see our [detailed library
comparison](https://aurora-opensource.github.io/au/alternatives).
