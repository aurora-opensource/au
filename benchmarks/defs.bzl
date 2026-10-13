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

"""Build rules for the compile time benchmarks.

See the comment at the top of `BUILD.bazel` in this directory for what these measure and why they
look the way they do.
"""

load("@rules_cc//cc:defs.bzl", "cc_library")

def compile_benchmark(name, srcs, deps):
    """Defines one compile time benchmark: a translation unit that exists to be timed.

    Sources live in `<group>/<file>`, where the group is a family of related benchmarks.  This gives
    them the same shape as our code examples, which we built our compile time tooling around: the
    folder names the group, and the file names the member within it.

    Args:
      name: Name of the benchmark target.
      srcs: Sources for the benchmark, relative to this directory.  All must sit directly in the
        same subdirectory.
      deps: Deps of the benchmark.
    """
    groups = {}
    for src in srcs:
        parts = src.split("/")
        if len(parts) != 2:
            fail(
                "Benchmark '{}' lists the source '{}', ".format(name, src) +
                "which is not directly in a group folder." +
                "  The report groups translation units by the folder holding them, so each " +
                "source must be at `<group>/<file>`.",
            )
        if parts[1].rsplit(".", 1)[0] in ("au", "raw"):
            fail(
                "Benchmark '{}' lists the source '{}', whose name is reserved.".format(name, src) +
                "  The report treats `au` and `raw` as the two flavors of a code example.",
            )
        groups[parts[0]] = True
    if len(groups) > 1:
        fail(
            "Benchmark '{}' has sources in more than one folder: {}.".format(
                name,
                ", ".join(sorted(groups.keys())),
            ) + "  All of a benchmark's sources must share one group folder.",
        )

    cc_library(
        name = name,
        srcs = srcs,
        deps = deps,
    )
