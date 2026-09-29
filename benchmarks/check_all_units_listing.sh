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
# Checks that the `all_units` benchmark includes every unit header, exactly once.
#
# Usage: check_all_units_listing.sh INCLUDES_CC
#
# A benchmark called "all units" that quietly stopped being all of them would still compile, still
# produce numbers, and still be wrong, so the list is checked against `au/units/` itself.

set -euo pipefail

includes_cc="$1"

# `_fwd.hh` headers are the forward declarations that the real headers include; listing them would
# double-count.  This matches what `make-single-file --all-units` considers a unit.
expected="${TEST_TMPDIR:-/tmp}/expected_includes.txt"
actual="${TEST_TMPDIR:-/tmp}/actual_includes.txt"

find au/units -maxdepth 1 -name '*.hh' ! -name '*_fwd.hh' \
    | sed 's|^|#include "|; s|$|"|' | sort > "${expected}"

grep '^#include ' "${includes_cc}" | sort > "${actual}"

if ! diff -u "${expected}" "${actual}"; then
    echo "FAIL: ${includes_cc} does not list every unit header exactly once." >&2
    echo "  Lines marked '-' are missing from it; lines marked '+' should not be there." >&2
    exit 1
fi
