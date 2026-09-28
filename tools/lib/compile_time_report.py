# Copyright 2026 Aurora Operations, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


"""Render a compile time run as Markdown that pastes into a pull request comment."""

import textwrap

import compile_time_data as data

# How the report labels a change.  A delta smaller than the combined uncertainty in the two medians
# is reported as noise, not as an improvement or a regression: on a handful of reps, the sign of a
# small delta is not information.
BETTER, WORSE, NOISE = "\N{LARGE GREEN CIRCLE}", "\N{LARGE RED CIRCLE}", "\N{HEAVY MINUS SIGN}"

# The percentiles we use in this report, along with their column headers.
REPORTED_QUANTILES = [0.10, 0.25, 0.50, 0.75, 0.90]
PERCENTILE_COLUMNS = ["min"] + [data.quantile_label(q) for q in REPORTED_QUANTILES] + ["max"]

# Markdown that has to start a line of its own rather than be folded into the line above it: a
# heading, a bullet, a blockquote, a table row, or raw HTML.  See `_prose`.
STARTS_ITS_OWN_LINE = ("#", "- ", "> ", "|", "<")


#
# This file is structured according to the "newspaper rule": important stuff goes up top, and
# anything below is just the next level of detail for what's above.  This way, you can read the file
# until you get bored, and still get the gist of it to the level of detail that you want.
#
# We do this with sections of "Levels".  Level 0 is just `render()`, which is the whole of this
# module's interface; Level 1 is anything that was first mentioned in `render()`; and so on.
#

####################################################################################################
# Level 0: `render`.
####################################################################################################


def render(measurements, manifest, plot_files=()):
    """Return the full Markdown report as a string."""
    m = measurements
    out = ["# Compile time report", ""]
    out += _manifest_section(manifest, m)
    out += _machine_section(manifest)
    out += _headline_section(m)
    out += _au_penalty_section(m)
    out += _percentile_section(m)
    out += _plot_footer(plot_files)
    return "\n".join(out) + "\n"


####################################################################################################
# Level 1: first called by Level 0.
####################################################################################################


def _manifest_section(manifest, m):
    lines = _prose(
        """
        Comparing {branches} across {units} translation unit{unit_s}, {reps}
        repetition{rep_s} each.

        Baseline is `{baseline}`.  Times are for the compile step only (no link), measured by
        replaying the exact `bazel`-derived compiler invocation, so nothing here includes build
        system overhead.
        """.format(
            branches=", ".join("`{}`".format(data.branch_label(b)) for b in m.branches),
            units=len(m.translation_units),
            unit_s="" if len(m.translation_units) == 1 else "s",
            reps=manifest["reps"],
            rep_s="" if manifest["reps"] == 1 else "s",
            baseline=data.branch_label(m.baseline),
        )
    ) + [""]

    missing = m.translation_units_missing_somewhere()
    if missing:
        lines += _prose(
            """
            > Not every branch has every translation unit, so these are excluded from the
            comparisons below: {missing}
            """.format(missing=", ".join("`{}`".format(u.source) for u in missing))
        ) + [""]
    return lines


# What to record about the machine, and what to call it in the report.  Order is deliberate: the
# things that most affect a compile time come first.
MACHINE_ROWS = [
    ("cpu", "CPU"),
    ("cores", "Logical cores"),
    ("cpu_governor", "CPU governor"),
    ("turbo", "Turbo"),
    ("memory", "Memory"),
    ("compiler", "Compiler"),
    ("model", "Model"),
    ("os", "OS"),
    ("bazel", "Bazel"),
    # Not a hostname: see `_machine_id` in `measure-compile-time` for why this is a random local id.
    ("machine_id", "Machine ID"),
    ("load_average", "Load average at start"),
]


def _machine_section(manifest):
    """Record the machine alongside the numbers.

    A compile time is a property of a machine as much as of the code, and this report is meant to be
    filed on a pull request and read later.  Without this section, a future reader has no way to
    know whether two runs are even comparable.
    """
    machine = manifest.get("machine")
    if not machine:
        return []

    rows = [[label, "{}".format(machine[key])] for key, label in MACHINE_ROWS if machine.get(key)]
    timing = []
    if manifest.get("started"):
        timing.append(["Run started", manifest["started"]])
    if manifest.get("finished"):
        timing.append(["Run finished", manifest["finished"]])

    lines = ["<details><summary>Measurement machine</summary>", ""]
    if machine.get("recorded") == "after the fact":
        # Do not let a back-filled record pass for an observed one.  The hardware and the pinned
        # toolchain are almost certainly unchanged, but nothing below was seen while the compiler
        # was actually running, and the load average at the time is gone for good.
        lines += _prose(
            """
            > **Recorded after the run, not during it.**  The machine was described later, from
            the same checkout.  The hardware and the pinned toolchain are the same; the load
            average during the run was not captured and cannot be recovered.
            """
        ) + [""]
    return lines + _table(["Property", "Value"], rows + timing) + ["", "</details>", ""]


def _headline_section(m):
    """Per translation unit, the median on each branch, and how that median moved."""
    others = m.branches[1:]
    header = ["Example", "Unit"] + ["`{}` median".format(data.branch_label(b)) for b in m.branches]
    for b in others:
        # With a single comparison branch, "vs base" is unambiguous.  With more than one, every
        # delta column has a twin, so each has to say which branch it belongs to.
        which = "" if len(others) == 1 else " (`{}`)".format(data.branch_label(b))
        header += [
            "\N{GREEK CAPITAL LETTER DELTA} vs base" + which,
            "\N{GREEK CAPITAL LETTER DELTA} %" + which,
            "p10\N{RIGHTWARDS ARROW}p90 \N{GREEK CAPITAL LETTER DELTA}" + which,
        ]

    rows = []
    for u in m.translation_units_on_all_branches():
        base = _summary(m, u, m.baseline)
        row = [u.example, "`{}`".format(u.source.split("/")[-1]), _ms(base["p50"])]
        for b in others:
            row.append(_ms(_summary(m, u, b)["p50"]))
        for b in others:
            s = _summary(m, u, b)
            row.append(_signed_delta(base, s))
            row.append(_percent(base["p50"], s["p50"]))
            row.append(
                "{} / {}".format(
                    _ms(s["p10"] - base["p10"], signed=True),
                    _ms(s["p90"] - base["p90"], signed=True),
                )
            )
        rows.append(row)

    return ["## Headline: compile time per translation unit", ""] + _table(header, rows) + [""]


def _au_penalty_section(m):
    """What Au costs over hand-rolled C++, and whether this change moved that cost.

    This is the table that only exists because every example is maintained in both flavors.  The
    headline table says how a branch moved; this one says how much of each example's compile time is
    attributable to using the library at all.
    """
    pairs = m.ab_pairs()
    if not pairs:
        return []

    header = ["Example"] + ["`{}` Au penalty (ms)".format(data.branch_label(b)) for b in m.branches]
    others = m.branches[1:]
    for b in others:
        which = "" if len(others) == 1 else " (`{}`)".format(data.branch_label(b))
        header.append("\N{GREEK CAPITAL LETTER DELTA} penalty vs base (ms)" + which)

    rows = []
    for example, raw, au in pairs:
        penalties = {
            b: _summary(m, au, b)["p50"] - _summary(m, raw, b)["p50"] for b in m.branches
        }
        row = [example] + [_ms(penalties[b], signed=True) for b in m.branches]
        for b in others:
            row.append(_ms(penalties[b] - penalties[m.baseline], signed=True))
        rows.append(row)

    return _prose(
        """
        ## Au penalty: what the library costs over hand-rolled C++

        Median `au.cc` minus median `raw.cc`, for each example maintained in both flavors.

        Milliseconds only, deliberately.  A percentage would divide by whatever the `raw.cc` of
        that particular example happens to cost, and that denominator carries no meaning --- it is
        an artifact of how much unrelated work the example does.  Milliseconds are specific to the
        machine that ran the measurement, but they are at least a real quantity.

        **The change in the penalty is the number to trust.**  It is a difference in differences:
        how much the `au.cc` moved, minus how much its `raw.cc` moved.  Since `raw.cc` includes no
        Au headers, nothing in the library can affect it, so whatever it did is what the machine
        did --- and subtracting it cancels that out.  This is stricter than the marks in the
        headline table, which compare each translation unit only against itself on the other
        branch and so cannot detect anything that slowed the whole machine down.  Where the two
        disagree, believe this one.

        The penalty columns carry no mark, unlike the headline table: a difference of four medians
        needs an uncertainty of its own to judge, and this report does not compute one yet.  Read
        the column against the headline deltas beside it, which do come with a noise threshold.
        """
    ) + [""] + _table(header, rows) + _uncontrolled_note(m) + [""]


def _percentile_section(m):
    """Every branch of every translation unit, spelled out across the reported grid."""
    header = ["Unit", "Branch"] + PERCENTILE_COLUMNS
    rows = []
    for u in m.translation_units:
        for b in m.branches:
            if not m.has(u.source, b):
                continue
            s = _summary(m, u, b)
            rows.append(
                ["`{}`".format(u.source), "`{}`".format(data.branch_label(b))]
                + [_ms(s[k]) for k in PERCENTILE_COLUMNS]
            )
    return ["<details><summary>Full percentile breakdown</summary>", ""] + _table(header, rows) + [
        "",
        "</details>",
        "",
    ]


def _plot_footer(plot_files):
    """Point at the PNGs, which are the only plots this report has."""
    if not plot_files:
        return []
    return _prose(
        """
        ## Plots

        The tables above are the whole report in text; these are the pictures.  GitHub cannot
        render an image from a local path, so they are not linked inline: drag the files into the
        comment box and GitHub will upload them and insert a URL that works.

        - `percentiles.png`, `percentiles_zoom.png`: the distribution per branch, time against
        percentile.  The zoomed one is for seeing small differences.
        - `percentile_diff.png`: difference from the baseline, with each example's `raw.cc` drawn
        as a control.  A regression is `au.cc` pulling away from that control, not `au.cc` merely
        sitting above zero.
        - `weather.png`: every measurement against the clock.  Use this to sanity check the
        results, to see whether they were affected by machine load.
        - `au_penalty.png`: what Au costs over hand-rolled C++, per example.

        Files, relative to this report:
        """
    ) + [""] + ["- `{}`".format(p) for p in plot_files] + [""]


####################################################################################################
# Level 2: first called by Level 1.
####################################################################################################


def _prose(text):
    """Unwrap a block of Markdown written as prose into the lines this report emits.

    Prose in this file is wrapped to fit the file; prose in the report is not, and that is not a
    style choice.  GitHub renders a newline inside a paragraph of a comment as a line break, so a
    paragraph that arrives wrapped is displayed wrapped, at whatever width this file happened to
    use.  Every paragraph, and every bullet, therefore has to leave here as exactly one line.

    Writing the paragraphs like this and unwrapping them here is what keeps that requirement from
    being the author's problem.  The alternative --- one implicitly concatenated string literal per
    source line --- puts a load-bearing trailing space at the end of every one of those lines,
    where it cannot be seen and a reflow silently drops it.

    Blank lines separate paragraphs and are preserved; the block is not blank-terminated, so a
    caller appends its own `""` where the report wants one.
    """
    lines = []
    for block in textwrap.dedent(text).strip("\n").split("\n\n"):
        for line in (ln.strip() for ln in block.splitlines()):
            if lines and lines[-1] and not line.startswith(STARTS_ITS_OWN_LINE):
                lines[-1] += " " + line
            else:
                lines.append(line)
        lines.append("")
    return lines[:-1]


def _table(header, rows):
    widths = [len(h) for h in header]
    for row in rows:
        for i, cell in enumerate(row):
            widths[i] = max(widths[i], len(cell))
    fmt = lambda cells: "| " + " | ".join(c.ljust(widths[i]) for i, c in enumerate(cells)) + " |"
    return [fmt(header), "|" + "|".join("-" * (w + 2) for w in widths) + "|"] + [
        fmt(r) for r in rows
    ]


def _summary(m, unit, branch):
    """One translation unit on one branch, summarized over the grid this report reports."""
    return data.summary(m.times_ms(unit.source, branch), quantiles=REPORTED_QUANTILES)


def _ms(value, signed=False):
    return "{:+.1f}".format(value) if signed else "{:.1f}".format(value)


def _signed_delta(base_summary, new_summary):
    """A signed delta of medians, marked for whether it clears the measurement noise."""
    delta = new_summary["p50"] - base_summary["p50"]
    uncertainty = (
        base_summary["median_ci_half_width"] ** 2 + new_summary["median_ci_half_width"] ** 2
    ) ** 0.5
    if abs(delta) <= uncertainty:
        mark = NOISE
    else:
        mark = WORSE if delta > 0 else BETTER
    return "{} {}".format(mark, _ms(delta, signed=True))


def _percent(base, new):
    if not base:
        return "n/a"
    return "{:+.1f}%".format((new - base) / base * 100.0)


def _uncontrolled_note(m):
    """Name the units that cannot appear above, for want of a control to difference against."""
    paired = {u.source for _, raw, au in m.ab_pairs() for u in (raw, au)}
    orphans = [u for u in m.translation_units_on_all_branches() if u.source not in paired]
    if not orphans:
        return []
    return [""] + _prose(
        """
        > Not shown here: {orphans}.  These come from `single_example`, which has no plain-C++
        counterpart, so there is no control to difference against, and only the weaker per-unit
        marks above apply to them.
        """.format(orphans=", ".join("`{}`".format(u.source) for u in orphans))
    )
