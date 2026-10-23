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
# Checks that Bazel's `au/version.hh` and the single-file package carry the same version.
#
# Each comes from its own parser of `MODULE.bazel` (`generate_version_hh.sh`, and
# `tools/bin/make-single-file`), so a bug in either would otherwise go unnoticed: the result still
# compiles, it just reports the wrong version.
#
# Usage: check_version_headers.sh au/version.hh release/au.hh

set -euo pipefail

version_hh="$1"
single_file="$2"

# The three `#define AU_VERSION_<COMPONENT> <number>` lines.  Prints nothing if they are missing.
components() {
    grep -E '^#define AU_VERSION_(MAJOR|MINOR|PATCH) [0-9]+$' "$1" || true
}

from_bazel="$(components "${version_hh}")"
from_single_file="$(components "${single_file}")"

if [ "$(wc -l <<<"${from_bazel}")" -ne 3 ] || [ -z "${from_bazel}" ]; then
    echo "FAIL: expected three numeric version macros in ${version_hh}; found:" >&2
    echo "${from_bazel:-  (none)}" >&2
    exit 1
fi

if [ "${from_bazel}" != "${from_single_file}" ]; then
    echo "FAIL: ${version_hh} and ${single_file} disagree on the version." >&2
    echo "--- ${version_hh}:" >&2
    echo "${from_bazel}" >&2
    echo "--- ${single_file}:" >&2
    echo "${from_single_file:-  (none)}" >&2
    exit 1
fi
