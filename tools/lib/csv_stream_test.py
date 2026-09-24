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

"""Tests for the two promises `csv_stream` makes: the columns are the caller's, and every row is
on disk by the time `writerow` returns.
"""

import csv
import os
import tempfile
import unittest

import csv_stream


class CsvStreamTest(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.addCleanup(self.dir.cleanup)
        self.path = os.path.join(self.dir.name, "raw.csv")

    def lines(self):
        with open(self.path) as f:
            return f.read().splitlines()

    def test_header_is_the_first_rows_keys_in_the_first_rows_order(self):
        with csv_stream.create(self.path) as f:
            csv_stream.RowWriter(f).writerow({"b": 2, "a": 1})
        self.assertEqual(self.lines(), ["b,a", "2,1"])

    def test_columns_can_be_anything_the_caller_wants(self):
        # The writer has no opinion about the schema, so a harness can record a new column without
        # this module knowing about it.
        with csv_stream.create(self.path) as f:
            csv_stream.RowWriter(f).writerow({"cpu_temp_c": 41})
        self.assertEqual(self.lines(), ["cpu_temp_c", "41"])

    def test_writer_reports_the_columns_it_settled_on(self):
        with csv_stream.create(self.path) as f:
            writer = csv_stream.RowWriter(f)
            self.assertIsNone(writer.columns)
            writer.writerow({"b": 2, "a": 1})
            self.assertEqual(writer.columns, ["b", "a"])

    def test_later_row_may_order_its_keys_differently(self):
        # Rows are written by name, so a reordered row still lands under the right columns.
        with csv_stream.create(self.path) as f:
            writer = csv_stream.RowWriter(f)
            writer.writerow({"a": 1, "b": 2})
            writer.writerow({"b": 4, "a": 3})
        self.assertEqual(self.lines(), ["a,b", "1,2", "3,4"])

    def test_later_row_with_different_columns_is_refused(self):
        with csv_stream.create(self.path) as f:
            writer = csv_stream.RowWriter(f)
            writer.writerow({"a": 1, "b": 2})
            for bad in ({"a": 1}, {"a": 1, "b": 2, "c": 3}, {"a": 1, "c": 3}):
                with self.assertRaises(ValueError):
                    writer.writerow(bad)

    def test_refusal_names_both_sets_of_columns(self):
        with csv_stream.create(self.path) as f:
            writer = csv_stream.RowWriter(f)
            writer.writerow({"a": 1, "b": 2})
            with self.assertRaisesRegex(ValueError, "a, c.*a, b"):
                writer.writerow({"a": 1, "c": 3})

    def test_rows_are_readable_before_the_file_is_closed(self):
        # A run that gets killed partway through has to leave behind what it already measured.
        with csv_stream.create(self.path) as f:
            writer = csv_stream.RowWriter(f)
            writer.writerow({"a": 1, "b": 2})
            with open(self.path) as reading:
                self.assertEqual(list(csv.DictReader(reading)), [{"a": "1", "b": "2"}])
            writer.writerow({"a": 3, "b": 4})

    def test_create_refuses_to_touch_an_existing_file(self):
        with csv_stream.create(self.path) as f:
            csv_stream.RowWriter(f).writerow({"a": 1})
        with self.assertRaises(FileExistsError):
            csv_stream.create(self.path)
        # The earlier run's measurements are still there, unspliced and unerased.
        self.assertEqual(self.lines(), ["a", "1"])


if __name__ == "__main__":
    unittest.main()
