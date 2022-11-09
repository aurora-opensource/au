load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

string_flag(
    name = "requested_compiler_flag",
    build_setting_default = "",
    values = [
        "clang11",
        "clang14",
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
