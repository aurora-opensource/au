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

"""Write rows to a CSV incrementally, so a run that dies leaves behind what it already has.

This knows nothing about compile times.  The columns come from the first row it is handed, which
is what lets the measurement harness own its own schema.
"""

import csv


def create(path):
    """Open a file for a new run's measurements, refusing one that is already there.

    A run writes its own file, and the default output directory is stamped to the second, so this
    only fires when `--out` names a directory that has been measured into before.  Refusing is
    better than the alternatives: appending would splice two runs into one file, and truncating
    would throw away measurements that took an hour to make.
    """
    return open(path, "x", newline="")


class RowWriter:
    """Writes rows to a CSV, taking its columns from the first row it is given.

    The columns are that row's keys, in that row's order, and this class does not care what they
    are.  What it does insist on is that every later row carries the same ones: a file whose rows
    stop matching its own header is not readable by anything.

    Every row is flushed as it lands, which is what makes a killed run leave behind a complete file
    rather than a truncated buffer.
    """

    def __init__(self, f):
        self._f = f
        self._writer = None
        self.columns = None

    def writerow(self, row):
        if self._writer is None:
            self.columns = list(row)
            self._writer = csv.DictWriter(self._f, fieldnames=self.columns)
            self._writer.writeheader()
        elif set(row) != set(self.columns):
            raise ValueError(
                "row has columns {}, but the header says {}".format(
                    ", ".join(sorted(row)), ", ".join(self.columns)
                )
            )
        self._writer.writerow(row)
        self._f.flush()
