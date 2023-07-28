# Supported Compilers

We've endeavored to use only standards-compliant C++14 in Au, with the goal of making it work with
any compiler that fully supports the C++14 standard (or any newer one). Of course, there's no
substitute for testing!  So we also include certain platform/compiler combinations in our automated
test suites, which we run on every commit.

!!! note
    We'll use "configuration" as a shorthand for "platform/compiler combination" for the rest of
    this doc.

Formally, we have three tiers of support.

1. **Full Support.**  These configurations must be kept passing on every commit.
2. **Best Effort Support.**  We measure _whether_ these configurations are passing on every commit,
   and strive to _keep_ them passing.  However, if we cannot do so, we may let them break.
3. **Assumed Support.**  These include all other C++14-compatible configurations which we do not
   measure explicitly.

Each tier has requirements that must be met for a configuration to be included in it. Any
configuration that doesn't meet the bar for any higher tier is in the "Assumed Support" tier.

## Status changes

We expect most status changes will be user-led.  Users who want a higher tier of support for their
preferred configuration are welcome to file an issue, especially if they can provide some guidance
as to how we can help it meet the requirements for that tier (that is, how to set up suitable GitHub
workflows or bazel toolchains for it).

Some status changes may be out of our control.  For example, if GitHub actions drops support for
Windows Server 2019, we won't be able to continue running GitHub workflows for MSVC x64 19.29.  If
that happened, we would have to move that configuration to the "Assumed Support" tier.

## Tiers: Requirements and Members

Now we'll go into more detail about each tier.  We'll explain the requirements which a configuration
must meet to join that tier, and we'll list the configurations that are currently included.

### Full Support

To add a configuration to the Full Support tier, we must be able to add a hermetic toolchain to our
bazel workspace. The motivation for this requirement is that we want any developer to be able to
check out the repository, and have a tight iteration cycle with full _local_ reproducibility for any
CI errors.

Here are the configurations that have Full Support status.

| Platform | Compiler | Status |
|----------|----------|--------|
| Ubuntu | clang 11 | [![clang11-ubuntu]( https://github.com/aurora-opensource/au/actions/workflows/clang11-ubuntu.yml/badge.svg?branch=main&event=push)]( https://github.com/aurora-opensource/au/actions/workflows/clang11-ubuntu.yml) |
| Ubuntu | clang 14 | [![clang14-ubuntu]( https://github.com/aurora-opensource/au/actions/workflows/clang14-ubuntu.yml/badge.svg?branch=main&event=push)]( https://github.com/aurora-opensource/au/actions/workflows/clang14-ubuntu.yml) |
| Ubuntu | gcc 10 | [![gcc10-ubuntu]( https://github.com/aurora-opensource/au/actions/workflows/gcc10-ubuntu.yml/badge.svg?branch=main&event=push)]( https://github.com/aurora-opensource/au/actions/workflows/gcc10-ubuntu.yml) |

### Best Effort Support

To add a configuration to the Best Effort tier, we must be able to set up an automated GitHub
workflow job which runs tests on that configuration.  This means that any developer will be able to
iterate on these configurations, although some may be forced to push commits remotely and trigger CI
jobs to do so.

Although we reserve the right to drop support for these configurations, we will strive to avoid
doing so if at all possible.  We expect to be able to succeed and keep these passing in perpetuity.
Even in the unlikely event that any of these configurations break, we'll continue to provide
measurement badges, so that potential users with these configurations are forewarned.

Here are the configurations that have Best Effort Support status.

| Platform | Compiler | Status |
|----------|----------|--------|
| Windows Server 2019 | MSVC x64 19.29 | [![MSVC x64 19.29]( https://github.com/aurora-opensource/au/actions/workflows/msvc-x64-19-29-30151.yml/badge.svg?branch=main&event=push)]( https://github.com/aurora-opensource/au/actions/workflows/msvc-x64-19-29-30151.yml) |
| Windows Server 2022 | MSVC x64 19.35 | [![MSVC x64 19.35]( https://github.com/aurora-opensource/au/actions/workflows/msvc-x64-19-35-32217-1.yml/badge.svg?branch=main&event=push)]( https://github.com/aurora-opensource/au/actions/workflows/msvc-x64-19-35-32217-1.yml) |
