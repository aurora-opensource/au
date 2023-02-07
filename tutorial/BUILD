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
