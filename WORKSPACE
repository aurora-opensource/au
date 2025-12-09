# Copyright 2022 Aurora Operations, Inc.
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

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("//build:copts.bzl", "EXTRA_COPTS")

http_archive(
    name = "aspect_bazel_lib",
    sha256 = "a7e356f8a5cb8bf1e9be38c2c617ad22f5a1606792e839fc040971bdfbecf971",
    strip_prefix = "bazel-lib-1.40.2",
    url = "https://github.com/aspect-build/bazel-lib/archive/refs/tags/v1.40.2.tar.gz",
)

http_archive(
    name = "gcc_toolchain",
    sha256 = "e6a00a9f999b29ba4eee0bf73f74abeeec1cb85740c8c197a8bfa89a73e722b0",
    strip_prefix = "gcc-toolchain-0.9.0",
    urls = [
        "https://github.com/f0rmiga/gcc-toolchain/releases/download/0.9.0/gcc-toolchain-0.9.0.tar.gz",
    ],
)

load("@gcc_toolchain//toolchain:defs.bzl", "ARCHS", "gcc_register_toolchain")

gcc_register_toolchain(
    name = "gcc_toolchain_x86_64",
    extra_cflags = EXTRA_COPTS,
    extra_ldflags = ["-l:libstdc++.a"],
    gcc_version = "14.3.0",
    target_arch = ARCHS.x86_64,
    target_compatible_with = ["@//build/compiler:gcc_14"],
)

# This is not a "real" local bazel repository.  We define this in this WORKSPACE
# file because it will prevent bazel from looking for packages in this folder
# and its children.
local_repository(
    name = "ignore_cmake",
    path = "./cmake",
)
