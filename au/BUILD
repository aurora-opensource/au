# Copyright 2022 Aurora Operations, Inc.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

################################################################################
# Public libraries and tests

cc_library(
    name = "au",
    hdrs = ["au.hh"],
    visibility = ["//visibility:public"],
    deps = [":math"],
)

cc_test(
    name = "au_test",
    size = "small",
    srcs = ["au_test.cc"],
    deps = [
        ":au",
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "io",
    hdrs = ["io.hh"],
    visibility = ["//visibility:public"],
    deps = [
        ":quantity",
        ":quantity_point",
        ":zero",
    ],
)

cc_test(
    name = "io_test",
    size = "small",
    srcs = ["io_test.cc"],
    deps = [
        ":io",
        ":prefix",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "testing",
    testonly = True,
    hdrs = ["testing.hh"],
    visibility = ["//visibility:public"],
    deps = [
        ":io",
        ":stdx",
        ":unit_of_measure",
    ],
)

cc_test(
    name = "testing_test",
    size = "small",
    srcs = ["testing_test.cc"],
    deps = [
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "units",
    hdrs = glob(["units/*.hh"]),
    visibility = ["//visibility:public"],
    deps = [
        ":prefix",
        ":quantity",
        ":quantity_point",
        ":unit_of_measure",
    ],
)

cc_test(
    name = "units_test",
    size = "small",
    srcs = glob(["units/test/*.cc"]),
    deps = [
        ":testing",
        ":units",
        "@com_google_googletest//:gtest_main",
    ],
)

################################################################################
# Implementation detail libraries and tests

cc_library(
    name = "chrono_policy_validation",
    testonly = True,
    hdrs = ["chrono_policy_validation.hh"],
    deps = [
        ":dimension",
        ":quantity",
        ":stdx",
        ":unit_of_measure",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "chrono_policy_validation_test",
    size = "small",
    srcs = ["chrono_policy_validation_test.cc"],
    deps = [
        ":chrono_policy_validation",
        ":prefix",
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "operators",
    hdrs = ["operators.hh"],
)

cc_test(
    name = "operators_test",
    size = "small",
    srcs = ["operators_test.cc"],
    deps = [
        ":operators",
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "conversion_policy",
    hdrs = ["conversion_policy.hh"],
    deps = [
        ":magnitude",
        ":stdx",
        ":unit_of_measure",
    ],
)

cc_test(
    name = "conversion_policy_test",
    size = "small",
    srcs = ["conversion_policy_test.cc"],
    deps = [
        ":conversion_policy",
        ":unit_of_measure",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "dimension",
    hdrs = ["dimension.hh"],
    deps = [":packs"],
)

cc_test(
    name = "dimension_test",
    size = "small",
    srcs = ["dimension_test.cc"],
    deps = [
        ":dimension",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "magnitude",
    hdrs = ["magnitude.hh"],
    deps = [
        ":packs",
        ":stdx",
        ":utility",
        ":zero",
    ],
)

cc_test(
    name = "magnitude_test",
    size = "small",
    srcs = ["magnitude_test.cc"],
    deps = [
        ":magnitude",
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "math",
    hdrs = ["math.hh"],
    deps = [
        ":quantity",
        ":quantity_point",
        ":units",
    ],
)

cc_test(
    name = "math_test",
    size = "small",
    srcs = ["math_test.cc"],
    deps = [
        ":math",
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "packs",
    hdrs = ["packs.hh"],
    deps = [
        ":stdx",
        ":utility",
    ],
)

cc_test(
    name = "packs_test",
    size = "small",
    srcs = ["packs_test.cc"],
    deps = [
        ":packs",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "prefix",
    hdrs = ["prefix.hh"],
    deps = [
        ":quantity",
        ":quantity_point",
        ":unit_of_measure",
    ],
)

cc_test(
    name = "prefix_test",
    size = "small",
    srcs = ["prefix_test.cc"],
    deps = [
        ":prefix",
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "quantity",
    hdrs = ["quantity.hh"],
    deps = [
        ":conversion_policy",
        ":operators",
        ":unit_of_measure",
        ":zero",
    ],
)

cc_test(
    name = "quantity_test",
    size = "small",
    srcs = [
        "quantity_chrono_policy_correspondence_test.cc",
        "quantity_test.cc",
    ],
    deps = [
        ":chrono_policy_validation",
        ":prefix",
        ":quantity",
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "quantity_point",
    hdrs = ["quantity_point.hh"],
    deps = [
        ":quantity",
        ":stdx",
        ":utility",
    ],
)

cc_test(
    name = "quantity_point_test",
    size = "small",
    srcs = ["quantity_point_test.cc"],
    deps = [
        ":prefix",
        ":quantity_point",
        ":testing",
        "@com_google_googletest//:gtest_main",
    ],
)

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
    name = "unit_of_measure",
    hdrs = ["unit_of_measure.hh"],
    deps = [
        ":dimension",
        ":magnitude",
        ":stdx",
        ":utility",
        ":zero",
    ],
)

cc_test(
    name = "unit_of_measure_test",
    size = "small",
    srcs = ["unit_of_measure_test.cc"],
    deps = [
        ":prefix",
        ":testing",
        ":unit_of_measure",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "utility",
    hdrs = glob(["utility/*.hh"]),
    deps = [":stdx"],
)

cc_test(
    name = "utility_test",
    size = "small",
    srcs = glob(["utility/test/*.cc"]),
    deps = [
        ":utility",
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

################################################################################
# Filegroups for the single-file tool

filegroup(
    name = "headers",
    srcs = glob(["**/*.hh"]),
    visibility = ["//:__pkg__"],
)
