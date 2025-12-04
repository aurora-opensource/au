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
    name = "gcc_toolchain",
    sha256 = "4fb44eeb43a086556c934a8b9526df0c2ccb47bf5941c05806b0163ad92baadf",
    strip_prefix = "gcc-toolchain-0.5.0",
    urls = [
        "https://github.com/f0rmiga/gcc-toolchain/archive/refs/tags/0.5.0.tar.gz",
    ],
)

load("@gcc_toolchain//sysroot:flags.bzl", gcc_sysroot_cflags = "cflags", gcc_sysroot_cxxflags = "cxxflags")
load("@gcc_toolchain//toolchain:defs.bzl", "ARCHS", "gcc_register_toolchain")

gcc_register_toolchain(
    name = "gcc_toolchain_x86_64",
    extra_cflags = gcc_sysroot_cflags + EXTRA_COPTS,
    extra_cxxflags = gcc_sysroot_cxxflags + EXTRA_COPTS,
    gcc_version = "10.3.0",
    sysroot_variant = "x86_64",
    target_arch = ARCHS.x86_64,
    target_compatible_with = ["@//build/compiler:gcc_10"],
)

# This is not a "real" local bazel repository.  We define this in this WORKSPACE
# file because it will prevent bazel from looking for packages in this folder
# and its children.
local_repository(
    name = "ignore_cmake",
    path = "./cmake",
)
