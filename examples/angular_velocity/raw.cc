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
// with corresponding constructs on corresponding lines.  `//examples:angular_velocity_test`
// enforces the line count; keeping the lines *meaningfully* aligned is on you.
//
// That region is fenced off from clang-format, which would otherwise collapse short function
// bodies onto one line and silently destroy the alignment.  Format it by hand, in house style.
// The fences sit outside the snippet markers, so they never show up on the website.

// --8<-- [start:frontmatter]
#include <cmath>
#include <iostream>
// --8<-- [end:frontmatter]

// clang-format off
// --8<-- [start:example]
// --8<-- [start:headline]
// Speed must be m/s.  Radius must be meters.  Returns RPM.
float wheel_rpm(float v_mps, float r_m) {
    return v_mps / (2.0f * static_cast<float>(M_PI) * r_m) * 60.0f;
}
// --8<-- [end:headline]

int main() {
    // The wheel radius is 350 mm, so convert it to meters by hand first.
    const float omega_rpm = wheel_rpm(15.0f, 350.0f / 1000.0f);
    std::cout << omega_rpm << " rev / min" << '\n';  // Unit label typed by hand.
}
// --8<-- [end:example]
// clang-format on
