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
// IMPORTANT (release model): these numbers are a contract for _tagged releases_ only.  On a tagged
// release, `AU_VERSION` names exactly the feature set of that release, so version comparisons are
// sound _release to release_ --- both "is the feature added in `X.Y.Z` present?" (`>=`) and "does
// this predate the breaking change in `X.Y.Z`?" (`<`).  On `main`, these macros name the _most
// recent release_ (mirroring the version in the root `CMakeLists.txt`, which is derived from this
// file), and `main` is re-bumped to match _every_ release it contains (patches included; see
// `RELEASE.md`).
//
// Do NOT use these macros to select behavior against a `main` checkout.  Because `main`'s number
// lags the changes that have actually landed on it since the last release, such a check is
// unreliable --- and the two directions fail differently: an additive `>=` check merely
// under-reports (a safe false negative), but a breaking-change `<` check can silently report the
// _old_ behavior on a `main` commit that already has the _new_ one (an unsafe false positive).
// Version-gate behavior only against tagged releases.
//
// For detecting a _specific_ change robustly --- including on `main`, or to distinguish two changes
// that ship in the same release --- introduce a dedicated per-feature macro in the same commit that
// makes the change, rather than reaching for `AU_VERSION`.
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
