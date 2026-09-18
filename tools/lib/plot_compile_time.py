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
      Good for showing the absolute compile times, and how much they vary, in context.
* `percentiles_zoom`
    * The same plot, zoomed in to the range of the data.  Better for seeing small differences, or
      for eyeballing the difference between the lines, relative to how much each line varies.
* `percentile_diff`
    * The difference from the baseline across the percentiles.  This is the best one for estimating
      the overall impact of a change, and seeing how consistent that estimate is.
* `weather`
    * Simply shows all data points in time order.  This can reveal cases where the CPU load or other
      environmental factors changed over the course of the run, which could make some results
      unreliable.
* `au_penalty`
    * For cases where we have both an Au and a raw flavor, this shows how expensive Au itself is.
      Includes before and after for each instance.
"""

import argparse
import datetime
import pathlib

import matplotlib

# The default backend wants a display.  Nothing here draws to a screen, and this runs under `bazel
# run` from a terminal, so pick the file-writing backend before anything imports `pyplot`.
matplotlib.use("Agg")

import matplotlib.dates as mdates  # noqa: E402
import matplotlib.pyplot as plt  # noqa: E402
import matplotlib.ticker as mticker  # noqa: E402

import compile_time_data as data  # noqa: E402


# Panel geometry.  The per-translation-unit plots lay themselves out by example (see
# `_example_grid`); `COLUMNS` applies only to the plots with one panel per example, which wrap into
# a grid.
COLUMNS = 4
PANEL_SIZE = (4.2, 3.4)
DPI = 110

# The percentiles a difference curve is drawn over.
QUANTILES = [i / 100.0 for i in range(101)]

# How many percentiles to trim from each end of a difference curve before deriving the shared
# vertical scale: five, so the scale is set by p5 through p95.  See `_plot_percentile_diff` for why
# the extremes are excluded, and `_central` for the trimming itself.
SCALE_TRIM = 5


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

    _plot_percentiles(m, args.out_dir / "percentiles.png", from_zero=True)
    _plot_percentiles(m, args.out_dir / "percentiles_zoom.png", from_zero=False)

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


def _plot_percentiles(m, path, from_zero):
    """Compile time against percentile: an inverse CDF, with time on the vertical axis.

    That orientation is what makes the plot read like a duration rather than a probability.  With
    `from_zero`, the axis starts at zero, so a difference is seen against the size of the whole
    compile; without it, the same plot is zoomed to whatever range the data occupies.
    """
    title = "Compile time by percentile" + ("" if from_zero else " (zoomed)")
    fig, rows = _example_grid(m, title)
    for _, panels in rows:
        for tu, ax in panels:
            for branch in m.branches:
                quantiles, values = _percentile_curve(m.times_ms(tu.source, branch))
                ax.plot(
                    quantiles,
                    values,
                    label=data.branch_label(branch),
                    linewidth=1.6,
                )
            ax.set_title(_short(tu.source), fontsize=10)
            _label_the_percentile_axis(ax)
            ax.set_ylabel("Compile time (ms)")
            ax.grid(alpha=0.3)

        if from_zero:
            _share_a_scale_starting_at_zero(m, panels)

    _put_one_legend_on_the_figure(fig)
    _save(fig, path)


def _plot_percentile_diff(m, path):
    """Difference from the baseline, one panel per example, with `raw.cc` shown as the control.

    Two choices matter here, and both are about not being fooled:

      * `raw.cc` and `au.cc` share a panel.  `raw.cc` includes no Au headers, so its curve is pure
        measurement noise -- the change cannot touch it.  A regression is `au.cc` pulling away from
        that control, not `au.cc` being above zero.

      * Every panel shares a vertical scale, and that scale is set by the central percentiles.  The
        extreme percentiles of a compile time distribution are dominated by whatever else the
        machine was doing; left to set the scale, they squash the part worth reading into a line.
    """
    groups = _difference_curves_by_example(m)
    limits = _common_limits(groups)

    title = "Difference from '{}', by percentile".format(data.branch_label(m.baseline))
    fig, axes = _wrapped_grid(len(groups), title)
    for ax, (example, curves, has_control) in zip(axes, groups):
        for label, diffs, style in curves:
            ax.plot(QUANTILES, diffs, label=label, linewidth=1.6, **style)
        ax.axhline(0, color="black", linewidth=1)
        ax.set_ylim(limits)
        ax.set_title(example + ("" if has_control else " (no control)"), fontsize=10)
        _label_the_percentile_axis(ax)
        ax.set_ylabel("\N{GREEK CAPITAL LETTER DELTA} compile time (ms)")
        ax.grid(alpha=0.3)
        ax.legend(fontsize=7)
    _save(fig, path)


def _plot_weather(m, path):
    """Every measurement against the clock, so drift in the machine is visible as drift here.

    Connected by lines, not just dots: a run that slowly heats up looks like a trend, and a trend is
    much easier to see on a line than in a cloud of points.
    """
    fig, rows = _example_grid(m, "Individual measurements over time (CPU weather check)")
    for _, panels in rows:
        for tu, ax in panels:
            for branch in m.branches:
                when, times_ms = _measurements_in_time_order(m, tu.source, branch)
                ax.plot(
                    when,
                    times_ms,
                    marker=".",
                    markersize=3,
                    linewidth=0.7,
                    label=data.branch_label(branch),
                )
            ax.set_title(_short(tu.source), fontsize=10)
            ax.set_ylabel("Compile time (ms)")
            ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
            ax.grid(alpha=0.3)

    _put_one_legend_on_the_figure(fig)
    _save(fig, path)


def _plot_au_penalty(m, path):
    """Median `au.cc` minus median `raw.cc`: the price of the library, per example.

    One group of bars per example, one bar per branch, so the change in the penalty is a change in
    bar height within a group.
    """
    pairs = m.ab_pairs()
    fig, ax = plt.subplots(figsize=(max(6.0, 2.0 * len(pairs) + 3.0), 5.0))

    width = 0.8 / len(m.branches)
    for i, branch in enumerate(m.branches):
        ax.bar(
            [j + i * width for j in range(len(pairs))],
            _median_au_penalties_ms(m, pairs, branch),
            width=width,
            label=data.branch_label(branch),
        )

    ax.set_xticks([j + 0.4 - width / 2 for j in range(len(pairs))])
    ax.set_xticklabels([example for example, _, _ in pairs])
    ax.set_ylabel("Median au.cc \N{MINUS SIGN} median raw.cc (ms)")
    ax.set_title("Au compile time penalty, per example")
    ax.grid(alpha=0.3, axis="y")
    _put_one_legend_on_the_figure(fig)
    _save(fig, path)


####################################################################################################
# Level 2: first called by Level 1.
####################################################################################################


def _example_grid(m, title):
    """A grid with one row per example and one column per translation unit within it.

    Rows grow with the number of examples, which is the dimension that actually grows here, and a
    tall figure is no problem for the place these end up: a pull request comment scrolls vertically.
    Ragged rows (an example with fewer translation units than the widest one) get their spare panels
    hidden.

    Returns `(fig, [(example, [(translation_unit, ax), ...]), ...])`.
    """
    groups = _translation_units_by_example(m)
    columns = max(len(tus) for _, tus in groups)
    fig, axes = plt.subplots(
        len(groups),
        columns,
        figsize=(PANEL_SIZE[0] * columns, PANEL_SIZE[1] * len(groups)),
        squeeze=False,
    )
    laid_out = []
    for row, (example, tus) in enumerate(groups):
        for ax in axes[row][len(tus) :]:
            ax.axis("off")
        laid_out.append((example, list(zip(tus, axes[row]))))
    fig.suptitle(title)
    return fig, laid_out


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


def _short(source):
    """`examples/adc_millivolts/au.cc` reads better on an axis as `adc_millivolts/au.cc`."""
    parts = source.split("/")
    return "/".join(parts[-2:]) if len(parts) > 1 else source


def _label_the_percentile_axis(ax):
    """Both percentile plots put a percentile on the horizontal axis; label it the same way."""
    ax.set_xlabel("Percentile")
    ax.xaxis.set_major_formatter(mticker.PercentFormatter(xmax=1.0, decimals=0))


def _share_a_scale_starting_at_zero(m, panels):
    """Give every panel in one row the same vertical scale, running from zero.

    An example's Au penalty then reads directly as the height difference between its `raw.cc` and
    `au.cc` panels.  The zoomed variant deliberately does not do this: there, each panel is scaled
    to its own data, which is what makes a small difference visible at all.
    """
    top = max(max(m.times_ms(tu.source, b)) for tu, _ in panels for b in m.branches)
    for _, ax in panels:
        ax.set_ylim(0, top * 1.02)


def _put_one_legend_on_the_figure(fig):
    """One legend, on the first panel.

    Every panel carries the same branches in the same colors, so a legend on each would be the same
    key over and over, in the space the data should have.
    """
    fig.axes[0].legend(fontsize=8)


def _save(fig, path):
    fig.tight_layout()
    fig.savefig(path, dpi=DPI)
    plt.close(fig)
    print("Wrote {}".format(path))


def _difference_curves_by_example(m):
    """One group per example: its difference curves, and whether one of them is a control.

    `atomic_units` and anything else built by `single_example` has no `raw.cc`, so it gets a group
    with no control in it.  Pairing its two translation units against each other would look like a
    control without being one.

    Returns `[(example, [(label, diffs, style), ...], has_control), ...]`.
    """
    groups = []
    for example, tus in _translation_units_by_example(m):
        curves = [
            (
                _curve_label(m, tu, branch),
                _difference_from_the_baseline(m, tu, branch),
                _curve_style(tu),
            )
            for tu in tus
            for branch in m.branches[1:]
        ]
        groups.append((example, curves, any(tu.flavor == "raw" for tu in tus)))
    return groups


def _common_limits(groups):
    """One vertical scale for every difference panel, set by the central percentiles alone."""
    central = [
        d for _, curves, _ in groups for _, diffs, _ in curves for d in _central(diffs)
    ]
    if not central:
        return (-1.0, 1.0)
    lo, hi = min(central + [0.0]), max(central + [0.0])
    pad = 0.1 * ((hi - lo) or 1.0)
    return (lo - pad, hi + pad)


def _wrapped_grid(count, title):
    """A wrapped grid of `count` panels, for plots with one panel per example."""
    columns = min(COLUMNS, count)
    rows = -(-count // columns)
    fig, axes = plt.subplots(
        rows,
        columns,
        figsize=(PANEL_SIZE[0] * columns, PANEL_SIZE[1] * rows),
        squeeze=False,
    )
    flat = [ax for row in axes for ax in row]
    for ax in flat[count:]:
        ax.axis("off")
    fig.suptitle(title)
    return fig, flat[:count]


def _measurements_in_time_order(m, source, branch):
    """When each measurement of `source` on `branch` was taken, and how long it took.

    `m.rows` is in measurement order, which is the whole point of this plot: it is the one view
    where a run's history is the horizontal axis rather than something aggregated away.
    """
    rows = [r for r in m.rows if r["source"] == source and r["branch"] == branch]
    return [_parse_time(r["timestamp"]) for r in rows], [r["nanos"] / 1e6 for r in rows]


def _median_au_penalties_ms(m, pairs, branch):
    """For each `(example, raw, au)` pair, how much longer the Au flavor takes, at the median."""
    return [
        data.summary(m.times_ms(au.source, branch))["p50"]
        - data.summary(m.times_ms(raw.source, branch))["p50"]
        for _, raw, au in pairs
    ]


####################################################################################################
# Level 3: first called by Level 2.
####################################################################################################


def _translation_units_by_example(m):
    """The measured translation units, one list per example, `raw` before `au`.

    This grouping is the layout for the per-translation-unit plots: one row per example, so an
    example's `raw.cc` and `au.cc` sit side by side.  That is the comparison a reader actually
    makes, and it is the same pairing `_plot_percentile_diff` gets by overlaying the two curves.
    """
    groups = {}
    for tu in m.translation_units_on_all_branches():
        groups.setdefault(tu.example, []).append(tu)
    return [
        (example, sorted(tus, key=lambda tu: (tu.flavor != "raw", tu.source)))
        for example, tus in sorted(groups.items())
    ]


def _curve_label(m, tu, branch):
    """The file name, plus the branch when there is more than one to tell apart."""
    label = _short(tu.source).split("/")[-1]
    if len(m.branches) > 2:
        return "{} @ {}".format(label, data.branch_label(branch))
    return label


def _difference_from_the_baseline(m, tu, branch):
    """`branch` minus the baseline, percentile by percentile, for one translation unit.

    Percentile against percentile, rather than measurement against measurement: the two runs are
    interleaved but not paired, so the `k`th compile on one branch has no partner on the other.
    """
    base = sorted(m.times_ms(tu.source, m.baseline))
    other = sorted(m.times_ms(tu.source, branch))
    return [data.percentile(other, q) - data.percentile(base, q) for q in QUANTILES]


def _curve_style(tu):
    """Dashed and grey for the control, so the eye reads it as a reference rather than a result."""
    if tu.flavor == "raw":
        return {"color": "0.45", "linestyle": "--"}
    return {"linestyle": "-"}


def _central(curve):
    """A difference curve with `SCALE_TRIM` percentiles trimmed from each end: p5 through p95."""
    return curve[SCALE_TRIM : len(QUANTILES) - SCALE_TRIM]


def _parse_time(stamp):
    return datetime.datetime.fromisoformat(stamp)


if __name__ == "__main__":
    main()
