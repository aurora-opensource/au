#!/bin/bash
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
#
# Checks that an A/B example pair stays trustworthy.  See `ab_example.bzl` for the rationale.
#
# Usage: check_ab_example.sh RAW_BIN AU_BIN EXPECTED_TXT RAW_SRC AU_SRC

set -euo pipefail

raw_bin="$1"
au_bin="$2"
expected_txt="$3"
raw_src="$4"
au_src="$5"

status=0

# 1. Both programs must produce exactly the expected output.  The Au version cannot be "better" by
#    quietly computing something else.
for pair in "raw:${raw_bin}" "au:${au_bin}"; do
    label="${pair%%:*}"
    binary="${pair#*:}"
    actual_txt="${TEST_TMPDIR:-/tmp}/${label}_actual.txt"
    if ! "${binary}" > "${actual_txt}"; then
        echo "FAIL: the ${label} program exited nonzero." >&2
        status=1
        continue
    fi
    # Compare byte for byte, so that trailing whitespace and newlines count too.
    if ! cmp -s "${actual_txt}" "${expected_txt}"; then
        echo "FAIL: the ${label} program printed the wrong output." >&2
        echo "  expected: $(od -c "${expected_txt}" | head -5)" >&2
        echo "  actual:   $(od -c "${actual_txt}" | head -5)" >&2
        status=1
    fi
done

# 2. The doc-visible regions must have equal line counts, so corresponding lines sit at the same
#    height when the reader flips between tabs on the doc website.
# Count what actually renders, which is not the same as what the region contains:
#   - nested snippet markers are stripped by `pymdownx.snippets`, and
#   - *leading and trailing blank lines are dropped* when the code block is rendered.
# That second one matters: padding a region at its very top or bottom buys no height on the page,
# so it cannot be used to line the tabs up.  Put padding between two non-blank lines, where it
# survives -- or restructure so it isn't needed.  Counting the way the renderer does is what keeps
# this test honest; counting raw source lines would call a visibly broken page aligned.
region_lines() {
    sed -n '/--8<-- \[start:example\]/,/--8<-- \[end:example\]/p' "$1" |
        sed '1d;$d' |
        grep -v -- '--8<--' |
        sed -e '/./,$!d' |
        sed -e :a -e '/^\n*$/{$d;N;ba' -e '}' |
        grep -c '' || true  # An empty region makes `grep` exit non-zero; report it below instead.
}
raw_lines="$(region_lines "${raw_src}")"
au_lines="$(region_lines "${au_src}")"

if [ "${raw_lines}" -eq 0 ]; then
    echo "FAIL: no '[start:example]'/'[end:example]' region found in ${raw_src}." >&2
    status=1
elif [ "${raw_lines}" != "${au_lines}" ]; then
    echo "FAIL: the doc-visible regions are not line-aligned." >&2
    echo "  ${raw_src}: ${raw_lines} lines" >&2
    echo "  ${au_src}: ${au_lines} lines" >&2
    echo "  Pad the shorter region with blank lines so the tabs blink cleanly." >&2
    status=1
fi

exit "${status}"
