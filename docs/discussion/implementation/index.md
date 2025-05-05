# Implementation Details

This section provides in-depth discussion about core implementation details of Au.  If you want to
understand _how_ the library works, this is a good place to go.  Here's a rough guide.

- **[Applying Magnitudes](./applying_magnitudes.md)**.  A conversion factor is a _magnitude_:
  a nonzero real number.  The best way to apply it to a value of type `T` depends on what kind of
  number it is (integer, rational, and so on), the runtime performance, and whether we can get exact
  answers.

- **[Vector Space Representations](./vector_space.md)**.  We're not talking about position vectors
  or velocity vectors!  There's a different kind of vector at the heart of every units library.
  This is the core foundational concept on which we built Au's implementation.
