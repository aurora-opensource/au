<!--
This page isn't for our published doc website.  I had Claude create and maintain it to give guidance
to contributors (including "future us") who want to add more examples.
-->

# Code examples

The source behind the [Code examples](https://aurora-opensource.github.io/au/main/examples/)
section of the doc website.

Every code block on those pages is inlined from a file here, via `pymdownx.snippets`.  Nothing on
those pages is hand-copied, so nothing there can drift out of date with the library without a test
going red.  The README is the one exception, and it has its own drift check.

There are two flavors.  Most examples are an **A/B pair** (`ab_example`) --- the next two sections
are about those.  A few are **standalone** (`single_example`), for when the example's subject *is*
Au and there is no plain-C++ version to compare against; see
[Standalone examples](#standalone).

## The two invariants

An A/B example is a **pair**: `raw.cc` (plain C++, no units library) and `au.cc` (the same problem,
using Au).  The website shows them as a pair of tabs, and the reader flips between them to compare
corresponding lines *in place*.  That presentation only works if two things stay true, and
`check_ab_example.sh` enforces both:

1. **Both programs produce identical output.**  This is what lets the pages claim "same answer,
   better code" without asking the reader to take our word for it.

2. **The doc-visible regions have the same number of lines**, with corresponding constructs on
   corresponding lines.  Where Au needs less code, pad with a blank line.  The resulting empty
   space is the point --- it falls exactly where the raw version needed work.

Note what invariant 2 is *not*: it is not "pad both files to equal length."  Align at structural
landmarks (includes, signature, the key computation, the closing brace) and let genuine differences
show.  Hiding a real difference under padding would defeat the example.

**Padding at the very top or bottom of a region does nothing.**  The renderer drops leading and
trailing blank lines from a code block, so blanks there buy no height on the page.  Put padding
between two non-blank lines, where it survives --- or restructure so it isn't needed, which is why
`angular_velocity` keeps its unit aliases in `frontmatter` rather than at the head of the region.
`check_ab_example.sh` counts the way the renderer does, so it will catch this; an earlier dev
version counted raw source lines and called a visibly broken page aligned.

## Adding an A/B example

1. Write `<name>/raw.cc` and `<name>/au.cc` as complete, runnable programs.  Mark the doc-visible
   region of each with `// --8<-- [start:example]` and `// --8<-- [end:example]`.

2. **Fence the region off from clang-format.**  Put `// clang-format off` / `// clang-format on`
   *outside* the snippet markers, so they never appear on the website.  Without this, clang-format
   collapses short function bodies onto one line and silently destroys the alignment.  Format the
   region by hand, in house style.

3. Register it in `BUILD.bazel` with `ab_example(...)`, giving the exact expected stdout.  The Au
   side needs `//au:io` in its deps if it prints (see below).

4. Add `":<name>_doc_sources"` to the `doc_sources` filegroup at the top of `BUILD.bazel`.  Both
   macros define that filegroup for you; listing it there is the one explicit edit per example, and
   it is what lets the docs build see the source.

5. Add a page under `docs/examples/`, modeled on `angular-velocity.md`: problem statement, tabs,
   "What's happening", "Related reading".  Link it from `docs/examples/index.md`.

One rule for the doc page:

- **Keep each banner to one short line.**  An uneven banner shifts that tab's code down and ruins
  the comparison.  The actual explanation belongs under "What's happening", where its height costs
  nothing.

Reuse the existing tab labels --- `⚠️ Before: raw C++` and `✅ After: with Au` --- so the pages look
of a piece.  Nothing breaks if you don't, but it's good to be consistent.

Nested regions work, and inner markers are stripped from the outer region's output.  That is how
`angular_velocity` exposes short `headline` and `aliases` regions (used by the README and the docs
front page) inside the full `example` region.

## Standalone examples {#standalone}

Reach for `single_example` when there is nothing meaningful to compare against, because the
example's subject *is* Au rather than a computation Au happens to improve.  `atomic_units` is the
case that motivated it: it builds a whole new system of units on top of the library, and a
"plain C++" version of that is not a thing that exists.

The *second* invariant above does not apply: there is no second tab, so there is nothing to line up.
The first one does, in adapted form --- the single program must print exactly `expected_output` ---
and it is the whole test.  `check_output.sh` performs the check (and `check_ab_example.sh` calls that same
script once per program).

The layout differs from an A/B example in one way.  Where a pair is two standalone programs, a
standalone example usually *defines* something reusable, so `single_example` splits it:

- `hdrs` and `lib_srcs` become a `cc_library`.  In C++14 `lib_srcs` is where the out-of-line unit
  label definitions go (`constexpr const char Hartrees::label[];`); a C++17 project would not need
  the file at all.
- `main_src` (default `<name>/main.cc`) becomes the program, depending on that library.

In other words, this will build it the way a reader's project would.  The doc page for `atomic_units` invites
people to copy the files straight into their own code, and that invitation should be backed by a
target shaped like the one they will write.

To add an example: write the files, register with `single_example(...)`, add `":<name>_doc_sources"` to the
`doc_sources` filegroup, and write the page.  Say on the index page why the example has no
"before" version, so its entry does not read like an omission.

## Namespaces, and the `frontmatter` region

Every _program_ file here carries a `frontmatter` region --- includes, plus the `using`
declarations --- marked the same way as `example`, and every doc page shows it in a collapsed
"Includes and usings" block.  Add one to any new example's program.

For an A/B pair that means both `raw.cc` and `au.cc`, and the collapsing is what makes it safe to
show in both tabs: collapsed blocks are the same height in each, so the alignment of the code below
survives.  A standalone example has one program, so it carries exactly one region and there is no
alignment to protect.  The files such an example *defines* --- a header, plus the out-of-line
definitions beside it --- carry regions of their own instead (`definitions`, `assert`, `labels`),
and no frontmatter: their includes are part of what the example is teaching.

The frontmatter exists such that we can follow our style guidance in these examples without
distracting from the example itself:

- **`.cc` files import names individually** --- `using au::meters;`, one per line, GoogleTest-style.
  **No `using namespace au;`.**  We do not recommend it to users, so we do not model it: `au::` is
  two characters, and a directive that pulls in every unit, maker, and symbol is a bad trade for
  them.

- **`.hh` files qualify instead.**  A namespace-scope `using` in a header leaks transitively into
  every consumer, and the resulting ambiguities surface far from the cause.  Where qualifying would
  drown a formula --- unit symbols, mostly --- put a `using` *inside the function body*, where it
  expires at the closing brace.  Type aliases are fine at namespace scope either way, since they
  introduce one name you chose rather than importing a set you didn't.

## Printing

Every example prints with `std::cout`, and exits via `.as(unit)` rather than `.in(unit)`, so the
value stays a `Quantity` right up to the stream.  Two things fall out of that, and both are
deliberate:

- **The unit label is derived, not typed.**  `omega.as(revolutions / minute)` prints
  `409.256 rev / min` on its own.  The raw side has to hand-write that string, which is exactly the
  kind of manual bookkeeping these examples exist to contrast --- so let it, and say so in a comment
  rather than quietly making both sides print something neutral.

- **Don't reach for `setprecision`.**  The test only requires that the program --- or, for a pair,
  both programs --- print exactly `expected_output`.  Default stream precision is fine; if the
  printed digits change, update `expected_output` rather than contorting the example to preserve an
  old string.  For a pair, do check that the two sides still agree bit-for-bit, since Au composes
  conversion factors differently from hand-written arithmetic and the last digit can move.

  This holds even when a printed value carries a *claim*.  `atomic_units` prints a line whose whole
  point is that a value is exactly 1 --- and it still does not set the precision, because no
  precision could prove that: an exact 1 and a `0.999...` that rounds to it print identically at
  every width.  Make the claim to the compiler with a `static_assert` instead, and let the output
  stay readable.  Wanting `setprecision` is a sign the claim belongs in an assertion.

Printing pulls in `<iostream>`, so the Au target needs `//au:io`.  Many embedded projects avoid
`<iostream>` for code size; nothing in Au depends on it, and `adc_millivolts` --- the embedded
example --- spells out the `printf` alternative on its page rather than making that one example
inconsistent with the rest.

## Design decisions

Recorded here mostly so they don't get re-litigated.

**Raw version first, styled as the "before".**  The reader lands on the problem, and the click is
the reveal.  This only works because the banner and the tinted tab label make it unmistakable which
version is which --- otherwise the default view of an Au doc page would be non-Au code.

**Banner colors reuse the comparison-matrix legend** from the alternatives page: `#fdae61` (its
`poor` orange) and `#abd9e9` (its `good` blue).  That scheme was chosen for colorblind
accessibility --- see the footnote on `docs/alternatives/index.md` --- and readers of that page have
already learned to read orange→blue as worse→better.  The tab *label* tint stays at `#2c7bb6`,
because that is foreground text on white and the light blue lacks contrast there.

The same accessibility reasoning is why every banner carries an emoji **and** an explicit word.
Color is never the only signal.

**No emoji inside the `.cc` files.**  The examples are meant to be copied into real projects, where
a ⚠️ has no referent --- there is no "before" tab in the reader's codebase.  These files would also
be the only non-ASCII in any C++ source in the repository.  To draw the eye to a specific line, use
the docs layer (`hl_lines`) rather than the source.

**Banners are plain `<p class="ab-banner">` via `attr_list`, not admonitions.**  Material
admonitions bring a title bar, an icon slot, and padding that make a fixed one-line height awkward.

**Line-count parity rather than CSS.**  An earlier design stacked the tab blocks in a grid cell so
the container never resized.  Material sets `display: contents` on `.tabbed-labels` and
`.tabbed-content` and hoists everything into one flex container ordered by `order:`, so that
approach means fighting the framework on every Material upgrade.  Equal line counts solve the same
problem structurally: the code blocks are naturally the same height, so nothing below the tabset
moves when you flip.

**Line numbers off.**  A 1-digit versus 2-digit gutter would shift the columns horizontally.

## Verifying

The tests cover output and alignment:

```sh
bazel test //examples/...
```

CI builds `//...` with `-Werror`, so these examples are compiled by the clang11/14/17, gcc12/15,
C++20, asan, and ubsan jobs.  All are verified clean.  The MSVC and CUDA jobs do not
build this directory (MSVC builds the single-file package with `cl.exe`; CUDA builds only
`//au/cuda:cuda_test`), and neither does the CMake build, which only does `add_subdirectory(au)`.

To check how the pages actually render --- labels identical, banners present, tab code blocks
line-aligned:

```sh
au-docs-serve
```

## Still to do

- **Feature tags on the examples index.**  Pill-box tags, clickable to filter the list.  Each pill
  should link to the reference or discussion page for that Au feature, so the index becomes a
  second navigation axis rather than only a filter.  Material's built-in `tags` plugin does not
  include live filtering outside Insiders, so this is a small amount of hand-rolled JS plus CSS.
  Do not inherit the tag vocabulary from any comparison-site taxonomy; use Au's own feature names.
- **Compile-error examples** ("this must not compile").  Deferred because they need a
  negative-compilation harness, which does not exist here yet.
