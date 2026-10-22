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

"""Plot the results of a `measure-compile-time` run.

The plots are intended to be uploaded to the same GitHub comment where the report is copy-pasted.

Here are the plots, and their purposes:

* `percentiles`
    * The inverse CDF function, showing the distribution of compile times on the vertical axis.
      One panel per group (an example, or a family of benchmarks), with every member overlaid on an
      axis that starts at zero.  Good for showing the absolute compile times, and how much they
      vary, in context.
* `percentiles_zoom`
    * The same curves, one panel per translation unit, each zoomed in to the range of its own data.
      Better for seeing small differences, or for eyeballing the difference between the lines,
      relative to how much each line varies.
* `percentile_diff`
    * The difference from the baseline across the percentiles.  This is the best one for estimating
      the overall impact of a change, and seeing how consistent that estimate is.
* `weather`
    * Simply shows all data points in time order.  This can reveal cases where the CPU load or other
      environmental factors changed over the course of the run, which could make some results
      unreliable.
* `au_penalty`
    * For cases where we have both an Au and a raw flavor, this shows how expensive Au itself is.
      Includes before and after for each instance, with the uncertainty of each.
"""

import argparse
import datetime
import pathlib

import matplotlib

# The default backend wants a display.  Nothing here draws to a screen, and this runs under `bazel
# run` from a terminal, so pick the file-writing backend before anything imports `pyplot`.
matplotlib.use("Agg")

import matplotlib.dates as mdates  # noqa: E402
import matplotlib.lines as mlines  # noqa: E402
import matplotlib.pyplot as plt  # noqa: E402
import matplotlib.ticker as mticker  # noqa: E402

import compile_time_data as data  # noqa: E402


# Panel geometry.  Plots with one panel per group, or per translation unit, wrap into a grid this
# many columns wide.
COLUMNS = 4
PANEL_SIZE = (4.2, 3.4)
DPI = 110

# The percentiles a difference curve is drawn over.
QUANTILES = [i / 100.0 for i in range(101)]

# How many percentiles to trim from each end of a difference curve before deriving the shared
# vertical scale: five, so the scale is set by p5 through p95.  See `_plot_percentile_diff` for why
# the extremes are excluded, and `_central` for the trimming itself.
SCALE_TRIM = 5

# Which top-level directories come first.  Code examples lead, since they are what users actually
# write; benchmarks follow.  Anything else goes last.
GROUP_ORDER = ["examples", "benchmarks"]

# Colors for the members of a group in `percentile_diff`.  Deliberately disjoint from matplotlib's
# default cycle (`C0`, `C1`, ...), which is what the branches use everywhere else.  Red and green
# are left out too, since a reader takes those for "worse" and "better".
MEMBER_COLORS = ["tab:purple", "tab:brown", "tab:olive", "tab:cyan", "tab:pink"]

# The control: grey and dashed, so the eye reads it as a reference rather than a result.
CONTROL_STYLE = {"color": "0.45", "linestyle": "--"}

# Line styles that tell apart the branches in `percentile_diff`, for runs with more than two.  With
# two (the usual case) there is one comparison branch, and every curve is solid.
COMPARISON_LINESTYLES = ["-", ":", "-."]


#
# This file is structured according to the "newspaper rule", the same way
# `tools/bin/measure-compile-time` is: see the comment there for the rule, and for why the levels
# are derived from the *first* place a function is called rather than the deepest.
#

####################################################################################################
# Level 0: `main`.
####################################################################################################


def main():
    args = _parse_args()
    _make_sure_the_output_directory_exists(args.out_dir)
    m = data.load(args.csv)

    _plot_percentiles(m, args.out_dir / "percentiles.png")
    _plot_percentiles_zoom(m, args.out_dir / "percentiles_zoom.png")

    # A one-branch run has nothing to difference against, and an example with no `au.cc`/`raw.cc`
    # pair has no penalty to show.  Both are ordinary, so both are skipped rather than complained
    # about.
    if len(m.branches) > 1:
        _plot_percentile_diff(m, args.out_dir / "percentile_diff.png")
    _plot_weather(m, args.out_dir / "weather.png")
    if m.ab_pairs():
        _plot_au_penalty(m, args.out_dir / "au_penalty.png")


####################################################################################################
# Level 1: first called by Level 0.
####################################################################################################


def _parse_args():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--csv", type=pathlib.Path, required=True)
    parser.add_argument("--out-dir", type=pathlib.Path, required=True)
    return parser.parse_args()


def _make_sure_the_output_directory_exists(out_dir):
    out_dir.mkdir(parents=True, exist_ok=True)


def _plot_percentiles(m, path):
    """Compile time against percentile, from zero: one panel per group, every member overlaid.

    Time goes on the vertical axis, which is what makes the plot read like a duration rather than a
    probability.  Starting from zero shows a difference against the size of the whole compile, and
    sharing a panel puts an example's `raw.cc` and `au.cc` on one scale, so its Au penalty reads
    directly as the gap between them.  Each curve is labeled with its file at the median, since
    color is already spoken for by the branch.
    """
    groups = _groups(m)
    fig, axes = _wrapped_grid(len(groups), "Compile time by percentile")
    for ax, (example, tus) in zip(axes, groups):
        for tu in tus:
            for branch in m.branches:
                quantiles, values = _percentile_curve(m.times_ms(tu.source, branch))
                ax.plot(quantiles, values, linewidth=1.6, color=_branch_color(m, branch))
            if len(tus) > 1:
                _label_the_curve_at_its_median(ax, tu, m.times_ms(tu.source, m.baseline))
        ax.set_title(example, fontsize=10)
        _label_the_percentile_axis(ax)
        ax.set_ylabel("Compile time (ms)")
        ax.set_ylim(0, _slowest(m, tus) * 1.05)
        ax.grid(alpha=0.3)

    _put_one_branch_legend_on_the_figure(fig, m)
    _save(fig, path)


def _plot_percentiles_zoom(m, path):
    """The same curves as `percentiles`, one panel per translation unit, each scaled to its data.

    Members of a group stay out of each other's panels here on purpose.  A shared panel has to span
    every member, and for an example, the distance from `raw.cc` to `au.cc` dwarfs anything a change
    does to either one; the zoom would be gone.
    """
    tus = _translation_units_in_group_order(m)
    fig, axes = _wrapped_grid(len(tus), "Compile time by percentile (zoomed)")
    for ax, tu in zip(axes, tus):
        for branch in m.branches:
            quantiles, values = _percentile_curve(m.times_ms(tu.source, branch))
            ax.plot(quantiles, values, linewidth=1.6, color=_branch_color(m, branch))
        ax.set_title(_short(tu.source), fontsize=10)
        _label_the_percentile_axis(ax)
        ax.set_ylabel("Compile time (ms)")
        ax.grid(alpha=0.3)

    _put_one_branch_legend_on_the_figure(fig, m)
    _save(fig, path)


def _plot_percentile_diff(m, path):
    """Difference from the baseline, one panel per group, with `raw.cc` shown as the control.

    Two choices matter here, and both are about not being fooled:

      * `raw.cc` and `au.cc` share a panel.  `raw.cc` includes no Au headers, so its curve is pure
        measurement noise -- the change cannot touch it.  A regression is `au.cc` pulling away from
        that control, not `au.cc` being above zero.

      * Every panel shares a vertical scale, and that scale is set by the central percentiles.  The
        extreme percentiles of a compile time distribution are dominated by whatever else the
        machine was doing; left to set the scale, they squash the part worth reading into a line.
        What runs off that scale is marked at the edge, rather than silently cut off.
    """
    groups = _difference_curves_by_group(m)
    limits = _common_limits(groups)

    title = "\n".join(
        [
            "Difference from '{}', by percentile".format(data.branch_label(m.baseline)),
            "Grey dashed: the raw.cc control, where a group has one.  "
            "\N{BLACK UP-POINTING SMALL TRIANGLE}\N{BLACK DOWN-POINTING SMALL TRIANGLE} at an "
            "edge: off the scale.",
        ]
    )
    fig, axes = _wrapped_grid(len(groups), title)
    for ax, (example, curves) in zip(axes, groups):
        for label, diffs, style in curves:
            ax.plot(QUANTILES, diffs, label=label, linewidth=1.6, **style)
            _mark_what_runs_off_the_scale(ax, diffs, limits, style["color"])
        ax.axhline(0, color="black", linewidth=1)
        ax.set_ylim(limits)
        ax.set_title(example, fontsize=10)
        _label_the_percentile_axis(ax)
        ax.set_ylabel("\N{GREEK CAPITAL LETTER DELTA} compile time (ms)")
        ax.grid(alpha=0.3)
        ax.legend(fontsize=7)
    _save(fig, path)


def _plot_weather(m, path):
    """Every measurement against the clock, so drift in the machine is visible as drift here.

    Connected by lines, not just dots: a run that slowly heats up looks like a trend, and a trend is
    much easier to see on a line than in a cloud of points.  Every panel shares one time axis, so an
    event that hit the whole machine lines up from panel to panel.
    """
    tus = _translation_units_in_group_order(m)
    fig, axes = _wrapped_grid(
        len(tus), "Individual measurements over time (CPU weather check)", sharex=True
    )
    for ax, tu in zip(axes, tus):
        for branch in m.branches:
            when, times_ms = _measurements_in_time_order(m, tu.source, branch)
            ax.plot(
                when,
                times_ms,
                marker=".",
                markersize=3,
                linewidth=0.7,
                color=_branch_color(m, branch),
            )
        ax.set_title(_short(tu.source), fontsize=10)
        ax.set_ylabel("Compile time (ms)")
        ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
        ax.grid(alpha=0.3)

    _put_one_branch_legend_on_the_figure(fig, m)
    _save(fig, path)


def _plot_au_penalty(m, path):
    """Median `au.cc` minus median `raw.cc`: the price of the library, per example.

    One group of bars per example, one bar per branch, so the change in the penalty is a change in
    bar height within a group.  Each bar carries the ~95% uncertainty of its penalty, which is what
    tells a real change from two bars that differ by chance.
    """
    pairs = m.ab_pairs()
    fig, ax = plt.subplots(
        figsize=(max(6.0, 2.0 * len(pairs) + 3.0), 5.0), layout="constrained"
    )

    width = 0.8 / len(m.branches)
    for i, branch in enumerate(m.branches):
        penalties_with_error_ms = [
            data.au_penalty_with_error_ms(m, raw, au, branch) for _, raw, au in pairs
        ]
        ax.bar(
            [j + i * width for j in range(len(pairs))],
            [penalty_ms for penalty_ms, _ in penalties_with_error_ms],
            yerr=[error_ms for _, error_ms in penalties_with_error_ms],
            capsize=4,
            width=width,
            color=_branch_color(m, branch),
            label=data.branch_label(branch),
        )

    ax.set_xticks([j + 0.4 - width / 2 for j in range(len(pairs))])
    ax.set_xticklabels([example for example, _, _ in pairs])
    ax.set_ylabel("Median au.cc \N{MINUS SIGN} median raw.cc (ms)")
    ax.set_title("Au compile time penalty, per example (whiskers: ~95% uncertainty)")
    ax.grid(alpha=0.3, axis="y")
    ax.legend(fontsize=8)
    _save(fig, path)


####################################################################################################
# Level 2: first called by Level 1.
####################################################################################################


def _groups(m):
    """The measured translation units, one list per group, in the order the plots lay them out.

    A group is an example, or a family of benchmarks sharing a folder.  Examples come first, then
    benchmarks; see `GROUP_ORDER`.  Within a group, see `_member_order`.

    Returns `[(example, [translation_unit, ...]), ...]`.
    """
    groups = {}
    for tu in m.translation_units_on_all_branches():
        groups.setdefault(tu.example, []).append(tu)
    return [
        (example, sorted(tus, key=lambda tu: _member_order(m, tu)))
        for example, tus in sorted(
            groups.items(), key=lambda item: (_top_level_rank(item[1][0]), item[0])
        )
    ]


def _wrapped_grid(count, title, sharex=False):
    """A grid of `count` panels, wrapped `COLUMNS` wide, with any spare panels hidden.

    Constrained layout, rather than `tight_layout`, because it leaves room for the figure's title
    and legend; `tight_layout` lays the panels out as if neither existed, and the first row ends up
    underneath them.
    """
    columns = min(COLUMNS, count)
    rows = -(-count // columns)
    fig, axes = plt.subplots(
        rows,
        columns,
        figsize=(PANEL_SIZE[0] * columns, PANEL_SIZE[1] * rows),
        squeeze=False,
        sharex=sharex,
        layout="constrained",
    )
    flat = [ax for row in axes for ax in row]
    for ax in flat[count:]:
        ax.axis("off")
    if sharex:
        # A shared axis labels only the bottom row.  When the last row is ragged, the panels above
        # its gaps are the bottom of their columns, and would otherwise have no time labels at all.
        for ax in flat[max(0, count - columns) : count]:
            ax.xaxis.set_tick_params(labelbottom=True)
    fig.suptitle(title)
    return fig, flat[:count]


def _percentile_curve(times_ms):
    """Compile times, sorted, paired with the percentile each one sits at.

    The positions are `i / (n - 1)`, so the fastest measurement is at 0% and the slowest at 100%.
    That is not the textbook ECDF, which is a step function with the `i`th measurement at `i / n`;
    it is the piecewise-linear curve of `data.percentile`, and the vertices below are exactly the
    points that function interpolates between.  Matching it is the whole point: the reader who
    measures a percentile off this plot and the report that computes one from the same numbers
    should not be able to disagree, and `percentile_diff` is drawn with `data.percentile` outright.

    Returns `(quantiles, times_ms)`, in that order, because that is the order `ax.plot` wants them.
    """
    values = sorted(times_ms)
    if len(values) == 1:
        # One measurement is every percentile, so no position is more honest than any other.  The
        # median is the least misleading place to put the single point this draws.
        return [0.5], values
    return [i / (len(values) - 1) for i in range(len(values))], values


def _branch_color(m, branch):
    """The color of a branch, the same in every plot: matplotlib's default cycle, in CLI order."""
    return "C{}".format(m.branches.index(branch) % 10)


def _label_the_curve_at_its_median(ax, tu, times_ms):
    """Name a curve's file just above its middle.

    The tails are where a group's curves bunch up, since the extremes are dominated by the machine
    rather than by the code, so the middle is where they are furthest apart.
    """
    ax.annotate(
        _file_name(tu),
        xy=(0.5, data.percentile(sorted(times_ms), 0.5)),
        xytext=(0, 4),
        textcoords="offset points",
        ha="center",
        va="bottom",
        fontsize=7,
        color="0.2",
        bbox={
            "boxstyle": "round,pad=0.15",
            "facecolor": "white",
            "edgecolor": "none",
            "alpha": 0.8,
        },
    )


def _label_the_percentile_axis(ax):
    """Both percentile plots put a percentile on the horizontal axis; label it the same way."""
    ax.set_xlabel("Percentile")
    ax.xaxis.set_major_formatter(mticker.PercentFormatter(xmax=1.0, decimals=0))


def _slowest(m, tus):
    """The slowest measurement of any of these translation units, on any branch."""
    return max(max(m.times_ms(tu.source, b)) for tu in tus for b in m.branches)


def _put_one_branch_legend_on_the_figure(fig, m):
    """One legend for the whole figure, below the panels.

    Every panel carries the same branches in the same colors, so a legend on each would be the same
    key over and over, in the space the data should have.
    """
    handles = [
        mlines.Line2D([], [], color=_branch_color(m, b), linewidth=1.6)
        for b in m.branches
    ]
    fig.legend(
        handles,
        [data.branch_label(b) for b in m.branches],
        # Not above: constrained layout puts an upper legend in the same band as the title.
        loc="outside lower center",
        ncols=len(m.branches),
        fontsize=9,
    )


def _save(fig, path):
    fig.savefig(path, dpi=DPI)
    plt.close(fig)
    print("Wrote {}".format(path))


def _translation_units_in_group_order(m):
    """Every translation unit, group by group, so related ones sit next to each other."""
    return [tu for _, tus in _groups(m) for tu in tus]


def _short(source):
    """`examples/adc_millivolts/au.cc` reads better on an axis as `adc_millivolts/au.cc`."""
    parts = source.split("/")
    return "/".join(parts[-2:]) if len(parts) > 1 else source


def _difference_curves_by_group(m):
    """One entry per group: its difference curves, styled.

    `atomic_units` and the benchmarks have no `raw.cc`, so their curves are all results, and none
    is styled as a control.  Pairing two of them against each other would look like a control
    without being one.

    Returns `[(example, [(label, diffs, style), ...]), ...]`.
    """
    groups = []
    for example, tus in _groups(m):
        results = [tu for tu in tus if tu.flavor != "raw"]
        curves = [
            (
                _curve_label(m, tu, branch),
                _difference_from_the_baseline(m, tu, branch),
                _curve_style(tu, results, k),
            )
            for tu in tus
            for k, branch in enumerate(m.branches[1:])
        ]
        groups.append((example, curves))
    return groups


def _common_limits(groups):
    """One vertical scale for every difference panel, set by the central percentiles alone."""
    central = [d for _, curves in groups for _, diffs, _ in curves for d in _central(diffs)]
    if not central:
        return (-1.0, 1.0)
    lo, hi = min(central + [0.0]), max(central + [0.0])
    pad = 0.1 * ((hi - lo) or 1.0)
    return (lo - pad, hi + pad)


def _mark_what_runs_off_the_scale(ax, diffs, limits, color):
    """A small triangle on the edge at every percentile where a curve leaves the shared scale.

    The scale is chosen to cut off the extremes, so cut-off curves are expected.  But a line that
    simply exits the panel looks like a line that ends, and that hides the most alarming kind of
    result: the one too big to fit.
    """
    lo, hi = limits
    for marker, edge, off in (("^", hi, lambda d: d > hi), ("v", lo, lambda d: d < lo)):
        qs = [q for q, d in zip(QUANTILES, diffs) if off(d)]
        if qs:
            ax.plot(
                qs,
                [edge] * len(qs),
                linestyle="none",
                marker=marker,
                markersize=4,
                color=color,
                clip_on=False,
            )


def _measurements_in_time_order(m, source, branch):
    """When each measurement of `source` on `branch` was taken, and how long it took.

    `m.rows` is in measurement order, which is the whole point of this plot: it is the one view
    where a run's history is the horizontal axis rather than something aggregated away.
    """
    rows = [r for r in m.rows if r["source"] == source and r["branch"] == branch]
    return [_parse_time(r["timestamp"]) for r in rows], [r["nanos"] / 1e6 for r in rows]


####################################################################################################
# Level 3: first called by Level 2.
####################################################################################################


def _member_order(m, tu):
    """Where a translation unit sits within its group: `raw` first, then fastest to slowest.

    `raw` first, because it is the control, and the eye should meet the reference before the
    result.  Then by median compile time on the baseline, which puts a family of benchmarks at
    different sizes in order of size, without anyone having to spell that order out.
    """
    return (tu.flavor != "raw", data.summary(m.times_ms(tu.source, m.baseline))["p50"])


def _top_level_rank(tu):
    """Where a translation unit's top-level directory comes in `GROUP_ORDER`."""
    top = tu.source.split("/")[0]
    return GROUP_ORDER.index(top) if top in GROUP_ORDER else len(GROUP_ORDER)


def _file_name(tu):
    return tu.source.split("/")[-1]


def _curve_label(m, tu, branch):
    """The file name, plus the branch when there is more than one to tell apart."""
    label = _file_name(tu)
    if len(m.branches) > 2:
        return "{} @ {}".format(label, data.branch_label(branch))
    return label


def _difference_from_the_baseline(m, tu, branch):
    """`branch` minus the baseline, percentile by percentile, for one translation unit.

    Percentile against percentile, rather than measurement against measurement: the two runs are
    interleaved, which spreads the machine's weather evenly across both, but run-to-run variation
    is high even between neighbors, so the `k`th compile on one branch is no better a partner for
    the `k`th on the other than any compile is.
    """
    base = sorted(m.times_ms(tu.source, m.baseline))
    other = sorted(m.times_ms(tu.source, branch))
    return [data.percentile(other, q) - data.percentile(base, q) for q in QUANTILES]


def _curve_style(tu, results, k):
    """How one difference curve is drawn: the control in grey, each result in a color of its own.

    `results` is the group's non-control members, which is what the colors are assigned over; `k`
    counts the comparison branches, which get line styles only when there is more than one.
    """
    if tu.flavor == "raw":
        return dict(CONTROL_STYLE)
    return {
        "color": MEMBER_COLORS[results.index(tu) % len(MEMBER_COLORS)],
        "linestyle": COMPARISON_LINESTYLES[k % len(COMPARISON_LINESTYLES)],
    }


def _central(curve):
    """A difference curve with `SCALE_TRIM` percentiles trimmed from each end: p5 through p95."""
    return curve[SCALE_TRIM : len(QUANTILES) - SCALE_TRIM]


def _parse_time(stamp):
    return datetime.datetime.fromisoformat(stamp)


if __name__ == "__main__":
    main()
