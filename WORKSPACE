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
load("//build:copts.bzl", "BASE_CLANG_COPTS", "EXTRA_COPTS")

BAZEL_TOOLCHAIN_RELEASE = "v1.2.0"

BAZEL_TOOLCHAIN_SHA = "e3fb6dc6b77eaf167cb2b0c410df95d09127cbe20547e5a329c771808a816ab4"

http_archive(
    name = "toolchains_llvm",
    canonical_id = BAZEL_TOOLCHAIN_RELEASE,
    sha256 = BAZEL_TOOLCHAIN_SHA,
    strip_prefix = "toolchains_llvm-{ref}".format(ref = BAZEL_TOOLCHAIN_RELEASE),
    url = "https://github.com/bazel-contrib/toolchains_llvm/releases/download/{ref}/toolchains_llvm-{ref}.tar.gz".format(ref = BAZEL_TOOLCHAIN_RELEASE),
)

load("@toolchains_llvm//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@toolchains_llvm//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_11_toolchain",
    compile_flags = {
        "": BASE_CLANG_COPTS + EXTRA_COPTS,
    },
    cxx_standard = {
        "": "c++14",
    },
    llvm_version = "11.1.0",
    target_settings = {
        "": ["@//build:clang11_requested"],
    },
)

load("@llvm_11_toolchain//:toolchains.bzl", llvm_11_register_toolchains = "llvm_register_toolchains")

llvm_11_register_toolchains()

llvm_toolchain(
    name = "llvm_14_toolchain",
    compile_flags = {
        "": BASE_CLANG_COPTS + EXTRA_COPTS,
    },
    cxx_standard = {
        "": "c++14",
    },
    llvm_version = "14.0.0",
    target_settings = {
        "": ["@//build:clang14_requested"],
    },
)

load("@llvm_14_toolchain//:toolchains.bzl", llvm_14_register_toolchains = "llvm_register_toolchains")

llvm_14_register_toolchains()

http_archive(
    name = "aspect_gcc_toolchain",
    patch_args = ["-p1"],
    patches = [
        "@//:third_party/aspect_gcc_toolchain/0001-Expose-target_settings-and-set-std-c-14.patch",
    ],
    sha256 = "0651c0d595417b71fdbd903bf852c59a4a576a82e15651bd9672416b64019530",
    strip_prefix = "gcc-toolchain-ac745d4685e2095cc4f057862800f3f0a473c201",
    type = "tar.gz",
    urls = [
        "https://github.com/f0rmiga/gcc-toolchain/archive/ac745d4685e2095cc4f057862800f3f0a473c201.tar.gz",
    ],
)

load("@aspect_gcc_toolchain//sysroot:flags.bzl", gcc_sysroot_cflags = "cflags", gcc_sysroot_cxxflags = "cxxflags")
load("@aspect_gcc_toolchain//toolchain:defs.bzl", "ARCHS", "gcc_register_toolchain")

gcc_register_toolchain(
    name = "gcc_toolchain_x86_64",
    extra_cflags = gcc_sysroot_cflags + EXTRA_COPTS,
    extra_cxxflags = gcc_sysroot_cxxflags + EXTRA_COPTS,
    gcc_version = "10.3.0",
    sysroot_variant = "x86_64",
    target_arch = ARCHS.x86_64,
    target_settings = ["@//build:gcc10_requested"],
)

# This is not a "real" local bazel repository.  We define this in this WORKSPACE
# file because it will prevent bazel from looking for packages in this folder
# and its children.
local_repository(
    name = "ignore_cmake",
    path = "./cmake",
)
