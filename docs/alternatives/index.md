# Comparison of Alternatives

There are plenty of other open source C++ units libraries, many quite well established.  However,
the tradeoffs required to use these libraries can be so significant that many people can't or won't
use them.  For example: the compiler errors may be inscrutable or overwhelming; the compilation
process may become unacceptably slow; or, it may require a C++ standard that is simply too new for a
user.

Au is an accessible, production-tested alternative.  We provide a number of rare or outright novel
features, with a small compile time footprint --- and we're compatible with every C++ version back
to the mature and widely available C++14 standard. Key features include:

- Fully unit-safe APIs, on both entry and exit.
- The "safety surface": conversions that adapt to the overflow risk based on both conversion
  magnitude, and storage type.
- Highly composable "quantity maker" APIs make it easy to both compose new units, and apply unit
  prefixes, on the fly.
- Human-readable and concise compiler errors, via strong typenames for units.
- `ZERO`: novel, fluent handling of construction, comparison, and sign handling for quantities.
- Ease of migration (both to and from Au): with minimal setup, we support bidirectional implicit
  conversions with equivalent types from any other units library.
- Support for single-header-file delivery, but with easy customization of units and features to
  include.
- Proven track record supporting embedded applications as first class citizens, via such features as
  our safe handling of integer Rep, treating all Reps on equal footing, and our easy ability to
  exclude expensive `<iostream>` support.
- Intelligent, unit-aware functions for rounding and computing inverses.
- Minimal friction by using a single, short namespace: everything's in `au::`.

## Detailed comparison matrix

Here's a more detailed comparison to the most prominent alternatives.  We'll use the following
legend (chosen to be colorblind-friendly):

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

li.check, li.x {
    list-style: none;
    margin-left: 0;
    text-indent: -2ch;
}

li.check:before {
    content: "\2713  ";
}

li.x:before {
    content: "\2717  ";
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

<table>
    <tr>
        <th></th>
        <th>Boost</th>
        <th>nholthaus</th>
        <th>mp-units</th>
        <th class="highlight">Au</th>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>C++ Version Compatibility</summary>
                <p>The minimum C++ standard required to use the library.</p>
            </details>
        </td>
        <td class="best">
            C++98 or C++03<br>
            (unclear which, but best in either case)
        </td>
        <td class="good">C++14</td>
        <td class="poor">C++20</td>
        <td class="good">C++14</td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Ease of Acquisition</summary>
                <p>Ease of including this library in projects using a wide variety of build environments</p>
            </details>
        </td>
        <td class="fair">Part of boost</td>
        <td class="good">Single, self-contained header</td>
        <td class="fair">First class conan support; available on vcpkg</td>
        <td class="best">
            <p>Supports single-header delivery, with features:
            <ul>
                <li>Easy to customize units and I/O support</li>
                <li>Version-stamped for full reproducibility</li>
            </ul>
        </td>
    </tr>
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
        <td class="na"></td>
        <td class="poor">Very slow, but can be <i>greatly</i> improved by removing I/O support and most units</td>
        <td class="na"></td>
        <td class="good">Possibly "best", but will need to assess all libraries on the same code</td>
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
        <td class="good">Pioneered strong typedefs for units</td>
        <td class="best">No dimension in type name leads to shorter types</td>
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
                        <b>Fair:</b> can achieve indirectly, by casting to known type before
                        retrieving value.
                    </li>
                    <li><b>Good:</b> provide unit-safe interfaces.</li>
                </ul>
            </details>
        </td>
        <td class="fair"></td>
        <td class="fair"></td>
        <td class="fair"></td>
        <td class="best">Only contains unit-safe interfaces</td>
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
        <td class="fair"></td>
        <td class="good">
            <ul>
                <li class="check">Single file is very easy</li>
                <li class="check">User-friendly API typenames (<code>meter_t</code>)</li>
                <li class="x">
                    Namespaces add verbosity, and friction (for example, <code>math::</code>
                    namespace prevents <a href="https://abseil.io/tips/49">ADL</a>)
                </li>
            </ul>
        </td>
        <td class="good"></td>
        <td class="best">
            <ul>
                <li>Namespaces: just one, and it's short</li>
                <li>Includes: either single-header, or easily-guessable header per unit</li>
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
        <td class="na"></td>
        <td class="na"></td>
        <td class="na"></td>
        <td class="best">QuantityMaker and PrefixApplier APIs</td>
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
        <td class="good"></td>
        <td class="good"></td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Generic dimensions</summary>
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
        <td class="na"></td>
        <td class="na"></td>
        <td class="best">Concepts excel here</td>
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
        <td class="na"></td>
        <td class="fair">
            <ul>
                <li class="check">One-line macro defines new units</li>
                <li class="x"><a href="https://github.com/nholthaus/units/issues/131">Can't add dimensions</a></li>
            </ul>
        </td>
        <td class="best">Can even handle, e.g., systems of "natural" units</td>
        <td class="good">
            Can add <a href="https://aurora-opensource.github.io/au/howto/new-units">new units</a>
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
        <td class="fair"></td>
        <td class="fair"></td>
        <td class="good">
            <a href="https://mpusz.github.io/units/use_cases/interoperability.html">
                <code>quantity_like_traits</code>
            </a>
        </td>
        <td class="best">"Equivalent types" feature gives better API compatibility</td>
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
        <td class="good">
            <a href="https://www.boost.org/doc/libs/1_65_0/doc/html/boost/units/absolute.html"><code>absolute</code>
            wrapper</a> for unit
        </td>
        <td class="fair">
            Optional "offset" for units, but <a
            href="https://github.com/nholthaus/units/issues/240">can't distinguish quantity from
            point</a>.
        </td>
        <td class="good"></td>
        <td class="good"></td>
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
        <td class="fair">Close: lacks only basis, and instance arithmetic.  Ahead of its time!</td>
        <td class="fair">Uses ratio plus "pi powers": good angle handling, but vulnerable to overflow</td>
        <td class="good"></td>
        <td class="good">
            Formerly, Au alone was best, but we shared
            <a href="https://mpusz.github.io/units/framework/magnitudes.html">Magnitudes</a>
            with mp-units
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
                    <li>The easy ability to exclude <code>&lt;iostreams&gt;</code>.</li>
                </ul>
            </details>
        </td>
        <td class="good">Assumed to be good, based on mixed-Rep support</td>
        <td class="fair">
            Can trim by excluding <code>&lt;iostream&gt;</code>, but integer-Rep support is poor.
        </td>
        <td class="good">Assumed to be good, based on mixed-Rep support</td>
        <td class="best">
            Best choice of all:
            <ul>
                <li>No "preferred" Rep.</li>
                <li>Safe integer operations.</li>
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
        <td class="best">
            Well defined
            <a href="https://mpusz.github.io/units/reference/core/concepts.html#_CPPv4I0EN5units14RepresentationE">Representation
            concept</a>
        </td>
        <td class="fair">
            Rep can only be <code>is_arithmetic</code> for now, but
            <a href="https://github.com/aurora-opensource/au/issues/52">plan
            to upgrade</a>
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
        <td class="fair">Supports <code>copysign()</code>, but no special construction or comparison</td>
        <td class="fair">Has <code>q::zero()</code> member, but no special construction or comparison</td>
        <td class="best">Can use <code>ZERO</code> to construct or compare any quantity</td>
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
        <td class="good"></td>
        <td class="good"></td>
        <td class="good"></td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Non-linear scales (such as dB)</summary>
                <p>Support for logarithmic "units", such as decibels or nepers</p>
            </details>
        </td>
        <td class="poor"></td>
        <td class="best"></td>
        <td class="poor"></td>
        <td class="poor">
            Plan to support someday; see
            <a href="https://github.com/aurora-opensource/au/issues/41">#41</a>.
        </td>
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
        <td class="poor">Single, implicit global system</td>
        <td class="good"></td>
        <td class="poor">
            Single, implicit global system. (Intentional design tradeoff: reduces learning curve,
            and makes compiler errors shorter.)
        </td>
    </tr>
    <tr>
        <td>
            <details class="criterion">
                <summary>Units as types</summary>
                <p>
                    Types that represent abstract units (clearly distinct from quantities of that
                    unit).
                </p>
            </details>
        </td>
        <td class="good"></td>
        <td class="fair">Types exist, but conflated with quantity names</td>
        <td class="good"></td>
        <td class="best">Can form instances and do arithmetic</td>
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
        <td class="poor">Present in user-facing APIs</td>
        <td class="poor">Present in user-facing APIs</td>
        <td class="good">Confined to outer compatibility layer</td>
        <td class="best">No macros</td>
    </tr>
</table>
