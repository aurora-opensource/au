# Copyright 2022 Aurora Operations, Inc.

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    sha256 = "74d544d96f4a5bb630d465ca8bbcfe231e3594e5aae57e1edbf17a6eb3ca2506",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
    ],
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

BAZEL_TOOLCHAIN_REF = "056aeaa01900f5050a9fed9b11e2d365a684831a"

BAZEL_TOOLCHAIN_SHA = "93aa940bcaa2bfdd8153d4d029bad1ccc6c0601e29ffff3a23e1d89aba5f61fa"

http_archive(
    name = "com_grail_bazel_toolchain",
    canonical_id = BAZEL_TOOLCHAIN_REF,
    sha256 = BAZEL_TOOLCHAIN_SHA,
    strip_prefix = "bazel-toolchain-{ref}".format(ref = BAZEL_TOOLCHAIN_REF),
    url = "https://github.com/grailbio/bazel-toolchain/archive/{ref}.tar.gz".format(ref = BAZEL_TOOLCHAIN_REF),
)

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_11_toolchain",
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
    sha256 = "e2e12202dd83f67d71101b24554044de25e1625d16b4b56bc453ecaa8f7c6bd0",
    strip_prefix = "aspect-build-gcc-toolchain-ac745d4",
    type = "tar.gz",
    urls = [
        "https://github.com/aspect-build/gcc-toolchain/tarball/ac745d4685e2095cc4f057862800f3f0a473c201",
    ],
)

load("@aspect_gcc_toolchain//sysroot:flags.bzl", gcc_sysroot_cflags = "cflags", gcc_sysroot_cxxflags = "cxxflags")
load("@aspect_gcc_toolchain//toolchain:defs.bzl", "ARCHS", "gcc_register_toolchain")

gcc_register_toolchain(
    name = "gcc_toolchain_x86_64",
    extra_cflags = gcc_sysroot_cflags,
    extra_cxxflags = gcc_sysroot_cxxflags,
    gcc_version = "10.3.0",
    sysroot_variant = "x86_64",
    target_arch = ARCHS.x86_64,
    target_settings = ["@//build:gcc10_requested"],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "24564e3b712d3eb30ac9a85d92f7d720f60cc0173730ac166f27dda7fed76cb2",
    strip_prefix = "googletest-release-1.12.1",
    urls = ["https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip"],
)

http_archive(
    name = "rules_python",
    sha256 = "a868059c8c6dd6ad45a205cca04084c652cfe1852e6df2d5aca036f6e5438380",
    strip_prefix = "rules_python-0.14.0",
    url = "https://github.com/bazelbuild/rules_python/archive/refs/tags/0.14.0.tar.gz",
)

load("@rules_python//python:repositories.bzl", "python_register_toolchains")

python_register_toolchains(
    name = "python3_10",
    python_version = "3.10",
)

load("@python3_10//:defs.bzl", "interpreter")
load("@rules_python//python:pip.bzl", "pip_parse")

pip_parse(
    name = "pip_deps",
    python_interpreter_target = interpreter,
    requirements_lock = "@//:requirements_lock.txt",
)

load("@pip_deps//:requirements.bzl", "install_deps")

install_deps()

################################################################################
# SECTION: Install buildifier.
#
# Instructions adapted from:
# https://github.com/bazelbuild/buildtools/blob/762712d8/buildifier/README.md#setup-and-usage-via-bazel-not-supported-on-windows

# buildifier is written in Go and hence needs rules_go to be built.
# See https://github.com/bazelbuild/rules_go for the up to date setup instructions.
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "d6b2513456fe2229811da7eb67a444be7785f5323c6708b38d851d2b51e54d83",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.30.0/rules_go-v0.30.0.zip",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.30.0/rules_go-v0.30.0.zip",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.17.2")

http_archive(
    name = "bazel_gazelle",
    sha256 = "de69a09dc70417580aabf20a28619bb3ef60d038470c7cf8442fafcf627c21cb",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-gazelle/releases/download/v0.24.0/bazel-gazelle-v0.24.0.tar.gz",
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.24.0/bazel-gazelle-v0.24.0.tar.gz",
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
