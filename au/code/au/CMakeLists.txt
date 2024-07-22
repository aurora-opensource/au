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
    au.hh
  DEPS
    chrono_interop
    constant
    math
  GTEST_SRCS
    au_test.cc
  GTEST_EXTRA_DEPS
    testing
)

header_only_library(
  NAME chrono_interop
  HEADERS
    chrono_interop.hh
  DEPS
    quantity
    units
  GTEST_SRCS
    chrono_interop_test.cc
  GTEST_EXTRA_DEPS
    prefix
    testing
)

header_only_library(
  NAME constant
  HEADERS
    constant.hh
  DEPS
    quantity
    unit_of_measure
    wrapper_operations
  GTEST_SRCS
    constant_test.cc
  GTEST_EXTRA_DEPS
    chrono_interop
    testing
    units
)

header_only_library(
  NAME io
  HEADERS
    io.hh
  DEPS
    quantity
    quantity_point
    zero
  GTEST_SRCS
    io_test.cc
  GTEST_EXTRA_DEPS
    prefix
)

header_only_library(
  NAME math
  HEADERS
    math.hh
  DEPS
    constant
    quantity
    quantity_point
    units
  GTEST_SRCS
    math_test.cc
  GTEST_EXTRA_DEPS
    testing
)

header_only_library(
  NAME prefix
  HEADERS
    prefix.hh
  DEPS
    quantity
    quantity_point
    unit_of_measure
    unit_symbol
  GTEST_SRCS
    prefix_test.cc
  GTEST_EXTRA_DEPS
    testing
)

header_only_library(
  NAME quantity
  HEADERS
    quantity.hh
  DEPS
    apply_magnitude
    conversion_policy
    operators
    rep
    unit_of_measure
    zero
  GTEST_SRCS
    quantity_chrono_policy_correspondence_test.cc
    quantity_test.cc
  GTEST_EXTRA_DEPS
    chrono_policy_validation
    prefix
    testing
)

header_only_library(
  NAME quantity_point
  HEADERS
    quantity_point.hh
  DEPS
    quantity
    stdx
    utility
  GTEST_SRCS
    quantity_point_test.cc
  GTEST_EXTRA_DEPS
    prefix
    testing
)

header_only_library(
  NAME testing
  HEADERS
    testing.hh
  DEPS
    io
    stdx
    unit_of_measure
    GTest::gmock
  GTEST_SRCS
    testing_test.cc
)

header_only_library(
  NAME units
  HEADERS
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
    units/volts.hh
    units/watts.hh
    units/webers.hh
    units/yards.hh
  DEPS
    prefix
    quantity
    quantity_point
    unit_of_measure
    unit_symbol
  GTEST_SRCS
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
    units/test/volts_test.cc
    units/test/watts_test.cc
    units/test/webers_test.cc
    units/test/yards_test.cc
  GTEST_EXTRA_DEPS
    testing
)

header_only_library(
  NAME unit_symbol
  HEADERS
    unit_symbol.hh
  DEPS
    wrapper_operations
  GTEST_SRCS
    unit_symbol_test.cc
  GTEST_EXTRA_DEPS
    testing
    units
)

#
# Private implementation detail targets
#

header_only_library(
  NAME apply_magnitude
  INTERNAL_ONLY
  HEADERS
    apply_magnitude.hh
  DEPS
    apply_rational_magnitude_to_integral
  GTEST_SRCS
    apply_magnitude_test.cc
  GTEST_EXTRA_DEPS
    testing
)

header_only_library(
  NAME apply_rational_magnitude_to_integral
  INTERNAL_ONLY
  HEADERS
    apply_rational_magnitude_to_integral.hh
  DEPS
    magnitude
  GTEST_SRCS
    apply_rational_magnitude_to_integral_test.cc
  GTEST_EXTRA_DEPS
    testing
)

header_only_library(
  NAME chrono_policy_validation
  INTERNAL_ONLY
  HEADERS
    chrono_policy_validation.hh
  DEPS
    dimension
    quantity
    stdx
    unit_of_measure
    GTest::gtest
  GTEST_SRCS
    chrono_policy_validation_test.cc
  GTEST_EXTRA_DEPS
    prefix
    testing
)


header_only_library(
  NAME conversion_policy
  INTERNAL_ONLY
  HEADERS
    conversion_policy.hh
  DEPS
    magnitude
    stdx
    unit_of_measure
  GTEST_SRCS
    conversion_policy_test.cc
)

header_only_library(
  NAME dimension
  INTERNAL_ONLY
  HEADERS
    dimension.hh
  DEPS
    packs
    power_aliases
  GTEST_SRCS
    dimension_test.cc
  GTEST_EXTRA_DEPS
    testing
    units
)

header_only_library(
  NAME magnitude
  INTERNAL_ONLY
  HEADERS
    magnitude.hh
  DEPS
    packs
    power_aliases
    stdx
    utility
    zero
  GTEST_SRCS
    magnitude_test.cc
  GTEST_EXTRA_DEPS
    testing
)

header_only_library(
  NAME operators
  INTERNAL_ONLY
  HEADERS
    operators.hh
  GTEST_SRCS
    operators_test.cc
  GTEST_EXTRA_DEPS
    testing
)

header_only_library(
  NAME packs
  INTERNAL_ONLY
  HEADERS
    packs.hh
  DEPS
    stdx
    utility
  GTEST_SRCS
    packs_test.cc
)

header_only_library(
  NAME power_aliases
  INTERNAL_ONLY
  HEADERS
    power_aliases.hh
  GTEST_SRCS
    power_aliases_test.cc
  GTEST_EXTRA_DEPS
    packs
)

header_only_library(
  NAME rep
  INTERNAL_ONLY
  HEADERS
    rep.hh
  DEPS
    stdx
  GTEST_SRCS
    rep_test.cc
  GTEST_EXTRA_DEPS
    chrono_interop
    constant
    magnitude
    prefix
    quantity
    quantity_point
    unit_symbol
    units
)

header_only_library(
  NAME stdx
  INTERNAL_ONLY
  HEADERS
    stdx/experimental/is_detected.hh
    stdx/functional.hh
    stdx/type_traits.hh
    stdx/utility.hh
  GTEST_SRCS
    stdx/test/utility_test.cc
)

header_only_library(
  NAME unit_of_measure
  INTERNAL_ONLY
  HEADERS
    unit_of_measure.hh
  DEPS
    dimension
    magnitude
    power_aliases
    stdx
    utility
    zero
  GTEST_SRCS
    unit_of_measure_test.cc
  GTEST_EXTRA_DEPS
    prefix
    testing
    units
)

header_only_library(
  NAME utility
  INTERNAL_ONLY
  HEADERS
    utility/factoring.hh
    utility/string_constant.hh
    utility/type_traits.hh
  DEPS
    stdx
  GTEST_SRCS
    utility/test/factoring_test.cc
    utility/test/string_constant_test.cc
    utility/test/type_traits_test.cc
)

header_only_library(
  NAME wrapper_operations
  INTERNAL_ONLY
  HEADERS
    wrapper_operations.hh
  DEPS
    quantity
    stdx
  GTEST_SRCS
    wrapper_operations_test.cc
  GTEST_EXTRA_DEPS
    testing
    units
)

header_only_library(
  NAME zero
  INTERNAL_ONLY
  HEADERS
    zero.hh
  GTEST_SRCS
    zero_test.cc
)
