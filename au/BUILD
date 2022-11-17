# Aurora Innovation, Inc. Proprietary and Confidential. Copyright 2022.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "stdx",
    srcs = glob([
        "stdx/*.cc",
        "stdx/experimental/*.cc",
    ]),
    hdrs = glob([
        "stdx/*.hh",
        "stdx/experimental/*.hh",
    ]),
)

cc_test(
    name = "stdx_test",
    size = "small",
    srcs = glob(["stdx/test/*.cc"]),
    deps = [
        ":stdx",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "zero",
    hdrs = ["zero.hh"],
)

cc_test(
    name = "zero_test",
    size = "small",
    srcs = ["zero_test.cc"],
    deps = [
        ":zero",
        "@com_google_googletest//:gtest_main",
    ],
)
