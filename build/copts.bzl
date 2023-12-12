# Copyright 2023 Aurora Operations, Inc.
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

# Extra flags we want to pass to the compilers.
# -Wall is already set by aspect_gcc_toolchain.
EXTRA_COPTS = [
    "-Wextra",
    "-pedantic",
]

# Since the clang toolchain we're using doesn't let us extract the default flags, we have to
# manually specify the default flags here. These are copied from:
# https://github.com/grailbio/bazel-toolchain/blob/069ee4e20ec605a6c76c1798658e17175b2eb35e/toolchain/cc_toolchain_config.bzl#L118
BASE_CLANG_COPTS = [
    "--target=x86_64-unknown-linux-gnu",
    # Security
    "-U_FORTIFY_SOURCE",  # https://github.com/google/sanitizers/issues/247
    "-fstack-protector",
    "-fno-omit-frame-pointer",
    # Diagnostics
    "-fcolor-diagnostics",
    "-Wall",
    "-Wshadow",
    "-Wthread-safety",
    "-Wself-assign",
]
