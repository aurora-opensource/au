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
# Keeps the bazel module version honest.
#
# `au/version.hh` is the source of truth for the library version, and `CMakeLists.txt` derives its
# own version from it.  Bazel cannot do that: `MODULE.bazel` is Starlark, evaluated before any file
# can be read, so its `version` has to be a literal.  That makes it a copy, and a copy can drift.
# This test compares the copy against the source it claims to come from.
#
# Usage: check_module_version.sh MODULE.bazel au/version.hh

set -euo pipefail

module_file="$1"
version_hh="$2"

# One component of the version, from its `#define`.  Prints nothing if it is missing.
component() {
    sed -n "s/^#define AU_VERSION_$1[[:space:]]\+\([0-9]\+\)[[:space:]]*\$/\1/p" "${version_hh}"
}

expected="$(component MAJOR).$(component MINOR).$(component PATCH)"

if ! [[ "${expected}" =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "FAIL: could not read the version from ${version_hh} (got '${expected}')." >&2
    echo "  Expected '#define AU_VERSION_MAJOR 0'-style lines for all three components." >&2
    exit 1
fi

# The version in the `module()` block, and only that one: the file is full of other `version =`
# assignments, in the `bazel_dep` calls.
actual="$(
    sed -n '/^module(/,/^)/{s/^[[:space:]]*version = "\([^"]*\)".*/\1/p;}' "${module_file}" | head -n 1
)"

if [ -z "${actual}" ]; then
    echo "FAIL: no version found in the module() block of ${module_file}." >&2
    echo "  The Bazel Central Registry requires one; see RELEASE.md." >&2
    exit 1
fi

if [ "${actual}" != "${expected}" ]; then
    echo "FAIL: ${module_file} and ${version_hh} disagree on the version." >&2
    echo "  ${module_file}: ${actual}" >&2
    echo "  ${version_hh}: ${expected}" >&2
    echo "  Update ${module_file} to match, as RELEASE.md describes." >&2
    exit 1
fi
