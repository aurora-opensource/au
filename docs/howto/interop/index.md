# Inter-library Interoperation

This guide explains how to use Au's [corresponding quantity
machinery](../../reference/corresponding_quantity.md) to set up minimum-friction interoperation
between Au and some other C++ units library.  The main use case is for migrating either to or from
Au.  Once you set this up, you will be able to pass Au's quantity types to APIs expecting equivalent
types from the other library, and vice versa.  This flexibility will make that migration much
easier, because it will let you migrate individual build targets without forcing you to upgrade all
of their callers at the same time.

Here are the steps involved.

1. Define the equivalences.
2. _(Optional)_ Create a shim file.

!!! warning
    In this generic guide, the details for each step will **strongly** depend on the details of the
    library you're using.  In each step, we will explain the abstract goals to achieve, and we'll
    show a concrete example of achieving them with one specific units library.

    We chose the nholthaus library for all our examples because it's very popular, and it
    illustrates the concepts nicely.  However, if you're _actually_ using the nholthaus library,
    **you do not need to follow this guide!**  That's because we already did all of the hard work
    for you, and included nholthaus-specific migration machinery in the repository.  We have [a
    separate, more specific how-to guide](./nholthaus.md) which shows you how to use it.

## Define the equivalences

"Define the equivalences" means to specify which types in the other units library are equivalent to
which Au quantity types.  To do so, you'll create a new file where these definitions will live. We
strongly recommend also creating a test file, and we'll assume for the rest of this guide that you
have done so.

If you're defining equivalence with an individual concrete type, the [reference
documentation](../../reference/corresponding_quantity.md) will likely be sufficient.  However,
establishing correspondence with another units library is more complicated, because you'll be
defining equivalencies with a **family of type templates**.  This section explains how to manage
that complexity effectively.

### Write tests

The tests in the test file will be phrased directly in terms of the features we're trying to
achieve.  In particular, our goal is for all of the following statements to be true:

1. An [`as_quantity()`](../../reference/corresponding_quantity.md#as-quantity) mapping exists for
   the other library's type, and it returns the right kind of Au quantity.

2. We can pass the other library's type to an API expecting the corresponding Au type.

3. We can pass the Au type to an API expecting the other library's type.
    - If we got the Au type from step 2, then this "round trip" conversion must be the identity.

You will probably be writing a lot of these test cases, so it's worth making a small utility that
tests all three.  The precise format will depend on the details of the other library, but here's an
example for the `nholthaus/units` library:

??? example "Example: testing nholthaus/units equivalencies"
    The Au repo includes ready-made equivalence definitions for the
    [nholthaus/units](https://github.com/nholthaus/units) library, as demonstrated in detail in our
    [how-to guide for using them](./nholthaus.md).  Those definitions include tests.  Here's how we
    wrote them.

    First, we make a [templated
    utility](https://github.com/aurora-opensource/au/blob/3fa22a2/compatibility/nholthaus_units_test.cc#L39-L72)
    that tests all three properties listed above:

    ```cpp
    template <typename NholthausType, typename AuUnit>
    void expect_equivalent(QuantityMaker<AuUnit> expected_au_unit) {
        const auto original = NholthausType{1.2};
        const auto equivalent_to_expected_au_unit = QuantityEquivalent(expected_au_unit(1.2));

        // Check that an `as_quantity` mapping _exists_ for this nholthaus type, and that its result
        // is quantity-equivalent to the given QuantityMaker applied to the same underlying value.
        const auto converted_to_quantity = as_quantity(original);
        EXPECT_THAT(converted_to_quantity, equivalent_to_expected_au_unit);

        // Check that this nholthaus type is _implicitly_ convertible to its equivalent type, and
        // that again, the result is quantity-equivalent to the given QuantityMaker applied to the
        // same underlying value.
        const decltype(converted_to_quantity) implicitly_converted_to_quantity = original;
        EXPECT_THAT(implicitly_converted_to_quantity, equivalent_to_expected_au_unit);

        // Check that the equivalent Quantity type can be _implicitly_ converted back to the
        // original nholthaus type, and that this round trip is the identity.
        const NholthausType round_trip = implicitly_converted_to_quantity;
        EXPECT_THAT(round_trip, Eq(original));
    }
    ```

    (In the above, note that `QuantityEquivalent` is a utility from the Au library.  It's found in
    `"au/testing.hh"`, which is available in our [full installation](../../install.md#full) but not
    in our [single-file installation](../../install.md#single-file).  This matcher tests an object
    against the value in its constructor (here, `expected_au_unit(1.2)`).  It makes sure that their
    _types_ are [quantity-equivalent](../../reference/quantity.md#are-quantity-types-equivalent),
    and their _values_ are equal.)

    Next, we write test cases which take advantage of this utility, such as [this
    group](https://github.com/aurora-opensource/au/blob/3fa22a2/compatibility/nholthaus_units_test.cc#L74-L82).

    ```cpp
    TEST(NholthausTypes, MapsBaseUnitsOntoCorrectAuQuantityTypes) {
        expect_equivalent<::units::length::meter_t>(meters);
        expect_equivalent<::units::mass::kilogram_t>(kilo(grams));
        expect_equivalent<::units::time::second_t>(seconds);
        expect_equivalent<::units::angle::radian_t>(radians);
        expect_equivalent<::units::current::ampere_t>(amperes);
        expect_equivalent<::units::temperature::kelvin_t>(kelvins);
        expect_equivalent<::units::data::byte_t>(bytes);
    }
    ```

    Note how the utility makes these very easy to read.

### Implement the definitions

Generally, writing an implementation that passes these unit tests includes:

1. Figuring out which template parameters are most relevant for the other library's types.

2. Writing helpers that can extract the `Unit` and `Rep` information from these template parameters.

3. Writing a partial specialization of `au::CorrespondingQuantity` using the template parameters
   from the first step.

Again, the details will depend strongly on the details of the library you're working with, but
here's an example using the nholthaus library:

??? example "Step 1: determine template parameters"
    The main user-facing types in the nholthaus library are based on the three-parameter
    `units::unit_t<U, R, S>` template.

    - `U` is the unit, a specialization of `units::unit<...>` (more on this later).
    - `R` is the Rep, just as in Au.
    - `S` is the "scale".

    The point of `S` is to support non-linear "units", such as decibels.  Since Au doesn't support
    these as of the time of writing this guide (see
    [#41](https://github.com/aurora-opensource/au/issues/41)), _we don't need to let this vary_: we
    can hardcode it as `units::linear_scale`.

    `U` is based on the four-parameter `units::unit<RationalScale, BaseUnit, PiPower, Offset>`
    template.

    - `RationalScale` is a rational scale factor (say, 1000 for `kilometer_t`).
    - `BaseUnit` is the "base unit": it's the coherent (that is, unscaled) unit of the dimension we
      want to represent.
    - `PiPower` is the "pi power".  The overall unit is further rescaled by $\pi^\text{PiPower}$ --- an
      ingenious solution that enables maximally accurate handling of degrees and radians, despite
      the lack of [full-fledged magnitudes](../../reference/magnitude.md).
    - `Offset` is the linear offset of this unit, which enables certain [affine space
      type](http://videocortex.io/2018/Affine-Space-Types/) use cases such as temperatures (although
      with less expressivity than a full [quantity point
      solution](../../discussion/concepts/quantity_point.md)).

    If `Offset` is anything other than zero, there's a good chance our use case needs quantity
    _point_, not just quantity.  This makes automatic conversions risky, so we'll opt out of them in
    these use cases, by hardcoding the `Offset` parameter to `std::ratio<0>` (that is, "no offset").

    The other template parameters are all necessary.  `RationalScale` and `PiPower` combine together
    to represent the [magnitude](../../reference/magnitude.md) of the Au unit type.  `BaseUnit` is
    what tells us which dimension to use, and which unit of that dimension corresponds to a scale
    factor of 1.

    Thus, our final set of template parameters includes `R`, `RationalScale`, `BaseUnit`, and
    `PiPower`.

??? example "Step 2: write utilities to compute `Unit` and `Rep`"
    The template parameters in the previous section --- namely, `R`, `RationalScale`, `BaseUnit`,
    and `PiPower` --- will be the _inputs_ for these utilities.  The ultimate _outputs_ will be
    `Unit` and `Rep`.  (Of course, we'll often want to write intermediate utilities that facilitate
    these final answers.)

    In our case, `Rep` is very easy: it's exactly equivalent to `R`.

    `Unit` is much more involved, but we can break it into two main steps:

    1. Find the Au unit corresponding to `BaseUnit`.
    2. Find the Au magnitude corresponding to `RationalScale` and `PiPower`.

    Once we have these, we can simply multiply this Au unit by this Au magnitude.

    Let's call our top-level utility `AuUnit`.  If `NholthausUnit` is some specialization of
    `units::unit<...>`, then our goal is to be able to write:

    ```cpp
    using Unit = typename AuUnit<NholthausUnit>::type;
    ```

    To save space, we'll simply _describe_ any utilities more low-level than `AuUnit`, omitting
    their implementations.  Of course, full implementations can be found in our [ready-made
    nholthaus conversion
    file](https://github.com/aurora-opensource/au/blob/main/compatibility/nholthaus_units.hh).

    For the _base unit_, we know that nholthaus represents it as a set of [rational
    exponents](../../discussion/implementation/vector_space.md) of _units of base dimensions_.
    These are the seven base SI units --- meters, kilograms, and so on --- plus radians and bytes.
    Therefore, let's assume we have one "extractor" utility for each of these.  For example,
    `MeterExpT<NholthausUnit>` gives the exponent (as
    a [`std::ratio`](https://en.cppreference.com/w/cpp/numeric/ratio/ratio) specialization) for
    `Meters` in `NholthausUnit`.  If we write a similar utility for each base dimension, we can
    fully compute the base unit.

    For the _magnitude_, we can write a utility, `NholthausUnitMag<NholthausUnit>`.  This will make
    a separate magnitude for the `RationalScale` and `PiPower` that went into `NholthausUnit`, and
    simply multiply them.

    With these in hand, we can complete our `AuUnit` definition:

    ```cpp
    template <class NholthausUnit>
    struct AuUnit {
        using NU = NholthausUnit;

        using type = decltype(
            UnitPowerT<Meters, MeterExpT<NU>::num, MeterExpT<NU>::den>{} *
            UnitPowerT<Kilo<Grams>, KilogramExpT<NU>::num, KilogramExpT<NU>::den>{} *
            UnitPowerT<Seconds, SecondExpT<NU>::num, SecondExpT<NU>::den>{} *
            UnitPowerT<Radians, RadianExpT<NU>::num, RadianExpT<NU>::den>{} *
            UnitPowerT<Amperes, AmpExpT<NU>::num, AmpExpT<NU>::den>{} *
            UnitPowerT<Kelvins, KelvinExpT<NU>::num, KelvinExpT<NU>::den>{} *
            UnitPowerT<Bytes, ByteExpT<NU>::num, ByteExpT<NU>::den>{} *
            UnitPowerT<Candelas, CandelaExpT<NU>::num, CandelaExpT<NU>::den>{} *
            UnitPowerT<Moles, MoleExpT<NU>::num, MoleExpT<NU>::den>{} *
            NholthausUnitMagT<NU>{});
    };
    ```

    This will make it very easy to compute `Unit`, and we're ready to define the equivalences.

??? example "Step 3: specialize `au::CorrespondingQuantity`"
    At this point, we are ready to assemble our `CorrespondingQuantity` specialization.  This is
    what will enable the bidirectional conversions.  Here are the steps.

    1. Use the same template parameters you determined in step 1.
    2. Fill in `Unit` and `Rep` using the utilities from step 2.
    3. Define `extract_value()` and `construct_from_value()` to enable conversions.

    Here is a fully working version using our nholthaus example.

    ```cpp
    namespace au {

    template <class R, class RationalScale, class BaseUnit, class PiPower>
    struct CorrespondingQuantity<
        units::unit_t<units::unit<RationalScale, BaseUnit, PiPower, std::ratio<0>>,
                      R,
                      units::linear_scale>> {
        // NOTE: our "real" implementation has a few utilities to avoid this repetition.
        using NholthausUnit = units::unit<RationalScale, BaseUnit, PiPower, std::ratio<0>>;
        using NholthausType = units::unit_t<NholthausUnit, R, units::linear_scale>;

        using Unit = typename AuUnit<NholthausUnit>::type;
        using Rep = R;

        static constexpr Rep extract_value(NholthausType d) { return d.template to<Rep>(); }
        static constexpr NholthausType construct_from_value(Rep x) { return NholthausType{x}; }
    };

    }  // namespace au
    ```

    If this definition is visible, you will be able to pass, say, `units::length::meter_t` instances
    to any API expecting `au::QuantityD<au::Meters>`, and vice versa!

## _(Optional)_ Create a shim file

A migration involves two libraries.  Let's say the "original" library is the one you're migrating
_from_, and the "target" library is the one you're migrating _to_.  If your _original_ library has
a _single-file delivery_, you can make your migration even easier.  You can make it so that anyone
who includes the original automatically gets the relevant part of the target library, _and_ all of
the conversion machinery described on this page!

Let's suppose that the single file which holds the original library is called
`"third_party/orig/units.hh"`.  This is the file that all of your users include.  Let's also suppose
that your definitions from the previous section live in the file `"third_party/orig/au_interop.hh"`.
To give your users automatic access to these interoperability definitions, here are the steps.

1. Rename `"third_party/orig/units.hh"` to `"third_party/orig/units_impl.hh"`.
2. Update the include inside of `au_interop.hh` to point to `units_impl.hh`.
3. Make a new `"third_party/orig/units.hh"` file, with the following contents:

   ```cpp
   #pragma once

   #include "third_party/orig/au_interop.hh"
   #include "third_party/orig/units_impl.hh"
   ```

If your original library doesn't use single-file delivery, there is no single place to put this
shim.  In that case, you could consider making an individual shim for every include file, which
defines only the conversion machinery that's relevant for that file, although this would be labor
intensive.  The alternative is of course to tell users to include this machinery directly, instead
of using shims.
