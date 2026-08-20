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

"""Build rules for the code examples shown on the doc website.

Every example on the website is real source from this directory, compiled and run by CI, and
inlined into the docs by `pymdownx.snippets`.  Nothing on those pages can drift out of date with
the library without a test going red.

`ab_example`: the "A/B" code examples.

Each A/B example is a pair of complete, runnable programs that solve the same problem: `raw.cc`
(plain C++, no units library) and `au.cc` (the same thing, using Au).  The doc website shows them
as a pair of tabs, so a reader can flip back and forth and compare corresponding lines in place.

That presentation only works if the two stay honest about two things, and `ab_example` enforces
both:

  1. They must produce *identical* output.  This is what lets the page claim "same answer, better
     code" without the reader having to take our word for it.

  2. The regions marked for inclusion in the docs must have the same number of lines, so that
     corresponding constructs sit at the same height in both tabs.  Where Au needs less code, pad
     with a blank line rather than letting the two versions drift out of alignment.
"""

load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_shell//shell:sh_test.bzl", "sh_test")

def ab_example(name, expected_output, au_deps, raw_srcs = None, au_srcs = None):
    """Defines a raw-vs-Au example pair, plus the tests that keep the pair trustworthy.

    Args:
      name: Name of the example.  Sources are read from this subdirectory.
      expected_output: The exact stdout both programs must produce.
      au_deps: Deps for the Au version (the raw version must have none by construction).
      raw_srcs: Sources for the raw version.  Defaults to `<name>/raw.cc`.
      au_srcs: Sources for the Au version.  Defaults to `<name>/au.cc`.
    """
    raw_srcs = raw_srcs or ["{}/raw.cc".format(name)]
    au_srcs = au_srcs or ["{}/au.cc".format(name)]

    cc_binary(
        name = "{}_raw".format(name),
        srcs = raw_srcs,
    )

    cc_binary(
        name = "{}_au".format(name),
        srcs = au_srcs,
        deps = au_deps,
    )

    native.genrule(
        name = "{}_expected".format(name),
        outs = ["{}_expected.txt".format(name)],
        cmd = "printf '%b' {} > $@".format(repr(expected_output)),
    )

    # The doc website inlines these sources directly (via `pymdownx.snippets`).  Declaring them
    # here, rather than globbing in the BUILD file, guarantees that the sources the docs show are
    # exactly the ones the tests below compile and run.
    native.filegroup(
        name = "{}_doc_sources".format(name),
        srcs = raw_srcs + au_srcs,
    )

    sh_test(
        name = "{}_test".format(name),
        srcs = ["check_ab_example.sh"],
        args = [
            "$(rootpath :{}_raw)".format(name),
            "$(rootpath :{}_au)".format(name),
            "$(rootpath :{}_expected.txt)".format(name),
            "$(rootpath {})".format(raw_srcs[0]),
            "$(rootpath {})".format(au_srcs[0]),
        ],
        data = [
            ":{}_au".format(name),
            ":{}_expected.txt".format(name),
            ":{}_raw".format(name),
        ] + raw_srcs + au_srcs,
    )
