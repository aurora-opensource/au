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

#include "tutorial/102_api_types.hh"

double stopping_distance_m(double speed_mps, double acceleration_mpss) {
    // This first implementation uses only the most basic kinematic equations.
    // We could refactor it later to be more efficient.

    // t = (v - v0) / a
    double t_s = -speed_mps / acceleration_mpss;

    // (x - x0) = (v0 * t) + (1/2)(a * t^2)
    return speed_mps * t_s + 0.5 * acceleration_mpss * t_s * t_s;
}
