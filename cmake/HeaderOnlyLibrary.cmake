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
  set(noValueVars INTERNAL_ONLY)
  set(singleValueVars NAME)
  set(multiValueVars
    HEADERS
    DEPS
    GTEST_SRCS
    GTEST_EXTRA_DEPS
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
    BASE_DIRS ${CMAKE_SOURCE_DIR}
    FILES ${ARG_HEADERS}
  )
  if (DEFINED ARG_DEPS)
    target_link_libraries(${ARG_NAME} INTERFACE ${ARG_DEPS})
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
  endif()

  # Install the library.  (This is required for other projects to use Au via CMake.)
  include(GNUInstallDirs)
  install(
    TARGETS ${ARG_NAME}
    EXPORT ${AU_EXPORT_SET_NAME}
    FILE_SET HEADERS
  )

  # Add the test, if requested.
  if (DEFINED ARG_GTEST_SRCS)
    add_executable("${ARG_NAME}_test")
    target_sources("${ARG_NAME}_test" PRIVATE ${ARG_GTEST_SRCS})
    target_link_libraries(
      "${ARG_NAME}_test"
      PRIVATE
      ${ARG_NAME}
      ${ARG_GTEST_EXTRA_DEPS}
      GTest::gmock_main
    )
    gtest_discover_tests("${ARG_NAME}_test")
  endif()
endfunction()
