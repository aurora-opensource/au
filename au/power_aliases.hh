// Copyright 2022 Aurora Operations, Inc.
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

#include <cstdint>
#include <utility>

namespace au {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic mathematical convenience functions.
//
// The reason these exist is to be able to make unit expressions easier to read in common cases.
// They also work for dimensions and magnitudes.

//
// This section works around an error:
//
//    warning: use of function template name with no prior declaration in function call with
//    explicit template arguments is a C++20 extension [-Wc++20-extensions]
//
// We work around it by providing declarations, even though those declarations are never used.
//
namespace no_prior_declaration_workaround {
struct Dummy;
}  // namespace no_prior_declaration_workaround
template <std::intmax_t N>
auto root(no_prior_declaration_workaround::Dummy);
template <std::intmax_t N>
auto pow(no_prior_declaration_workaround::Dummy);

// Make "inverse" an alias for "pow<-1>" when the latter exists (for anything).
template <typename T>
constexpr auto inverse(T x) -> decltype(pow<-1>(x)) {
    return pow<-1>(x);
}
template <typename T>
using Inverse = decltype(inverse(std::declval<T>()));

// Make "squared" an alias for "pow<2>" when the latter exists (for anything).
template <typename T>
constexpr auto squared(T x) -> decltype(pow<2>(x)) {
    return pow<2>(x);
}
template <typename T>
using Squared = decltype(squared(std::declval<T>()));

// Make "cubed" an alias for "pow<3>" when the latter exists (for anything).
template <typename T>
constexpr auto cubed(T x) -> decltype(pow<3>(x)) {
    return pow<3>(x);
}
template <typename T>
using Cubed = decltype(cubed(std::declval<T>()));

// Make "sqrt" an alias for "root<2>" when the latter exists (for anything).
template <typename T>
constexpr auto sqrt(T x) -> decltype(root<2>(x)) {
    return root<2>(x);
}
template <typename T>
using Sqrt = decltype(sqrt(std::declval<T>()));

// Make "cbrt" an alias for "root<3>" when the latter exists (for anything).
template <typename T>
constexpr auto cbrt(T x) -> decltype(root<3>(x)) {
    return root<3>(x);
}
template <typename T>
using Cbrt = decltype(cbrt(std::declval<T>()));

}  // namespace au
