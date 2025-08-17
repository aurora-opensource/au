# Installation

Au can be installed in multiple ways.  First, we'll help you decide which one is right for you.
Then, we'll provide full instructions for each option.

Broadly, you can either do a "full install" of the library, or you can package it into a single
header file.  For the latter approach, there are two options:

- Pre-built versions you can download right away.
- Custom versions with exactly the units you choose.

## Choosing a method

These days, Au supports both bazel and CMake build systems natively.  We also have community support
for the most popular C++ package managers, conan and vcpkg.  Setup via any of these methods is
pretty quick, so **just doing a full install is usually best**.

The main reason to consider a single-file approach is if you're not using any of these build
systems: clearly, a single file works with any build system imaginable.  The _pre-built_ single file
packages are also the quickest way to start playing with the library.

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
    <th colspan=2>Full Install</th>
    <th colspan=2>Single File</th>
  </tr>
  <tr>
    <td>bazel, CMake, conan, vcpkg</td>
    <td>Other build systems</td>
    <td>Pre-built</td>
    <td>Custom</td>
  </tr>
  <tr>
    <td>Setup time</td>
    <td class="good">Fast (a few minutes)</td>
    <td rowspan="4" class="poor">Full Install unsupported (use single-file instead)</td>
    <td class="best">Instant</td>
    <td class="good">Fast (a few minutes)</td>
  </tr>
  <tr>
    <td>Unit selection</td>
    <td class="best">Any units desired, <i>without</i> needing "reinstall"</td>
    <td class="fair">Base units only<br>(or too many units)</td>
    <td class="good">Any units desired</td>
  </tr>
  <tr>
    <td>Compile time cost</td>
    <td class="best"><i>Each file</i> only pays for the units it uses</td>
    <td class="good">Cost of core, plus ~10 units</td>
    <td class="good">Very competitive up to a few dozen units</td>
  </tr>
  <tr>
    <td>Flexibility</td>
    <td class="best">
      Include I/O, testing utilities, individual units as desired, on a per-file basis
    </td>
    <td colspan=2 class="fair">
      Awkward: would need to download <pre>io.hh</pre> and/or <pre>testing.hh</pre> separately, and
      modify their includes manually
    </td>
  </tr>
</table>

## Installation instructions

Here are the instructions for each installation method we support.

### Full library installation {#full}

#### bazel

1. **Choose your Au version**.
    - This can be a tag, or a commit hash.  Let's take `0.3.5` as an example.

2. **Form the URL to the archive**.
    - For `0.3.5`, this would be:
      ```
      https://github.com/aurora-opensource/au/releases/download/0.3.5/au-0.3.5.tar.gz
                             NOTE: Your au version ID goes HERE ^^^^^    ^^^^^
      ```


3. **Compute your SHA256 hash**.
    1. Follow the URL from the previous step to download the archive.
    2. Compute the SHA256 hash: `sha256sum au-0.3.5.tar.gz`
    3. The first token that appears is the hash.  Save it for the next step.

4. **Add `http_archive` rule to `WORKSPACE`**.
    - Follow this pattern:
      ```python
      http_archive(
          name = "au",
          sha256 = "7ec826dc42968dc1633de56e4f9d06e70de73e820d2ac4788e8453343a622c9b",
          strip_prefix = "au-0.3.5",
          urls = ["https://github.com/aurora-opensource/au/releases/download/0.3.5/au-0.3.5.tar.gz"],
      )
      ```
    - In particular, here's how to fill out the fields:
        - `sha256`: Use the SHA256 hash you got from step 3.
        - `strip_prefix`: write `"au-0.3.5"`, except use your ID from step 1 instead of `0.3.5`.
        - `urls`: This should be a list, whose only entry is the URL you formed in step 2.

At this point, the Au library is installed, and you can use it in your project!

Here are the headers provided by each Au target.  To use, add the entry from the "Dependency" column
to your `deps` attribute, and include the appropriate files.

| Dependency | Headers provided | Notes |
|------------|------------------|-------|
| `@au//au` | `"au/au.hh"`<br>`"au/fwd.hh"`<br>`"au/units/*.hh"`<br>`"au/units/*_fwd.hh"`<br>`"au/constants/*.hh"` | Core library functionality.  See [all available units](https://github.com/aurora-opensource/au/tree/main/au/units) and [constants](./reference/constant.md#built-in) |
| `@au//au:io` | `"au/io.hh"` | `operator<<` support |
| `@au//au:std_format` | `"au/std_format.hh"` | `std::format` support[^1] |
| `@au//au:testing` | `"au/testing.hh"` | Utilities for writing googletest tests<br>_Note:_ `testonly = True` |

#### CMake

There are two ways to include the Au library in your CMake project.

1. **(Recommended)** Use the `FetchContent` module to download the library directly from GitHub.

2. Install the library to the system, and use `find_package`.

We recommend `FetchContent` because each project can get the exact version of Au that they need, and
can update it independently of other projects.  `FetchContent` also means you don't need to manually
clone the Au repo, or build and run the tests.  On the other hand, if you want a single global
system-wide version of Au, then you can install it to the system, and simply use `find_package`.

In either case, here are the main targets and include files provided by the Au library:

| Target | Headers provided | Notes |
|--------|------------------|-------|
| `Au::au` | `"au/au.hh"`<br>`"au/fwd.hh"`<br>`"au/io.hh"`<br>`"au/std_format.hh"`[^1]<br>`"au/units/*.hh"`<br>`"au/units/*_fwd.hh"`<br>`"au/constants/*.hh"` | Core library functionality.  See [all available units](https://github.com/aurora-opensource/au/tree/main/au/units) |
| `Au::testing` | `"au/testing.hh"` | Utilities for writing googletest tests |

[^1]: Do not include `"au/std_format.hh"` unless you know that both your compiler and your build
configuration fully supports `std::format`.  This requires at least C++20, but many compilers with
nominal C++20 support do not actually support `std::format`.

!!! note
    These instructions are for adding Au to a _project_ that uses CMake, not building Au itself
    using CMake.

    Au is a bazel-first project, so most Au development would normally be done using bazel.
    However, we do have instructions for doing this with CMake as well: see the `find_package` tab
    below.

=== "Using `FetchContent`"
    Add the following to your `CMakeLists.txt` file:

    ```cmake
    include(FetchContent)
    FetchContent_Declare(
      Au
      GIT_REPOSITORY https://github.com/aurora-opensource/au
      GIT_TAG "main"  # Or a specific tag.
      EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(Au)
    ```

    You should now be able to depend on Au targets, such as `Au::au` or `Au::testing`, and include
    headers from them, such as `#include "au/au.hh"` or `#include "au/testing.hh"`.

=== "Using `find_package`"
    Before you can use `find_package`, you need to install the library to your system.  This means
    cloning the repo, building the library, running the tests, and installing it.

    First, clone the repository.

    ```sh
    git clone https://github.com/aurora-opensource/au.git
    cd au
    ```

    If you want a specific release, check out the tag you want.  Note that the first version of Au
    that supports CMake is 0.3.5.

    ```sh
    # Optional, but recommended:
    git checkout "0.4.1"  # Or whichever tag you prefer.
    ```

    Now, build and test the library.  These commands will include both the explicit tests, and also
    several CMake-generated tests to make sure that the includes are set up correctly.

    ```sh
    # CMake is a "meta build system", not a build system.
    # This first command generates the actual build files.
    cmake -S . -B cmake/build -DCMAKE_VERIFY_INTERFACE_HEADER_SETS=TRUE

    # This command builds Au, checks include paths, and runs unit tests.
    cmake \
      --build cmake/build \
      --target \
        all \
        all_verify_interface_header_sets \
        test
    ```

    If the tests pass, you can install the library to your system.

    ```sh
    sudo cmake --install cmake/build
    ```

    At this point, the `Au` CMake library is installed to your system, and can be found via the
    usual `find_package` mechanism!

    ```cmake
    find_package(Au)
    ```

<script src="../assets/hrh4.js" async=false defer=false></script>

#### Package managers (conan, vcpkg)

If you're using these in your project, we assume you already know how to add new libraries.  These
methods have "community support", which means:

- The recipes are added and maintained by external users
- Au's maintainers will provide our _best effort_ to respond to issues, but we'll need to rely on
  users of these package managers to help us reproduce and understand them

Each package manager contains setup instructions on its page for Au.  Here are the packages:

- **Conan:** [au](https://conan.io/center/recipes/au)
- **vcpkg:** [aurora-au](https://vcpkg.io/en/package/aurora-au)

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
      example, `meters` will include the contents of `"au/units/meters.hh"`.
    - Similarly, to see the full list of available constants, search the `.hh` files in the
      `au/constants/` folder.  For example, `speed_of_light` will include the contents of
      `"au/constants/speed_of_light.hh"`, which provides the constant `au::SPEED_OF_LIGHT`.
    - Provide the `--noio` flag if you prefer to avoid the expense of the `<iostream>` library.

Now you have a file, `~/au.hh`, which you can add to your `third_party` folder.
