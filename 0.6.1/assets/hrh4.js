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

// The delineation between h4 sections can be too subtle.  Add hr to make it stand out better.
Array.from(document.getElementsByTagName("H4"))
  .forEach(function(e, i) {
    while (e = e.nextElementSibling) {
      // Break at the next tag name.
      if (["H1", "H2", "H3", "H4"].includes(e.tagName)) {
        e.prepend(document.createElement("hr"));
        break;
      }
    }
  });
