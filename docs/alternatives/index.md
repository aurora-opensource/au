# Comparison of Alternatives

There are plenty of other open source C++ units libraries, many quite well established.  Why does Au
exist?  What do we add to the C++ units library ecosystem?

## Big picture

The answer is twofold.

First, we address the most common reasons that people can't, or won't, use the most prominent
alternatives.

- [mp-units](https://mpusz.github.io/units/) is a modern, high quality library, but its minimum
  C++20 requirement is too steep for many.

- [nholthaus/units](https://github.com/nholthaus/units) is accessible and easy to learn, but its
  compile time penalty is severe, and it's hard to use different Rep types.

Second, we bring a number of rare or outright novel features in our own right, including:

- Fully unit-safe APIs, on both entry and exit.
- The "safety surface": conversions that adapt to the overflow risk based on both conversion
  magnitude, and storage type.
- Highly compsable "quantity maker" APIs make it easy to both compose new units, and apply unit
  prefixes, on the fly.
- `ZERO`: novel, fluent handling of construction, comparison, and sign handling for quantities.
- Ease of migration (both to and from Au): we support bidirectional implicit conversions with
  equivalent types from any other units library.
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
                <summary>Unit Safety</summary>
                <p>
                    The ability to judge the unit-correctness of every individual line of code
                    by inspection, in isolation.
                </p>
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
        <td class="best">QuantityMaker APIs</td>
    </tr>
    <tr>
        <td><a href="#mixed_rep_support">Mixed-Rep Support</a></td>
        <td class="good"></td>
        <td class="fair"></td>
        <td class="good"></td>
        <td class="good"></td>
    </tr>
    <tr>
        <td><a href="#compilation_speed">Compilation Speed</a></td>
        <td class="na"></td>
        <td class="poor">Very slow, but can be <i>greatly</i> improved by removing I/O support and most units</td>
        <td class="na"></td>
        <td class="good">Probably "best", but will need to assess all libraries on the same code</td>
    </tr>
    <tr>
        <td><a href="#compiler_error_readability">Compiler Error Readability</a></td>
        <td class="poor">Infamously challenging</td>
        <td class="fair">Positional dimensions</td>
        <td class="good">Pioneered strong typedefs for units</td>
        <td class="best">No dimension in type name leads to shorter types</td>
    </tr>
    <tr>
        <td><a href="#units_as_types">Units as types</a></td>
        <td class="good"></td>
        <td class="fair">Types exist, but conflated with quantity names</td>
        <td class="good"></td>
        <td class="best">Can form instances and do arithmetic</td>
    </tr>
    <tr>
        <td><a href="#generic_dimensions">Generic dimensions</a></td>
        <td class="na"></td>
        <td class="na"></td>
        <td class="best">Concepts excel here</td>
        <td class="fair">
            Currently clunky.  Could be better by adding concepts in extra
            C++20-only file, without compromising C++14 support.
        </td>
    </tr>
    <tr>
        <td><a href="#embedded_friendliness">Embedded Friendliness</a></td>
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
        <td><a href="#ease_of_migration">Ease of Migration</a></td>
        <td class="fair"></td>
        <td class="fair"></td>
        <td class="fair"></td>
        <td class="best">"Equivalent types" feature</td>
    </tr>
    <tr>
        <td><a href="#extensibility">Extensibility</a></td>
        <td class="na"></td>
        <td class="fair"></td>
        <td class="best">Can even handle, e.g., systems of "natural" units</td>
        <td class="fair">
            Rep can only be <code>is_arithmetic</code> for now, but
            <a href="https://github.com/aurora-opensource/au/issues/52">plan
            to upgrade</a>
        </td>
    </tr>
    <tr>
        <td><a href="#friction">Friction</a></td>
        <td class="fair"></td>
        <td class="good">
            <ul>
                <li>Single file is good</li>
                <li>
                    Namespaces add verbosity, and friction (e.g.,
                    <code>math::</code> namespace complicates name lookup)
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
        <td><a href="#explicit_measurement_systems">Explicit Systems of Measurement</a></td>
        <td class="good"></td>
        <td class="poor"></td>
        <td class="good"></td>
        <td class="poor">
            Intentional design tradeoff: reduces learning curve, and makes compiler errors shorter.
        </td>
    </tr>
    <tr>
        <td><a href="#nonlinear_scales">Non-linear scales (such as dB)</a></td>
        <td class="poor"></td>
        <td class="best"></td>
        <td class="poor"></td>
        <td class="poor">
            Plan to support someday; see
            <a href="https://github.com/aurora-opensource/au/issues/41">#41</a>.
        </td>
    </tr>
    <tr>
        <td><a href="#ease_of_bringing_into_project">Ease of bringing into project</a></td>
        <td class="fair"></td>
        <td class="good">Single, self-contained header</td>
        <td class="fair"></td>
        <td class="best">Supports single-header delivery, with easy customization</td>
    </tr>
    <tr>
        <td><a href="#point_types">Point types</a></td>
        <td class="good">
            <a href="https://www.boost.org/doc/libs/1_65_0/doc/html/boost/units/absolute.html"><code>absolute</code>
            wrapper</a>
        </td>
        <td class="fair">
            Optional "offset" for units, but can't distinguish quantity from point.
        </td>
        <td class="good"></td>
        <td class="good"></td>
    </tr>
    <tr>
        <td><a href="#zero">Zero</a></td>
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
        <td><a href="#angles">Angles</a></td>
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
        <td><a href="#magnitudes">Magnitudes</a></td>
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
        <td><a href="#macro_usage">Macro usage</a></td>
        <td class="poor">Present in user-facing APIs</td>
        <td class="poor">Present in user-facing APIs</td>
        <td class="good">Confined to outer compatibility layer</td>
        <td class="best">No macros</td>
    </tr>
    <tr>
        <td><a href="#compatibility">C++ Version Compatibility</a></td>
        <td class="best">
            C++98 or C++03<br>
            (unclear which, but best in either case)
        </td>
        <td class="good">C++14</td>
        <td class="poor">C++20</td>
        <td class="good">C++14</td>
    </tr>
</table>

### Explanation of criteria

<span class="criterion" id="mixed_rep_support">Mixed-Rep Support:<a href="#mixed_rep_support" class="headerlink" title="Permanent link">&para;</a></span>

- Can we use arbitrary numeric types to store our quantities' values?  The worst score here means
  there is only a single type (usually `double`). A medium score means there is one type, but it can
  be customized at the library level.  The ideal here is to permit seamlessly and freely mixing
  multiple Rep types.

<span class="criterion" id="compilation_speed">Compilation Speed:<a href="#compilation_speed" class="headerlink" title="Permanent link">&para;</a></span>

- How much does the library prolong compilation?
    - "Good" means that the penalty is close to the level of measurement noise.
    - "Fair" means "measurable, but not subjectively noticeable".
    - "Poor" means that the penalty is so severe that end users tend to notice it.

<span class="criterion" id="compiler_error_readability">Compiler Error Readability:<a href="#compiler_error_readability" class="headerlink" title="Permanent link">&para;</a></span>

- When the library catches a mistake it was designed to catch, how hard is it to understand what has
  gone wrong?
    - The ideal here is to print out the actual unit names which users see in their code.
    - Failing this, the type should at least label its dimensions, instead of using positional
      dimensions.
    - Most important of all is brevity: excessively long error messages are automatically "poor".

<span class="criterion" id="units_as_types">Units as Types:<a href="#units_as_types" class="headerlink" title="Permanent link">&para;</a></span>

- The library should maintain a clear distinction between the concept of a unit, and a quantity of
  that unit.  Units should be represented by types so that we can perform computations on those
  types.  "Poor" means that no such types exist; "fair" means that they do, but that the name is
  conflated with the quantity of that unit.

<span class="criterion" id="generic_dimensions">Generic Dimensions:<a href="#generic_dimensions" class="headerlink" title="Permanent link">&para;</a></span>

- How hard is it to write a generic function which works for any dimensionally-consistent set of
  units?  (For example, a "speed" function which divides a "length" and a "time")

<span class="criterion" id="embedded_friendliness">Embedded friendliness:<a href="#embedded_friendliness" class="headerlink" title="Permanent link">&para;</a></span>

- Key concerns of embedded developers include:
    - Flexibility in the Rep (usually a variety of integral types, and perhaps `float`, but rarely
      `double`).
    - The easy ability to exclude `<iostreams>`.

<span class="criterion" id="ease_of_migration">Ease of Migration:<a href="#ease_of_migration" class="headerlink" title="Permanent link">&para;</a></span>

- How easy is it to migrate from "no units", or some other units library, to this one?
- How easy is it to migrate away from this one to another units library?

<span class="criterion" id="extensibility">Extensibility:<a href="#extensibility" class="headerlink" title="Permanent link">&para;</a></span>

- How easy is it to add a new unit?  How about a new Rep type?

<span class="criterion" id="friction">Friction:<a href="#friction" class="headerlink" title="Permanent link">&para;</a></span>

- How many headers do you have to include, and how hard are they to guess?
- Do users need to contend with a lot of nested namespaces (more friction), or is the structure more
  flat (less friction)?
- Do we provide reasonable implicit conversions?

<span class="criterion" id="explicit_measurement_systems">Explicit Measurement Systems:<a href="#explicit_measurement_systems" class="headerlink" title="Permanent link">&para;</a></span>

- Do we make the measurement system itself explicit, distinguishing between its "base units" and
  "derived units"?  Or do we pick a single, unified, implicit measurement system, so that the
  concept fades into the background?
    - Note that we can make a conscious tradeoff: by removing the ability to support _explicit_
      measurement systems, we can buy simplicity of implementation and use.  Au makes this choice.

<span class="criterion" id="nonlinear_scales">Non-linear scales:<a href="#nonlinear_scales" class="headerlink" title="Permanent link">&para;</a></span>

- Do we support "units" such as dB, which are logarithmic rather than linear?

<span class="criterion" id="ease_of_bringing_into_project">Ease of Bringing Into Project:<a href="#ease_of_bringing_into_project" class="headerlink" title="Permanent link">&para;</a></span>

- How easy is it to add support for this library to a project?

<span class="criterion" id="point_types">Point types:<a href="#point_types" class="headerlink" title="Permanent link">&para;</a></span>

- Can we support "point-type" quantities (temperature; position; etc.) as opposed to
  "displacement-type" quantities (temperature change; displacement; etc.)?  See [this
  article](http://videocortex.io/2018/Affine-Space-Types/) for background.

<span class="criterion" id="zero">Zero:<a href="#zero" class="headerlink" title="Permanent link">&para;</a></span>

- `0` is the only value that is meaningful for a quantity of any units.
    - How easy is it to construct a quantity with this value?
    - How easy is it to compare to zero, or work with the sign of a quantity generally?

<span class="criterion" id="angles">Angles:<a href="#angles" class="headerlink" title="Permanent link">&para;</a></span>

- Are angles given first class support, including degrees and radians?

<span class="criterion" id="magnitudes">Magnitudes:<a href="#magnitudes" class="headerlink" title="Permanent link">&para;</a></span>

- Does the library support different magnitudes of units (e.g., feet and inches) at all?
- If so: is it restricted to a simple ratio, or can it handle roots, large numbers (without
  overflow), and irrational quotients?

<span class="criterion" id="macro_usage">Macro usage:<a href="#macro_usage" class="headerlink" title="Permanent link">&para;</a></span>

- How much does the library depend on macros?  Any macros in the end user interfaces or key
  definitions?  Ideally, we should aim for zero here, or at most for working around specific
  compiler version issues.

<span class="criterion" id="compatibility">C++ Version Compatibility:<a href="#compatibility" class="headerlink" title="Permanent link">&para;</a></span>

- What versions of the C++ standard does the library support?
