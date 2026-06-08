// Copyright 2026 Aurora Operations, Inc.
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

#include <type_traits>

#include "au/magnitude.hh"
#include "au/stdx/type_traits.hh"

namespace au {

template <typename R>
class View;

template <typename R>
constexpr View<R> make_view(R &ref);

// A non-owning mutable view of a value.
//
// View<R> wraps a pointer to R, providing reference-like semantics:
// - Reading: implicit conversion to R
// - Writing: assignment writes through
// - Arithmetic: operations dereference and return values (not views)
// - Element access: returns View of the element type (preserves mutability)
//
// This enables `Quantity<U, View<R>>` to act as a mutable view of a Quantity's rep.
template <typename R>
class View {
 public:
    constexpr explicit View(R &ref) : ptr_{&ref} {}

    constexpr View(const View &) = default;

    // Reading: implicit conversion to underlying type.
    constexpr operator R() const { return *ptr_; }

    // Writing: assign through to the underlying value.
    constexpr View &operator=(const R &value) {
        *ptr_ = value;
        return *this;
    }

    // Copy assignment writes through rather than rebinding.
    constexpr View &operator=(const View &other) {
        *ptr_ = *other.ptr_;
        return *this;
    }

    // Element access: always return View to preserve mutability through the pointer.
    template <typename I>
    constexpr auto operator[](I i) const {
        return make_view((*ptr_)[i]);
    }

    template <typename... Is>
    constexpr auto operator()(Is... is) const {
        return make_view((*ptr_)(is...));
    }

    // Arithmetic: dereference and return values.
    friend constexpr R operator+(View a, View b) { return *a.ptr_ + *b.ptr_; }
    friend constexpr R operator+(View a, const R &b) { return *a.ptr_ + b; }
    friend constexpr R operator+(const R &a, View b) { return a + *b.ptr_; }

    friend constexpr R operator-(View a, View b) { return *a.ptr_ - *b.ptr_; }
    friend constexpr R operator-(View a, const R &b) { return *a.ptr_ - b; }
    friend constexpr R operator-(const R &a, View b) { return a - *b.ptr_; }

    // Scalar multiplication/division.
    template <typename T>
    friend constexpr auto operator*(View a, T s) -> decltype(std::declval<R>() * s) {
        return *a.ptr_ * s;
    }
    template <typename T>
    friend constexpr auto operator*(T s, View a) -> decltype(s * std::declval<R>()) {
        return s * *a.ptr_;
    }
    template <typename T>
    friend constexpr auto operator/(View a, T s) -> decltype(std::declval<R>() / s) {
        return *a.ptr_ / s;
    }
    template <typename T>
    friend constexpr auto operator/(T s, View a) -> decltype(s / std::declval<R>()) {
        return s / *a.ptr_;
    }

    // Compound assignment.
    constexpr View &operator+=(const R &other) {
        *ptr_ += other;
        return *this;
    }
    constexpr View &operator-=(const R &other) {
        *ptr_ -= other;
        return *this;
    }
    template <typename T>
    constexpr View &operator*=(T s) {
        *ptr_ *= s;
        return *this;
    }
    template <typename T>
    constexpr View &operator/=(T s) {
        *ptr_ /= s;
        return *this;
    }

 private:
    R *ptr_;
};

// Type trait to detect View types.
template <typename T>
struct IsView : std::false_type {};
template <typename R>
struct IsView<View<R>> : std::true_type {};

namespace detail {

// Get underlying type, stripping View wrapper if present.
template <typename T>
struct UnderlyingTypeImpl : stdx::type_identity<T> {};
template <typename R>
struct UnderlyingTypeImpl<View<R>> : stdx::type_identity<R> {};
template <typename T>
using UnderlyingType = typename UnderlyingTypeImpl<T>::type;

// RealPart for View delegates to the underlying type (legacy fallback path).
template <typename R>
struct RealPartImpl<View<R>> : RealPartImpl<R> {};

}  // namespace detail

// ScalarOfTrait for View delegates to the underlying type.
template <typename R>
struct ScalarOfTrait<View<R>, std::enable_if_t<
    stdx::experimental::is_detected<ScalarOf, R>::value>> : ScalarOfTrait<R> {};

// Helper to create a view of a value.
template <typename R>
constexpr View<R> make_view(R &ref) {
    return View<R>{ref};
}

}  // namespace au
