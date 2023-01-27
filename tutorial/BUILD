# Copyright 2022 Aurora Operations, Inc.

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

cc_library(
    name = "utils",
    hdrs = ["utils.hh"],
)

cc_binary(
    name = "101_quantity_makers",
    testonly = True,
    srcs = ["101_quantity_makers.cc"],
    deps = [
        ":utils",
        "//au",
        "//au:io",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "102_api_types",
    srcs = ["102_api_types.cc"],
    hdrs = ["102_api_types.hh"],
    deps = ["//au"],
)

cc_test(
    name = "102_api_types_test",
    testonly = True,
    srcs = ["102_api_types_test.cc"],
    deps = [
        ":102_api_types",
        "//au:testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "103_unit_conversions_test",
    size = "small",
    srcs = ["103_unit_conversions_test.cc"],
    tags = ["manual"],
    deps = [
        "//au",
        "//au:testing",
        "@com_google_googletest//:gtest_main",
    ],
)
