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

load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

string_flag(
    name = "requested_compiler_flag",
    build_setting_default = "",
    values = [
        "clang11",
        "clang14",
        "gcc10",
    ],
)

config_setting(
    name = "clang11_requested",
    flag_values = {"requested_compiler_flag": "clang11"},
    visibility = ["//visibility:public"],
)

config_setting(
    name = "clang14_requested",
    flag_values = {"requested_compiler_flag": "clang14"},
    visibility = ["//visibility:public"],
)

config_setting(
    name = "gcc10_requested",
    flag_values = {"requested_compiler_flag": "gcc10"},
    visibility = ["//visibility:public"],
)
