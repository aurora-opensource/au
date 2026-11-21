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

function(header_only_library)
  #
  # Handle argument parsing
  #

  set(prefix ARG)
  set(noValueVars INTERNAL_ONLY SKIP_HEADER_VERIFICATION)
  set(singleValueVars
    NAME
    GENERATED_BASE_DIR
  )
  set(multiValueVars
    HEADERS
    GENERATED_HEADERS
    DEPS
  )

  cmake_parse_arguments(
    PARSE_ARGV 0
    "${prefix}"
    "${noValueVars}"
    "${singleValueVars}"
    "${multiValueVars}"
  )

  #
  # Function body
  #

  # Add the main target.
  add_library(${ARG_NAME} INTERFACE)
  target_sources(
    ${ARG_NAME}
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS "${PROJECT_SOURCE_DIR}"
    FILES ${ARG_HEADERS}
  )

  # Generated headers live in the build tree, so they get a file set of their own: the base
  # directories of a single file set may not be nested, and an in-tree build directory (such as
  # `cmake/build`) would nest the build tree inside `PROJECT_SOURCE_DIR`.
  set(file_sets FILE_SET HEADERS)
  if (DEFINED ARG_GENERATED_HEADERS)
    if (NOT DEFINED ARG_GENERATED_BASE_DIR)
      message(FATAL_ERROR "header_only_library: GENERATED_HEADERS requires GENERATED_BASE_DIR")
    endif()
    target_sources(
      ${ARG_NAME}
      INTERFACE
      FILE_SET generated_headers
      TYPE HEADERS
      BASE_DIRS "${ARG_GENERATED_BASE_DIR}"
      FILES ${ARG_GENERATED_HEADERS}
    )
    list(APPEND file_sets FILE_SET generated_headers)
  endif()

  if (DEFINED ARG_DEPS)
    target_link_libraries(${ARG_NAME} INTERFACE ${ARG_DEPS})
  endif()

  # Some headers are intentionally not self-contained, so we must skip verifying them.
  if (ARG_SKIP_HEADER_VERIFICATION)
    set_target_properties(${ARG_NAME} PROPERTIES VERIFY_INTERFACE_HEADER_SETS OFF)
  endif()

  # Require C++14 support.
  #
  # (We can consider adding a parameter to make this customizable later.)
  target_compile_features(${ARG_NAME} INTERFACE cxx_std_14)

  # If it's internal-only, change its export name accordingly.
  # See: https://stackoverflow.com/a/68321274
  if (ARG_INTERNAL_ONLY)
    set_target_properties(${ARG_NAME} PROPERTIES EXPORT_NAME "_Au_private_${ARG_NAME}")
  else()
    add_library(Au::${ARG_NAME} ALIAS ${ARG_NAME})

    # Track every public target, so we can list all public headers (see `print_au_files`).
    set_property(GLOBAL APPEND PROPERTY AU_PUBLIC_TARGETS ${ARG_NAME})
  endif()

  # Install the library.  (This is required for other projects to use Au via CMake.)
  include(GNUInstallDirs)
  install(
    TARGETS ${ARG_NAME}
    EXPORT ${AU_EXPORT_SET_NAME}
    ${file_sets}
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  )
endfunction()

function(gtest_based_test)
  if (NOT AU_ENABLE_TESTING)
    message(VERBOSE "AU_ENABLE_TESTING not defined; will not create test target.")
    return()
  endif()

  #
  # Handle argument parsing
  #

  set(prefix ARG)
  set(singleValueVars NAME)
  set(multiValueVars
    SRCS
    DEPS
  )

  cmake_parse_arguments(
    PARSE_ARGV 0
    "${prefix}"
    "${noValueVars}"
    "${singleValueVars}"
    "${multiValueVars}"
  )

  #
  # Function body
  #

  # Add the test.
  add_executable(${ARG_NAME})
  target_sources(${ARG_NAME} PRIVATE ${ARG_SRCS})
  target_link_libraries(
    ${ARG_NAME}
    PRIVATE
    ${ARG_DEPS}
    GTest::gmock_main
  )
  gtest_discover_tests(${ARG_NAME})
endfunction()
