# Copyright 2022 Aurora Operations, Inc.

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "utils",
    hdrs = ["utils.hh"],
)

cc_binary(
    name = "101_quantity_makers",
    srcs = ["101_quantity_makers.cc"],
    deps = [
        ":utils",
        "//au",
        "//au:io",
        "@com_google_googletest//:gtest",
    ],
)
