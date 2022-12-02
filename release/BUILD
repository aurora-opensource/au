# Copyright 2022 Aurora Operations, Inc.

load("@rules_cc//cc:defs.bzl", "cc_test")

cc_library(
    name = "common_test_cases",
    hdrs = ["common_test_cases.hh"],
)

cc_test(
    name = "au_hh_test",
    size = "small",
    srcs = ["au_hh_test.cc"],
    deps = [
        ":common_test_cases",
        "//:au_hh_lib",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "au_noio_hh_test",
    size = "small",
    srcs = ["au_noio_hh_test.cc"],
    deps = [
        ":common_test_cases",
        "//:au_noio_hh_lib",
        "@com_google_googletest//:gtest_main",
    ],
)
