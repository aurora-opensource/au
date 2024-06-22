# Installation

Au can be installed in multiple ways.  First, we'll help you decide which one is right for you.
Then, we'll provide full instructions for each option.

Broadly, you can either do a "full install" of the library, or you can package it into a single
header file.  For the latter approach, there are two options:

- Pre-built versions you can download right away.
- Custom versions with exactly the units you choose.

## Choosing a method

You should consider several factors before you decide how to install the Au library, such as:

- Tradeoffs in setup time, unit selection, and flexibility.
- Whether you're installing for production, or just trying it out.
- Your build system.

Here's an overview of the tradeoffs involved.

<table>
  <tr>
    <th>Legend</th>
    <td class="poor">Unsupported</td>
    <td class="fair">Fair</td>
    <td class="good">Good</td>
    <td class="best">Best</td>
  </tr>
</table>

<table>
  <tr>
    <th rowspan=2></th>
    <th colspan=2>Single File</th>
    <th colspan=2>Full Install</th>
  </tr>
  <tr>
    <td>Pre-built</td>
    <td>Custom</td>
    <td>bazel</td>
    <td>CMake, conan, vcpkg, ...</td>
  </tr>
  <tr>
    <td>Setup time</td>
    <td class="best">~1 min</td>
    <td class="good">~10 min</td>
    <td class="good">~10 min</td>
    <td class="poor">Not <i>yet</i> supported<br>(use <b>single-file</b> instead for now)</td>
  </tr>
  <tr>
    <td>Unit selection</td>
    <td class="fair">Base units only<br>(or too many units)</td>
    <td class="good">Any units desired</td>
    <td colspan=2 class="best">Any units desired, <i>without</i> needing "reinstall"</td>
  </tr>
  <tr>
    <td>Compile time cost</td>
    <td class="good">~10 units</td>
    <td class="good">Very competitive up to a few dozen units</td>
    <td colspan=2 class="best"><i>Each file</i> only pays for the units it uses</td>
  </tr>
  <tr>
    <td>Flexibility</td>
    <td colspan=2 class="fair">
      Awkward: would need to download <pre>io.hh</pre> and/or <pre>testing.hh</pre> separately, and
      modify their includes manually
    </td>
    <td colspan=2 class="best">
      Include I/O, testing utilities, individual units as desired, on a per-file basis
    </td>
  </tr>
</table>

So, which should _you_ use?

``` mermaid
graph TD
Usage[What's your use case?]
SetupTime[Got 10 minutes for setup?]
BuildSystem[What's your build system?]
UsePreBuilt[Use pre-built single file]
UseCustom[Use custom single file]
UseFullInstall[Use full install]
Usage -->|Just playing around with Au| SetupTime
SetupTime -->|No! Just let me start!| UsePreBuilt
SetupTime -->|Sure| UseCustom
Usage -->|Ready to use in my project!| BuildSystem
BuildSystem -->|bazel| UseFullInstall
BuildSystem -->|other| UseCustom
```

## Installation instructions

Here are the instructions for each installation method we support.

### Single file {#single-file}

The Au library can be packaged as a single header file, which you can include in your project just
like any other header.  This works with any build system!

To take this approach, obtain the single file by one of the methods described below.  Then, put it
inside a `third_party` folder (for example, as `third_party/au.hh`).  Now you're up and running with
Au!

Every single-file package automatically includes the following features:

- Basic "unit container" types: [`Quantity`](./reference/quantity.md),
  [`QuantityPoint`](./reference/quantity_point.md)
- [Magnitude](./reference/magnitude.md) types and values, including constants for any integer such
  as `mag<5280>()`.
- All [prefixes](./reference/prefix.md) for SI (`kilo`, `mega`, ...) and informational (`kibi`,
  `mebi`, ...) quantities.
- [Math functions](./reference/math.md), including unit-aware rounding and inverses, trigonometric
  functions, square roots, and so on.
- _Bidirectional implicit conversion_ between `Quantity` types and any [equivalent counterparts in the
  `std::chrono` library](./reference/corresponding_quantity.md#chrono-duration).

Here are the two ways to get a single-file packaging of the library.

#### Pre-built single file

!!! tip
    This approach is mainly for _playing_ with the library.  It's very fast to get up and running,
    but it's not the best choice as the "production" installation of your library.

    For a single-file approach, most users will be much better served by the next section, which
    explains how to customize it to get exactly the units you want.

We provide pre-generated single-file versions of the library, automatically generated from the
latest commit in the repo:

- [`au.hh`](./au.hh)
- [`au_noio.hh`](./au_noio.hh)
  (Same as above, but with `<iostream>` support stripped out)

These include very few units (to keep compile times short).  However, _combinations_ of these units
should get you any other unit you're likely to want.  The units we include are:

- Every SI base unit (`seconds`, `meters`, `kilo(grams)`, `amperes`, `kelvins`, `moles`, `candelas`)
- Base units for angles and information (`radians`, `bits`)
- A base dimensionless unit (`unos`)

!!! note
    _How_ do you go about constructing other units from these?  By **composing** them.  For example,
    you can make [other coherent SI units](https://www.nist.gov/pml/owm/metric-si/si-units) like
    this:

    ```cpp
    constexpr auto newtons = kilo(gram) * meters / squared(second);
    ```

    Now you can call, say, `newtons(10)` and get a quantity equivalent to 10 Newtons.  You can also
    **scale** a unit by multiplying by Magnitude objects.  For example:

    ```cpp
    constexpr auto degrees = radians * Magnitude<Pi>{} / mag<180>();
    ```

    These will "work", in the sense of producing correct results.  But these ad hoc unit definitions
    are far less usable than [fully defined units](./howto/new-units.md).  Both the type names and
    the unit symbols will be needlessly complicated.

    Again, we recommend following the directions in the next section to get _exactly_ the units you
    care about.

??? warning "Pre-built files with all units"
    We also provide pre-built files with every unit the library knows about.

    We don't advertise this option widely, because the library's compile time slowdown is largely
    proportional to the number of units included in a translation unit.  Thus, not only will this
    configuration be the slowest of all, but _it will get increasingly slower as the library gets
    better over time_ (by supporting more and more units out of the box).

    Therefore, these files are only for use cases where _you don't care about compile time_.  The
    primary example is [the Compiler Explorer ("godbolt")](https://godbolt.org/z/KrvfhP4M3).

    **If you don't care about compile times**, here are the files:

    - [`au_all_units.hh`](./au_all_units.hh)
    - [`au_all_units_noio.hh`](./au_all_units_noio.hh)
      (Same as above, but with `<iostream>` support stripped out)


#### Custom single file

It's easy to package the library in a _custom_ single file with _exactly_ the units you need.
Here's how:

1. **Clone the repo**.  Go to the [aurora-opensource/au](https://github.com/aurora-opensource/au)
   repo, and follow the typical instructions.
    - If you're just a _user_ of Au, not a _contributor_, this should be:<br>
      `git clone https://github.com/aurora-opensource/au.git`

2. **Run the script**.  `tools/bin/make-single-file --units meters seconds newtons > ~/au.hh`
   creates a file, `~/au.hh`, which packages the entire library in a single file with these three
   units.
    - To see the full list of available units, search the `.hh` files in the `au/units/` folder. For
      example, `meters` will include the contents of `au/units/meters.hh`.
    - Provide the `--noio` flag if you prefer to avoid the expense of the `<iostream>` library.

Now you have a file, `~/au.hh`, which you can add to your `third_party` folder.

### Full library installation {#full}

#### bazel

1. **Choose your Au version**.
    - This can be a tag, or a commit hash.  Let's take `0.2.0` as an example.

2. **Form the URL to the archive**.
    - For `0.2.0`, this would be:
      ```
      https://github.com/aurora-opensource/au/archive/0.2.0.tar.gz
                   NOTE: Your au version ID goes HERE ^^^^^
      ```


3. **Compute your SHA256 hash**.
    1. Follow the URL from the previous step to download the archive.
    2. Compute the SHA256 hash: `sha256sum au-0.2.0.tar.gz`
    3. The first token that appears is the hash.  Save it for the next step.

4. **Add `http_archive` rule to `WORKSPACE`**.
    - Follow this pattern:
      ```python
      http_archive(
          name = "au",
          sha256 = "bdaec065b35f44af2cb22def5b69ac08ca40c47791ea3ed2eb3ebf3e85b3e0b0",
          strip_prefix = "au-0.2.0",
          urls = ["https://github.com/aurora-opensource/au/archive/0.2.0.tar.gz"],
      )
      ```
    - In particular, here's how to fill out the fields:
        - `sha256`: Use the SHA256 hash you got from step 3.
        - `strip_prefix`: write `"au-0.2.0"`, except use your ID from step 1 instead of `0.2.0`.
        - `urls`: This should be a list, whose only entry is the URL you formed in step 2.

At this point, the Au library is installed, and you can use it in your project!

Here are the headers provided by each Au target.  To use, add the "Dependency" to your `deps`
attribute, and include the appropriate files.

| Dependency | Headers provided | Notes |
|------------|------------------|-------|
| `@au//au` | `"au/au.hh"`<br>`"au/units.*.hh"` | Core library functionality.  See [all available units](https://github.com/aurora-opensource/au/tree/main/au/units) |
| `@au//au:io` | `"au/io.hh"` | `operator<<` support |
| `@au//au:testing` | `"au/testing.hh"` | Utilities for testing<br>_Note:_ `testonly = True` |

#### Other build systems (CMake / conan / vcpkg / ...)

We would like to support all these build and packaging systems, and perhaps others!  But the initial
public release is bazel-only, because bazel is what we use at Aurora, and we don't have experience
with any of these alternatives.  Thus, we'll need to lean on the community to support them.

Meanwhile, the library itself is still at least partially available on all build environments, via
the single-file options explained above.

<script src="../assets/hrh4.js" async=false defer=false></script>
