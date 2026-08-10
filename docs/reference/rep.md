# Representation types ("Rep")

The _rep_ of a `Quantity<U, R>` is its second template parameter, `R`: the underlying raw numeric
type which holds the wrapped value.  Most reps are arithmetic types such as `double` or `int32_t`,
but Au also supports other numeric types: `std::complex`, Eigen vectors and matrices, and other
"custom" reps.

Full documentation of the requirements for a valid rep is still in progress (see [#52]).  This page
documents the traits that Au provides for working with reps.

## `ScalarOf` / `ScalarOfTrait` {#scalar-of}

`ScalarOf<T>` is the _scalar type_ that the rep `T` is "based on".  For a scalar rep, this is simply
the rep itself.  For a "compound" rep --- a complex number, a vector, a matrix --- it's the type of
the underlying components.

The defining property of the scalar type is that it supports ordinary numeric comparisons, and has
a meaningful notion of "min" and "max" values.  Au's [conversion
risk](../discussion/concepts/conversion_risks.md) checks are built on these properties, so
`ScalarOf<T>` is what makes the overflow and truncation safety checks work for compound reps.

**Syntax:**

- `ScalarOf<T>` is an alias for `typename ScalarOfTrait<T>::type`.
- `ScalarOfTrait<T>` is the customization point: specialize it for types that Au can't handle
  automatically.

### Automatic detection

For most types, Au can detect the scalar type automatically, with no user action required.  We apply
a prioritized sequence of probes, where the first matching probe wins:

| Priority | Probe | Result | Convention served |
|----------|-------|--------|-------------------|
| 1 | `std::is_arithmetic<T>` | `T` itself | Arithmetic types |
| 2 | `T::Scalar` member | `T::Scalar` | Eigen |
| 3 | `T::value_type` member | `T::value_type` | STL containers |
| 4 | `.real()` member function | `decltype(t.real())` | `std::complex` |
| 5 | _(none matched)_ | No result; `ScalarOf<T>` does not exist | (hard compiler error) |

Every probe tests an _intrinsic_ property of `T`, so the answer is guaranteed to be the same in
every translation unit.  This makes the automatic detection safe with respect to the One Definition
Rule (ODR).

### Specializing `ScalarOfTrait`

If none of the probes match your type, you must specialize `ScalarOfTrait` for it.  For example:

```cpp
namespace au {

template <>
struct ScalarOfTrait<my::UnusualNumber> {
    using type = double;
};

}  // namespace au
```

!!! warning "Avoiding ODR violations"
    It is critical to **ensure that `ScalarOfTrait<T>` has only one implementation for every type
    `T`**.  This is called the "One Definition Rule" (ODR), and if you violate it, your program's
    behavior will be undefined.

    To avoid this problem, go through these steps in order until you find the first one that matches
    your situation.

    1. First, if the default `ScalarOfTrait` implementation works (see "Automatic detection" above),
       then just use that.  _Never_ specialize `ScalarOfTrait<T>` unless it gives a hard compiler
       error.
    2. If it _does_ give an error, that means you need the specialization.  `ScalarOfTrait` is
       highly greppable, so grep your codebase to see whether there's an existing specialization for
       your specific type.  If there is, then include that existing file.
    3. If neither of the above worked, then create the specialization, and put it in a file that
       everyone can include.

### Timeline

To provide a soft landing, `ScalarOf` support rolls out over two releases.

- 0.6.0 will be the first release that _provides_ `ScalarOfTrait`, but it will not yet be _required_
  (Au will fall back to the old machinery).

- 0.7.0 will be the first release that _requires_ `ScalarOfTrait` to exist for every rep.

The upshot is that if you have a custom rep, and it is not covered under automatic detection, then
you should specialize `ScalarOfTrait` now, to avoid breakages when 0.7.0 is released.

[#52]: https://github.com/aurora-opensource/au/issues/52
[0.6.0]: https://github.com/aurora-opensource/au/milestone/9
