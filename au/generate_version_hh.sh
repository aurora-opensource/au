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
# Prints `au/version.hh`, filling in its template with the version declared in `MODULE.bazel`.
#
# This is Bazel's generator.  CMake (`CMakeLists.txt`) and `tools/bin/make-single-file` each have
# their own, because neither can rely on the others' tools; `//release:version_header_test` checks
# that Bazel's and `make-single-file`'s agree.
#
# Usage: generate_version_hh.sh MODULE.bazel au/version.hh.in > au/version.hh

set -euo pipefail

module_file="$1"
template="$2"

# The version in the `module()` block.  (No `\+` or `\|`, so that BSD `sed` handles it too.)
version="$(sed -n '/^module(/,/^)/{s/^[[:space:]]*version = "\([^"]*\)".*/\1/p;}' "${module_file}")"

if ! [[ "${version}" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
    echo "ERROR: could not read a MAJOR.MINOR.PATCH version from the module() block of" \
        "${module_file} (got '${version}')." >&2
    exit 1
fi

sed \
    -e "s/@AU_VERSION_MAJOR@/${BASH_REMATCH[1]}/" \
    -e "s/@AU_VERSION_MINOR@/${BASH_REMATCH[2]}/" \
    -e "s/@AU_VERSION_PATCH@/${BASH_REMATCH[3]}/" \
    "${template}"
