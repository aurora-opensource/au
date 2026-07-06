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

// Deterministic copy-count gate for the materialization APIs.
//
// Heap-backed reps (the motivating example is `Eigen::VectorXd`) must not be silently deep-copied
// as they flow through `make_quantity`, the `QuantityMaker`, the `Quantity` constructors, and the
// `.as()` / `.in()` conversion chain.  Timing benchmarks can hide a stray copy behind
// memory-bandwidth noise; exact copy counts cannot.  So here we use an *instrumented* rep that
// counts every copy, move, and "evaluation" (materialization of a lazy expression rep), and assert
// the theoretical minimum for each path.  Each `Tracked` copy is O(n) (it duplicates a
// `std::vector<double>`); each move is O(1); so "copies" is the number that actually matters.

#include <utility>
#include <vector>

#include "au/au.hh"
#include "au/compatibility/eigen.hh"  // For the unit-aware `eval()` free function.
#include "au/testing.hh"
#include "gtest/gtest.h"

namespace au {
namespace {

using ::testing::Eq;

struct Meters : UnitImpl<Length> {};
constexpr auto meters = QuantityMaker<Meters>{};

struct Feet : decltype(Meters{} * mag<381>() / mag<1250>()) {};
constexpr auto feet = QuantityMaker<Feet>{};

// Global tallies.  We reset them immediately before each measured operation.
struct Counts {
    int copies = 0;  // copy-constructions + copy-assignments of `Tracked`
    int moves = 0;   // move-constructions + move-assignments of `Tracked`
    int evals = 0;   // materializations of a lazy expression rep into a concrete `Tracked`
};
Counts &counts() {
    static Counts c;
    return c;
}
void reset_counts() { counts() = Counts{}; }

// A concrete, heap-backed rep that instruments every copy and move.
//
// `value_type = double` lets Au deduce `RealPart<Tracked> == double`, exactly as it does for Eigen
// vectors, so unit-conversion scale factors get applied in `double` (this is what makes the
// magnitude machinery work for a vector-like rep).
class Tracked {
 public:
    using value_type = double;

    Tracked() = default;
    explicit Tracked(std::vector<double> data) : data_(std::move(data)) {}

    Tracked(const Tracked &other) : data_(other.data_) { ++counts().copies; }
    Tracked(Tracked &&other) noexcept : data_(std::move(other.data_)) { ++counts().moves; }
    Tracked &operator=(const Tracked &other) {
        data_ = other.data_;
        ++counts().copies;
        return *this;
    }
    Tracked &operator=(Tracked &&other) noexcept {
        data_ = std::move(other.data_);
        ++counts().moves;
        return *this;
    }
    ~Tracked() = default;

    // Eager scalar scaling.  This is the "one conversion pass" for a unit conversion: it produces a
    // brand-new `Tracked` from a fresh computation, so it is NOT a copy of any existing `Tracked`.
    Tracked operator*(double s) const {
        std::vector<double> out = data_;
        for (auto &x : out) {
            x *= s;
        }
        return Tracked{std::move(out)};
    }

    // Element-wise add/subtract.  These exist because instantiating `Quantity<Meters, Tracked>`
    // requires `Tracked + Tracked` and `Tracked - Tracked` to form valid types (the same
    // requirement Au's arithmetic operators place on any rep, met by Eigen out of the box).  Each
    // builds a fresh `Tracked`, so neither is a copy of an existing one.
    Tracked operator+(const Tracked &other) const {
        std::vector<double> out = data_;
        for (std::size_t i = 0; i < out.size(); ++i) {
            out[i] += other.data_[i];
        }
        return Tracked{std::move(out)};
    }
    Tracked operator-(const Tracked &other) const {
        std::vector<double> out = data_;
        for (std::size_t i = 0; i < out.size(); ++i) {
            out[i] -= other.data_[i];
        }
        return Tracked{std::move(out)};
    }

    // Mirrors Eigen's `.eval()`: returns a materialized copy.  This is the single inherent copy of
    // `eval(q)`.
    Tracked eval() const { return *this; }

    const std::vector<double> &data() const { return data_; }

 private:
    std::vector<double> data_;
};

// A stand-in for a lazy Eigen *expression* rep: it refers to a `Tracked` and materializes into a
// concrete `Tracked` on demand.  The materialization is counted as one "eval", and --- crucially
// --- it builds the result straight from the underlying vector, so it does NOT invoke `Tracked`'s
// copy constructor.  This models "the expression is evaluated exactly once, with zero extra copies
// of the concrete rep".
class TrackedExpr {
 public:
    using value_type = double;

    explicit TrackedExpr(const Tracked &src) : src_(&src) {}

    operator Tracked() const {
        ++counts().evals;
        return Tracked{src_->data()};
    }

    // Present only so that `Quantity<Meters, TrackedExpr>` instantiates (its arithmetic operators
    // require `TrackedExpr +/- TrackedExpr` to name a type, just as a real Eigen expression does).
    // The bodies are never exercised by these tests.
    Tracked operator+(const TrackedExpr &) const { return Tracked{src_->data()}; }
    Tracked operator-(const TrackedExpr &) const { return Tracked{src_->data()}; }

 private:
    const Tracked *src_;
};

Tracked make_tracked(std::size_t n) {
    std::vector<double> data(n, 1.0);
    return Tracked{std::move(data)};
}

// (1a) `meters(v)` from an lvalue must copy exactly once (the input is not ours to steal).
TEST(MaterializationCopyCount, MakerFromLvalueCopiesExactlyOnce) {
    Tracked v = make_tracked(8);

    reset_counts();
    auto q = meters(v);
    EXPECT_THAT(counts().copies, Eq(1));

    // Keep `q` alive / used.
    EXPECT_THAT(q.data_in(meters).data().size(), Eq(std::size_t{8}));
}

// (1b) `meters(rvalue)` must never copy: an rvalue may be moved from freely.
TEST(MaterializationCopyCount, MakerFromRvalueNeverCopies) {
    Tracked v = make_tracked(8);

    reset_counts();
    auto q = meters(std::move(v));
    EXPECT_THAT(counts().copies, Eq(0));

    EXPECT_THAT(q.data_in(meters).data().size(), Eq(std::size_t{8}));
}

// (1c) `meters(prvalue)` (a pure temporary) must never copy either.
TEST(MaterializationCopyCount, MakerFromPrvalueNeverCopies) {
    reset_counts();
    auto q = meters(make_tracked(8));
    EXPECT_THAT(counts().copies, Eq(0));

    EXPECT_THAT(q.data_in(meters).data().size(), Eq(std::size_t{8}));
}

// (2) `eval(q)` on an lvalue quantity with a concrete rep: exactly one copy (the inherent one from
// `.eval()`); everything downstream (`make_quantity` and the constructor) must move.
TEST(MaterializationCopyCount, EvalCopiesExactlyOnce) {
    auto q = meters(make_tracked(8));

    reset_counts();
    auto r = eval(q);
    EXPECT_THAT(counts().copies, Eq(1));

    EXPECT_THAT(r.data_in(meters).data().size(), Eq(std::size_t{8}));
}

// (3) Converting-constructor from an expression-rep quantity to the same unit with a concrete rep:
// zero copies of the concrete rep, and the expression evaluated exactly once.
TEST(MaterializationCopyCount, ConvertingCtorFromExpressionRepZeroCopiesOneEval) {
    Tracked src = make_tracked(8);
    auto q_expr = meters(TrackedExpr{src});

    reset_counts();
    Quantity<Meters, Tracked> r = q_expr;
    EXPECT_THAT(counts().copies, Eq(0));
    EXPECT_THAT(counts().evals, Eq(1));

    EXPECT_THAT(r.data_in(meters).data().size(), Eq(std::size_t{8}));
}

// (4a) A genuine unit conversion on a concrete rep: exactly one conversion pass (the scalar
// multiply, which builds a fresh vector), and zero copies of any existing `Tracked`.
TEST(MaterializationCopyCount, UnitConversionZeroCopies) {
    auto q = meters(make_tracked(8));

    reset_counts();
    auto in_feet = q.as<Tracked>(feet);
    EXPECT_THAT(counts().copies, Eq(0));

    EXPECT_THAT(in_feet.data_in(feet).data().size(), Eq(std::size_t{8}));
}

// (4b) A same-unit rep "conversion" (`.as<Rep>(same_unit)`) is an identity magnitude.  Here the one
// conversion pass IS a copy: the result gets its own vector, duplicated from the (const) source
// member --- there is nothing to move from --- so exactly one copy is the true minimum.
TEST(MaterializationCopyCount, SameUnitAsCopiesExactlyOnce) {
    auto q = meters(make_tracked(8));

    reset_counts();
    auto same = q.as<Tracked>(meters);
    EXPECT_THAT(counts().copies, Eq(1));

    EXPECT_THAT(same.data_in(meters).data().size(), Eq(std::size_t{8}));
}

}  // namespace
}  // namespace au
