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

// NOTE TO EDITORS: this file is line-aligned with `au.cc`, and the doc-visible region is fenced off
// from clang-format so the alignment survives.  See `examples/angular_velocity/raw.cc` for the
// full explanation.

// --8<-- [start:frontmatter]
#include <iostream>
// --8<-- [end:frontmatter]

// clang-format off
// --8<-- [start:example]
// A 12-bit analog-to-digital converter (ADC) reports `counts` out of 4095,
// spanning a 3300 millivolt (mV) reference.
int adc_to_millivolts(int counts) {
    return counts * 3300 / 4095;  // Multiply first: 3300 / 4095 would truncate to 0.
}



int main() {
    const int mv = adc_to_millivolts(2048);
    // Careful: `mv / 1000` would quietly report 1650 mV as "1 V".
    std::cout << mv << " mV" << std::endl;
}
// --8<-- [end:example]
// clang-format on
