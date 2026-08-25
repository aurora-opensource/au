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

// NOTE TO EDITORS: this file is line-aligned with `au.cc`, so that readers can flip between the two
// on the doc website and compare corresponding lines in place.  The region between the
// `[start:example]` and `[end:example]` markers must keep the same number of lines in both files,
// with corresponding constructs on corresponding lines.  `//examples:eigen_kinematics_test`
// enforces the line count; keeping the lines *meaningfully* aligned is on you.
//
// That region is fenced off from clang-format, which would otherwise reflow the parameter lists and
// silently destroy the alignment.  Format it by hand, in house style.  The fences sit outside the
// snippet markers, so they never show up on the website.
//
// Every declaration here names `Eigen::Vector3d` rather than using `const auto`, which would blink
// more neatly against the Au tab's `const auto`.  It is deliberate: `v_mps`'s initializer contains
// a multiply, so `auto` there deduces an Eigen expression template rather than a vector.  With
// `v_kph` named on its own line that expression would at least not dangle, but storing a lazy
// expression is still a habit the safety guide warns against -- and using `auto` on the other
// declarations but not that one would read as arbitrary.  So all three name the type, which is
// what a careful Eigen user writes anyway.
//
// `v_kph` exists to show what the raw version actually costs: the same velocity, stored twice, in
// two units, told apart only by a name suffix.  Do not "simplify" it back into one statement --
// the Au tab's blank line opposite it is the point.

// --8<-- [start:frontmatter]
#include <iostream>

#include "Eigen/Core"
// --8<-- [end:frontmatter]

// clang-format off
// --8<-- [start:example]
// Position must be meters, velocity m/s, acceleration m/s^2, and the timestep seconds.
Eigen::Vector3d advanced_position(const Eigen::Vector3d &x_m,
                                  const Eigen::Vector3d &v_mps,
                                  const Eigen::Vector3d &a_mps2,
                                  double dt_s) {
    return x_m + v_mps * dt_s + 0.5 * a_mps2 * dt_s * dt_s;
}

int main() {
    const Eigen::Vector3d x_m     {0.0, 0.0, 120.0};

    // The velocity arrives as 72 km/h downrange, so keep a second copy of it, scaled into m/s.
    const Eigen::Vector3d v_kph   {72.0, 0.0, 0.0};
    const Eigen::Vector3d v_mps = v_kph * (1000.0 / 3600.0);
    const Eigen::Vector3d a_mps2  {0.0, 0.0, -9.80665};

    // The timestep is 250 ms; convert that by hand too.
    const Eigen::Vector3d x_new_m = advanced_position(x_m, v_mps, a_mps2, 250.0 / 1000.0);

    std::cout << x_new_m.transpose() << " m" << '\n';  // Unit label typed by hand.
    std::cout << x_new_m.norm() << " m" << '\n';       // ...and again, nothing checks it.
}
// --8<-- [end:example]
// clang-format on
