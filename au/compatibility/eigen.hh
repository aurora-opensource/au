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

#include <cstddef>
#include <type_traits>
#include <utility>

#include "au/au.hh"

namespace au {

// Force evaluation of a Quantity whose rep is an Eigen expression template.
//
// Use this to materialize lazy expressions before the operands go out of scope.
//
// We bind Eigen's `.eval()` result --- a *`const`* plain object --- to a non-`const` local, then
// move it through `make_quantity`.  Because `make_quantity` stores `std::decay_t<T>` and forwards
// its argument, this materializes the data exactly once and then moves it into place.
template <typename U, typename R>
auto eval(const Quantity<U, R> &q) {
    auto result = q.data_in(U{}).eval();
    return make_quantity<U>(std::move(result));
}

//
// Free-function forms of Eigen member functions, made unit-aware.
//
// Eigen's `.transpose()`, `.norm()`, etc. are member functions, and `Quantity` does not forward
// arbitrary member calls, so `q.norm()` does not compile.  These free functions fill that gap while
// tracking how each operation transforms the unit.
//
// LIFETIME NOTE: operations whose Eigen counterpart returns a scalar (e.g. `norm`, `sum`, `dot`,
// `trace`) are evaluated eagerly and are always safe.  Operations whose Eigen counterpart returns
// an _expression template_ -- the coefficient-wise ops (`cwiseProduct`, ...) and the view/accessor
// ops (`transpose`, `cross`, `diagonal`, `block`, `head`, ...) -- return a lazy `Quantity` that
// references its operands, exactly like `Quantity`'s arithmetic operators.  Such a result is valid
// only while its operands are alive; use `eval()` to materialize it otherwise.
//

// The Euclidean (L2) norm.  Unit-preserving.
template <typename U, typename R>
auto norm(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).norm());
}

// The squared Euclidean norm (sum of squared coefficients).  Squares the unit; avoids a `sqrt`.
template <typename U, typename R>
auto squaredNorm(const Quantity<U, R> &q) {
    return make_quantity<UnitPowerT<U, 2>>(q.data_in(U{}).squaredNorm());
}

// The sum of all coefficients.  Unit-preserving.
template <typename U, typename R>
auto sum(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).sum());
}

// The mean of all coefficients.  Unit-preserving.
template <typename U, typename R>
auto mean(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).mean());
}

// The smallest coefficient.  Unit-preserving.
template <typename U, typename R>
auto minCoeff(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).minCoeff());
}

// The largest coefficient.  Unit-preserving.
template <typename U, typename R>
auto maxCoeff(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).maxCoeff());
}

// The transpose.  Unit-preserving.  LAZY: see the lifetime note above.
template <typename U, typename R>
auto transpose(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).transpose());
}

// The dot (inner) product.  The result unit is the product of the operand units.
template <typename U1, typename R1, typename U2, typename R2>
auto dot(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b) {
    return make_quantity<UnitProductT<U1, U2>>(a.data_in(U1{}).dot(b.data_in(U2{})));
}

// `dot` overloads where one operand is a raw (dimensionless) Eigen object.  The result carries the
// unit of the `Quantity` operand.
template <typename U, typename R, typename V>
auto dot(const Quantity<U, R> &a, const V &b) {
    return make_quantity<U>(a.data_in(U{}).dot(b));
}
template <typename V, typename U, typename R>
auto dot(const V &a, const Quantity<U, R> &b) {
    return make_quantity<U>(a.dot(b.data_in(U{})));
}

// The cross product.  The result unit is the product of the operand units.  LAZY: see the lifetime
// note above.
template <typename U1, typename R1, typename U2, typename R2>
auto cross(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b) {
    return make_quantity<UnitProductT<U1, U2>>(a.data_in(U1{}).cross(b.data_in(U2{})));
}

// `cross` overloads where one operand is a raw (dimensionless) Eigen object.  The result carries
// the unit of the `Quantity` operand.  LAZY: see the lifetime note above.
template <typename U, typename R, typename V>
auto cross(const Quantity<U, R> &a, const V &b) {
    return make_quantity<U>(a.data_in(U{}).cross(b));
}
template <typename V, typename U, typename R>
auto cross(const V &a, const Quantity<U, R> &b) {
    return make_quantity<U>(a.cross(b.data_in(U{})));
}

// The normalized (unit-length) vector.
//
// The norm always carries the operand's unit, so it cancels exactly: the result is unitless *by
// definition of the operation, for all inputs*.  Unlike operations whose unit is a function of the
// operand units (`norm`, `squaredNorm`, `dot`, ...), there is no operand-dependent unit for a
// `Quantity` wrapper to track, so we return the raw Eigen vector directly.  Evaluated eagerly
// (Eigen's `.normalized()` returns a concrete vector).
template <typename U, typename R>
auto normalized(const Quantity<U, R> &q) {
    return q.data_in(U{}).normalized();
}

//
// Additional scalar-returning reductions.  All unit-preserving and evaluated eagerly.
//

// Numerically stable Euclidean norm.
template <typename U, typename R>
auto stableNorm(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).stableNorm());
}

// Euclidean norm computed with Kahan's "Blue's" algorithm (overflow/underflow safe).
template <typename U, typename R>
auto blueNorm(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).blueNorm());
}

// Euclidean norm computed via a recursive `hypot`-based scheme (overflow/underflow safe).
template <typename U, typename R>
auto hypotNorm(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).hypotNorm());
}

// The L^p norm.  `p` is a compile-time `int`; pass `Eigen::Infinity` for the max-absolute norm.
template <int p, typename U, typename R>
auto lpNorm(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).template lpNorm<p>());
}

// The trace (sum of diagonal coefficients).  Unit-preserving.
template <typename U, typename R>
auto trace(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).trace());
}

//
// Coefficient-wise binary/unary ops.  LAZY: see the lifetime note above.
//

// Coefficient-wise (Hadamard) product.  The result unit is the product of the operand units.
template <typename U1, typename R1, typename U2, typename R2>
auto cwiseProduct(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b) {
    return make_quantity<UnitProductT<U1, U2>>(a.data_in(U1{}).cwiseProduct(b.data_in(U2{})));
}
template <typename U, typename R, typename V>
auto cwiseProduct(const Quantity<U, R> &a, const V &b) {
    return make_quantity<U>(a.data_in(U{}).cwiseProduct(b));
}
template <typename V, typename U, typename R>
auto cwiseProduct(const V &a, const Quantity<U, R> &b) {
    return make_quantity<U>(a.cwiseProduct(b.data_in(U{})));
}

// Coefficient-wise quotient.  The result unit is the quotient of the operand units.
template <typename U1, typename R1, typename U2, typename R2>
auto cwiseQuotient(const Quantity<U1, R1> &a, const Quantity<U2, R2> &b) {
    return make_quantity<UnitQuotientT<U1, U2>>(a.data_in(U1{}).cwiseQuotient(b.data_in(U2{})));
}
template <typename U, typename R, typename V>
auto cwiseQuotient(const Quantity<U, R> &a, const V &b) {
    return make_quantity<U>(a.data_in(U{}).cwiseQuotient(b));
}
template <typename V, typename U, typename R>
auto cwiseQuotient(const V &a, const Quantity<U, R> &b) {
    return make_quantity<UnitInverseT<U>>(a.cwiseQuotient(b.data_in(U{})));
}

// Coefficient-wise absolute value.  Unit-preserving.
template <typename U, typename R>
auto cwiseAbs(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).cwiseAbs());
}

//
// View / accessor ops.  All unit-preserving.  LAZY: see the lifetime note above.
//

// The main diagonal.
template <typename U, typename R>
auto diagonal(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).diagonal());
}
// The `index`-th diagonal (positive for super-diagonals, negative for sub-diagonals).
template <typename U, typename R>
auto diagonal(const Quantity<U, R> &q, std::ptrdiff_t index) {
    return make_quantity<U>(q.data_in(U{}).diagonal(index));
}

// The `i`-th row.
template <typename U, typename R>
auto row(const Quantity<U, R> &q, std::ptrdiff_t i) {
    return make_quantity<U>(q.data_in(U{}).row(i));
}

// The `j`-th column.
template <typename U, typename R>
auto col(const Quantity<U, R> &q, std::ptrdiff_t j) {
    return make_quantity<U>(q.data_in(U{}).col(j));
}

// The reverse (coefficients in reverse order).
template <typename U, typename R>
auto reverse(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).reverse());
}

// The complex conjugate (a no-op for real reps).
template <typename U, typename R>
auto conjugate(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).conjugate());
}

// The first `N` (fixed-size) coefficients of a vector.
template <int N, typename U, typename R>
auto head(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).template head<N>());
}
// The first `n` (dynamic-size) coefficients of a vector.
template <typename U, typename R>
auto head(const Quantity<U, R> &q, std::ptrdiff_t n) {
    return make_quantity<U>(q.data_in(U{}).head(n));
}

// The last `N` (fixed-size) coefficients of a vector.
template <int N, typename U, typename R>
auto tail(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).template tail<N>());
}
// The last `n` (dynamic-size) coefficients of a vector.
template <typename U, typename R>
auto tail(const Quantity<U, R> &q, std::ptrdiff_t n) {
    return make_quantity<U>(q.data_in(U{}).tail(n));
}

// A fixed-size length-`N` segment of a vector starting at `start`.
template <int N, typename U, typename R>
auto segment(const Quantity<U, R> &q, std::ptrdiff_t start) {
    return make_quantity<U>(q.data_in(U{}).template segment<N>(start));
}
// A dynamic-size length-`n` segment of a vector starting at `start`.
template <typename U, typename R>
auto segment(const Quantity<U, R> &q, std::ptrdiff_t start, std::ptrdiff_t n) {
    return make_quantity<U>(q.data_in(U{}).segment(start, n));
}

// A fixed-size `Rows`x`Cols` block whose top-left corner is at (`i`, `j`).
template <int Rows, int Cols, typename U, typename R>
auto block(const Quantity<U, R> &q, std::ptrdiff_t i, std::ptrdiff_t j) {
    return make_quantity<U>(q.data_in(U{}).template block<Rows, Cols>(i, j));
}
// A dynamic-size `rows`x`cols` block whose top-left corner is at (`i`, `j`).
template <typename U, typename R>
auto block(const Quantity<U, R> &q,
           std::ptrdiff_t i,
           std::ptrdiff_t j,
           std::ptrdiff_t rows,
           std::ptrdiff_t cols) {
    return make_quantity<U>(q.data_in(U{}).block(i, j, rows, cols));
}

// Tile into a fixed-size `RowFactor`x`ColFactor` grid of copies.
template <int RowFactor, int ColFactor, typename U, typename R>
auto replicate(const Quantity<U, R> &q) {
    return make_quantity<U>(q.data_in(U{}).template replicate<RowFactor, ColFactor>());
}
// Tile into a dynamic-size `row_factor`x`col_factor` grid of copies.
template <typename U, typename R>
auto replicate(const Quantity<U, R> &q, std::ptrdiff_t row_factor, std::ptrdiff_t col_factor) {
    return make_quantity<U>(q.data_in(U{}).replicate(row_factor, col_factor));
}

//
// Ops whose result unit is a non-trivial power of the operand unit.
//

// Coefficient-wise square root.  Takes the square root of the unit.  LAZY: see the lifetime note
// above.
template <typename U, typename R>
auto cwiseSqrt(const Quantity<U, R> &q) {
    return make_quantity<UnitPowerT<U, 1, 2>>(q.data_in(U{}).cwiseSqrt());
}

// The matrix inverse.  Inverts the unit (since `A * A.inverse()` is dimensionless).  LAZY: see the
// lifetime note above.
//
// This is exact for a matrix with a single shared unit, which is all `Quantity<U, Matrix>` can
// represent today: every cell carries `U`, so every cell of the inverse carries `1 / U`.  A
// heterogeneous matrix (distinct per-row/per-col units) also has a well-defined inverse --- the
// cell units are the reciprocal of the transposed original --- but representing such matrices is
// planned future work.  Until then, this is a known (temporary) gap.
template <typename U, typename R>
auto inverse(const Quantity<U, R> &q) {
    return make_quantity<UnitInverseT<U>>(q.data_in(U{}).inverse());
}

// The product of all coefficients.  Raises the unit to the power of the coefficient count.
//
// Restricted to fixed-size operands: the result unit is `U^N` where `N` is the number of
// coefficients, which cannot be expressed in the type system when the size is only known at
// runtime.
template <typename U, typename R>
auto prod(const Quantity<U, R> &q) {
    constexpr int N = R::SizeAtCompileTime;
    static_assert(N != -1,  // i.e. not Eigen::Dynamic
                  "prod() requires a fixed-size operand: its result unit is U^(coefficient count), "
                  "which cannot be expressed when the size is only known at runtime");
    return make_quantity<UnitPowerT<U, N>>(q.data_in(U{}).prod());
}

// The determinant.  Raises the unit to the power of the matrix dimension.
//
// Restricted to fixed-size operands: the result unit is `U^N` for an `N`x`N` matrix, which cannot
// be expressed in the type system when the size is only known at runtime.
template <typename U, typename R>
auto determinant(const Quantity<U, R> &q) {
    constexpr int N = R::RowsAtCompileTime;
    static_assert(N != -1,  // i.e. not Eigen::Dynamic
                  "determinant() requires a fixed-size operand: its result unit is U^N for an NxN "
                  "matrix, which cannot be expressed when the size is only known at runtime");
    return make_quantity<UnitPowerT<U, N>>(q.data_in(U{}).determinant());
}

}  // namespace au
