# au: A C++14 units library

This repository is the future home of the `au` library.  In the initial stages, we are focused on
building out the machinery for building the code and docs: making it easy to get and run bazel, and
get the toolchains we need.  Once that foundation is in place, we'll port the full code.

## How to build

This is very much a work in progress and the top priority for improvement right now.  You can try
the following command:

```sh
bazel test //au/...:all --cxxopt='-std=c++14'
```

This assumes you have set up `direnv` or installed `bazel` on your system.  If you haven't done
either of those, then run `./tools/bin/bazel` instead of `bazel`.  (But you should probably set up
`direnv`; it makes your life much easier!)

The build will likely use whatever toolchain components you happen to have on your system.  Our
immediate goal is to add one hermetic toolchain for each officially supported target platform, and
to be able to select them by passing, say, `--config=clang11`, `--config=gcc10`, etc. While getting
this to work, we have added only enough code to be able to kick the tires on the build process.
Once we have it working, we'll add the rest of the code.
