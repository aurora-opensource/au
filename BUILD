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
        ":single_file_au",
        ":single_file_au_noio",
    ] + glob(["docs/**"]),
    deps = [
        requirement("mkdocs"),
        requirement("mkdocs-material"),
    ],
)

genrule(
    name = "single_file_au",
    srcs = ["//au:headers"],
    outs = ["docs/au.hh"],
    cmd = "$(location tools/bin/make-single-file) au/au.hh au/io.hh > $(OUTS)",
    tools = ["tools/bin/make-single-file"],
)

genrule(
    name = "single_file_au_noio",
    srcs = ["//au:headers"],
    outs = ["docs/au_noio.hh"],
    cmd = "$(location tools/bin/make-single-file) au/au.hh > $(OUTS)",
    tools = ["tools/bin/make-single-file"],
)
