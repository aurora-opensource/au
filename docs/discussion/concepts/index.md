# Generic Concepts

This section discusses concepts about units libraries generally. Some may be more precise
formulations of principles that most users know intuitively, such as [Arithmetic
Operations](./arithmetic.md).  Others may be novel innovations which originated with Au, but would
enhance _any_ units library, such as [Zero](./zero.md).  From A to Z, all will sharpen your thinking
and help you use units libraries more effectively.

- **[Arithmetic Operations](./arithmetic.md)**.  Find out about the two kinds of math you can do
  with quantities, and learn a simple rule for reasoning about the results of products and
  quotients.

- **[Common Units](./common_unit.md)**.  To compare, add, or subtract quantities expressed in
  different units, the first step is to convert them to the same unit.  But which one?  This page
  teaches you what choice we make, and what advantages it has.

- **[Conversion Risks](./conversion_risks.md)**.  Unit conversions can be lossy.  This page explains
  how to think about conversoin lossiness: both for individual input values, and for the overall
  conversion as a whole.  We'll learn about the safety checks built into Au's APIs, and how to
  circumvent them when necessary.

- **[Dimensionless Units and Quantities](./dimensionless.md)**.  "Dimensionless" isn't the same
  thing as "unitless"; we support dimensionless units, like `Percent`.  Here we explain how the
  library handles these situations, and avoids common pitfalls.

- **[Overflow](./overflow.md)**.  Unit conversions risk overflow.  The degree of risk depends on
  both the conversion factor, and the range of values that fit in the destination type.  Learn how
  different units libraries have approached this problem, including Au's novel contribution, the
  "overflow safety surface".

- **[Quantity Point](./quantity_point.md)**.  An abstraction for "point types" that have units.
  Most use cases don't need this, but for a few --- including temperatures --- it's indispensable.

- **[Truncation](./truncation.md)**.  Unit conversions truncate when the destination type can't hold
  the result with acceptable precision.  This page will explain Au's approach to managing this risk,
  both for individual values, and for the overall conversion.

- **[Zero](./zero.md)**.  Switching to a units library can make some easy computations hard.  Learn
  how we make them easy again with a special constant, `ZERO`, that works with quantities of any
  units.
