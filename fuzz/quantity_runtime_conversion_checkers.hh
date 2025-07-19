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

#pragma once

#include <cxxabi.h>

#include <cstdint>
#include <random>

#include "au/abstract_operations.hh"
#include "au/stdx/type_traits.hh"
#include "au/utility/type_traits.hh"

namespace au {

template <typename T>
std::string type_name() {
    return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
}

enum class GeneratorStrategy {
    INTEGRAL,
    FLOAT,
    UNSUPPORTED,
};

template <typename T>
constexpr GeneratorStrategy get_generator_strategy() {
    if (std::is_integral<T>::value) {
        return GeneratorStrategy::INTEGRAL;
    }
    if (std::is_floating_point<T>::value) {
        return GeneratorStrategy::FLOAT;
    }
    return GeneratorStrategy::UNSUPPORTED;
}

template <typename T, GeneratorStrategy Strategy>
struct RandomValueGeneratorImpl;

template <typename T>
struct RandomValueGeneratorImpl<T, GeneratorStrategy::INTEGRAL> {
    T next_value(std::mt19937_64 &engine) {
        static std::uniform_int_distribution<std::uint64_t> uniform{};
        return static_cast<T>(uniform(engine));
    }
};

template <typename T>
struct RandomValueGeneratorImpl<T, GeneratorStrategy::FLOAT> {
    T next_value(std::mt19937_64 &engine) {
        static std::uniform_int_distribution<std::uint8_t> uniform{};
        std::uint8_t bytes[sizeof(T)];
        for (auto i = 0u; i < sizeof(T); ++i) {
            bytes[i] = uniform(engine);
        }

        T result{};
        memcpy(&result, bytes, sizeof(T));
        return result;
    }
};

template <typename T>
class RandomValueGenerator {
 public:
    RandomValueGenerator(std::uint64_t seed) : engine_{seed} {}

    T next_value() {
        return RandomValueGeneratorImpl<T, get_generator_strategy<T>()>{}.next_value(engine_);
    }

 private:
    std::mt19937_64 engine_;
};

template <typename... Ts>
class RandomValueGenerators {
 public:
    RandomValueGenerators(std::uint64_t seed) : generators_{RandomValueGenerator<Ts>{seed}...} {}

    template <typename T>
    T next_value() {
        return std::get<RandomValueGenerator<T>>(generators_).next_value();
    }

 private:
    std::tuple<RandomValueGenerator<Ts>...> generators_;
};

template <template <class...> class Tuple, typename... Packs>
struct CartesianProductImpl;
template <template <class...> class Tuple, typename... Packs>
using CartesianProduct = typename CartesianProductImpl<Tuple, Packs...>::type;
template <template <class...> class Tuple, template <class...> class Pack, typename... Ts>
struct CartesianProductImpl<Tuple, Pack<Ts...>> : stdx::type_identity<Pack<Tuple<Ts>...>> {};

template <typename... Packs>
struct FlattenImpl;
template <typename... Packs>
using Flatten = typename FlattenImpl<Packs...>::type;
template <template <class...> class Pack, typename... Ts>
struct FlattenImpl<Pack<Ts...>> : stdx::type_identity<Pack<Ts...>> {};
template <template <class...> class Pack, typename... Ts, typename... Us, typename... Packs>
struct FlattenImpl<Pack<Ts...>, Pack<Us...>, Packs...>
    : stdx::type_identity<Flatten<Pack<Ts..., Us...>, Packs...>> {};

template <typename T, typename... Packs>
struct PrependToEachImpl;
template <typename T, typename... Packs>
using PrependToEach = typename PrependToEachImpl<T, Packs...>::type;
template <template <class...> class Pack, typename T, typename... Ts>
struct PrependToEachImpl<T, Pack<Ts...>> {
    using type = Pack<detail::PrependT<Ts, T>...>;
};

template <template <class...> class Tuple,
          template <class...>
          class Pack,
          typename... Ts,
          typename... Packs>
struct CartesianProductImpl<Tuple, Pack<Ts...>, Packs...> {
    using type = Flatten<PrependToEach<Ts, CartesianProduct<Tuple, Packs...>>...>;
};

namespace detail {

template <typename OpSeq>
struct FloatingPointPrefixPartImpl;
template <typename OpSeq>
using FloatingPointPrefixPart = typename FloatingPointPrefixPartImpl<OpSeq>::type;

template <typename Op, typename... Ops>
struct FloatingPointPrefixPartImpl<OpSequenceImpl<Op, Ops...>>
    : std::conditional<stdx::conjunction<std::is_floating_point<RealPart<OpInput<Op>>>,
                                         std::is_floating_point<RealPart<OpOutput<Op>>>>::value,
                       OpSequence<Op, FloatingPointPrefixPart<OpSequence<Ops...>>>,
                       OpSequence<>> {};

template <>
struct FloatingPointPrefixPartImpl<OpSequenceImpl<>> : stdx::type_identity<OpSequence<>> {};

}  // namespace detail
}  // namespace au
