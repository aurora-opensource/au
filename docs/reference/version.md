# Version macros

Au provides preprocessor macros that let downstream code detect that the library is present, and
which version of it is present.  They live in [`"au/version.hh"`](https://github.com/aurora-opensource/au/blob/main/au/version.hh), which every
Au header transitively includes, so they are available whenever any part of Au is in scope --- and
also in the single-file packages, since those always include `au/au.hh`.

## Macros

- `AU_VERSION_MAJOR`, `AU_VERSION_MINOR`, `AU_VERSION_PATCH`: the three components of the version, as
  integer literals.

- `AU_VERSION`: a single integer that increases monotonically with the version, so that ordinary
  integer comparisons order versions correctly.

- `AU_VERSION_NUMBER(major, minor, patch)`: computes the `AU_VERSION`-style integer for an arbitrary
  version, so you can compare against it.  Each component must be strictly less than `1000`.

- `AU_VERSION_IS_RELEASE`: `1` if this is an official tagged release, and `0` for an in-development
  (`main`) checkout.  See ["`main` versus tagged releases"](#caveat-main-versus-tagged-releases)
  below.

## Use cases

**Detecting that Au was included.**  Because every Au header pulls in the version macros, you can
`#error` (or warn) if the library is _not_ in scope:

```cpp
#if !defined(AU_VERSION)
#error "This file requires the Au units library to be included first."
#endif
```

**Detecting the version.**  This is useful when writing code that must support more than one Au
version during a migration:

```cpp
#if AU_VERSION < AU_VERSION_NUMBER(0, 5, 1)
    // ... code path for Au older than 0.5.1 ...
#else
    // ... code path for Au 0.5.1 and newer ...
#endif
```

## Caveat: `main` versus tagged releases

These macros are only authoritative for **tagged releases**.  Every Au release is cut from a
dedicated release branch, and the released commit never lives on `main`.  On `main`, the macros name
the _most recent release_ (they are kept in sync with the version in the root `CMakeLists.txt`,
which is derived from `au/version.hh`).  That means a `main` checkout can report version `X.Y.Z`
while actually containing changes made _after_ `X.Y.Z` was released.  In fact, because a
minor/major release branch is cut _from_ the `main` commit that bumps the version, that `main`
commit's three version numbers are byte-identical to the release's --- so the version numbers alone
cannot tell them apart.

Use `AU_VERSION_IS_RELEASE` to close that gap.  It is `1` only on official tagged releases, and `0`
on `main`:

```cpp
#if !AU_VERSION_IS_RELEASE
#warning "Building against an in-development checkout of Au, not a tagged release."
#endif
```
