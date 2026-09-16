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

"""Shared vocabulary for compile time measurements: the CSV schema, and the statistics we read."""

import collections
import csv
import math
import sys

# The expected CSV columns.  We require that the _actual_ columns supply all of these.
FIELDS = ["timestamp", "example", "flavor", "source", "target", "branch", "rep", "nanos"]

# One translation unit: the thing we actually time.  Its source path is its identity, since every
# example source lives at a distinct path, and that path is stable across branches.
TranslationUnit = collections.namedtuple("TranslationUnit", ["source", "example", "flavor"])


def classify(source):
    """Derive the example name and flavor from an example source path.

    `examples/adc_millivolts/au.cc` is the `au` flavor of the `adc_millivolts` example; `raw.cc` is
    its counterpart.  Any other stem gets the `single` flavor: it was built by the `single_example`
    macro rather than `ab_example`, so it has no counterpart to pair against.
    """
    parts = source.split("/")
    example = parts[1] if len(parts) > 2 else parts[0]
    stem = parts[-1].rsplit(".", 1)[0]
    flavor = stem if stem in ("raw", "au") else "single"
    return TranslationUnit(source=source, example=example, flavor=flavor)


class Measurements:
    """Every measurement from one run, indexed in the ways the report needs to read it."""

    def __init__(self, rows, branch_order=None):
        self.rows = rows
        self.branches = _ordered_branches(rows, branch_order)
        # Sorted by path, not by order of appearance: the measurement loop shuffles translation
        # units, and a report whose rows come out in a different order every run is needlessly hard
        # to compare.
        self.translation_units = [classify(s) for s in sorted({r["source"] for r in rows})]
        self._by_unit_branch = collections.defaultdict(list)
        for r in rows:
            self._by_unit_branch[(r["source"], r["branch"])].append(r["nanos"] / 1e6)

    @property
    def baseline(self):
        """The branch everything else is compared against: the first one the user named.

        Which is only true if someone said what that order was -- see `_ordered_branches`.
        """
        return self.branches[0]

    def times_ms(self, source, branch):
        """Every measurement of one translation unit on one branch, in milliseconds."""
        return self._by_unit_branch.get((source, branch), [])

    def has(self, source, branch):
        return bool(self._by_unit_branch.get((source, branch)))

    def translation_units_on_all_branches(self):
        """The translation units we can legitimately diff.

        A branch that adds an example has a translation unit the baseline never had.  Diffing that
        against nothing would be a lie, so the diff tables use this list and report the rest
        separately.
        """
        return [
            u
            for u in self.translation_units
            if all(self.has(u.source, b) for b in self.branches)
        ]

    def translation_units_missing_somewhere(self):
        complete = {u.source for u in self.translation_units_on_all_branches()}
        return [u for u in self.translation_units if u.source not in complete]

    def ab_pairs(self):
        """`(example, raw, au)` for every example with both flavors on every branch."""
        by_example = collections.defaultdict(dict)
        for u in self.translation_units_on_all_branches():
            by_example[u.example][u.flavor] = u
        pairs = []
        for example in _unique(u.example for u in self.translation_units):
            flavors = by_example.get(example, {})
            if "raw" in flavors and "au" in flavors:
                pairs.append((example, flavors["raw"], flavors["au"]))
        return pairs


def load(path, branch_order=None):
    """Read a measurement CSV back, keyed by the column names in its own header row.

    Every column in `FIELDS` has to be there.  Anything else the file carries is fine and is read
    along with the rest: the statistics here only ever look at the columns they know about, so a
    file from a version that recorded more than we do is still a file we can report on.

    Pass `branch_order` -- the branches as the user named them, which the run's manifest records
    -- to say which one is the baseline.  Without it, see `_ordered_branches`.
    """
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        _check_header(reader.fieldnames, path)
        rows = []
        for row in reader:
            row["rep"] = int(row["rep"])
            row["nanos"] = int(row["nanos"])
            rows.append(row)
    return Measurements(rows, branch_order=branch_order)


def _ordered_branches(rows, branch_order):
    """The branches in this data, in the order the comparison is meant to be read.

    `branch_order` provides the branches whose ordering we care about.  We'll return those first, in
    that order, and then return the rest, in any order.

    The goal is to preserve information about the order specified by the user, because we document
    that the first branch named is the one that acts as the "baseline".
    """
    seen = _unique(r["branch"] for r in rows)
    if not branch_order:
        return seen
    named = [b for b in branch_order if b in set(seen)]
    return named + [b for b in seen if b not in set(named)]


def _check_header(columns, path):
    """Refuse a CSV that is missing something we need; mention one that carries more than we read.
    """
    if not columns:
        raise ValueError(
            "{}: no header row; expected one naming {}".format(path, ", ".join(FIELDS))
        )
    missing = [f for f in FIELDS if f not in columns]
    if missing:
        raise ValueError(
            "{}: header is missing {}; it names {}".format(
                path, ", ".join(missing), ", ".join(columns)
            )
        )
    unrecognized = [c for c in columns if c not in FIELDS]
    if unrecognized:
        print(
            "warning: {}: extra column(s) not used here: {}".format(
                path, ", ".join(unrecognized)
            ),
            file=sys.stderr,
        )


def percentile(sorted_values, q):
    """The `q`th quantile (0 to 1) of an already-sorted list, interpolating between neighbors.

    This is the piecewise-linear curve through the points `((i - 1) / (n - 1), x_i)`: plot the
    sorted measurements and join them to their neighbors with straight lines.  It is the default in
    numpy (`method="linear"`), in R (`type=7`), and in `statistics.quantiles(method="inclusive")`,
    so a reader who recomputes one of our numbers with any of those gets our answer back.
    """
    if not sorted_values:
        return float("nan")
    if len(sorted_values) == 1:
        return sorted_values[0]
    pos = q * (len(sorted_values) - 1)
    lo = int(math.floor(pos))
    hi = min(lo + 1, len(sorted_values) - 1)
    return sorted_values[lo] + (sorted_values[hi] - sorted_values[lo]) * (pos - lo)


def median_ci_half_width(values):
    """Half-width of the ~95% confidence interval of the median.

    This is the same `1.58 * IQR / sqrt(n)` that gives a notched box plot its notch.  We use it to
    decide whether a difference of medians is worth reporting as a difference at all: compile time
    measurements are noisy enough that a small delta on a small number of reps means nothing, and
    the report should say so rather than let the reader over-read a sign.
    """
    if len(values) < 2:
        return float("inf")
    s = sorted(values)
    iqr = percentile(s, 0.75) - percentile(s, 0.25)
    return 1.58 * iqr / math.sqrt(len(s))


def quantile_label(q):
    """The key `summary` files a quantile under: `0.1` is `p10`, `0.999` is `p99.9`."""
    return "p" + "{:.10f}".format(q * 100).rstrip("0").rstrip(".")


def summary(values, quantiles=()):
    """The spread of one series: `n`, `min`, `max`, the median and its CI, and any grid asked for.

    `n`, `min`, `max`, `p50` and `median_ci_half_width` are always here.  The first three describe
    the series; the last two are the pair the report reasons with --- `p50` is what a comparison
    moves, and the half-width is how it decides whether the move means anything.
    """
    s = sorted(values)
    result = {
        "n": len(s),
        "min": s[0] if s else float("nan"),
        "max": s[-1] if s else float("nan"),
        "p50": percentile(s, 0.50),
        "median_ci_half_width": median_ci_half_width(s),
    }
    for q in quantiles:
        result[quantile_label(q)] = percentile(s, q)
    return result


def branch_label(branch):
    """How a branch is named in a report or a plot legend.

    `measure-branch-impact` passes a merge-base commit as the baseline, and a full 40 character hash
    turns every table column into a wall.  Abbreviate hashes; leave real branch names alone, since
    those are what a reader recognizes.
    """
    if len(branch) == 40 and all(c in "0123456789abcdef" for c in branch):
        return branch[:10]
    return branch


def _unique(items):
    """The distinct items, in first-seen order.  Order is meaningful: branches keep CLI order."""
    return list(dict.fromkeys(items))
