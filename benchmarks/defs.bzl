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

    The name of the benchmark is the name of the subfolder that holds its sources.  This gives it
    the same shape as our code examples, which we built our compile time tooling around.

    Args:
      name: Name of the benchmark.  Its sources live in this subdirectory, and this is what the
        measurement report calls it.
      srcs: Sources for the benchmark, relative to this directory.  Must be under `<name>/`.
      deps: Deps of the benchmark.
    """
    for src in srcs:
        if not src.startswith(name + "/"):
            fail(
                "Benchmark '{}' lists the source '{}', which is not under '{}/'.".format(
                    name,
                    src,
                    name,
                ) +
                "  The report names a translation unit after the directory holding it, so a " +
                "source outside this one would be reported under the wrong name.",
            )

    cc_library(
        name = name,
        srcs = srcs,
        deps = deps,
    )
