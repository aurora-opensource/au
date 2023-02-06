# Implementation Details

This section is for reference docs which most end users won't often encounter directly.  We collect
these docs in this subfolder to keep the main folder less cluttered.  Here's a guide to the content:

- **[Monovalue Types](./monovalue_types.md).**  This isn't a _specific type_ in the library; rather,
  it's a "concept" (loosely defined)[^1] which many core library types model.

- **[Parameter Packs](./packs.md).**  The most critical foundation for the library is _parameter
  packs_, because this is how we implement the [vector
  spaces](../../discussion/implementation/vector_space.md) which underpin
  [dimensions](./dimension.md), [magnitudes](../magnitude.md), and compound units.

[^1]: We're using "concept" in a loose, conceptual sense, rather than in the sense of the [language
element](https://en.cppreference.com/w/cpp/language/constraints) which was added in C++20.
