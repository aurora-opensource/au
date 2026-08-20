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
# Keeps the README's code blocks honest.
#
# The doc website inlines example source directly, so it can never go stale.  The README cannot do
# that -- GitHub renders plain Markdown, with no include mechanism -- so the code there is a copy,
# and a copy can drift.  This test compares each copy against the source it claims to come from.
#
# Mark a README block like this, naming the file and the snippet region it was copied from:
#
#     <!-- BEGIN EXAMPLE: examples/angular_velocity/au.cc:headline -->
#     ```cpp
#     ...copied code...
#     ```
#     <!-- END EXAMPLE -->
#
# Usage: check_readme_snippets.sh README.md

set -euo pipefail

readme="$1"
tmp="${TEST_TMPDIR:-/tmp}"
status=0
found=0

# Each marked block, as "<line number> <file>:<region>".
# `|| true`: no matches is not an error *here* --- we report it below, with a better message.
grep -n 'BEGIN EXAMPLE:' "${readme}" |
    sed 's/^\([0-9]*\):.*BEGIN EXAMPLE: *\([^ ]*\) *-->.*/\1 \2/' > "${tmp}/markers.txt" || true

if [ ! -s "${tmp}/markers.txt" ]; then
    echo "FAIL: no 'BEGIN EXAMPLE:' blocks found in ${readme}." >&2
    echo "  If the README no longer quotes example source, delete this test." >&2
    exit 1
fi

# Redirect rather than pipe, so the loop body runs in this shell and can set `status`.
while read -r line_no spec; do
    src="${spec%:*}"
    region="${spec##*:}"
    found=$((found + 1))

    if [ ! -f "${src}" ]; then
        echo "FAIL: ${readme} line ${line_no} refers to '${src}', which does not exist." >&2
        status=1
        continue
    fi

    # The README copy: everything between the BEGIN marker and the END marker, minus the fences.
    awk -v start="${line_no}" '
        NR <= start { next }
        /<!-- END EXAMPLE -->/ { exit }
        /^```/ { next }
        { print }
    ' "${readme}" > "${tmp}/readme_block.txt"

    # The source of truth: the named region, minus any nested snippet markers.
    sed -n "/--8<-- \[start:${region}\]/,/--8<-- \[end:${region}\]/p" "${src}" |
        sed '1d;$d' |
        grep -v -- '--8<--' > "${tmp}/source_block.txt" || true

    if [ ! -s "${tmp}/source_block.txt" ]; then
        echo "FAIL: no '[start:${region}]' region found in ${src}." >&2
        status=1
        continue
    fi

    if ! diff -u "${tmp}/source_block.txt" "${tmp}/readme_block.txt" \
        --label "${src} (region '${region}')" --label "${readme} (line ${line_no})"; then
        echo "FAIL: the ${readme} copy no longer matches ${src}." >&2
        echo "  Update the README block to match the source." >&2
        status=1
    fi
done < "${tmp}/markers.txt"

if [ "${found}" -eq 0 ]; then
    echo "FAIL: no README blocks were checked." >&2
    status=1
fi

exit "${status}"
