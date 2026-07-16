// Copyright 2026 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

//
// Version macros for the Au library.
//
// These serve two purposes.  First, they let downstream code detect that Au has been included at
// all (for example, to `#error` if it has _not_ been): any Au header transitively includes this
// one, so `#if defined(AU_VERSION)` will be true whenever any part of Au is in scope.  Second, they
// let downstream code detect _which version_ of Au is present, which is useful for writing code
// that must support multiple Au versions during a migration.
//
// The individual components are available as `AU_VERSION_MAJOR`, `AU_VERSION_MINOR`, and
// `AU_VERSION_PATCH`.  For convenience, `AU_VERSION` combines them into a single integer that
// increases monotonically with the version, so that ordinary integer comparisons work:
//
//     #if AU_VERSION < AU_VERSION_NUMBER(0, 5, 1)
//         // ... code for Au older than 0.5.1 ...
//     #endif
//
// IMPORTANT (release model): these numbers are only authoritative for _tagged releases_.  Every Au
// release is cut from a dedicated release branch, and the released commit never lives on `main`
// (see `RELEASE.md`).  On `main`, these macros name the _most recent release_, mirroring the
// version in the root `CMakeLists.txt` (which is derived from this file).  That means a `main`
// checkout can report version `X.Y.Z` while actually containing changes made _after_ `X.Y.Z` was
// released.  There is currently no programmatic way to tell such a `main` checkout apart from the
// tagged `X.Y.Z` release; they report the same version number.
//
// To keep the two build systems in sync, `CMakeLists.txt` parses the three component macros below
// to populate its `project(... VERSION ...)`.  This file is therefore the single source of truth
// for the library version, and it is the _only_ place that needs to be edited when bumping the
// version for a release.
//

#define AU_VERSION_MAJOR 0
#define AU_VERSION_MINOR 5
#define AU_VERSION_PATCH 0

// Combine major/minor/patch components into a single monotonically increasing integer.  Each
// component gets three decimal digits, so components must be strictly less than 1000.
#define AU_VERSION_NUMBER(major, minor, patch) ((major) * 1000000 + (minor) * 1000 + (patch))

#define AU_VERSION AU_VERSION_NUMBER(AU_VERSION_MAJOR, AU_VERSION_MINOR, AU_VERSION_PATCH)

// Whether this is an actual tagged release (`1`) or an in-development checkout (`0`).
//
// Because a minor/major release branch is cut _from_ the `main` commit that bumps the version, that
// commit's version numbers above are byte-identical to the release's --- so the version numbers
// alone cannot tell `main` apart from the release it was branched from.  This flag closes that gap.
// It is `0` on `main` at all times, and is flipped to `1` only on the release branch (as part of
// the release-only step that also updates the doc links; see `RELEASE.md`).  Downstream code that
// must be sure it is building against an official release, and not an arbitrary `main` checkout,
// can therefore test `#if AU_VERSION_IS_RELEASE`.
#define AU_VERSION_IS_RELEASE 0
