// Copyright 2022 Aurora Operations, Inc.
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

#include "au/au.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"

// For testing/tutorial purposes.
using namespace au;

// The distance (in meters) it would take to stop, starting from a given speed and acceleration.
//
// Parameters:
// - speed_mps:  The starting speed, in meters per second.
// - acceleration_mpss:  The braking acceleration, in meters per second squared.
//
// Preconditions:
// - speed_mps >= 0.0
// - acceleration_mpss < 0.0
double stopping_distance_m(double speed_mps, double acceleration_mpss);
