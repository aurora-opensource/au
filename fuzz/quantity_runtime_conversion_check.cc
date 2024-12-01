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

#include "au/testing.hh"
#include "au/units/inches.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/yards.hh"
#include "fuzz/quantity_runtime_conversion_checkers.hh"
#include "gtest/gtest.h"

namespace au {

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

TEST(RuntimeConversionCheckers, Fuzz) {
    GeneratorFor<IntTypes> generators{9876543210u};
    constexpr auto for_each_params = ForEach<CartesianProduct<std::tuple, IntTypes, Units>>{};
    for (auto i = 0u; i < 100'000u; ++i) {
        for_each_params([&](auto source_params) {
            using IntT = decltype(type_of(std::get<0>(source_params)));
            using UnitT = decltype(type_of(std::get<1>(source_params)));

            const auto value = make_quantity<UnitT>(generators.template next_value<IntT>());

            for_each_params([&](auto dest_params) {
                using DestIntT = decltype(type_of(std::get<0>(dest_params)));
                using DestUnitT = decltype(type_of(std::get<1>(dest_params)));

                if (source_params == dest_params) {
                    return;
                }

                const bool expect_loss = is_conversion_lossy<DestIntT>(value, DestUnitT{});

                const auto round_trip = value.template coerce_as<DestIntT>(DestUnitT{})
                                            .template coerce_as<IntT>(UnitT{});
                const bool actual_loss = (value != round_trip);

                if (expect_loss != actual_loss) {
                    std::cout << "Error found for <" << typeid(IntT).name() << ">("
                              << unit_label(UnitT{}) << ") -> <" << typeid(DestIntT).name() << ">("
                              << unit_label(DestUnitT{}) << ")! "
                              << "Value: " << value << " Round trip: " << round_trip << std::endl;
                    std::terminate();
                }
            });
        });
    }
}

}  // namespace au
