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

include("${PROJECT_SOURCE_DIR}/cmake/HeaderOnlyLibrary.cmake")

#
# Publicly exported targets
#

header_only_library(
  NAME au
  HEADERS
    apply_magnitude.hh
    apply_rational_magnitude_to_integral.hh
    au.hh
    chrono_interop.hh
    constant.hh
    conversion_policy.hh
    dimension.hh
    io.hh
    magnitude.hh
    math.hh
    operators.hh
    packs.hh
    power_aliases.hh
    prefix.hh
    quantity.hh
    quantity_point.hh
    rep.hh
    unit_of_measure.hh
    unit_symbol.hh
    wrapper_operations.hh
    zero.hh
    stdx/experimental/is_detected.hh
    stdx/functional.hh
    stdx/type_traits.hh
    stdx/utility.hh
    units/amperes.hh
    units/bars.hh
    units/becquerel.hh
    units/bits.hh
    units/bytes.hh
    units/candelas.hh
    units/celsius.hh
    units/coulombs.hh
    units/days.hh
    units/degrees.hh
    units/fahrenheit.hh
    units/farads.hh
    units/fathoms.hh
    units/feet.hh
    units/furlongs.hh
    units/grams.hh
    units/grays.hh
    units/henries.hh
    units/hertz.hh
    units/hours.hh
    units/inches.hh
    units/joules.hh
    units/katals.hh
    units/kelvins.hh
    units/knots.hh
    units/liters.hh
    units/lumens.hh
    units/lux.hh
    units/meters.hh
    units/miles.hh
    units/minutes.hh
    units/moles.hh
    units/nautical_miles.hh
    units/newtons.hh
    units/ohms.hh
    units/pascals.hh
    units/percent.hh
    units/pounds_force.hh
    units/pounds_mass.hh
    units/radians.hh
    units/revolutions.hh
    units/seconds.hh
    units/siemens.hh
    units/slugs.hh
    units/standard_gravity.hh
    units/steradians.hh
    units/tesla.hh
    units/unos.hh
    units/us_gallons.hh
    units/us_pints.hh
    units/us_quarts.hh
    units/volts.hh
    units/watts.hh
    units/webers.hh
    units/yards.hh
    utility/factoring.hh
    utility/string_constant.hh
    utility/type_traits.hh
)

if (NOT AU_EXCLUDE_GTEST_DEPENDENCY)
  header_only_library(
    NAME testing
    HEADERS testing.hh
    DEPS
      au
      GTest::gmock
  )
endif()

#
# Private implementation detail targets
#

if (NOT AU_EXCLUDE_GTEST_DEPENDENCY)
  header_only_library(
    NAME chrono_policy_validation
    INTERNAL_ONLY
    HEADERS chrono_policy_validation.hh
    DEPS
      au
      GTest::gtest
  )
endif()

#
# Tests
#

gtest_based_test(
  NAME au_test
  SRCS
    au_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME chrono_interop_test
  SRCS
    chrono_interop_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME constant_test
  SRCS
    constant_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME io_test
  SRCS
    io_test.cc
  DEPS
    au
)

gtest_based_test(
  NAME math_test
  SRCS
    math_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME prefix_test
  SRCS
    prefix_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME quantity_test
  SRCS
    quantity_chrono_policy_correspondence_test.cc
    quantity_test.cc
  DEPS
    au
    chrono_policy_validation
    testing
)

gtest_based_test(
  NAME quantity_point_test
  SRCS
    quantity_point_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME testing_test
  SRCS
    testing_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME units_test
  SRCS
    units/test/amperes_test.cc
    units/test/bars_test.cc
    units/test/becquerel_test.cc
    units/test/bits_test.cc
    units/test/bytes_test.cc
    units/test/candelas_test.cc
    units/test/celsius_test.cc
    units/test/coulombs_test.cc
    units/test/days_test.cc
    units/test/degrees_test.cc
    units/test/fahrenheit_test.cc
    units/test/farads_test.cc
    units/test/fathoms_test.cc
    units/test/feet_test.cc
    units/test/furlongs_test.cc
    units/test/grams_test.cc
    units/test/grays_test.cc
    units/test/henries_test.cc
    units/test/hertz_test.cc
    units/test/hours_test.cc
    units/test/inches_test.cc
    units/test/joules_test.cc
    units/test/katals_test.cc
    units/test/kelvins_test.cc
    units/test/knots_test.cc
    units/test/liters_test.cc
    units/test/lumens_test.cc
    units/test/lux_test.cc
    units/test/meters_test.cc
    units/test/miles_test.cc
    units/test/minutes_test.cc
    units/test/moles_test.cc
    units/test/nautical_miles_test.cc
    units/test/newtons_test.cc
    units/test/ohms_test.cc
    units/test/pascals_test.cc
    units/test/percent_test.cc
    units/test/pounds_force_test.cc
    units/test/pounds_mass_test.cc
    units/test/radians_test.cc
    units/test/revolutions_test.cc
    units/test/seconds_test.cc
    units/test/siemens_test.cc
    units/test/slugs_test.cc
    units/test/standard_gravity_test.cc
    units/test/steradians_test.cc
    units/test/tesla_test.cc
    units/test/unos_test.cc
    units/test/us_gallons_test.cc
    units/test/us_pints_test.cc
    units/test/us_quarts_test.cc
    units/test/volts_test.cc
    units/test/watts_test.cc
    units/test/webers_test.cc
    units/test/yards_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME unit_symbol_test
  SRCS
    unit_symbol_test.cc
  DEPS
    au
    testing
)

#
# Private implementation detail targets
#

gtest_based_test(
  NAME apply_magnitude_test
  SRCS
    apply_magnitude_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME apply_rational_magnitude_to_integral_test
  SRCS
    apply_rational_magnitude_to_integral_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME chrono_policy_validation_test
  SRCS
    chrono_policy_validation_test.cc
  DEPS
    au
    chrono_policy_validation
    testing
)


gtest_based_test(
  NAME conversion_policy_test
  SRCS
    conversion_policy_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME dimension_test
  SRCS
    dimension_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME magnitude_test
  SRCS
    magnitude_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME operators_test
  SRCS
    operators_test.cc
  DEPS
    testing
)

gtest_based_test(
  NAME packs_test
  SRCS
    packs_test.cc
  DEPS
    au
)

gtest_based_test(
  NAME power_aliases_test
  SRCS
    power_aliases_test.cc
  DEPS
    au
)

gtest_based_test(
  NAME rep_test
  SRCS
    rep_test.cc
  DEPS
    au
)

gtest_based_test(
  NAME stdx_test
  SRCS
    stdx/test/utility_test.cc
  DEPS
    au
)

gtest_based_test(
  NAME unit_of_measure_test
  SRCS
    unit_of_measure_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME utility_test
  SRCS
    utility/test/factoring_test.cc
    utility/test/string_constant_test.cc
    utility/test/type_traits_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME wrapper_operations_test
  SRCS
    wrapper_operations_test.cc
  DEPS
    au
    testing
)

gtest_based_test(
  NAME zero_test
  SRCS
    zero_test.cc
  DEPS
    au
)
