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
# Checks that an example program prints exactly what the docs say it prints.  This is the whole
# test for a `single_example`, and it is the first of the two checks that `check_ab_example.sh`
# runs -- that script invokes this one once per program rather than keeping its own copy of the
# comparison, so the two flavors of example cannot drift in what "correct output" means.
#
# Usage: check_output.sh BINARY EXPECTED_TXT [LABEL]
#
# LABEL names the program in failure messages, and keeps the scratch files of concurrent
# invocations apart.  It defaults to "example", which reads correctly when there is only one.

set -euo pipefail

binary="$1"
expected_txt="$2"
label="${3:-example}"
actual_txt="${TEST_TMPDIR:-/tmp}/${label}_actual.txt"

if ! "${binary}" > "${actual_txt}"; then
    echo "FAIL: the ${label} program exited nonzero." >&2
    exit 1
fi

# Compare byte for byte, so that trailing whitespace and newlines count too.
if ! cmp -s "${actual_txt}" "${expected_txt}"; then
    echo "FAIL: the ${label} program printed the wrong output." >&2
    echo "  expected: $(od -c "${expected_txt}" | head -5)" >&2
    echo "  actual:   $(od -c "${actual_txt}" | head -5)" >&2
    exit 1
fi
