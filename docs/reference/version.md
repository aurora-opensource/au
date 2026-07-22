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

These macros are a contract for **tagged releases** only.  On a tagged release, `AU_VERSION` names
exactly that release's feature set, so version comparisons are sound when comparing one release
against another --- both "is the feature from `X.Y.Z` present?" (`>=`) and "does this predate the
breaking change in `X.Y.Z`?" (`<`).

On `main`, the macros name the _most recent release_ (kept in sync with the version in the root
`CMakeLists.txt`, which is derived from `au/version.hh`).  But `main`'s number lags the changes that
have actually landed on it since that release, so **do not use these macros to select behavior
against a `main` checkout.**  The two directions even fail differently: an additive `>=` check
merely under-reports a not-yet-released feature (a safe false negative), but a breaking-change `<`
check can silently report the _old_ behavior on a `main` commit that already has the _new_ one (an
unsafe false positive).  Version-gate behavior only against tagged releases.

If you need to detect a _specific_ change robustly --- including on `main`, or to distinguish two
changes that ship in the same release --- the version number is the wrong tool.  The right one is a
dedicated per-feature macro, introduced in the same commit that makes the change, which you can test
with `#if defined(...)`.
