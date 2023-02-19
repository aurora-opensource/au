// Copyright 2023 Aurora Operations, Inc.
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

// clang-format off

// First, include your Au installation.
//
// For "full install" options, it will likely look like what is included here.
//
// For "single file" options, it will simply be your single file.  However, make sure that it
// includes all of the following units!
#include "au/au.hh"
#include "au/units/amperes.hh"
#include "au/units/bytes.hh"
#include "au/units/candelas.hh"
#include "au/units/grams.hh"
#include "au/units/kelvins.hh"
#include "au/units/meters.hh"
#include "au/units/moles.hh"
#include "au/units/radians.hh"
#include "au/units/seconds.hh"

// Next, include your nholthaus units installation.
#include "nholthaus_units/units.h"

// Finally, include the compatibility layer.
//
// This file MUST be included AFTER all others.
#include "compatibility/nholthaus_units.hh"

// clang-format on
