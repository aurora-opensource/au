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
# We also use the `SKIP_LINTING` property, which was added in CMake 3.27, which means the minimum
# must also be at least 3.27.
#
# The maximum version should be the latest version we've tested the project with.
#
# [1]: https://cliutils.gitlab.io/modern-cmake/chapters/intro/dodonot.html
cmake_minimum_required(VERSION 3.27...3.29)

# Read the library version from the `module()` block of `MODULE.bazel`, the only place it is written
# down (Bazel evaluates that file before it can read any other, so it can't live anywhere else).  It
# feeds `project()` below, and `au/CMakeLists.txt` generates `au/version.hh` from it.
set(_au_module_bazel "${CMAKE_CURRENT_SOURCE_DIR}/MODULE.bazel")
file(READ "${_au_module_bazel}" _au_module_bazel_contents)

# The `module()` block only: the file is full of other `version =` assignments, in the `bazel_dep`
# calls.
string(REGEX MATCH "\nmodule\\([^)]*\n\\)" _au_module_block "${_au_module_bazel_contents}")
string(REGEX MATCH
   "\n[ \t]*version = \"([0-9]+)\\.([0-9]+)\\.([0-9]+)\""
   _au_module_version_line
   "${_au_module_block}"
)

# Fail loudly rather than silently building a malformed version such as "0.5." (which would
# otherwise flow into `project()`, the docs URL, `au/version.hh`, and installed configs).
if(NOT _au_module_version_line)
   message(FATAL_ERROR
      "Could not read a MAJOR.MINOR.PATCH version from the module() block of ${_au_module_bazel}. "
      "Expected a line like 'version = \"1.2.3\",'.")
endif()

set(AU_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(AU_VERSION_MINOR "${CMAKE_MATCH_2}")
set(AU_VERSION_PATCH "${CMAKE_MATCH_3}")
set(AU_VERSION "${AU_VERSION_MAJOR}.${AU_VERSION_MINOR}.${AU_VERSION_PATCH}")

# CMake can't know that the configuration depends on a file we merely read; tell it, so that bumping
# the version regenerates `au/version.hh`.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_au_module_bazel}")

project(
   Au
   VERSION ${AU_VERSION}
   DESCRIPTION "A C++ quantities and units library, by Aurora Innovation"
   HOMEPAGE_URL "https://aurora-opensource.github.io/au/${AU_VERSION}/"
   LANGUAGES CXX
)

include(CMakeDependentOption)

option(
   AU_EXCLUDE_GTEST_DEPENDENCY
   "Avoid taking any dependency on googletest. This option implies that tests will be skipped"
   OFF
)
cmake_dependent_option(
   AU_ENABLE_TESTING
   "Build Unit Tests"
   ON
   "NOT AU_EXCLUDE_GTEST_DEPENDENCY"
   OFF
)

# C++ module support requires CMake 3.28 for the (non-experimental) `CXX_MODULES` file set.
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.28)
   option(AU_BUILD_MODULE "Build the `au` C++ module (requires C++20)" OFF)
else()
   set(AU_BUILD_MODULE OFF)
endif()

# The export set for all of our headers.
set(AU_EXPORT_SET_NAME AuHeaders)

if(AU_ENABLE_TESTING)
   message(STATUS "Unit tests enabled")
   enable_testing()
else()
   message(STATUS "Unit tests disabled")
endif()

# Bring in GoogleTest so we can build and run the tests.
include(FetchContent)
if (NOT AU_EXCLUDE_GTEST_DEPENDENCY)
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

# Enable listing all public headers, so that we can test that CMake has identical headers to bazel.
get_property(AU_PUBLIC_TARGETS GLOBAL PROPERTY AU_PUBLIC_TARGETS)
set(AU_ALL_HEADERS "")
foreach(target IN LISTS AU_PUBLIC_TARGETS)
   # Every header set, not just the default one: generated headers have a set of their own (see
   # `header_only_library`).
   get_target_property(header_sets ${target} INTERFACE_HEADER_SETS)
   foreach(header_set IN LISTS header_sets)
      if(header_set STREQUAL "HEADERS")
         get_target_property(target_headers ${target} HEADER_SET)
      else()
         get_target_property(target_headers ${target} HEADER_SET_${header_set})
      endif()
      list(APPEND AU_ALL_HEADERS ${target_headers})
   endforeach()
endforeach()
add_custom_target(
   print_au_files
   COMMAND
      echo "PRESERVE: ${AU_ALL_HEADERS}"
   COMMAND_EXPAND_LISTS
)
