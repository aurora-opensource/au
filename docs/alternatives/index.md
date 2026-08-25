# Comparison of Alternatives

There are plenty of other open source C++ units libraries, many quite well established.  However,
the tradeoffs required to use these libraries can be so significant that many people can't or won't
use them.  For example: the compiler errors may be inscrutable or overwhelming; the compilation
process may become unacceptably slow; or, the required C++ standard may simply be too new for
a user.

Au is an accessible, production-tested alternative.  We provide a number of rare or outright novel
features, with a small compile time footprint --- and we're compatible with every C++ version back
to the mature and widely available C++14 standard. Key features include:

- Fully unit-safe APIs, on both entry and exit.
- The "safety surface": conversions that adapt to the overflow risk based on both conversion
  magnitude, and storage type.
- Highly composable "quantity maker" APIs make it easy to both compose new units, and apply unit
  prefixes, on the fly.
- Human-readable and concise compiler errors, via strong typenames for units.
- Flexible `Constant` types with perfect conversion policies.
- Abbreviated construction via both [unit symbols] and [user-defined literals].
- The `Zero` type: novel, fluent handling of construction, comparison, and sign handling for
  quantities.
- Ease of migration (both to and from Au): with minimal setup, we support bidirectional implicit
  conversions with equivalent types from any other units library.
- Support for single-header-file delivery, but with easy customization of units and features to
  include.
- Proven track record supporting embedded applications as first class citizens, via such features as
  our safe handling of integer Rep, treating all Reps on equal footing, and our easy ability to
  exclude expensive `<iostream>` support.
- Intelligent, unit-aware functions for rounding and computing inverses.
- Minimal friction by using a single, short namespace: everything's in `au::`.

## Alternatives considered here

We'll consider several of the most prominent alternatives in more detail.  While there are [many
more libraries](https://github.com/topics/dimensional-analysis?l=c%2B%2B), the ones we consider here
are included for being especially pioneering or popular (or both).  Here, we list those libraries,
indicate which version we considered, and say a few words about why we included it in the analysis.

- [**Boost Units**](https://www.boost.org/doc/libs/1_82_0/doc/html/boost_units.html) (version:
  1.2, from Boost version 1.89.0)
    - One of the longest-standing C++ unit libraries, and the most prominent pre-C++14 option.
- [**nholthaus/units**](https://github.com/nholthaus/units) (versions: 2.3.5, and 3.6.1)
    - Kicked off the revolution in modern (that is, post-C++11 watershed) units libraries.
    - Its laser-sharp focus on accessibility and low friction have made it probably the most widely
      used C++ units library to date.
    - We assess the 2.x and 3.x lines in **separate columns**, because they are effectively
      different libraries.
        - 3.x requires C++23 (2.x targeted C++14), renames the core vocabulary, and closes a large
          number of long-standing gaps.
        - 2.x has been archived as of August 2026 (branch `archive/2.3.5`), but we retain it because
          it is very commonly used in the wild.
    - If you are choosing today, the deciding question is usually simply whether you can meet the
      C++23 requirement.  If you can't, the 2.x column is the one that applies to you --- and it now
      describes an unmaintained library.
- [**bernedom/SI**](https://github.com/bernedom/SI) (version: 2.5.4)
    - A newer, C++17-compatible offering with a large number of GitHub stars.
- [**mp-units**](https://github.com/mpusz/mp-units) (version: 2.5.0)
    - A library designed to take full advantage of ultra-modern (that is, post-C++20 watershed)
      features, such as concepts and non-template type parameters (NTTPs).
    - mp-units is leading the efforts towards a standard C++ units library, both by field testing
      new API designs, and by coordinating with the authors of other leading units libraries.

## Detailed comparison matrices

Here's a more detailed comparison to the alternatives listed above.  We'll use the following
legend[^1]:

[^1]: Users may have expected a "traffic light" style, green/yellow/red color scheme.  However,
these traditional color schemes have poor accessibility for colorblind readers.  The present color
scheme was designed to be colorblind-friendly.

<style>
td.na::before {
    content: "(Not assessed)";
}

th.highlight {
    background-color: #ccc;
}

span.criterion {
    font-weight: bold;
}

.md-typeset details.criterion summary {
    padding-left: 0.5rem;
}

details.criterion > summary::before {
    display: none;
}
</style>

<table>
    <tr>
        <th>Legend</th>
        <td class="na"></td>
        <td class="poor">Lacks feature /<br> poor support</td>
        <td class="fair">Fair /<br> basic support</td>
        <td class="good">Good /<br> solid support</td>
        <td class="best"><b>Best support</b><br> (of libraries considered here)</td>
    </tr>
</table>

### Obtaining the library

These are the first criteria to consider.  They will tell you whether you can even use the library
at all, and if so, how hard it will be to obtain.

<table class="matrix">
    <thead>
        <tr>
            <th></th>
            <th></th>
            <th colspan="2">nholthaus</th>
            <th></th>
            <th></th>
            <th class="highlight"></th>
        </tr>
        <tr>
            <th></th>
            <th>Boost</th>
            <th>2.x</th>
            <th class="nh3">3.x</th>
            <th>bernedom/SI</th>
            <th>mp-units</th>
            <th class="highlight">Au</th>
        </tr>
    </thead>
    <tbody>
    <tr>
        <td>
            <details class="criterion">
                <summary>C++ Version Compatibility</summary>
                <p>The minimum C++ standard required to use the library.</p>
            </details>
        </td>
        <td class="best">C++98</td>
        <td class="good">C++14</td>
        <td class="poor nh3">C++23</td>
        <td class="fair">C++17</td>
        <td class="poor">C++20</td>
        <td class="good">C++14</td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Ease of Acquisition</summary>
                <p>Ease of including this library in projects using a wide variety of build environments</p>
                <p>Examples of things we look for:</p>
                <ol>
                    <li>Can you vendor it <i>without</i> a package manager?</li>
                    <li>Does it support your build system natively (CMake, bazel)?</li>
                    <li>Is it in the package managers you already use (conan, vcpkg)?</li>
                    <li>Is there a single-header option?</li>
                </ol>
            </details>
        </td>
        <td class="fair">Part of boost</td>
        <td class="good">
            <ul>
                <li class="check">Single, self-contained header</li>
                <li class="check">On conan and vcpkg</li>
            </ul>
        </td>
        <td class="fair nh3">
            <ul>
                <li class="check">Header-only; full CMake support</li>
                <li class="check">Debian, RPM, tarball, and Ubuntu PPA artifacts</li>
                <li>Available on vcpkg</li>
                <li class="x">No more single header option</li>
            </ul>
        </td>
        <td class="fair">Available on conan</td>
        <td class="fair">Available on conan and vcpkg</td>
        <td class="best">
            <ul>
                <li class="check">Full support for bazel and CMake</li>
                <li class="check">Available on conan and vcpkg, thanks to community support</li>
                <li class="check">Supports single-header delivery, with features:
                    <ul>
                        <li class="check">Easy to customize units and I/O support</li>
                        <li class="check">Version-stamped for full reproducibility</li>
                    </ul>
                </li>
            </ul>
        </td>
    </tr>
    </tbody>
</table>

!!! note
    These ratings are written with **all** users and projects in mind.  Keep in mind that what
    matters for _you_ is **your** project.

    For example: mp-units gets low accessibility ratings because of its steep C++20 minimum
    requirement, and its dependence on a package manager to make the installation easy.  However, if
    your project is _already_ compatible with C++20, and _already_ uses conan, then these "low"
    ratings would be completely irrelevant for you.

    The same goes for nholthaus 3.x, whose C++23 requirement is the steepest here: if you're already
    on C++23, that rating simply doesn't apply to you.  Note, though, that this one cuts both ways.
    Because the 2.x line is now archived, a project that _can't_ move to C++23 no longer has a
    maintained version of this library to choose.

### Generic developer experience

Next: how will this library change the generic developer experience?  Leaving aside any library
features, conventions, or implementation strategies, there are two main impacts to developer
experience.

1. Your program will take **longer to compile**, because the compiler is doing more work to produce
   essentially the same program.

2. You will get **more compiler errors** that developers will need to understand and fix.

These costs purchase significant benefits, but we still want them to be as small as possible.

!!! tip
    Note that Au is the **only** units library that provides _both_ readable compiler errors _and_
    fast compilation times!

<table class="matrix">
    <thead>
        <tr>
            <th></th>
            <th></th>
            <th colspan="2">nholthaus</th>
            <th></th>
            <th></th>
            <th class="highlight"></th>
        </tr>
        <tr>
            <th></th>
            <th>Boost</th>
            <th>2.x</th>
            <th class="nh3">3.x</th>
            <th>bernedom/SI</th>
            <th>mp-units</th>
            <th class="highlight">Au</th>
        </tr>
    </thead>
    <tbody>
    <tr>
        <td>
            <details class="criterion">
                <summary>Compilation Speed</summary>
                <p>The extra time the library adds to compiling a translation unit, compared to no units library.</p>
                <ul>
                    <li><b>Poor:</b> typically adds several seconds per translation unit</li>
                    <li><b>Fair:</b> enough that end users tend to notice</li>
                    <li><b>Good:</b> not "subjectively noticeable"</li>
                </ul>
            </details>
        </td>
        <td class="good">Comparable to Au; sometimes less, sometimes more</td>
        <td class="poor">Very slow (adds multiple seconds), but can be <i>greatly</i> improved by removing I/O support and most units</td>
        <td class="poor nh3">
            <ul>
                <li class="x">
                    Default <code>&lt;units.h&gt;</code> is the most expensive include we measured
                    (~4x the 2.x penalty), and dropping <code>&lt;iostream&gt;</code> no longer
                    helps
                </li>
                <li class="check">
                    <a href="https://github.com/nholthaus/units/blob/main/docs/how-to/subset-headers-compile-time.md">Per-dimension
                    headers</a> cut that ~4x, to roughly the 2.x penalty
                </li>
            </ul>
        </td>
        <td class="good">Overall speed champ: roughly 2/3 the penalty of Au and Boost</td>
        <td class="poor">
            <ul>
                <li class="x">Default config 2 to 3 times as expensive as nholthaus</li>
                <li class="x"><a href="https://mpusz.github.io/mp-units/latest/users_guide/systems/si/#lean-umbrella-mp-unitssystemssicoreh">Lean includes</a> brings it on par with nholthaus, though lack of symbols hurts code quality</li>
                <li>C++20 modules should mitigate all concerns</li>
                <li>
                    Substantial compile-time work is on main, slated for 2.6.0, including an
                    "essential" symbols header that should blunt the tradeoff above --- worth
                    re-measuring once it ships
                </li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">Baseline performance is great (only bernedom/SI is faster)</li>
                <li class="check">Includes `fwd.hh` headers for even more flexibility</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Compiler Error Readability</summary>
                <p>
                    The ability to understand errors when the library catches a mistake it was
                    designed to catch.
                </p>
                <ul>
                    <li><b>Poor:</b> Excessively long, nested types</li>
                    <li><b>Fair:</b> Short, but dimension names lacking</li>
                    <li><b>Good:</b> Brief typenames with user-facing unit names</li>
                </ul>
            </details>
        </td>
        <td class="poor">
            <a href="https://mpusz.github.io/wg21-papers/papers/1935R0_a_cpp_approach_to_physical_units.html#type-aliasing-issues">
                Infamously challenging
            </a>
        </td>
        <td class="fair">Positional dimensions</td>
        <td class="good nh3">
            <ul>
                <li class="check">
                    Named unit types in errors (<code>meters&lt;double&gt;</code>), not 2.x's
                    positional <code>std::ratio</code> lists
                </li>
            </ul>
        </td>
        <td class="fair">Alias for unit template</td>
        <td class="good">Pioneered strong typedefs for units</td>
        <td class="best">
            <ul>
                <li class="check">Strong unit typenames appear in errors</li>
                <li class="check">Short namespace minimizes clutter</li>
                <li class="check">
                    Detailed <a
                    href="https://aurora-opensource.github.io/au/main/troubleshooting/">troubleshooting
                    guide</a>
                </li>
            </ul>
        </td>
    </tr>
    </tbody>
</table>

### Ongoing maintenance

The last thing to consider before diving into features is how the library will evolve over time.
There are two extremes that a production-worthy library must avoid:

- _Changing too much_, especially when a new version forces a monolithic, codebase-wide change.
- _Changing too little_, especially when the library becomes unmaintained or abandoned.

These opposite extremes have the same effect: they lock you into an old and ever-aging version of
the code, depriving you of bugfixes and improvements.  The ideal library would be one that is
actively maintained, but that respects its production users and makes upgrades as smooth and
incremental as possible.

<table class="matrix">
    <thead>
        <tr>
            <th></th>
            <th></th>
            <th colspan="2">nholthaus</th>
            <th></th>
            <th></th>
            <th class="highlight"></th>
        </tr>
        <tr>
            <th></th>
            <th>Boost</th>
            <th>2.x</th>
            <th class="nh3">3.x</th>
            <th>bernedom/SI</th>
            <th>mp-units</th>
            <th class="highlight">Au</th>
        </tr>
    </thead>
    <tbody>
    <tr>
        <td>
            <details class="criterion">
                <summary>Actively Maintained</summary>
                <p>
                    Does the library respond to issues?  Is it continuing to receive regular
                    commits?  Does it put out new releases, ideally at least once per year?
                </p>
            </details>
        </td>
        <td class="poor">
            <p>Long unmaintained.</p>
            <ul>
                <li class="x">
                    No <a
                    href="https://www.boost.org/doc/libs/latest/doc/html/boost_units/ReleaseNotes.html">releases</a>
                    since 2010; no <i>significant</i> releases since March 2007
                </li>
                <li class="x">
                    Most <a href="https://github.com/boostorg/units/issues">issues</a> are open and
                    unanswered
                </li>
            </ul>
        </td>
        <td class="poor">
            <p>No longer maintained.</p>
            <ul>
                <li class="x">
                    Last release September 2025, archived as of August 2026 (branch
                    <code>archive/2.3.5</code>); superseded by 3.x
                </li>
            </ul>
        </td>
        <td class="good nh3">
            <ul>
                <li class="check">The project's default branch, with frequent releases</li>
                <li class="check">
                    Backlog cleared: all pre-existing <a
                    href="https://github.com/nholthaus/units/issues">issues</a> closed in August
                    2026
                </li>
            </ul>
        </td>
        <td class="fair">
            <ul>
                <li>Still receiving commits, but no release since 2022</li>
                <li>
                    Most issues closed/addressed, but some issues have gone multiple years with no
                    response
                </li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">Regular commits and releases</li>
                <li class="check">
                    <a href="https://github.com/mpusz/mp-units/issues">Issues</a> responded to and
                    closed over time
                </li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">Regular commits and releases</li>
                <li class="check">
                    <a
                    href="https://github.com/aurora-opensource/au/issues?q=is%3Aissue%20state%3Aopen%20-author%3Achiphogg%20-author%3Ahoffbrinkle%20-author%3Ageoffviola%20-author%3Atobin">External
                    issues</a> always promptly responded to, usually closed
                </li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Smooth Upgrades</summary>
                <p>
                    The ideal library...
                    <ul>
                        <li>enables incremental upgrades (no "megadiff" required)</li>
                        <li>clearly calls out each breaking change, and explains how to adapt</li>
                    </ul>
                </p>
            </details>
        </td>
        <td class="invalid"><p><b>Not Applicable:</b><br>No new releases to judge</p></td>
        <td class="good">
            <ul>
                <li class="check">Updates within the 2.x family were usually smooth</li>
                <li class="x">
                    Now that 2.x is archived, the only upgrade left is the monolithic <a
                    href="https://github.com/nholthaus/units/issues/313">move to 3.x</a>
                </li>
            </ul>
        </td>
        <td class="fair nh3">
            <ul>
                <li class="check">
                    Thorough <a
                    href="https://github.com/nholthaus/units/blob/main/docs/meta/migrate-v2-to-v3.md">migration
                    guide</a>
                </li>
                <li class="x">
                    2.x to 3.x is monolithic, with no syntax that works in both versions
                </li>
                <li class="x">
                    Breaking changes ship in <i>patch</i> releases (e.g., 3.4.4 changed the
                    <code>cosh</code>/<code>sinh</code>/<code>tanh</code> signatures)
                </li>
            </ul>
        </td>
        <td class="good">
            No evidence of user-reported issues with upgrades
        </td>
        <td class="fair">
            <ul>
                <li class="check">
                    Good quality release notes, with breaking changes clearly called out
                </li>
                <li class="x">
                    Upgrade from 0.8 to 2.0 badly broke many users who were treating it as
                    production software
                </li>
            </ul>
        </td>
        <td class="best">
            <p>As incremental as possible:</p>
            <ul>
                <li class="check">
                    Every breaking change provides a syntax that works in both old and new versions
                </li>
                <li class="check">
                    Starting from 0.5.0, <a
                    href="https://aurora-opensource.github.io/au/main/howto/upgrade/#future-proof-releases">future-proof
                    releases</a> let you tackle breaking changes one at a time
                </li>
            </ul>
        </td>
    </tr>
    </tbody>
</table>

### Known best practices violations

Users have a right to expect that a units library follows standard best practices: both for C++
specifically, and for programming more generally.  Any violations of this expectation should be
catalogued explicitly, so that users can be aware of them.  This final generic category gives us
a place to do that.

Note that the expected state for every library is an empty state: either grey ("N/A") for libraries
we haven't assessed in detail, or blue ("good") for libraries we're more familiar with.  Any library
with a non-empty cell can improve their rating by fixing the issues.

!!! warning
    A blue cell means "we don't know of any", **not** "we audited this library and found none".  We
    are not in a position to audit libraries in depth for best practices.  Any issues we _do_ find
    are generally discovered incidentally, while researching other rows.

    Read an empty cell as absence of evidence, not evidence of absence.

<table class="matrix">
    <thead>
        <tr>
            <th></th>
            <th></th>
            <th colspan="2">nholthaus</th>
            <th></th>
            <th></th>
            <th class="highlight"></th>
        </tr>
        <tr>
            <th></th>
            <th>Boost</th>
            <th>2.x</th>
            <th class="nh3">3.x</th>
            <th>bernedom/SI</th>
            <th>mp-units</th>
            <th class="highlight">Au</th>
        </tr>
    </thead>
    <tbody>
    <tr>
        <td>
            <details class="criterion">
                <summary>Known best practice violations</summary>
                <p>
                    Departures from the rules C++ programs rely on: operators that don't
                    mean what the language says they mean, undefined behavior, etc.
                </p>
            </details>
        </td>
        <td class="na"></td>
        <td class="fair">
            <ul>
                <li class="x">
                    Floating point <code>==</code> is not transitive (<a
                    href="https://github.com/nholthaus/units/issues/118">#118</a>, closed as
                    wontfix)
                </li>
            </ul>
        </td>
        <td class="poor nh3">
            <ul>
                <li class="x">
                    Floating point <code>==</code> is not transitive (<a
                    href="https://github.com/nholthaus/units/issues/118">#118</a>, closed as
                    wontfix)
                </li>
                <li class="x">
                    Per-dimension include files are prone to ODR violations and UB (<a
                    href="https://github.com/nholthaus/units/issues/378">#378</a>, acknowledged as
                    genuine UB, kept as-is for now)
                </li>
                <li class="x">
                    Addition is not commutative (takes the left operand's unit instead of common
                    unit; <a href="https://github.com/nholthaus/units/pull/381">#381</a>).
                </li>
            </ul>
        </td>
        <td class="na"></td>
        <td class="good"></td>
        <td class="good"></td>
    </tr>
    </tbody>
</table>

### Library features

At this point, you've assessed:

- whether you can use each library at all;
- how hard it will be to add to your project;
- what costs you'll pay in developer experience if you do;
- how you can expect it to evolve over time;
- and, whether any of them is known to break the rules.

Now we're ready to compare the libraries "as units libraries" --- that is, in terms of their core
features.

!!! note
    The features are listed, _very_ roughly, in order of importance.  Counting up the colors in each
    column won't give an accurate picture.  The rows near the top matter more --- sometimes, _much_
    more --- than the rows further down.

    Of course, what matters the most for _you_ are _your_ use cases and criteria!

<table class="matrix">
    <thead>
        <tr>
            <th></th>
            <th></th>
            <th colspan="2">nholthaus</th>
            <th></th>
            <th></th>
            <th class="highlight"></th>
        </tr>
        <tr>
            <th></th>
            <th>Boost</th>
            <th>2.x</th>
            <th class="nh3">3.x</th>
            <th>bernedom/SI</th>
            <th>mp-units</th>
            <th class="highlight">Au</th>
        </tr>
    </thead>
    <tbody>
    <tr>
        <td>
            <details class="criterion">
                <summary>Conversion Safety</summary>
                <p>Guarding against unit conversions that are likely to produce large errors.</p>
                <p>
                    (For example: we can convert an integer number of feet to inches, but not vice
                    versa.)
                </p>
            </details>
        </td>
        <td class="good"></td>
        <td class="poor">
            Integer Reps <a href="https://github.com/nholthaus/units/issues/225">unsafe</a>
        </td>
        <td class="good nh3">
            <ul>
                <li class="check">
                    Implicit only when lossless, matching <code>std::chrono</code> (<a
                    href="https://github.com/nholthaus/units/issues/225">#225</a>)
                </li>
                <li class="check">
                    <code>consteval</code>-checked conversions for compile-time-known values
                </li>
                <li class="check">
                    Overflow protection for <i>intermediate</i> results
                </li>
                <li class="x">
                    Still no adaptation to overflow risk in the <i>result</i>
                </li>
            </ul>
        </td>
        <td class="poor">
            Integer Reps <a href="https://github.com/bernedom/SI/issues/122">unsafe</a>
        </td>
        <td class="good">
            Policy <a
            href="https://mpusz.github.io/units/framework/conversions_and_casting.html">consistent
            with <code>std::chrono</code> library</a>
        </td>
        <td class="best">
            Meets `std::chrono` baseline, plus:
            <ul>
                <li class="check">Automatically adapts to level of overflow risk</li>
                <li class="check">Can separately opt out of checking for overflow and truncation</li>
                <li class="check">Runtime conversion checkers</li>
                <li class="check">Constants have perfect conversion policy</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Unit Safety</summary>
                <p>
                    The ability to judge the unit-correctness of every individual line of code
                    by inspection, in isolation.
                </p>
                <ul>
                    <li>
                        <b>Poor:</b> can achieve indirectly, by casting to known type before
                        retrieving value.
                    </li>
                    <li><b>Fair:</b> provides unit-safe interfaces.</li>
                    <li><b>Good:</b> <i>only</i> provides unit-safe interfaces.</li>
                </ul>
            </details>
        </td>
        <td class="poor"></td>
        <td class="poor"></td>
        <td class="fair nh3">
            <code>.value()</code> documented as unsafe; the safe route is
            <code>q.to&lt;meters&gt;().value()</code>
        </td>
        <td class="poor"></td>
        <td class="good">Only contains unit-safe interfaces</td>
        <td class="good">Only contains unit-safe interfaces</td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Low Friction</summary>
                <p>How easy it is to develop with the library.  Criteria include:</p>
                <ul>
                    <li>Headers: few, or easily guessable</li>
                    <li>Simple namespace structure</li>
                    <li>Reasonable, safe implicit conversions</li>
                </ul>
            </details>
        </td>
        <td class="poor">
            <ul>
                <li class="x">Generally high learning curve</li>
                <li class="x">
                    No (<a
                    href="https://www.boost.org/doc/libs/1_79_0/doc/html/boost_units/Quantities.html#boost_units.Quantities.Quantity_Construction_and_Conversion">non-trivial</a>)
                    implicit conversions
                </li>
                <li class="x">Many headers; hard to guess</li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">Single file is very easy</li>
                <li class="check">User-friendly API typenames (<code>meter_t</code>, ...)</li>
                <li class="x">
                    Namespaces add verbosity, and friction (for example, <code>math::</code>
                    namespace prevents <a href="https://abseil.io/tips/49">ADL</a>)
                </li>
            </ul>
        </td>
        <td class="good nh3">
            <ul>
                <li class="check">
                    2.x's namespace friction is gone (inline namespaces; <a
                    href="https://abseil.io/tips/49">ADL</a> for math)
                </li>
                <li class="check">User-friendly typenames, with CTAD</li>
                <li class="x">
                    Literal suffixes still share one flat namespace, so they can collide
                </li>
            </ul>
        </td>
        <td class="fair">
            <ul>
                <li class="check">Single, short namespace</li>
                <li>Implicit conversions, but not safe ones</li>
                <li>Multiple headers, but easy to guess (one per dimension)</li>
            </ul>
        </td>
        <td class="fair">
            <ul>
                <li class="check">Implicit conversions with good basic safety</li>
                <li>
                    <a
                    href="https://mpusz.github.io/mp-units/2.0/users_guide/examples/hello_units/">Multiple
                    headers</a>, one per system
                </li>
                <li class="x">Longer and more nested namespaces</li>
            </ul>
        </td>
        <td class="best">
            <ul>
                <li class="check">Namespaces: just one (for non-abbreviated names), and it's short</li>
                <li class="check">Includes: either single-header, or easily-guessable header per unit</li>
                <li class="check">Implicit conversions, and they adapt to the overflow risk</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Composability</summary>
                <p>
                    The ability to fluently combine the abstractions for units and prefixes to form
                    new units on the fly.
                </p>
            </details>
        </td>
        <td class="good">
            <ul>
                <li class="check">
                    Can compose units, prefixes, dimensions, and quantity (point) makers
                </li>
                <li class="x">
                    Type names clunky to compose: must write <code>decltype</code>
                </li>
            </ul>
        </td>
        <td class="poor"><a href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1935r2.html#comparison">No</a></td>
        <td class="fair nh3">
            <ul>
                <li class="check">
                    Unit constants compose: <code>3.0 * kg * m / (s*s)</code> yields newtons
                </li>
                <li class="x">Prefixes don't apply to unit constants on the fly</li>
            </ul>
        </td>
        <td class="poor">No</td>
        <td class="best">
            <ul>
                <li class="check">Can compose units, prefixes, dimensions, and quantity types</li>
                <li class="check">
                    C++20's Non-type template parameters (NTTPs) enable composable <i>type names</i>
                </li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">
                    Can compose units, prefixes, dimensions, and quantity (point) makers
                </li>
                <li class="x">
                    Type names clunky to compose: must write <code>decltype</code> or use traits
                </li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Unit-aware I/O</summary>
                <p>
                    The ability to print quantities along with information about their units.
                    Examples:
                </p>
                <ul>
                    <li><code>&lt;iostream&gt;</code>, preferrably toggleable</li>
                    <li>Unit labels available even without <code>&lt;iostream&gt;</code></li>
                    <li><code>fmtlb</code> (<code>std::format</code> after C++20)</li>
                </ul>
            </details>
        </td>
        <td class="fair">
            <ul>
                <li class="check">Toggleable <code>&lt;iostream&gt;</code> support</li>
                <li class="check">
                    Impressively configurable output (<code>format_mode</code>,
                    <code>autoprefix_mode</code>)
                </li>
                <li class="x">No fmtlib support</li>
            </ul>
        </td>
        <td class="fair">
            <ul>
                <li class="check">Toggleable <code>&lt;iostream&gt;</code> support</li>
                <li class="x">No fmtlib or <code>std::format</code> support</li>
            </ul>
        </td>
        <td class="good nh3">
            <ul>
                <li class="check">Toggleable <code>&lt;iostream&gt;</code> support</li>
                <li class="check">Unit labels available even without <code>&lt;iostream&gt;</code></li>
                <li class="check">
                    Supports <code>std::format</code>, including the full numeric spec grammar
                </li>
            </ul>
        </td>
        <td class="fair">
            <ul>
                <li class="check">Toggleable <code>&lt;iostream&gt;</code> support</li>
                <li class="check">Unit labels available even without <code>&lt;iostream&gt;</code></li>
                <li class="x">No fmtlib support</li>
            </ul>
        </td>
        <td class="best">
            <ul>
                <li class="check">Supports <code>&lt;iostream&gt;</code></li>
                <li class="check">Unit labels available even without <code>&lt;iostream&gt;</code></li>
                <li class="check">Supports <code>std::format</code></li>
                <li class="check">Many customization options</li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">Toggleable <code>&lt;iostream&gt;</code> support</li>
                <li class="check">Unit labels available even without <code>&lt;iostream&gt;</code></li>
                <li class="check">Supports <code>std::format</code></li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Mixed-Rep Support</summary>
                <p>The ease of freely mixing different storage types ("Reps") in the same program.</p>
            </details>
        </td>
        <td class="good"></td>
        <td class="fair">Possible, but user-facing types use a global "preferred" Rep.</td>
        <td class="good nh3">
            Types name their own Rep (<code>meters&lt;int&gt;</code>); the global default applies
            only to <code>meters&lt;&gt;</code>
        </td>
        <td class="good"></td>
        <td class="good"></td>
        <td class="good"></td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Unit-aware math</summary>
                <p>
                    Unit-aware versions of common mathematical functions (`max`, `abs`, `sin`,
                    `round`, and so on).
                </p>
            </details>
        </td>
        <td class="fair">
            <ul>
                <li class="check">Wide variety of functions</li>
                <li class="x">
                    <code>round</code>, <code>ceil</code>, and so on are not unit-safe
                </li>
            </ul>
        </td>
        <td class="fair">
            <ul>
                <li class="check">Wide variety of functions</li>
                <li class="x">
                    <code>round</code>, <code>ceil</code>, and so on operate in whatever unit the
                    quantity happens to hold, so they are not unit-safe
                </li>
            </ul>
        </td>
        <td class="good nh3">
            <ul>
                <li class="check">Wide variety of functions, found by ADL</li>
                <li class="check">
                    <code>round</code>, <code>ceil</code>, <code>floor</code>, and
                    <code>trunc</code> have unit-safe versions
                </li>
                <li class="x">
                    No unit-aware inverse
                </li>
            </ul>
        </td>
        <td class="poor">No</td>
        <td class="good">
            <ul>
                <li class="check">Wide variety of functions</li>
                <li class="check">
                    Unit-safe APIs for <code>round</code>, <code>ceil</code>, and so on
                </li>
                <li class="check">
                    Smart, unit-aware inverse functions
                </li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">Wide variety of functions</li>
                <li class="check">
                    Unit-safe APIs for <code>round</code>, <code>ceil</code>, and so on
                </li>
                <li class="check">
                    Smart, unit-aware inverse functions
                </li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Generic Dimensions</summary>
                <p>
                    The ability to write (template) functions that operate on any dimensionally
                    consistent inputs.
                </p>
                <p>
                    (For example, a function that takes any length and time quantities, and returns
                    the appropriate speed quantity.)
                </p>
            </details>
        </td>
        <td class="fair">
            <a
            href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1935r2.html#boost.units.usage.example">Generic
            templates, constrained with traits</a>
        </td>
        <td class="fair">
            <a
            href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1935r2.html#nic-units.usage.example">Generic
            templates, constrained with traits</a>
        </td>
        <td class="good nh3">
            <ul>
                <li class="check">
                    Public <a
                    href="https://github.com/nholthaus/units/blob/main/docs/reference/concepts.md">concept
                    vocabulary</a> (<code>UnitType</code>, <code>same_dimension</code>, ...)
                </li>
                <li class="check">
                    A concept per dimension (<code>units::Velocity auto v</code>), 50 of them, one
                    beside each <code>is_&lt;dimension&gt;_unit</code> trait
                </li>
            </ul>
        </td>
        <td class="fair">Generic templates, constrained with traits</td>
        <td class="best">
            <ul>
                <li class="check">
                    Concepts name a <i>specific</i> quantity:
                    <code>QuantityOf&lt;isq::length&gt; auto</code>
                </li>
                <li class="x">
                    Which spelling you pick (a quantity spec, or its <code>kind_of</code>) changes
                    what the function accepts
                </li>
            </ul>
        </td>
        <td class="fair">
            Currently clunky.  Could be better by adding concepts in extra
            C++20-only file, without compromising C++14 support.
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Extensibility</summary>
                <p>How easy it is to add new units, dimensions, or systems.</p>
            </details>
        </td>
        <td class="good">
            <a
            href="https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p1935r2.html#comparison">Can
            add new units and dimensions</a>
        </td>
        <td class="fair">
            <ul>
                <li class="check">One-line macro defines new units</li>
                <li class="x"><a href="https://github.com/nholthaus/units/issues/131">Can't add dimensions</a></li>
            </ul>
        </td>
        <td class="good nh3">
            <ul>
                <li class="check">One-line macro defines new units, with prefixes and literals</li>
                <li class="check">
                    Can now add new base dimensions (<a
                    href="https://github.com/nholthaus/units/issues/131">#131</a>), and derive
                    compound dimensions from them
                </li>
            </ul>
        </td>
        <td class="good">Can add new units and dimensions</td>
        <td class="best">Can even handle, e.g., systems of "natural" units</td>
        <td class="good">
            Can add <a href="https://aurora-opensource.github.io/au/main/howto/new-units/">new units</a>
            and dimensions
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Ease of Migration</summary>
                <p>
                    Support for two migration use cases:
                </p>
                <ul>
                    <li>From "no units" to this library</li>
                    <li>Between this library and another units library (either direction)</li>
                </ul>
            </details>
        </td>
        <td class="fair">No interop with other units libraries</td>
        <td class="fair">No interop with other units libraries</td>
        <td class="fair nh3">
            <ul>
                <li class="check"><code>std::chrono::duration</code> interop, both directions</li>
                <li class="x">No interop with other units libraries</li>
            </ul>
        </td>
        <td class="fair">No interop with other units libraries</td>
        <td class="good">
            <ul>
                <li class="check">
                    <a href="https://mpusz.github.io/mp-units/2.1/users_guide/framework_basics/basic_concepts/#QuantityLike">
                        <code>QuantityLike&lt;T&gt;</code> supports bidirectional conversions
                    </a>
                </li>
                <li class="check">
                    Can specify implicit or explicit
                </li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">
                    <a href="https://aurora-opensource.github.io/au/main/howto/interop/">
                        <code>CorrespondingQuantity</code> supports bidirectional implicit conversions
                    </a>
                </li>
                <li class="check">
                    Supports <a
                    href="https://aurora-opensource.github.io/au/main/reference/corresponding_quantity/#conversions">"two-hop"
                    conversions</a>
                </li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Point Types</summary>
                <p>
                    Support for "point-like" quantities, also known as
                    <a href="http://videocortex.io/2018/Affine-Space-Types/">"affine space
                    types"</a>.
                </p>
            </details>
        </td>
        <td class="fair">
            <a href="https://www.boost.org/doc/libs/1_65_0/doc/html/boost/units/absolute.html"><code>absolute</code>
            wrapper</a> for unit, but no point-specific conversion safety
        </td>
        <td class="poor">
            <ul>
                <li class="x">
                    Point arithmetic often produces incorrect results (see <a href="https://github.com/nholthaus/units/issues/240">#240</a>, closed as "won't fix")
                </li>
            </ul>
        </td>
        <td class="fair nh3">
            <ul>
                <li class="check">
                    Opt-in <code>absolute</code>/<code>delta</code> wrappers, for any unit: point +
                    point is ill-formed, and a difference is offset-free
                </li>
                <li class="x">
                    Plain affine units keep the old hazards: <code>kelvin&lt;int&gt;(273)</code>
                    still converts silently to <code>celsius&lt;int&gt;(0)</code>
                </li>
                <li class="x">
                    No origin-aware common unit; any integer arithmetic with offset scale silently
                    promotes to <code>double</code>
                </li>
            </ul>
        </td>
        <td class="poor">None; would be hard to add, since units conflated with quantity type</td>
        <td class="best">
            <ul>
                <li class="check">Custom origins really easy to use and compose</li>
                <li class="check">Elegant `point` and `delta` modifiers on quantity spec</li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">
                    <code>QuantityPoint</code> is a first class type, with the same conversion
                    safety surface as <code>Quantity</code>
                </li>
                <li class="check">Maximally efficient common units</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Magnitudes</summary>
                <p>
                    The features of the representation for different units' sizes.  Key features
                    include:
                </p>
                <ul>
                    <li>Irrational numbers (such as \(\pi\))</li>
                    <li>Powers (robust against overflow)</li>
                    <li>Roots (exact representations)</li>
                </ul>
            </details>
        </td>
        <td class="fair">
            Close: lacks only irrationals, basis, and instance arithmetic.  Ahead of its time!
        </td>
        <td class="fair" colspan="2">
            <code>std::ratio</code> plus a pi exponent: good angle handling, but overflows when
            compounding (<code>cubed&lt;pico&lt;meters_&gt;&gt;</code> fails to compile)
        </td>
        <td class="poor">`std::ratio` only, with no solution for pi</td>
        <td class="good">
            Full support for <a
            href="https://mpusz.github.io/units/framework/magnitudes.html">Magnitudes</a>
        </td>
        <td class="good">
            Formerly, Au alone was best, but we <a
            href="https://github.com/mpusz/units/issues/300">shared</a> Magnitudes with mp-units
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Embedded Friendliness</summary>
                <p>
                    Support common embedded use cases.  Key examples include:
                </p>
                <ul>
                    <li>
                        Flexibility in the Rep (usually a variety of integral types, and perhaps
                        <code>float</code>, but rarely <code>double</code>).
                    </li>
                    <li>The easy ability to exclude <code>&lt;iostream&gt;</code>.</li>
                </ul>
            </details>
        </td>
        <td class="good">Assumed to be good, based on mixed-Rep support</td>
        <td class="fair">
            Can trim by excluding <code>&lt;iostream&gt;</code>, but integer-Rep support is poor.
        </td>
        <td class="good nh3">
            Assumed to be good, based on mixed-Rep support (now safe for integer Rep) and
            droppable <code>&lt;iostream&gt;</code>
        </td>
        <td class="fair">
            <ul>
                <li class="check"><code>&lt;iostream&gt;</code> not automatically included</li>
                <li class="check">Supports integral rep</li>
                <li class="x">Integral rep conversions unsafe</li>
            </ul>
        </td>
        <td class="good">Assumed to be good, based on mixed-Rep support</td>
        <td class="best">
            Best choice of all:
            <ul>
                <li class="check">No "preferred" Rep.</li>
                <li class="check"><code>sizeof()</code>-friendly unit label representation</li>
                <li class="check">Safe integer operations.</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Abbreviated construction</summary>
                <p>
                    The ability to construct a Quantity using the symbol for its unit.
                </p>

                <p>
                    This is most commonly done with user-defined literals (UDLs), such as
                    <code>3_m</code> for "3 meters", but there are other alternatives.
                </p>
            </details>
        </td>
        <td class="na"></td>
        <td class="fair">User-defined literals (UDLs)</td>
        <td class="fair nh3">
            <ul>
                <li class="check">User-defined literals (UDLs)</li>
                <li>
                    "Unit symbol"-like syntax (<code>5.0 * m</code>), but <code>m</code> is just
                    a quantity with value <code>1.0</code>, and it force-routes through
                    <code>double</code>
                </li>
            </ul>
        </td>
        <td class="fair">User-defined literals (UDLs)</td>
        <td class="good">
            Unit symbols
        </td>
        <td class="good">
            <a href="https://aurora-opensource.github.io/au/main/reference/unit/#symbols">
                Unit symbols
            </a>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Linear algebra</summary>
                <p>
                    Good interoperability with matrix and vector libraries, such as Eigen
                </p>
                <p>
                    Historically, libraries could work with Eigen only if Eigen was patched:
                    Quantity types break several of Eigen's deeply embedded assumptions.
                </p>
            </details>
        </td>
        <td class="poor"></td>
        <td class="poor"></td>
        <td class="fair nh3">
            <a href="https://github.com/nholthaus/units/issues/90">Eigen interop</a> released in
            3.5.0: a "linalg-on-units" approach  It inverts the usual nesting: the <i>unit is the Eigen scalar</i>
            (<code>Eigen::Matrix&lt;meters&lt;double&gt;, 3, 1&gt;</code>).
            <ul>
                <li class="check">Dimension-<i>preserving</i> operations stay lazy</li>
                <li class="x">
                    Dimension-<i>changing</i> ones (dot, cross, norm) use scalar loops that
                    materialize, losing vectorization
                </li>
            </ul>
        </td>
        <td class="poor"></td>
        <td class="good">
            <ul>
                <li class="check">"units-on-linalg" approach (most flexible)</li>
                <li class="check">
                    Explicit Eigen, GLM, and Blaze support (on main, slated for 2.6.0)
                </li>
                <li>Only library with <a
                href="https://mpusz.github.io/mp-units/2.0/users_guide/framework_basics/character_of_a_quantity/">Quantity
                Character</a>, and with named vector components</li>
                <li class="x">
                    No quantity-level dot or cross product: the result's quantity spec can't be
                    named yet, so both stay on the raw rep
                </li>
            </ul>
        </td>
        <td class="best">
            <ul>
                <li class="check">"units-on-linalg" approach (most flexible)</li>
                <li class="check">Explicit Eigen support</li>
                <li class="check">
                    Only library that preserves full Eigen performance in all cases: unit-aware
                    <code>dot</code>, <code>cross</code>, <code>norm</code>, and coefficient-wise
                    ops, each as lazy as the Eigen member it wraps
                </li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Rep Variety</summary>
                <p>The range of different storage types ("Reps") permitted.</p>
                <ul>
                    <li><b>Poor:</b> only 1 or 2 types</li>
                    <li><b>Fair:</b> all built-in numeric types</li>
                    <li><b>Good:</b> also support custom numeric types</li>
                </ul>
            </details>
        </td>
        <td class="good">Supports custom numeric types</td>
        <td class="poor">
            Effectively floating-point only (integer types <a
            href="https://github.com/nholthaus/units/issues/225">unsafe</a>)
        </td>
        <td class="fair nh3">
            <ul>
                <li class="check">
                    All built-in numeric types, with integer Reps meeting <code>std::chrono</code>
                    safety standards
                </li>
                <li class="x">
                    Constrained to <code>std::is_arithmetic</code>: no custom numeric types
                </li>
            </ul>
        </td>
        <td class="fair">
            <ul>
                <li class="check">No "default" rep</li>
                <li class="x">
                    Integer reps <a href="https://github.com/bernedom/SI/issues/122">unsafe</a>
                </li>
            </ul>
        </td>
        <td class="best">
            Well defined
            <a href="https://mpusz.github.io/units/reference/core/concepts.html#_CPPv4I0EN5units14RepresentationE">Representation
            concept</a>
        </td>
        <td class="good">
            <ul>
                <li class="check">Mature support for <code>is_arithmetic</code> Rep</li>
                <li class="check">Experimental support for custom Rep</li>
                <li class="x">No constraints yet (<a
                href="https://github.com/aurora-opensource/au/issues/52">#52</a>)</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Zero</summary>
                <p>
                    Quantity support for constructing from, and comparing with, <code>0</code>: the
                    only number which is meaningful for every unit.  (Includes facilities for
                    working with quantity signs.)
                </p>
            </details>
        </td>
        <td class="fair">
            Guidance:
            <a href="https://www.boost.org/doc/libs/1_81_0/doc/html/boost_units/FAQ.html#boost_units.FAQ.NoConstructorFromValueType">use
            default constructor</a> to construct, but no special facility for comparison
        </td>
        <td class="fair" colspan="2">
            Supports <code>copysign()</code>, but comparing a quantity to <code>0</code> is
            ill-formed
        </td>
        <td class="poor">No special construction or comparison</td>
        <td class="good">
            <ul>
                <li class="check">
                    All six <a
                    href="https://mpusz.github.io/mp-units/latest/users_guide/framework_basics/quantity_arithmetics/#comparison-against-zero">comparison
                    operators</a> accept a literal <code>0</code>, enforced by a
                    <code>consteval</code> guard (on <code>main</code>, for 2.6.0)
                </li>
                <li class="x">No construction from <code>0</code></li>
            </ul>
        </td>
        <td class="good">
            Can use <a
            href="https://aurora-opensource.github.io/au/main/discussion/concepts/zero/"><code>ZERO</code></a>
            to construct or compare any quantity
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Angles</summary>
                <p>
                    First-class support for angular quantities, including degrees and radians.
                </p>
            </details>
        </td>
        <td class="fair">
            Curiously imprecise
            <a href="https://github.com/boostorg/units/blob/45787015/include/boost/units/base_units/angle/degree.hpp#L17">pi
            value </a>
        </td>
        <td class="good" colspan="2"></td>
        <td class="fair">
            <ul>
                <li class="check">Supports degrees and radians</li>
                <li class="x">pi represented as <code>std::ratio</code></li>
            </ul>
        </td>
        <td class="best">Simultaneous support for both strongly-typed and "pure SI" angles</td>
        <td class="good"></td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Physical constants</summary>
                <ul>
                    <li>How good is the core library support?</li>
                    <li>Does the library include built-in constants?</li>
                </ul>
            </details>
        </td>
        <td class="fair">Includes built-in constants as quantities</td>
        <td class="fair" colspan="2">
            Built-in constants as quantities (2018 CODATA values)
        </td>
        <td class="poor"></td>
        <td class="good">
            <a
            href="https://mpusz.github.io/mp-units/2.0/users_guide/framework_basics/faster_than_lightspeed_constants/">"Faster
            than lightspeed" constants</a>
        </td>
        <td class="best">
            <ul>
                <li class="check">Constants as types</li>
                <li class="check">Perfect conversion policy</li>
                <li class="check">Implicit Quantity conversion</li>
                <li class="check"><a href="https://aurora-opensource.github.io/au/main/reference/constant/#built-in">Includes</a> exact constants from SI 2019</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Non-linear scales (such as dB)</summary>
                <p>Support for logarithmic "units", such as decibels or nepers</p>
            </details>
        </td>
        <td class="poor"></td>
        <td class="good">
            <ul>
                <li class="check">Far more support than any library outside this family</li>
                <li class="x">
                    Not sound: <code>10 dBW + 10 dBW</code> compiles, and yields
                    <code>20 m^4 kg^2 s^-6</code>
                </li>
            </ul>
        </td>
        <td class="best nh3">
            <ul>
                <li class="check">The clear leader</li>
                <li class="check">
                    Decibels now modelled as affine
                </li>
                <li class="x">
                    Only power convention (<code>10*log10</code>); no root-power (amplitude) variant
                </li>
            </ul>
        </td>
        <td class="poor"></td>
        <td class="poor"></td>
        <td class="poor">
            Plan to support someday; see
            <a href="https://github.com/aurora-opensource/au/issues/41">#41</a>.
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Quantity template parameters</summary>
                <p>
                    The ability to use quantity <i>values</i> as template parameters.
                </p>
            </details>
        </td>
        <td class="poor"></td>
        <td class="poor"></td>
        <td class="good nh3">
            <ul>
                <li class="check">Supports integral and floating point Reps</li>
                <li class="check">Supports quantity families via concepts</li>
                <li class="x">User must provide exact unit and rep</li>
            </ul>
        </td>
        <td class="poor"></td>
        <td class="best">
            <ul>
                <li class="check">Supports all quantities</li>
                <li class="check">Supports automatic conversions</li>
                <li class="check">Supports quantity families via concepts</li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">Supports integral rep</li>
                <li class="check">Only library with pre-C++20 support</li>
                <li class="x">User must provide exact unit and rep</li>
                <li class="x">No floating point support</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Negative units</summary>
                <p>
                    Correctly supporting units, constants, and magnitudes that are negative: so,
                    larger stored values correspond to smaller quantities.
                </p>
            </details>
        </td>
        <td class="poor"></td>
        <td class="poor"></td>
        <td class="poor nh3">
            A negative conversion factor is accepted, but ordering then reflects the stored values
            rather than the quantities
        </td>
        <td class="poor"></td>
        <td class="poor">
            Unary minus on a magnitude, added on <code>main</code> for the CODATA constants, but
            no negative <i>units</i>
        </td>
        <td class="best">
            <ul>
                <li class="check">Negative constants</li>
                <li class="check">Negative units</li>
                <li class="check">Negative magnitudes</li>
                <li class="check">Comparison operators correctly account for sign</li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>"Kind" Types</summary>
                <p>
                    Any feature which supports robustly distinguishing between units that have the
                    same dimension and magnitude.
                </p>
                <p>
                    For example, "hertz" and "becquerel" both have the same dimension and magnitude
                    as "inverse seconds", but some libraries may prevent users from mixing them.
                </p>
            </details>
        </td>
        <td class="na"></td>
        <td class="poor"></td>
        <td class="good nh3">
            <ul>
                <li class="check">
                    Opt-in string-tagged kinds
                </li>
                <li class="x">
                    Built-in kinds not included (hertz vs. becquerel, torque vs. energy)
                </li>
            </ul>
        </td>
        <td class="poor"></td>
        <td class="best"></td>
        <td class="poor">No plans at present to support.</td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Explicit Systems of Measurement</summary>
                <p>
                    Support for different systems, each with their own (possibly incompatible)
                    collection of dimensions.
                </p>
            </details>
        </td>
        <td class="good"></td>
        <td class="poor" colspan="2">Single, implicit global system</td>
        <td class="poor"></td>
        <td class="good"></td>
        <td class="poor">
            Single, implicit global system. (Intentional design tradeoff: reduces learning curve,
            and makes compiler errors shorter.)
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Abstract Units/Dimensions</summary>
                <p>
                    <li>
                        Types that represent abstract units (clearly distinct from quantities of
                        that unit).
                    </li>
                    <li>Types that represent abstract dimensions.</li>
                    <li>The ability to do arithmetic with instances of these types.</li>
                </p>
            </details>
        </td>
        <td class="good"></td>
        <td class="fair">Types exist, but conflated with quantity names</td>
        <td class="good nh3">
            <ul>
                <li class="check">
                    Units and quantities are separate types (<code>meters_</code> vs.
                    <code>meters&lt;double&gt;</code>)
                </li>
                <li class="check">Types for dimensions; arithmetic on both</li>
            </ul>
        </td>
        <td class="poor">No separate types for units</td>
        <td class="good">
            <ul>
                <li class="check">Types for units</li>
                <li class="check">Types for dimensions</li>
                <li class="check">
                    Can do arithmetic (compound units on the fly; abstract dimensional analysis)
                </li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">Types for units</li>
                <li class="check">Types for dimensions</li>
                <li class="check">
                    Can do arithmetic (compound units on the fly; abstract dimensional analysis)
                </li>
            </ul>
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Macro Usage</summary>
                <p>
                    Avoidance of macros, especially in user-facing code.
                </p>
            </details>
        </td>
        <td class="poor">Common in user-facing APIs</td>
        <td class="poor" colspan="2">
            The <code>UNIT_ADD</code> family is the only way to define a <i>named</i> unit.  (3.x
            can at least name a <i>derived</i> type macro-free, with <code>decltype</code>.)
        </td>
        <td class="good">Very few, and confined to implementation helpers</td>
        <td class="fair">
            <ul>
                <li>Very few, mostly implementation helpers</li>
                <li>Only one user-facing macro for C++20 backwards compatibility</li>
            </ul>
        </td>
        <td class="good">
            <ul>
                <li class="check">No user-facing macros</li>
                <li>Internal macros only where no viable alternative exists (CUDA/HIP support,
                    C++ feature detection)</li>
            </ul>
        </td>
    </tr>
    </tbody>
</table>

[unit symbols]: ../reference/unit.md#symbols
[user-defined literals]: ../reference/constant.md#unit-literals
