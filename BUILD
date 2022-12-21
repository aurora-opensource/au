load("@pip_deps//:requirements.bzl", "requirement")
load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_python//python:defs.bzl", "py_binary")
load("@rules_python//python:pip.bzl", "compile_pip_requirements")

# This rule adds a convenient way to update the requirements file.
compile_pip_requirements(
    name = "requirements",
    requirements_in = "requirements.in",
    requirements_txt = "requirements_lock.txt",
)

py_binary(
    name = "update_docs",
    srcs = ["update_docs.py"],
    data = [
        "mkdocs.yml",
        ":au_hh",
        ":au_noio_hh",
    ] + glob(["docs/**"]),
    deps = [
        requirement("mkdocs"),
        requirement("mkdocs-material"),
    ],
)

BASE_UNITS = [
    "meters",
    "seconds",
    "grams",
    "kelvins",
    "amperes",
    "moles",
    "candelas",
    "radians",
    "bits",
    "unos",
]

BASE_UNIT_STRING = " ".join(BASE_UNITS)

GIT_ID_CMD = "cat bazel-out/stable-status.txt | grep STABLE_GIT_ID | sed 's/STABLE_GIT_ID \\(.*\\)/\\1/' | tr -d '\\n'"

CMD_ROOT = "$(location tools/bin/make-single-file) {extra_opts} --units {units} --version-id $$({id_cmd}) > $(OUTS)"

################################################################################
# Release single-file package `au.hh`

genrule(
    name = "au_hh",
    srcs = ["//au:headers"],
    outs = ["docs/au.hh"],
    cmd = CMD_ROOT.format(
        extra_opts = "",
        id_cmd = GIT_ID_CMD,
        units = BASE_UNIT_STRING,
    ),
    stamp = True,
    tools = ["tools/bin/make-single-file"],
)

cc_library(
    name = "au_hh_lib",
    hdrs = ["docs/au.hh"],
    visibility = ["//release:__pkg__"],
)

################################################################################
# Release single-file package `au_noio.hh`

genrule(
    name = "au_noio_hh",
    srcs = ["//au:headers"],
    outs = ["docs/au_noio.hh"],
    cmd = CMD_ROOT.format(
        extra_opts = "--noio",
        id_cmd = GIT_ID_CMD,
        units = BASE_UNIT_STRING,
    ),
    stamp = True,
    tools = ["tools/bin/make-single-file"],
    visibility = ["//release:__pkg__"],
)

cc_library(
    name = "au_noio_hh_lib",
    hdrs = ["docs/au_noio.hh"],
    visibility = ["//release:__pkg__"],
)
