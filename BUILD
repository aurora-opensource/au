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
    data = ["mkdocs.yml"] + glob(["docs/**"]),
    deps = [
        requirement("mkdocs"),
        requirement("mkdocs-material"),
    ],
)
