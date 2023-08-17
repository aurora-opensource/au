# `nholthaus/units` Interoperation

The Au repository includes ready-made conversion machinery for the
[nholthaus/units](https://github.com/nholthaus/units) library.  If you're migrating to or from this
library, you don't need to use our [more general guide](./index.md).  Instead, you can follow this
guide, which is both more specific and less labor-intensive.

Once you're finished with this guide, you'll be able to pass any nholthaus type to an API expecting
an equivalent Au quantity type, and vice versa!

## Step 1: Add the file to your project

The goal of this step is to make sure the file with the conversion machinery is available in your
project, and to know its include path.

This will depend on which [installation method](../../install.md) you used: single-file, or full
install.

- **Full Install:**  In this case, the file is already available.  Assuming `au/` is the root of
  your Au includes, the include path will be `"au/compatibility/nholthaus_units.hh"`.

- **Single-file Install:**  Copy the file `compatibility/nholthaus_units.hh` from the Au repository
  into a suitable location in your project.  Make a note of the include path.

## Step 2: Create a shim

The file we provide, `compatibility/nholthaus_units.hh`, is written in a particular and unusual way:
it doesn't have any explicit dependencies on either Au or nholthaus units.  This is because we can't
know how you've included either library in your project.

One consequence of this is that _include order matters_: this file must be included **after** both
the Au and nholthaus libraries.  In general, include order dependence is very fragile and should be
avoided.  _However_, it can be done safely if we confine this dependence to a single file --- the
_shim_ --- and provide _only that file_ to our end users.  This shim must do the following three
things, in this order.

1. Include the nholthaus-relevant parts of Au (in whatever manner is appropriate for your project).
2. Include the nholthaus units library (in whatever manner is appropriate for your project).
3. Include your copy of `compatibility/nholthaus_units.hh`, **after** every other file.
    - (The include path comes from Step 1 above.)

We have provided a [self-explaining example
shim](https://github.com/aurora-opensource/au/blob/3fa22a2212b37322a8243d57db024bb573b68813/compatibility/nholthaus_units_example_usage.hh)
to illustrate this procedure.  Note that it includes examples of comments to disable and re-enable
clang-format support, because without them, some projects that use clang-format might automatically
reorder the headers.

!!! tip
    Any users who include this file will get the full nholthaus library for free, along with the
    conversion machinery.  If you're migrating _from_ nholthaus _to_ Au, you may want to consider
    renaming your original nholthaus file, and giving this shim its original name!  That way, it
    will be as easy as possible for your nholthaus users to start using the new Au constructs.

## Outcome and limitations

Any user who includes the shim created in Step 2 will have low-friction interoperability between Au
and the nholthaus library, for a wide variety of units.  Each library's types will safely and
automatically convert to their counterparts in the other library, even across API boundaries!

Here are the main known limitations.

1. We don't support automatic conversion with any nholthaus type with a nonzero "linear offset".
   This mainly affects temperatures in Celsius and Fahrenheit.  The reason is that the nholthaus
   types for these units act like a mosaic of `au::Quantity` and `au::QuantityPoint` features, and
   we can't guess which one we should convert to.  (Even if we could, Au does not currently provide
   conversion machinery for `au::QuantityPoint`.)

2. We don't support converting non-linear units, such as decibels, because we haven't yet figured
   out how to implement them in Au (see [#41](https://github.com/aurora-opensource/au/issues/41)).
   Thus, there's nothing in Au to convert them _to_.
