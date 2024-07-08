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

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH AU_BASE_DIRECTORY)

#
# Private implementation detail targets
#

add_library(stdx INTERFACE)
target_sources(stdx
   INTERFACE
   FILE_SET HEADERS
   BASE_DIRS ${AU_BASE_DIRECTORY}
   FILES
      stdx/experimental/is_detected.hh
      stdx/functional.hh
      stdx/type_traits.hh
      stdx/utility.hh
)

add_executable(stdx_test)
target_sources(stdx_test
   PRIVATE
      stdx/test/utility_test.cc
)
target_link_libraries(stdx_test
   PRIVATE
      stdx
      GTest::gtest_main
)
gtest_discover_tests(stdx_test)
