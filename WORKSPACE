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

BAZEL_TOOLCHAIN_RELEASE = "0.10.3"

BAZEL_TOOLCHAIN_SHA = "b7cd301ef7b0ece28d20d3e778697a5e3b81828393150bed04838c0c52963a01"

http_archive(
    name = "com_grail_bazel_toolchain",
    canonical_id = BAZEL_TOOLCHAIN_RELEASE,
    sha256 = BAZEL_TOOLCHAIN_SHA,
    strip_prefix = "toolchains_llvm-{ref}".format(ref = BAZEL_TOOLCHAIN_RELEASE),
    url = "https://github.com/bazel-contrib/toolchains_llvm/releases/download/{ref}/toolchains_llvm-{ref}.tar.gz".format(ref = BAZEL_TOOLCHAIN_RELEASE),
)

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

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

################################################################################
# SECTION: Install buildifier.
#
# Instructions adapted from
# https://github.com/bazelbuild/buildtools/blob/7f01a3f/buildifier/README.md#setup-and-usage-via-bazel

# buildifier is written in Go and hence needs rules_go to be built.
# See https://github.com/bazelbuild/rules_go for the up to date setup instructions.
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "6dc2da7ab4cf5d7bfc7c949776b1b7c733f05e56edc4bcd9022bb249d2e2a996",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.39.1/rules_go-v0.39.1.zip",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.39.1/rules_go-v0.39.1.zip",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.20.3")

http_archive(
    name = "bazel_gazelle",
    sha256 = "727f3e4edd96ea20c29e8c2ca9e8d2af724d8c7778e7923a854b2c80952bc405",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-gazelle/releases/download/v0.30.0/bazel-gazelle-v0.30.0.tar.gz",
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.30.0/bazel-gazelle-v0.30.0.tar.gz",
    ],
)

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")

# If you use WORKSPACE.bazel, use the following line instead of the bare gazelle_dependencies():
# gazelle_dependencies(go_repository_default_config = "@//:WORKSPACE.bazel")
gazelle_dependencies()

http_archive(
    name = "com_google_protobuf",
    sha256 = "3bd7828aa5af4b13b99c191e8b1e884ebfa9ad371b0ce264605d347f135d2568",
    strip_prefix = "protobuf-3.19.4",
    urls = [
        "https://github.com/protocolbuffers/protobuf/archive/v3.19.4.tar.gz",
    ],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

http_archive(
    name = "com_github_bazelbuild_buildtools",
    sha256 = "e3bb0dc8b0274ea1aca75f1f8c0c835adbe589708ea89bf698069d0790701ea3",
    strip_prefix = "buildtools-5.1.0",
    urls = [
        "https://github.com/bazelbuild/buildtools/archive/refs/tags/5.1.0.tar.gz",
    ],
)

# END SECTION: Install buildifier.
################################################################################

# This is not a "real" local bazel repository.  We define this in this WORKSPACE
# file because it will prevent bazel from looking for packages in this folder
# and its children.
local_repository(
    name = "ignore_cmake",
    path = "./cmake",
)
