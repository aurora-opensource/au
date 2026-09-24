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

"""Tests for the contract `compile_time_data` offers its readers.

The statistics here are the kind that fail silently: a percentile off by one index produces a
plausible number rather than an error.  So we pin the cases whose answers can be worked out by
hand, plus the schema promises (column order, branch order, milliseconds) that the report and the
plots both rely on.
"""

import contextlib
import csv
import io
import math
import os
import tempfile
import unittest

import compile_time_data as data


def rows(*specs):
    """Rows from `(source, branch, nanos)` triples, with the derived fields filled in."""
    out = []
    for rep, (source, branch, nanos) in enumerate(specs):
        tu = data.classify(source)
        out.append(
            {
                "timestamp": "2026-09-16T00:00:00",
                "example": tu.example,
                "flavor": tu.flavor,
                "source": source,
                "target": "//examples/" + tu.example,
                "branch": branch,
                "rep": rep,
                "nanos": nanos,
            }
        )
    return out


def measurement(**overrides):
    """One plausible measurement row, with the columns `load` requires."""
    row = {
        "timestamp": "2026-09-16T00:00:00",
        "example": "adc_millivolts",
        "flavor": "au",
        "source": "examples/adc_millivolts/au.cc",
        "target": "//examples/adc_millivolts:au",
        "branch": "main",
        "rep": 3,
        "nanos": 5000000,
    }
    row.update(overrides)
    return row


class ClassifyTest(unittest.TestCase):
    def test_au_and_raw_stems_are_paired_flavors_of_one_example(self):
        au = data.classify("examples/adc_millivolts/au.cc")
        raw = data.classify("examples/adc_millivolts/raw.cc")
        self.assertEqual((au.example, au.flavor), ("adc_millivolts", "au"))
        self.assertEqual((raw.example, raw.flavor), ("adc_millivolts", "raw"))

    def test_other_stems_are_single_flavor(self):
        tu = data.classify("examples/nested_dimensionless/main.cc")
        self.assertEqual((tu.example, tu.flavor), ("nested_dimensionless", "single"))

    def test_shallow_path_names_the_example_after_the_file(self):
        # Nothing to pull an example name out of, so the first component has to serve as one.
        tu = data.classify("single-file-test.cc")
        self.assertEqual((tu.example, tu.flavor), ("single-file-test.cc", "single"))

    def test_source_is_preserved_verbatim_as_the_identity(self):
        self.assertEqual(data.classify("examples/a/au.cc").source, "examples/a/au.cc")


class PercentileTest(unittest.TestCase):
    def test_empty_is_nan(self):
        self.assertTrue(math.isnan(data.percentile([], 0.5)))

    def test_single_value_is_that_value_at_every_quantile(self):
        self.assertEqual(data.percentile([7.0], 0.0), 7.0)
        self.assertEqual(data.percentile([7.0], 0.99), 7.0)

    def test_endpoints_are_the_extremes(self):
        s = [1.0, 2.0, 3.0, 4.0]
        self.assertEqual(data.percentile(s, 0.0), 1.0)
        self.assertEqual(data.percentile(s, 1.0), 4.0)

    def test_interpolates_between_neighbors(self):
        # pos = 0.5 * 3 = 1.5, i.e. halfway between the 2nd and 3rd of four values.
        self.assertEqual(data.percentile([1.0, 2.0, 3.0, 4.0], 0.5), 2.5)
        # pos = 0.25 * 4 = 1.0 lands exactly on an element, with no interpolation.
        self.assertEqual(data.percentile([1.0, 2.0, 3.0, 4.0, 5.0], 0.25), 2.0)


class MedianCiHalfWidthTest(unittest.TestCase):
    def test_too_few_values_is_infinite_so_nothing_reads_as_significant(self):
        self.assertEqual(data.median_ci_half_width([]), float("inf"))
        self.assertEqual(data.median_ci_half_width([5.0]), float("inf"))

    def test_is_the_notched_box_plot_width(self):
        values = [1.0, 2.0, 3.0, 4.0, 5.0]
        # p75 = 4, p25 = 2, so IQR = 2 over sqrt(5) reps.
        self.assertAlmostEqual(
            data.median_ci_half_width(values), 1.58 * 2.0 / math.sqrt(5)
        )

    def test_does_not_require_sorted_input(self):
        self.assertAlmostEqual(
            data.median_ci_half_width([5.0, 1.0, 4.0, 2.0, 3.0]),
            data.median_ci_half_width([1.0, 2.0, 3.0, 4.0, 5.0]),
        )


class SummaryTest(unittest.TestCase):
    def test_describes_the_series_and_asks_for_no_grid_by_default(self):
        s = data.summary([3.0, 1.0, 2.0])
        self.assertEqual(s["n"], 3)
        self.assertEqual(s["min"], 1.0)
        self.assertEqual(s["p50"], 2.0)
        self.assertEqual(s["max"], 3.0)
        self.assertEqual(sorted(s), ["max", "median_ci_half_width", "min", "n", "p50"])

    def test_caller_picks_the_grid(self):
        s = data.summary(range(101), quantiles=(0.0, 0.333, 0.999, 1.0))
        self.assertEqual(
            sorted(s),
            ["max", "median_ci_half_width", "min", "n", "p0", "p100", "p33.3", "p50", "p99.9"],
        )
        self.assertEqual(s["p0"], 0)
        self.assertEqual(s["p100"], 100)
        self.assertAlmostEqual(s["p99.9"], 99.9)

    def test_median_and_its_half_width_survive_a_grid_that_omits_them(self):
        # The report reasons with these two; the grid is only what it prints.
        s = data.summary([1.0, 2.0, 3.0, 4.0, 5.0], quantiles=(0.25,))
        self.assertEqual(
            sorted(s), ["max", "median_ci_half_width", "min", "n", "p25", "p50"]
        )
        self.assertEqual(s["p50"], 3.0)
        self.assertAlmostEqual(
            s["median_ci_half_width"], data.median_ci_half_width([1.0, 2.0, 3.0, 4.0, 5.0])
        )

    def test_empty_is_reportable_rather_than_an_error(self):
        s = data.summary([])
        self.assertEqual(s["n"], 0)
        self.assertTrue(math.isnan(s["min"]))
        self.assertTrue(math.isnan(s["p50"]))
        self.assertTrue(math.isnan(s["max"]))


class QuantileLabelTest(unittest.TestCase):
    def test_labels_are_the_percentage_with_no_trailing_noise(self):
        # `0.1 * 100` is 10.000000000000002 in binary floating point; the label is still `p10`.
        self.assertEqual(data.quantile_label(0.10), "p10")
        self.assertEqual(data.quantile_label(0.5), "p50")
        self.assertEqual(data.quantile_label(0.999), "p99.9")
        self.assertEqual(data.quantile_label(0.0), "p0")
        self.assertEqual(data.quantile_label(1.0), "p100")

    def test_label_is_how_summary_files_a_quantile(self):
        s = data.summary([1.0, 2.0, 3.0], quantiles=(0.8,))
        self.assertIn(data.quantile_label(0.8), s)


class BranchLabelTest(unittest.TestCase):
    def test_full_hash_is_abbreviated(self):
        self.assertEqual(data.branch_label("a" * 40), "aaaaaaaaaa")

    def test_branch_name_is_left_alone(self):
        self.assertEqual(data.branch_label("main"), "main")
        # 40 characters, but not a hash: shortening this would destroy the name.
        self.assertEqual(data.branch_label("z" * 40), "z" * 40)


class MeasurementsTest(unittest.TestCase):
    def measurements(self):
        return data.Measurements(
            rows(
                ("examples/b/au.cc", "feature", 2_000_000),
                ("examples/b/au.cc", "main", 4_000_000),
                ("examples/b/raw.cc", "main", 1_000_000),
                ("examples/b/raw.cc", "feature", 1_000_000),
                ("examples/a/main.cc", "main", 3_000_000),
            )
        )

    def test_branch_order_decides_the_baseline(self):
        # The order the user named, not the order the rows happen to arrive in: `feature` holds
        # the first row here, but `main` is the baseline because that is what was asked for.
        m = data.Measurements(self.measurements().rows, branch_order=["main", "feature"])
        self.assertEqual(m.branches, ["main", "feature"])
        self.assertEqual(m.baseline, "main")

    def test_branches_fall_back_to_first_seen_order_with_no_order_given(self):
        # Only for a CSV with no manifest beside it.  Relying on this is the bug that reversed
        # every delta in a report, since the measurement loop interleaves the branches.
        m = self.measurements()
        self.assertEqual(m.branches, ["feature", "main"])
        self.assertEqual(m.baseline, "feature")

    def test_a_branch_the_order_forgot_is_kept_but_never_becomes_the_baseline(self):
        m = data.Measurements(self.measurements().rows, branch_order=["main"])
        self.assertEqual(m.branches, ["main", "feature"])
        self.assertEqual(m.baseline, "main")

    def test_an_order_naming_a_branch_with_no_measurements_skips_it(self):
        m = data.Measurements(self.measurements().rows, branch_order=["gone", "main", "feature"])
        self.assertEqual(m.branches, ["main", "feature"])

    def test_translation_units_are_sorted_by_source_so_reports_are_comparable(self):
        # The one place translation unit order is asserted; everything below derives its order from
        # here, so those tests do not repeat the claim.
        m = self.measurements()
        self.assertEqual(
            [u.source for u in m.translation_units],
            ["examples/a/main.cc", "examples/b/au.cc", "examples/b/raw.cc"],
        )

    def test_times_are_milliseconds(self):
        m = self.measurements()
        self.assertEqual(m.times_ms("examples/b/au.cc", "main"), [4.0])

    def test_unmeasured_combinations_are_empty_rather_than_missing(self):
        m = self.measurements()
        self.assertEqual(m.times_ms("examples/a/main.cc", "feature"), [])
        self.assertFalse(m.has("examples/a/main.cc", "feature"))
        self.assertTrue(m.has("examples/a/main.cc", "main"))

    def test_partitions_translation_units_by_whether_every_branch_measured_them(self):
        # Which side each translation unit falls on, not what order they come out in: the order is
        # `translation_units`' sorted order, pinned by its own test above.
        m = self.measurements()
        self.assertCountEqual(
            [u.source for u in m.translation_units_on_all_branches()],
            ["examples/b/au.cc", "examples/b/raw.cc"],
        )
        self.assertCountEqual(
            [u.source for u in m.translation_units_missing_somewhere()], ["examples/a/main.cc"]
        )

    def test_ab_pairs_are_raw_then_au(self):
        # The order *within* a pair is the contract being tested; the order of the pairs is not.
        m = self.measurements()
        self.assertCountEqual(
            [(e, r.source, a.source) for e, r, a in m.ab_pairs()],
            [("b", "examples/b/raw.cc", "examples/b/au.cc")],
        )

    def test_half_measured_example_is_not_an_ab_pair(self):
        m = data.Measurements(
            rows(
                ("examples/b/au.cc", "main", 1),
                ("examples/b/au.cc", "feature", 1),
                ("examples/b/raw.cc", "main", 1),
            )
        )
        self.assertEqual(m.ab_pairs(), [])


class LoadTest(unittest.TestCase):
    """`load` names the columns it needs, and reads them by name."""

    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.addCleanup(self.dir.cleanup)
        self.path = os.path.join(self.dir.name, "raw.csv")

    def write(self, columns, *rows):
        buf = io.StringIO()
        writer = csv.DictWriter(buf, fieldnames=columns)
        writer.writeheader()
        for row in rows:
            writer.writerow(row)
        with open(self.path, "w", newline="") as f:
            f.write(buf.getvalue())

    def write_text(self, text):
        with open(self.path, "w", newline="") as f:
            f.write(text)

    def test_reads_back_what_the_harness_writes(self):
        self.write(data.FIELDS, measurement())
        m = data.load(self.path)
        self.assertEqual(m.branches, ["main"])
        self.assertEqual(m.rows[0]["rep"], 3)
        self.assertEqual(m.times_ms("examples/adc_millivolts/au.cc", "main"), [5.0])

    def test_columns_are_read_by_name_not_by_position(self):
        self.write(list(reversed(data.FIELDS)), measurement())
        m = data.load(self.path)
        self.assertEqual(m.times_ms("examples/adc_millivolts/au.cc", "main"), [5.0])
        self.assertEqual(m.rows[0]["rep"], 3)

    def test_missing_column_is_fatal_and_says_which(self):
        columns = [f for f in data.FIELDS if f != "nanos"]
        row = measurement()
        del row["nanos"]
        self.write(columns, row)
        with self.assertRaises(ValueError) as caught:
            data.load(self.path)
        self.assertIn("nanos", str(caught.exception))

    def test_extra_column_is_read_along_with_the_rest(self):
        self.write(data.FIELDS + ["cpu_temp"], dict(measurement(), cpu_temp=41))
        stderr = io.StringIO()
        with contextlib.redirect_stderr(stderr):
            m = data.load(self.path)
        self.assertIn("cpu_temp", stderr.getvalue())
        self.assertEqual(m.times_ms("examples/adc_millivolts/au.cc", "main"), [5.0])
        self.assertEqual(m.rows[0]["cpu_temp"], "41")

    def test_headerless_file_is_fatal_rather_than_read_as_a_header(self):
        # A data row where the header belongs: its values would become the column names.
        self.write_text(",".join(str(measurement()[f]) for f in data.FIELDS) + "\n")
        with self.assertRaises(ValueError):
            data.load(self.path)

    def test_empty_file_is_fatal(self):
        self.write_text("")
        with self.assertRaises(ValueError):
            data.load(self.path)


if __name__ == "__main__":
    unittest.main()
