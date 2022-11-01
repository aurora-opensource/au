# au: A C++14 units library

This repository is the future home of the `au` library.  In the initial stages, we are focused on
building out the machinery for building the code and docs: making it easy to get and run bazel, and
get the toolchains we need.  Once that foundation is in place, we'll port the full code.

## How to build

This is very much a work in progress and the top priority for improvement right now.  You can try
the following command:

```sh
bazel test //au/...:all
```

This assumes you have set up `direnv` or installed `bazel` on your system.  If you haven't done
either of those, then run `./tools/bin/bazel` instead of `bazel`.  (But you should probably set up
`direnv`; it makes your life much easier!)

The build will use a hermetic clang toolchain by default.

### Available compilers

Use the following `--config` settings to switch to different compilers.

| Compiler | Config option |
| --- | --- |
| Latest supported Clang | `--config=clang` |
| Clang 11 | `--config=clang11` |
| Clang 14 | `--config=clang14` |

Note that the sandboxed compilers do not use a sysroot.
