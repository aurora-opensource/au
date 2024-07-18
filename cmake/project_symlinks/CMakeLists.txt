# Copyright 2024 Aurora Operations, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# We must create the symlink to `au/` at configure time, because doing it via CMake means that it
# will work on both linux and windows.
execute_process(
   COMMAND ${CMAKE_COMMAND}
   -E create_symlink
   ${PROJECT_SOURCE_DIR}/au
   ${PROJECT_SOURCE_DIR}/cmake/project_symlinks/au
)

# Having created the symlink, we can now add the subdirectory.
add_subdirectory(au)
