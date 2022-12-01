load("@rules_python//python:pip.bzl", "compile_pip_requirements")
load("@pip_deps//:requirements.bzl", "requirement")

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

base_units = [
    "meters",
    "seconds",
    "grams",
    "kelvins",
    "amperes",
    "radians",
    "bits",
    "unos",
]

base_unit_string = " ".join(base_units)

cmd_root = "$(location tools/bin/make-single-file) {files} --units {units} > $(OUTS)"

################################################################################
# Release single-file package `au.hh`

genrule(
    name = "au_hh",
    srcs = ["//au:headers"],
    outs = ["docs/au.hh"],
    cmd = cmd_root.format(
        files = "au/au.hh au/io.hh",
        units = base_unit_string,
    ),
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
    cmd = cmd_root.format(
        files = "au/au.hh",
        units = base_unit_string,
    ),
    tools = ["tools/bin/make-single-file"],
    visibility = ["//release:__pkg__"],
)

cc_library(
    name = "au_noio_hh_lib",
    hdrs = ["docs/au_noio.hh"],
    visibility = ["//release:__pkg__"],
)
