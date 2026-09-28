# Development Setup

This page explains how to set up the repository for development.  This is for two main groups of
users:

- Those who want to **contribute** to the library, either the code or the docs.
- Those who want to work through the **tutorial exercises**.

!!! tip
    If all you want to do is **install** the library in your project, you can simply use our
    [installation guide](./install.md).

## Core development setup

These steps are common to all workflows.

### Step 0: Decide whether to fork

If you're _confident that you want to contribute_, you should start out by _forking_ the repository.
This gives you your own copy which you can modify, and from which you can create pull requests to
share your changes with the main repository.  Here are [GitHub's instructions for forking][fork].

**Everyone else can simply skip this step** and work directly from the main repository.  If you
decide later on to make a pull request, you'll still need to create a fork, but it's easy to set it
up after the fact.  We'll provide detailed instructions below.

??? info "Instructions for switching an existing clone to a fork"
    If you're here, you probably cloned the main repository directly, and then later decided to make
    a pull request.  For that, you'll need to switch your `origin` remote to point to your fork
    instead of the main repository.

    You can directly use the instructions below, which we adapted from [this
    guide](https://admcpr.com/what-the-fork/).

    1. **[Create the fork][fork] on GitHub.**

           You will need to note the name of your fork, in the format `user_name/repo_name`.
           Typically, the `user_name` part will be replaced with your GitHub username, and the
           `repo_name` part will simply be `au`.

    2. **Rename `origin` to `upstream`.**  We still want to track the main repository; we just need
       to give it a different name.  Run this command verbatim:

           ```sh
           git remote rename origin upstream
           ```

    3. **Make your fork the new `origin`.**  Your command will start with `git remote add origin`,
       and end with a URL for your repository.  If your GitHub username were `user_name`, the full
       command should look like this:

           ```sh
           git remote add origin git@github.com:user_name/au.git
           #                                    ^^^^^^^^^
           # Example only: remember to replace user name!
           ```

    4. **Fetch from `origin`.**  Run this command verbatim:

           ```sh
           git fetch origin
           ```

    5. **Track `origin` from `main`.**  Run this command verbatim:

           ```sh
           git branch -u origin/main main
           ```

    At this point, you should be able to create a pull request in the usual way, simply by pushing
    a local branch to your fork via the `origin` remote.


### Step 1: Clone the repository

This step gets the Au source code onto your machine.  The precise details depend on whether you
decided to fork or not in the previous step.

!!! question "Did you fork, or not?"

    === "No fork (default)"
        This is for users who just want to work through the tutorials, or play around with the code.

        ```sh
        git clone https://github.com/aurora-opensource/au.git
        ```

    === "Fork"
        Follow [GitHub's cloning instructions][clone], using the fork you created as the repository.

Whichever approach you took, you should have a folder named `au/` in your current directory which
contains the Au source code.

### Step 2: Set up `direnv`

[direnv] is a tool that makes it easy to use the correct version of every tool we use: `bazel`,
`clang-format`, `buildifier`, and other project-specific ones like `make-single-file` and
`au-docs-serve`.  It will add these tools to your `$PATH`, but **only** when you're inside your
copy of the repository.

??? warning "What if you skip this step?"
    This step is optional, but highly recommended.  If you skip this step, you'll need to prepend
    `tools/bin/` to every command that comes from Au's tools directory.  For example,

    - Replace `bazel` with `tools/bin/bazel`.
    - Replace `clang-format` with `tools/bin/clang-format`.

    And so on.

    Additionally, if you use `direnv`, you can run the tools from any folder.  However, if you
    don't, you can only run them from the project root folder.

The first step to set up `direnv` is to install the tool itself, using [their installation
instructions](https://direnv.net/docs/installation.html).

The next time you enter your Au folder, you'll get a warning message like this:

```
direnv: error .envrc is blocked. Run `direnv allow` to approve its content.
```

Simply do what it says and run `direnv allow`, and you're all set!

??? tip "Testing your installation"
    You can test that everything's working by running `bazel --version` inside your Au folder.  You
    should get a result compatible with the current contents of the `.bazelversion` file.  For
    example, at the time of writing, we're on bazel 6.0.0, so this command produces the output:

    ```
    bazel 6.0.0
    ```

## Specific workflows

Now that your basic development setup is complete, here are some types of workflows you can do.

### Building and testing the code

To build and test the entire repository, run `bazel test //...:all`.

!!! note
    The first time you run any command such as `bazel`, there may be some additional overhead from
    downloading or configuring the tool itself.  This is a one-time cost, and each subsequent run
    should be fast.

You can also specify any number of specific targets or target patterns, using [bazel's target
syntax](https://bazel.build/run/build#specifying-build-targets).  For example, if you wanted to test
the core library code (which lives in `//au`), _and_ test the generated single-file package (whose
target is `//release:au_hh_test`), you could write:

```sh
bazel test //au:all //release:au_hh_test
```

#### Using different toolchains

Au comes pre-packaged with support for several different compiler toolchains.  To use a specific
toolchain --- say, `X` --- pass it as a `--config=X` argument.  For example, here's how you would
run all of the tests using gcc 15:

```sh
bazel test --config=gcc15 //...:all
```

Here are the possible values we support for `--config`:

| `--config` value | compiler |
|------------------|----------|
| `clang17` | Clang 17 (default) |
| `clang14` | Clang 14 |
| `clang11` | Clang 11 |
| `gcc12` | gcc 12 |
| `gcc12` | gcc 15 |

??? question "What if your preferred compiler isn't in this list?"
    Our goal is for Au to work with any standards-compliant compiler that fully supports C++14, or
    any later language standard.  We've had good results with a variety of compilers already, so we
    recommend simply trying Au in yours!

    If you do, and you find a bug, please feel free to file an issue.  If the compiler is fully
    C++14-compatible, we'll do our best to find a fix, or an acceptable workaround.  We may also
    consider adding the compiler to our officially supported list, as long as we can use it via
    a hermetic bazel toolchain.

### Building and viewing documentation

It's easy to set up a local version of the documentation website.  Simply run the included command,
`au-docs-serve`.  Here's some example output:

```
INFO: Analyzed target //:update_docs (0 packages loaded, 0 targets configured).
INFO: Found 1 target...
Target //:update_docs up-to-date:
  bazel-bin/update_docs
INFO: Elapsed time: 0.309s, Critical Path: 0.06s
INFO: 1 process: 1 internal.
INFO: Build completed successfully, 1 total action
INFO: Build completed successfully, 1 total action
INFO     -  Building documentation...
INFO     -  Cleaning site directory
INFO     -  Documentation built in 0.35 seconds
INFO     -  [16:02:34] Watching paths for changes: 'docs', 'mkdocs.yml'
INFO     -  [16:02:34] Serving on http://127.0.0.1:8000/au/
```

The last line shows that you can view the website at the included (local) URL,
`http://127.0.0.1:8000/au/`.

The command stays running until you interrupt it (typically via `Ctrl-C`, or [the equivalent on your
terminal](https://superuser.com/a/1210919)).  Note that as long as it stays running, it will
automatically regenerate the website whenever you edit any file, and the browser will automatically
reload the page!

??? note "Viewing documentation on a remote machine"
    Some users connect to a remote machine via `ssh` to do their development.  In this case,
    `au-docs-serve` won't work out of the box.  The reason is that it's running on your remote
    machine, but your web browser is on your local machine.  When you try to open the URL
    `http://127.0.0.1:8000/au/`, which points to the _local machine_, it can't find the web server,
    because it's running on the remote machine.

    The solution is to forward the port, `8000`, when you connect to your remote machine.  If you
    do, then _local_ requests on that port will be forwarded along to the _remote_ machine, where
    the web server is running.  Here's how you do that.

    1. **Find your usual ssh command.**  This is whatever you run on your _local_ machine to connect
       to the remote host.  For example, if your username is `user` and your remote hostname is
       `remote.host`, this might look like the following:

        ```sh
        ssh user@remote.host
        ```

    2. **Add an option for port forwarding.**  Expanding on the previous example, this would be:

        ```sh
        ssh -L 8000:localhost:8000 user@remote.host
        #  ^^^^^^^^^^^^^^^^^^^^^^^ Note: this is added
        ```

    3. **Run `au-docs-serve` on the remote host.**  Naturally, you'll need to be in your Au folder
       to do this.

    At this point, as long as that command stays running on your _remote_ host, you should be able
    to visit the URL in your _local_ browser, and view the documentation website.

### Measuring compile time impact

Our code examples don't just show how to use the library; they also double as a built-in set of
benchmarks for measuring its compile time costs.  We can measure _changes_ by compiling the same
example on different commits.  In fact, since most examples have a curated "raw" (no Au) version
built in, they even help us measure the _absolute magnitude_ of those costs.

The most common use case is to measure the compile time impact of a single branch (typically for
a pull request) against its parent.  For that, we provide a simple command:

```sh
measure-branch-impact -n 200
```

This compiles each example 200 times on both the current branch and its baseline --- the commit
where this branch left `main`, rather than the tip of `main`, so that other people's changes don't
get folded into yours.  200 repetitions is typically a good balance between getting enough data
points to be meaningful, and not taking too long to complete: on a typical development machine,
this takes less than an hour.  For best results, avoid running other programs while the measurement
is in progress, and consider setting the CPU governor to `performance` if that is relevant to your
machine (common on laptops).  However, even if you don't, the statistical analysis should be good
enough to indicate the impact of the change.

Note that only _committed_ work is measured: each branch is compiled in its own git worktree, so
uncommitted changes in your working tree don't take part (the tool says so if you have any).

The results can be found in the `~/compile-time-measurements/` folder, within a date/time-stamped
subfolder.  The report can be viewed locally, but it's also meant to be easily pasted into a pull
request comment on GitHub.  To do this, copy the contents of `report.md` into your comment, and
attach every file in the `plots/` subfolder.

??? note "Which commit is the baseline, for a branch in a stack"
    A branch that tracks another _local_ branch is a branch sitting on top of another that hasn't
    landed yet, so that branch is its parent, and the measurement is taken against it rather than
    against `main`.  Measuring against `main` would fold every branch underneath this one into its
    number.  Worse, it would hide whatever this branch adds: a target that exists on only one side
    of the comparison has nothing to compare against, so it drops out of the report entirely.

    A branch that tracks a _remote_ branch is just a published copy of itself, which says nothing
    about where the branch diverged.  That case is measured against `main`, as usual.

??? note "Disk space requirements and handling"
    Every branch is measured in its own git worktree, and each worktree needs its own bazel output
    base --- roughly 8 GiB, nearly all of it extracted toolchains.  These have to coexist, so
    a comparison of `N` branches wants about `8 * N` GiB free while it runs.  A run deletes them
    when it finishes, and sweeps up anything a previous run left behind when it starts, so this
    space is not permanently lost.  If a run is killed outright, `measure-compile-time --clean`
    reclaims it by hand.

#### Reading the plots

Begin by eyeballing the weather plot to look for any concerning trends.  If it looks reasonable,
check the percentile plots (regular and zoomed) to see if there is any noticeable difference.
After that, the percentile _difference_ plot can help you see how consistent that difference is.

For assessing _changes_, the overall Au penalty plot doesn't give any new information beyond the
plots above.  That plot is mainly useful for keeping track of the overall cost of the library, and
how it varies by use case.

#### More complicated comparisons

The above tool is built on our more general `measure-compile-time` tool.  You can run with `--help`
to see all of the options, but here's an example.

```sh
measure-compile-time -b main -b my-branch -n 200 -t //examples:eigen_kinematics_au
```

Note that you can specify any number of targets, and any number of branches (at least two: the
first one you name is the baseline that the others are compared against).  With no `-t`, every code
example is measured.

If you want to change the report or the plots without paying for the measurements again, point
`--report-only` at a finished run's folder: it re-renders both from the `raw.csv` that's already
there.


[fork]: https://docs.github.com/en/get-started/quickstart/fork-a-repo
[clone]: https://docs.github.com/en/repositories/creating-and-managing-repositories/cloning-a-repository
[direnv]: https://direnv.net/
[direnv instructions]: https://direnv.net/docs/installation.html
