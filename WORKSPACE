load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.3.0/bazel-skylib-1.3.0.tar.gz",
    ],
    sha256 = "74d544d96f4a5bb630d465ca8bbcfe231e3594e5aae57e1edbf17a6eb3ca2506",
)
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()

BAZEL_TOOLCHAIN_REF = "056aeaa01900f5050a9fed9b11e2d365a684831a"
BAZEL_TOOLCHAIN_SHA = "93aa940bcaa2bfdd8153d4d029bad1ccc6c0601e29ffff3a23e1d89aba5f61fa"

http_archive(
    name = "com_grail_bazel_toolchain",
    sha256 = BAZEL_TOOLCHAIN_SHA,
    strip_prefix = "bazel-toolchain-{ref}".format(ref = BAZEL_TOOLCHAIN_REF),
    canonical_id = BAZEL_TOOLCHAIN_REF,
    url = "https://github.com/grailbio/bazel-toolchain/archive/{ref}.tar.gz".format(ref = BAZEL_TOOLCHAIN_REF),
)

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_11_toolchain",
    llvm_version = "11.1.0",
    cxx_standard = {
        "": "c++14",
    },
    target_settings = {
        "": ["@//build:clang11_requested"],
    },
)

load("@llvm_11_toolchain//:toolchains.bzl", llvm_11_register_toolchains="llvm_register_toolchains")

llvm_11_register_toolchains()

llvm_toolchain(
    name = "llvm_14_toolchain",
    llvm_version = "14.0.0",
    cxx_standard = {
        "": "c++14",
    },
    target_settings = {
        "": ["@//build:clang14_requested"],
    },
)

load("@llvm_14_toolchain//:toolchains.bzl", llvm_14_register_toolchains="llvm_register_toolchains")

llvm_14_register_toolchains()

http_archive(
    name = "com_google_googletest",
    sha256 = "24564e3b712d3eb30ac9a85d92f7d720f60cc0173730ac166f27dda7fed76cb2",
    strip_prefix = "googletest-release-1.12.1",
    urls = ["https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip"],
)
