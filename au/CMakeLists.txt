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
    abstract_operations.hh
    au.hh
    chrono_interop.hh
    constant.hh
    conversion_policy.hh
    conversion_strategy.hh
    dimension.hh
    fwd.hh
    io.hh
    magnitude.hh
    math.hh
    operators.hh
    overflow_boundary.hh
    packs.hh
    power_aliases.hh
    prefix.hh
    quantity.hh
    quantity_point.hh
    rep.hh
    std_format.hh
    truncation_risk.hh
    unit_of_measure.hh
    unit_symbol.hh
    wrapper_operations.hh
    zero.hh
    constants/avogadro_constant.hh
    constants/boltzmann_constant.hh
    constants/cesium_hyperfine_transition_frequency.hh
    constants/elementary_charge.hh
    constants/luminous_efficacy_540_terahertz.hh
    constants/planck_constant.hh
    constants/reduced_planck_constant.hh
    constants/speed_of_light.hh
    constants/standard_gravity.hh
    stdx/experimental/is_detected.hh
    stdx/functional.hh
    stdx/type_traits.hh
    stdx/utility.hh
    units/amperes.hh
    units/amperes_fwd.hh
    units/arcminutes.hh
    units/arcminutes_fwd.hh
    units/arcseconds.hh
    units/arcseconds_fwd.hh
    units/bars.hh
    units/bars_fwd.hh
    units/becquerel.hh
    units/becquerel_fwd.hh
    units/bits.hh
    units/bits_fwd.hh
    units/bytes.hh
    units/bytes_fwd.hh
    units/candelas.hh
    units/candelas_fwd.hh
    units/celsius.hh
    units/celsius_fwd.hh
    units/coulombs.hh
    units/coulombs_fwd.hh
    units/days.hh
    units/days_fwd.hh
    units/degrees.hh
    units/degrees_fwd.hh
    units/fahrenheit.hh
    units/fahrenheit_fwd.hh
    units/farads.hh
    units/farads_fwd.hh
    units/fathoms.hh
    units/fathoms_fwd.hh
    units/feet.hh
    units/feet_fwd.hh
    units/football_fields.hh
    units/football_fields_fwd.hh
    units/furlongs.hh
    units/furlongs_fwd.hh
    units/grams.hh
    units/grams_fwd.hh
    units/grays.hh
    units/grays_fwd.hh
    units/henries.hh
    units/henries_fwd.hh
    units/hertz.hh
    units/hertz_fwd.hh
    units/hours.hh
    units/hours_fwd.hh
    units/inches.hh
    units/inches_fwd.hh
    units/joules.hh
    units/joules_fwd.hh
    units/katals.hh
    units/katals_fwd.hh
    units/kelvins.hh
    units/kelvins_fwd.hh
    units/knots.hh
    units/knots_fwd.hh
    units/liters.hh
    units/liters_fwd.hh
    units/lumens.hh
    units/lumens_fwd.hh
    units/lux.hh
    units/lux_fwd.hh
    units/meters.hh
    units/meters_fwd.hh
    units/miles.hh
    units/miles_fwd.hh
    units/minutes.hh
    units/minutes_fwd.hh
    units/moles.hh
    units/moles_fwd.hh
    units/nautical_miles.hh
    units/nautical_miles_fwd.hh
    units/newtons.hh
    units/newtons_fwd.hh
    units/ohms.hh
    units/ohms_fwd.hh
    units/pascals.hh
    units/pascals_fwd.hh
    units/percent.hh
    units/percent_fwd.hh
    units/pounds_force.hh
    units/pounds_force_fwd.hh
    units/pounds_mass.hh
    units/pounds_mass_fwd.hh
    units/radians.hh
    units/radians_fwd.hh
    units/rankine.hh
    units/rankine_fwd.hh
    units/revolutions.hh
    units/revolutions_fwd.hh
    units/seconds.hh
    units/seconds_fwd.hh
    units/siemens.hh
    units/siemens_fwd.hh
    units/slugs.hh
    units/slugs_fwd.hh
    units/standard_gravity.hh
    units/standard_gravity_fwd.hh
    units/steradians.hh
    units/steradians_fwd.hh
    units/tesla.hh
    units/tesla_fwd.hh
    units/unos.hh
    units/unos_fwd.hh
    units/us_gallons.hh
    units/us_gallons_fwd.hh
    units/us_pints.hh
    units/us_pints_fwd.hh
    units/us_quarts.hh
    units/us_quarts_fwd.hh
    units/volts.hh
    units/volts_fwd.hh
    units/watts.hh
    units/watts_fwd.hh
    units/webers.hh
    units/webers_fwd.hh
    units/yards.hh
    units/yards_fwd.hh
    utility/factoring.hh
    utility/mod.hh
    utility/probable_primes.hh
    utility/string_constant.hh
    utility/type_traits.hh
)

# Skip the header verification for `std_format.hh`, since we don't expect it to
# work on all configurations.  Users are responsible for only including it in
# configurations that fully support `std::format`.
set_source_files_properties(
  std_format.hh
  PROPERTIES SKIP_LINTING TRUE
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
  NAME constants_test
  SRCS
    constants/test/avogadro_constant_test.cc
    constants/test/boltzmann_constant_test.cc
    constants/test/cesium_hyperfine_transition_frequency_test.cc
    constants/test/elementary_charge_test.cc
    constants/test/luminous_efficacy_540_terahertz_test.cc
    constants/test/planck_constant_test.cc
    constants/test/reduced_planck_constant_test.cc
    constants/test/speed_of_light_test.cc
    constants/test/standard_gravity_test.cc
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
    units/test/arcminutes_test.cc
    units/test/arcseconds_test.cc
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
    units/test/football_fields_test.cc
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
    units/test/rankine_test.cc
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
  NAME abstract_operations_test
  SRCS
    abstract_operations_test.cc
  DEPS
    au
    testing
)

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
  NAME conversion_strategy_test
  SRCS
    conversion_strategy_test.cc
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
  NAME overflow_boundary_test
  SRCS
    overflow_boundary_test.cc
  DEPS
    au
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
  NAME static_cast_checkers_test
  SRCS
    static_cast_checkers_test.cc
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
  NAME truncation_risk_test
  SRCS
    truncation_risk_test.cc
  DEPS
    au
    testing
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
    utility/test/mod_test.cc
    utility/test/probable_primes_test.cc
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
