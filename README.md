![Au library logo](docs/assets/au-logo-color.png)

# Au: A C++14-compatible units library, by Aurora

Au (pronounced "ay yoo") is a C++ units library, by
[Aurora Innovation](https://aurora.tech/).  What the `<chrono>` library did for time
variables, _Au_ does for _all physical quantities_ (lengths, speeds, voltages, and so on). Namely:

- Catch unit errors at compile time, with **no runtime penalty**.
- Make unit conversions effortless to get right.
- _Accelerate and improve your general developer experience._

In short: if your C++ programs handle physical quantities, Au will make you faster and more
effective at your job.  You'll find everything you need in our [full documentation
website](https://aurora-opensource.github.io/au).

## Why Au?

There are many other C++ units libraries, several quite well established.  Each of them offers
_some_ of the following properties, but **only Au** offers _all_ of them:

- Wide compatibility with C++ versions (anything C++14 or newer).
- Easy installation in any project (including a customizable single-header option).
- Small compile time burden.
- Concise, readable typenames in compiler errors.

We also provide several totally new features, including fully unit-safe APIs, an adaptive "safety
surface" that protects conversions against overflow, unit-aware rounding and inverse functions, and
many more.

Forged in the crucible of Aurora's diverse, demanding use cases, Au has a proven track record of
usability and reliability. This **includes embedded support**: Aurora's embedded teams have been
first class customers since the library's inception.

To learn more about our place in the C++ units library ecosystem, see our [detailed library
comparison](https://aurora-opensource.github.io/au/alternatives).

## Getting started

Our [installation instructions](https://aurora-opensource.github.io/au/install) can have you up and
running in minutes, in any project that supports C++14 or newer.

To use the library effectively, we recommend working through the
[tutorials](https://aurora-opensource.github.io/au/tutorial/), starting with [Au 101: Quantity
Makers](https://aurora-opensource.github.io/au/tutorial/101-quantity-makers).  To
get set up with the tutorials — or, to contribute to the library — check out our [development
guide](https://aurora-opensource.github.io/au/develop).

## As seen at CppCon 2021

At CppCon 2021, we
[presented](https://cppcon2021.sched.com/event/nvCp/units-libraries-and-autonomous-vehicles-lessons-from-the-trenches)
the properties we found to be most important in units libraries, and advice on using them
effectively.  Because Au was designed from the ground up with these best practices in mind, it
thoroughly exemplifies them.  Check out the video below, and follow along with [the slide
deck](https://chogg.name/cppcon-2021-units/) if you like.

[![Chip Hogg's CppCon 2021 Aurora units talk](https://user-images.githubusercontent.com/10720055/203602853-9437f26a-9b1f-4b54-8a4d-2fb242ed9953.png)](https://www.youtube.com/watch?v=5dhFtSu3wCo)

> NOTE: This open-source version has been significantly improved from what was presented in the
talk: both in its user interfaces, _and_ under the hood!  The one downside is that matrix and vector
support hasn't yet been implemented.  See [#70](https://github.com/aurora-opensource/au/issues/70)
for more details, and subscribe to that issue to watch for progress.
