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

# CMake should always be at least as new as your compiler[1].  Clang 14 was released on March 25,
# 2022, and is our newest officially supported compiler.  The next CMake release was v3.23.0, on
# March 29, 2022.  Therefore, our minimum version must be at least CMake 3.23.
#
# We also use the `CMAKE_VERIFY_INTERFACE_HEADER_SETS`, which was added in CMake
# 3.24, which means the minimum must also be at least 3.24.
#
# The maximum version should be the latest version we've tested the project with.
#
# [1]: https://cliutils.gitlab.io/modern-cmake/chapters/intro/dodonot.html
cmake_minimum_required(VERSION 3.24...3.29)

project(
   Au
   VERSION 0.3.5
   DESCRIPTION "A C++ quantities and units library, by Aurora Innovation"
   HOMEPAGE_URL "https://aurora-opensource.github.io/au/0.3.5/"
   LANGUAGES CXX
)

option(AU_ENABLE_TESTING "Build Unit Tests" ON)

# The export set for all of our headers.
set(AU_EXPORT_SET_NAME AuHeaders)

if(AU_ENABLE_TESTING)
   message(STATUS "Unit tests enabled")
   enable_testing()

   # Bring in GoogleTest so we can build and run the tests.
   include(FetchContent)
   FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG 58d77fa8070e8cec2dc1ed015d66b454c8d78850 # Release 1.12.1
      FIND_PACKAGE_ARGS
         1.12.1
         NAMES GTest
   )

   # https://google.github.io/googletest/quickstart-cmake.html
   # For Windows: Prevent overriding the parent project's compiler/linker settings
   set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

   FetchContent_MakeAvailable(googletest)
   include(GoogleTest)

   set(AU_CONFIG_FIND_GTEST_LINE "find_dependency(googletest 1.12.1)")
else()
   message(STATUS "Unit tests disabled")
endif()

add_subdirectory(au)

# Configure how Au will be installed.
#
# (This is necessary for it to be usable in other CMake projects.)

set(AU_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Au)

install(
   EXPORT ${AU_EXPORT_SET_NAME}
   DESTINATION ${AU_CMAKE_DIR}
   NAMESPACE Au::
   FILE AuHeaders.cmake
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/AuConfig.cmake.in
   AuConfig.cmake
   INSTALL_DESTINATION ${AU_CMAKE_DIR}
)

write_basic_package_version_file(
   AuConfigVersion.cmake
   COMPATIBILITY SameMinorVersion
)

install(
   FILES
      ${CMAKE_CURRENT_BINARY_DIR}/AuConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/AuConfigVersion.cmake
   DESTINATION ${AU_CMAKE_DIR}
)
