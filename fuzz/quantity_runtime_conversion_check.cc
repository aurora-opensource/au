// Copyright 2024 Aurora Operations, Inc.
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
#include <string>

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

template <typename T>
T next_higher(T x, std::size_t n = 1u) {
    static_assert(std::is_floating_point<T>::value, "");
    while (n-- > 0u) {
        x = std::nextafter(x, std::numeric_limits<T>::infinity());
    }
    return x;
}

template <typename T>
T next_lower(T x, std::size_t n = 1u) {
    static_assert(std::is_floating_point<T>::value, "");
    while (n-- > 0u) {
        x = std::nextafter(x, -std::numeric_limits<T>::infinity());
    }
    return x;
}

template <typename U, typename R>
Quantity<U, R> next_higher_quantity(Quantity<U, R> q, std::size_t n = 1u) {
    return make_quantity<U>(next_higher(q.in(U{}), n));
}

template <typename U, typename R>
Quantity<U, R> next_lower_quantity(Quantity<U, R> q, std::size_t n = 1u) {
    return make_quantity<U>(next_lower(q.in(U{}), n));
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
using Units = List<Inches, Meters, Miles, Yards>;

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
    } else {
        return std::is_integral<DestRepT>::value ? TestCategory::FLOAT_TO_INTEGRAL
                                                 : TestCategory::FLOAT_TO_FLOAT;
    }

    return TestCategory::UNSUPPORTED;
}

template <typename RepT,
          typename UnitT,
          typename DestRepT,
          typename DestUnitT,
          TestCategory Category>
struct TestBodyImpl;

template <bool R1Unsigned, bool R2Unsigned>
struct SignFlipImpl;
template <>
struct SignFlipImpl<true, true> {
    template <typename U1, typename R1, typename U2, typename R2>
    static constexpr bool assess(const Quantity<U1, R1> &, const Quantity<U2, R2> &) {
        return false;
    }
};
template <>
struct SignFlipImpl<true, false> {
    template <typename U1, typename R1, typename U2, typename R2>
    static constexpr bool assess(const Quantity<U1, R1> &, const Quantity<U2, R2> &b) {
        return b < ZERO;
    }
};
template <>
struct SignFlipImpl<false, true> {
    template <typename U1, typename R1, typename U2, typename R2>
    static constexpr bool assess(const Quantity<U1, R1> &a, const Quantity<U2, R2> &) {
        return a < ZERO;
    }
};
template <>
struct SignFlipImpl<false, false> {
    template <typename U1, typename R1, typename U2, typename R2>
    static constexpr bool assess(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b) {
        return (a < ZERO) != (b < ZERO);
    }
};
template <typename U1, typename R1, typename U2, typename R2>
constexpr bool sign_flip(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b) {
    return SignFlipImpl<std::is_unsigned<R1>::value, std::is_unsigned<R2>::value>::assess(a, b);
}

struct LossCheck {
    bool is_lossy;
    std::string comment = "";
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT, TestCategory Cat>
struct LossChecker {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &,
                                    const Quantity<UnitT, RepT> &round_trip) {
        // Default implementation.
        const bool actual_loss = (value != round_trip);
        return {actual_loss};
    }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct LossChecker<RepT, UnitT, DestRepT, DestUnitT, TestCategory::INTEGRAL_TO_INTEGRAL> {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &destination,
                                    const Quantity<UnitT, RepT> &round_trip) {
        const bool flipped = sign_flip(value, destination);
        const bool actual_loss = (value != round_trip) || flipped;
        return {actual_loss, flipped ? "Sign flipped" : ""};
    }
};

// This is very lazy.  The actual epsilon depends on the _size of the conversion factor_.
//
// However, we're using a small and fixed number of conversion factors.  This is good enough to get
// us started.  Later on, we can explore the effects of more extreme conversion factors, and see
// whether they introduce any categorically new cases.
template <typename T>
struct FloatingPointRoundTripEpsilon;
template <>
struct FloatingPointRoundTripEpsilon<float> {
    // Adjust value as needed when we encounter failures that we don't think are failures.
    static constexpr float value() { return 1.0e-3f; }
};
template <>
struct FloatingPointRoundTripEpsilon<double> {
    static constexpr double value() { return 1.0e-10; }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct LossChecker<RepT, UnitT, DestRepT, DestUnitT, TestCategory::FLOAT_TO_FLOAT> {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &,
                                    const Quantity<UnitT, RepT> &round_trip) {
        const auto sign = value < ZERO ? -1 : 1;
        const auto min_ok = value * (1 - sign * FloatingPointRoundTripEpsilon<RepT>::value());
        const auto max_ok = value * (1 + sign * FloatingPointRoundTripEpsilon<RepT>::value());

        std::ostringstream oss;
        oss << "Breakdown:" << std::setprecision(std::numeric_limits<RepT>::digits10 + 1u)
            << std::endl
            << "  Initial:    " << value << std::endl
            << "  Min OK:     " << min_ok << std::endl
            << "  Round trip: " << round_trip << std::endl
            << "  Max OK:     " << max_ok << std::endl;

        const bool actual_loss = (round_trip < min_ok) || (round_trip > max_ok);
        return {actual_loss, oss.str()};
    }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct LossChecker<RepT, UnitT, DestRepT, DestUnitT, TestCategory::INTEGRAL_TO_FLOAT> {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &destination,
                                    const Quantity<UnitT, RepT> &round_trip) {
        auto one_pct = value / 100;
        if (std::is_signed<RepT>::value && value < ZERO) {
            one_pct = -one_pct;
        }
        const auto dx = one_pct + make_quantity<UnitT>(RepT{1});

        constexpr auto lowest = std::numeric_limits<Quantity<UnitT, RepT>>::lowest();
        const auto min_ok = (lowest + dx > value) ? lowest : rep_cast<RepT>(value - dx);

        constexpr auto highest = std::numeric_limits<Quantity<UnitT, RepT>>::max();
        const auto max_ok = (highest - dx < value) ? highest : rep_cast<RepT>(value + dx);

        std::ostringstream oss;
        oss << "We went through floating point; so, taking a very liberal margin.  Breakdown:"
            << std::endl
            << "  Initial:    " << value << std::endl
            << "  Min OK:     " << min_ok << std::endl
            << "  Round trip: " << round_trip << std::endl
            << "  Max OK:     " << max_ok << std::endl;

        return {
            round_trip < min_ok || round_trip > max_ok,
            oss.str(),
        };
    }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct LossChecker<RepT, UnitT, DestRepT, DestUnitT, TestCategory::FLOAT_TO_INTEGRAL> {
    static LossCheck check_for_loss(const Quantity<UnitT, RepT> &value,
                                    const Quantity<DestUnitT, DestRepT> &,
                                    const Quantity<UnitT, RepT> &round_trip) {
        if (round_trip == value) {
            return {false};
        }

        if (!std::is_same<UnitRatioT<UnitT, DestUnitT>, decltype(mag<1>())>::value) {
            if (next_higher_quantity(round_trip) == value ||
                next_lower_quantity(round_trip) == value) {
                return {false, "Within expected floating point error"};
            }
        }

        return {true};
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

    static void test(const Quantity<UnitT, RepT> &value) {
        const bool expect_loss = is_conversion_lossy<DestRepT>(value, DestUnitT{});
        const bool expect_trunc = will_conversion_truncate<DestRepT>(value, DestUnitT{});
        const bool expect_overflow = will_conversion_overflow<DestRepT>(value, DestUnitT{});

        const auto destination = value.template coerce_as<DestRepT>(DestUnitT{});
        const auto round_trip = destination.template coerce_as<RepT>(UnitT{});

        const auto loss_check = check_for_loss(value, destination, round_trip);
        const bool actual_loss = loss_check.is_lossy;

        if (expect_loss != actual_loss) {
            std::cout << "Error found for <" << type_name<RepT>() << ">(" << unit_label(UnitT{})
                      << ") -> <" << type_name<DestRepT>() << ">(" << unit_label(DestUnitT{})
                      << ")! " << std::endl
                      << "Initial value: " << print_to_string(value) << std::endl
                      << "Round trip:    " << print_to_string(round_trip) << std::endl
                      << "Expect loss: " << (expect_loss ? "true" : "false") << std::endl
                      << "     (trunc: " << (expect_trunc ? "true" : "false") << ")" << std::endl
                      << "     (overf: " << (expect_overflow ? "true" : "false") << ")" << std::endl
                      << "Actual loss: " << (actual_loss ? "true" : "false") << std::endl
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

TEST(RuntimeConversionCheckers, Fuzz) {
    GeneratorFor<Reps> generators{9876543210u};
    constexpr auto for_each_params = ForEach<CartesianProduct<std::tuple, Reps, Units>>{};
    for (auto i = 0u; i < 100'000u; ++i) {
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
}

}  // namespace detail
}  // namespace au
