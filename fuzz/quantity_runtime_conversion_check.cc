// Copyright 2025 Aurora Operations, Inc.
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

#include <cmath>
#include <limits>
#include <sstream>
#include <string>

#include "au/conversion_strategy.hh"
#include "au/math.hh"
#include "au/testing.hh"
#include "au/units/inches.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/yards.hh"
#include "fuzz/quantity_runtime_conversion_checkers.hh"
#include "gtest/gtest.h"

namespace au {
namespace detail {

enum class RoundTripResult {
    IDENTICAL,
    SIGNIFICANT_LOSS,
    MINOR_LOSS,
    IGNORE_SUBNORMALS,
    CAST_NON_INT_TO_INT_IS_DEFINITELY_LOSSY,
};

std::string print(RoundTripResult result) {
    switch (result) {
        case RoundTripResult::IDENTICAL:
            return "IDENTICAL";
        case RoundTripResult::SIGNIFICANT_LOSS:
            return "SIGNIFICANT_LOSS";
        case RoundTripResult::MINOR_LOSS:
            return "MINOR_LOSS";
        case RoundTripResult::IGNORE_SUBNORMALS:
            return "IGNORE_SUBNORMALS";
        case RoundTripResult::CAST_NON_INT_TO_INT_IS_DEFINITELY_LOSSY:
            return "CAST_NON_INT_TO_INT_IS_DEFINITELY_LOSSY";
    }
    return "UNKNOWN";
}

template <typename T>
T next_higher(T x, std::size_t n = 1u) {
    static_assert(std::is_floating_point<T>::value, "Utility only for floating point");
    while (n-- > 0u) {
        x = std::nextafter(x, std::numeric_limits<T>::infinity());
    }
    return x;
}

template <typename T>
T next_lower(T x, std::size_t n = 1u) {
    static_assert(std::is_floating_point<T>::value, "Utility only for floating point");
    while (n-- > 0u) {
        x = std::nextafter(x, -std::numeric_limits<T>::infinity());
    }
    return x;
}

template <typename U, typename R>
Quantity<U, R> next_higher_quantity(Quantity<U, R> q, std::size_t n = 1u) {
    return make_quantity<U>(next_higher(q.data_in(U{}), n));
}

template <typename U, typename R>
Quantity<U, R> next_lower_quantity(Quantity<U, R> q, std::size_t n = 1u) {
    return make_quantity<U>(next_lower(q.data_in(U{}), n));
}

constexpr std::size_t MAX_DIST = 1000u;

template <typename T>
std::size_t distance(T a, T b) {
    std::size_t dist = 0u;
    while (a != b) {
        ++dist;
        a = std::nextafter(a, b);
        if (dist >= MAX_DIST) {
            break;
        }
    }
    return dist;
}

template <typename T>
struct Tag {};

template <typename T, typename U>
constexpr bool operator==(Tag<T>, Tag<U>) {
    return std::is_same<T, U>::value;
}

template <typename T>
T type_of(const Tag<T> &);

template <typename T>
struct UnwrapTagImpl : stdx::type_identity<T> {};
template <typename T>
using UnwrapTag = typename UnwrapTagImpl<T>::type;
template <typename T>
struct UnwrapTagImpl<Tag<T>> : stdx::type_identity<T> {};

template <typename... Ts>
struct ListImpl;
template <typename... Ts>
using List = typename ListImpl<Ts...>::type;
template <typename... Ts>
struct ListImpl : stdx::type_identity<std::tuple<Tag<Ts>...>> {};

using Reps = List<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float>;
using Units = List<Inches, Feet, Meters, Miles, Yards, Centi<Meters>, Nano<Meters>>;

template <typename PackT>
struct GeneratorForImpl;
template <typename PackT>
using GeneratorFor = typename GeneratorForImpl<PackT>::type;
template <template <class...> class Pack, typename... Ts>
struct GeneratorForImpl<Pack<Ts...>>
    : stdx::type_identity<RandomValueGenerators<UnwrapTag<Ts>...>> {};

template <typename ListT>
struct ForEach;
template <>
struct ForEach<std::tuple<>> {
    template <typename Func>
    void operator()(const Func &) {}
};
template <typename T, typename... Ts>
struct ForEach<std::tuple<T, Ts...>> {
    template <typename Func>
    void operator()(const Func &f) const {
        f(T{});
        ForEach<std::tuple<Ts...>>{}(f);
    }
};

enum class TestCategory {
    INTEGRAL_TO_INTEGRAL,
    INTEGRAL_TO_FLOAT,
    FLOAT_TO_INTEGRAL,
    FLOAT_TO_FLOAT,
    TRIVIAL,
    IMPOSSIBLE,
    UNSUPPORTED,
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
constexpr TestCategory categorize_testing_scenario() {
    if (std::is_same<RepT, DestRepT>::value && std::is_same<UnitT, DestUnitT>::value) {
        return TestCategory::TRIVIAL;
    }

    using Common = std::common_type_t<RepT, DestRepT>;
    constexpr auto conversion_factor = UnitRatioT<UnitT, DestUnitT>{};

    if (is_integer(conversion_factor) &&
        (get_value_result<Common>(conversion_factor).outcome != MagRepresentationOutcome::OK)) {
        return TestCategory::IMPOSSIBLE;
    }

    if (is_integer(mag<1>() / conversion_factor) &&
        (get_value_result<Common>(mag<1>() / conversion_factor).outcome !=
         MagRepresentationOutcome::OK)) {
        return TestCategory::IMPOSSIBLE;
    }

    static_assert(std::is_integral<RepT>::value || std::is_floating_point<RepT>::value, "");
    static_assert(std::is_integral<DestRepT>::value || std::is_floating_point<DestRepT>::value, "");
    if (std::is_integral<RepT>::value) {
        return std::is_integral<DestRepT>::value ? TestCategory::INTEGRAL_TO_INTEGRAL
                                                 : TestCategory::INTEGRAL_TO_FLOAT;
    }

    return std::is_integral<DestRepT>::value ? TestCategory::FLOAT_TO_INTEGRAL
                                             : TestCategory::FLOAT_TO_FLOAT;
}

template <typename RepT,
          typename UnitT,
          typename DestRepT,
          typename DestUnitT,
          TestCategory Category>
struct TestBodyImpl;

enum class Unsigned {
    NEITHER,
    LHS_ONLY,
    RHS_ONLY,
    BOTH,
};

template <Unsigned>
struct SignFlipImpl;
template <>
struct SignFlipImpl<Unsigned::BOTH> {
    template <typename U1, typename R1, typename U2, typename R2>
    static constexpr bool assess(const Quantity<U1, R1> &, const Quantity<U2, R2> &) {
        return false;
    }
};
template <>
struct SignFlipImpl<Unsigned::LHS_ONLY> {
    template <typename U1, typename R1, typename U2, typename R2>
    static constexpr bool assess(const Quantity<U1, R1> &, const Quantity<U2, R2> &b) {
        return b < ZERO;
    }
};
template <>
struct SignFlipImpl<Unsigned::RHS_ONLY> {
    template <typename U1, typename R1, typename U2, typename R2>
    static constexpr bool assess(const Quantity<U1, R1> &a, const Quantity<U2, R2> &) {
        return a < ZERO;
    }
};
template <>
struct SignFlipImpl<Unsigned::NEITHER> {
    template <typename U1, typename R1, typename U2, typename R2>
    static constexpr bool assess(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b) {
        return (a < ZERO) != (b < ZERO);
    }
};
template <typename U1, typename R1, typename U2, typename R2>
constexpr bool sign_flip(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b) {
    constexpr auto unsignedness =
        std::is_unsigned<R1>::value
            ? (std::is_unsigned<R2>::value ? Unsigned::BOTH : Unsigned::LHS_ONLY)
            : (std::is_unsigned<R2>::value ? Unsigned::RHS_ONLY : Unsigned::NEITHER);
    return SignFlipImpl<unsignedness>::assess(a, b);
}

struct LossCheck {
    RoundTripResult result;
    std::string comment = "";
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT, TestCategory Cat>
struct LossChecker {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &,
                                    const Quantity<UnitT, RepT> &round_trip) {
        // Default implementation.
        return {value == round_trip ? RoundTripResult::IDENTICAL
                                    : RoundTripResult::SIGNIFICANT_LOSS};
    }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct LossChecker<RepT, UnitT, DestRepT, DestUnitT, TestCategory::INTEGRAL_TO_INTEGRAL> {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &destination,
                                    const Quantity<UnitT, RepT> &round_trip) {
        const bool expect_sign_flip =
            std::is_same<Sign<UnitRatioT<UnitT, DestUnitT>>, Magnitude<Negative>>::value;
        const bool wrong_sign = (sign_flip(value, destination) != expect_sign_flip);

        const bool actual_loss = (value != round_trip) || wrong_sign;
        return {
            actual_loss ? RoundTripResult::SIGNIFICANT_LOSS : RoundTripResult::IDENTICAL,
            wrong_sign ? "Sign was wrong" : "",
        };
    }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct LossChecker<RepT, UnitT, DestRepT, DestUnitT, TestCategory::FLOAT_TO_FLOAT> {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &destination,
                                    const Quantity<UnitT, RepT> &round_trip) {
        if (!std::isnormal(value.in(UnitT{})) || !std::isnormal(destination.in(DestUnitT{}))) {
            return {RoundTripResult::IGNORE_SUBNORMALS, "Subnormal value"};
        }
        const auto min_ok = next_lower_quantity(value, 2u);
        const auto max_ok = next_higher_quantity(value, 2u);

        std::ostringstream oss;
        oss << "Breakdown:" << std::setprecision(std::numeric_limits<RepT>::digits10 + 1u) << '\n'
            << "  Initial:    " << value << '\n'
            << "  Min OK:     " << min_ok << '\n'
            << "  Round trip: " << round_trip << '\n'
            << "  Max OK:     " << max_ok << '\n';

        const auto result =
            ((round_trip == value)                          ? RoundTripResult::IDENTICAL
             : (round_trip < min_ok || round_trip > max_ok) ? RoundTripResult::SIGNIFICANT_LOSS
                                                            : RoundTripResult::MINOR_LOSS);

        return {result, oss.str()};
    }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct LossChecker<RepT, UnitT, DestRepT, DestUnitT, TestCategory::INTEGRAL_TO_FLOAT> {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &destination,
                                    const Quantity<UnitT, RepT> &round_trip) {
        auto fraction = value / 100'000;
        if (std::is_signed<RepT>::value && value < ZERO) {
            fraction = -fraction;
        }
        const auto dx = max(fraction, make_quantity<UnitT>(RepT{1}));

        constexpr auto lowest = std::numeric_limits<Quantity<UnitT, RepT>>::lowest();
        const auto min_ok = (lowest + dx > value) ? lowest : rep_cast<RepT>(value - dx);

        constexpr auto highest = std::numeric_limits<Quantity<UnitT, RepT>>::max();
        const auto max_ok = (highest - dx < value) ? highest : rep_cast<RepT>(value + dx);

        const bool ignore_because_subnormal = !std::isnormal(destination.in(DestUnitT{}));

        std::ostringstream oss;
        oss << "We went through floating point; so, taking a very liberal margin.  Breakdown:"
            << std::endl
            << "  Initial:    " << value << std::endl
            << "  Destination: " << destination << (ignore_because_subnormal ? " (subnormal)" : "")
            << std::endl
            << "  Min OK:     " << min_ok << std::endl
            << "  Round trip: " << round_trip << std::endl
            << "  Max OK:     " << max_ok << std::endl;

        const auto result = ignore_because_subnormal ? RoundTripResult::IGNORE_SUBNORMALS
                            : round_trip == value    ? RoundTripResult::IDENTICAL
                            : round_trip < min_ok || round_trip > max_ok
                                ? RoundTripResult::SIGNIFICANT_LOSS
                                : RoundTripResult::MINOR_LOSS;

        return {result, oss.str()};
    }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct LossChecker<RepT, UnitT, DestRepT, DestUnitT, TestCategory::FLOAT_TO_INTEGRAL> {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &,
                                    const Quantity<UnitT, RepT> &round_trip) {
        constexpr auto MIN_GOOD_VALUE =
            max(std::numeric_limits<Quantity<DestUnitT, RepT>>::min().as(UnitT{}),
                std::numeric_limits<Quantity<UnitT, RepT>>::min());
        if (abs(value) < MIN_GOOD_VALUE) {
            return {RoundTripResult::IGNORE_SUBNORMALS, "Subnormal value"};
        }

        if (round_trip == value) {
            using Op = ConversionForRepsAndFactor<RepT, DestRepT, UnitRatioT<UnitT, DestUnitT>>;
            const auto dest_value = FloatingPointPrefixPart<Op>::apply_to(value.in(UnitT{}));
            const bool definitely_truncates = (std::trunc(dest_value) != dest_value);
            std::ostringstream oss;
            if (definitely_truncates) {
                oss << "Floating point portion of converting " << value << " to <"
                    << type_name<DestRepT>() << ">(" << unit_label(DestUnitT{}) << ") produced "
                    << dest_value << ", which differs from the truncated int by "
                    << std::abs(dest_value - std::trunc(dest_value));
            } else {
                oss << "Seems fine: floating point part produces " << dest_value
                    << ", which is an integer";
            }

            return {
                definitely_truncates ? RoundTripResult::CAST_NON_INT_TO_INT_IS_DEFINITELY_LOSSY
                                     : RoundTripResult::IDENTICAL,
                oss.str(),
            };
        }

        const auto dist = distance(value.in(UnitT{}), round_trip.in(UnitT{}));

        std::ostringstream oss;
        oss << "Distance was " << dist << " steps" << (dist >= MAX_DIST ? " (truncated)" : "");

        if (std::is_same<Abs<UnitRatioT<UnitT, DestUnitT>>, Magnitude<>>::value) {
            oss << ".  Saw round trip error on conversion factor whose absolute value was 1.";
            return {
                RoundTripResult::SIGNIFICANT_LOSS,
                oss.str(),
            };
        }

        return {
            (dist > 2u) ? RoundTripResult::SIGNIFICANT_LOSS : RoundTripResult::MINOR_LOSS,
            oss.str(),
        };
    }
};

template <typename T>
struct RepForImpl : stdx::type_identity<T> {};
template <typename T>
using RepFor = typename RepForImpl<T>::type;
template <typename U, typename R>
struct RepForImpl<Quantity<U, R>> : stdx::type_identity<R> {};

template <typename T>
std::string print_to_string(const T &value) {
    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<RepFor<T>>::digits10 + 3u) << value;
    return oss.str();
}

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT, TestCategory Cat>
struct NominalTestBodyImpl : LossChecker<RepT, UnitT, DestRepT, DestUnitT, Cat> {
    using LossChecker<RepT, UnitT, DestRepT, DestUnitT, Cat>::check_for_loss;

    using Op = ConversionForRepsAndFactor<RepT, DestRepT, UnitRatioT<UnitT, DestUnitT>>;

    static void test(const Quantity<UnitT, RepT> &value) {
        const bool expect_loss = is_conversion_lossy<DestRepT>(value, DestUnitT{});
        const bool expect_trunc = will_conversion_truncate<DestRepT>(value, DestUnitT{});
        const bool expect_overflow = will_conversion_overflow<DestRepT>(value, DestUnitT{});

        const auto destination = value.template as<DestRepT>(DestUnitT{}, ignore(ALL_RISKS));
        const auto dest_from_op = Op::apply_to(value.in(UnitT{}));
        if (!std::isnan(dest_from_op) && dest_from_op != destination.in(DestUnitT{})) {
            std::cerr << "Programming error: either `Op` is wrong, or it was applied wrong.\n"
                      << "<" << type_name<RepT>() << ">(" << unit_label(UnitT{}) << ") -> <"
                      << type_name<DestRepT>() << ">(" << unit_label(DestUnitT{}) << ")\n"
                      << "Initial value: " << print_to_string(value) << "\n"
                      << "Destination:   " << print_to_string(destination) << "\n"
                      << "Op applied:    " << print_to_string(Op::apply_to(value.in(UnitT{})))
                      << "\n"
                      << "Diff:          "
                      << print_to_string(destination.in(DestUnitT{}) - dest_from_op) << "\n";
            std::terminate();
        }

        const auto round_trip = destination.template as<RepT>(UnitT{}, ignore(ALL_RISKS));

        const auto loss_check = check_for_loss(value, destination, round_trip);
        const auto actual_loss = loss_check.result;

        const bool mismatch = (expect_loss && (actual_loss == RoundTripResult::IDENTICAL)) ||
                              (!expect_loss && (actual_loss == RoundTripResult::SIGNIFICANT_LOSS));

        if (mismatch) {
            std::cout << "Error found for <" << type_name<RepT>() << ">(" << unit_label(UnitT{})
                      << ") -> <" << type_name<DestRepT>() << ">(" << unit_label(DestUnitT{})
                      << ")! " << std::endl
                      << "Initial value: " << print_to_string(value) << std::endl
                      << "Destination:   " << print_to_string(destination) << std::endl
                      << "Round trip:    " << print_to_string(round_trip) << std::endl
                      << "Expect loss: " << (expect_loss ? "true" : "false") << std::endl
                      << "     (trunc: " << (expect_trunc ? "true" : "false") << std::endl
                      << "     (overf: " << (expect_overflow ? "true" : "false") << ")" << std::endl
                      << "Actual loss: " << print(actual_loss) << std::endl
                      << "Extra comments: " << loss_check.comment << std::endl;
            std::terminate();
        }
    }
};

template <typename U, typename R>
struct NoOpTestImpl {
    static void test(const Quantity<U, R> &) {}
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::INTEGRAL_TO_INTEGRAL>
    : NominalTestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::INTEGRAL_TO_INTEGRAL> {};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::INTEGRAL_TO_FLOAT>
    : NominalTestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::INTEGRAL_TO_FLOAT> {};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::FLOAT_TO_FLOAT>
    : NominalTestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::FLOAT_TO_FLOAT> {};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::FLOAT_TO_INTEGRAL>
    : NominalTestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::FLOAT_TO_INTEGRAL> {};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::TRIVIAL>
    : NoOpTestImpl<UnitT, RepT> {};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestCategory::IMPOSSIBLE>
    : NoOpTestImpl<UnitT, RepT> {};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBody : TestBodyImpl<RepT,
                               UnitT,
                               DestRepT,
                               DestUnitT,
                               categorize_testing_scenario<RepT, UnitT, DestRepT, DestUnitT>()> {};

int main() {
    GeneratorFor<Reps> generators{9876543210u};
    constexpr auto for_each_params = ForEach<CartesianProduct<std::tuple, Reps, Units>>{};
    auto print_if_equals = 1u;
    for (auto i = 0u; i < 100'000u; ++i) {
        if (i == print_if_equals) {
            std::cout << "Iteration: " << i << std::endl;
            if (print_if_equals > 1'000u) {
                print_if_equals += 10'000u;
            } else {
                print_if_equals *= 10u;
            }
        }
        for_each_params([&](auto source_params) {
            using RepT = decltype(type_of(std::get<0>(source_params)));
            using UnitT = decltype(type_of(std::get<1>(source_params)));

            const auto value = make_quantity<UnitT>(generators.template next_value<RepT>());

            for_each_params([&](auto dest_params) {
                using DestRepT = decltype(type_of(std::get<0>(dest_params)));
                using DestUnitT = decltype(type_of(std::get<1>(dest_params)));

                ::au::detail::TestBody<RepT, UnitT, DestRepT, DestUnitT>::test(value);
            });
        });
    }

    return 0;
}

}  // namespace detail
}  // namespace au

int main(int, char **) { return ::au::detail::main(); }
