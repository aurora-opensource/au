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

#include <cxxabi.h>

#include <string>

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
std::string type_name() {
    return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
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

using IntTypes = List<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t>;
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

enum class TestingScenarioCategory {
    NOMINAL,
    TRIVIAL,
    IMPOSSIBLE,
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
constexpr TestingScenarioCategory categorize_testing_scenario() {
    if (std::is_same<RepT, DestRepT>::value && std::is_same<UnitT, DestUnitT>::value) {
        return TestingScenarioCategory::TRIVIAL;
    }

    using Common = std::common_type_t<RepT, DestRepT>;
    constexpr auto conversion_factor = UnitRatioT<UnitT, DestUnitT>{};

    if (is_integer(conversion_factor) &&
        (get_value_result<Common>(conversion_factor).outcome != MagRepresentationOutcome::OK)) {
        return TestingScenarioCategory::IMPOSSIBLE;
    }

    if (is_integer(mag<1>() / conversion_factor) &&
        (get_value_result<Common>(mag<1>() / conversion_factor).outcome !=
         MagRepresentationOutcome::OK)) {
        return TestingScenarioCategory::IMPOSSIBLE;
    }

    return TestingScenarioCategory::NOMINAL;
}

template <typename RepT,
          typename UnitT,
          typename DestRepT,
          typename DestUnitT,
          TestingScenarioCategory Category>
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

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestingScenarioCategory::NOMINAL> {
    static constexpr void test(const Quantity<UnitT, RepT> &value) {
        const bool expect_loss = is_conversion_lossy<DestRepT>(value, DestUnitT{});

        const auto destination = value.template coerce_as<DestRepT>(DestUnitT{});
        const auto round_trip = destination.template coerce_as<RepT>(UnitT{});
        const bool flipped = sign_flip(value, destination);
        const bool actual_loss = (value != round_trip) || flipped;

        if (expect_loss != actual_loss) {
            std::cout << "Error found for <" << type_name<RepT>() << ">(" << unit_label(UnitT{})
                      << ") -> <" << type_name<DestRepT>() << ">(" << unit_label(DestUnitT{})
                      << ")! " << std::endl
                      << "Initial value: " << value << std::endl
                      << "Round trip:    " << round_trip << std::endl
                      << "Expect loss: " << (expect_loss ? "true" : "false") << std::endl
                      << "Actual loss: " << (actual_loss ? "true" : "false")
                      << (((round_trip == value) && flipped) ? " (sign flipped)" : "") << std::endl;
            std::terminate();
        }
    }
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestingScenarioCategory::TRIVIAL> {
    static constexpr void test(const Quantity<UnitT, RepT> &) {}
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBodyImpl<RepT, UnitT, DestRepT, DestUnitT, TestingScenarioCategory::IMPOSSIBLE> {
    static constexpr void test(const Quantity<UnitT, RepT> &) {}
};

template <typename RepT, typename UnitT, typename DestRepT, typename DestUnitT>
struct TestBody : TestBodyImpl<RepT,
                               UnitT,
                               DestRepT,
                               DestUnitT,
                               categorize_testing_scenario<RepT, UnitT, DestRepT, DestUnitT>()> {};

TEST(RuntimeConversionCheckers, Fuzz) {
    GeneratorFor<IntTypes> generators{9876543210u};
    constexpr auto for_each_params = ForEach<CartesianProduct<std::tuple, IntTypes, Units>>{};
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
