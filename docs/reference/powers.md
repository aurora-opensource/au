# Powers

Several of our [monovalue types](./detail/monovalue_types.md) (such as [units](./unit.md) and
[magnitudes](./magnitude.md)) can be raised to powers. This includes negative exponents (such as the
inverse), and fractional exponents (such as roots).

When [expressed as values](./detail/monovalue_types.md#switching) (as opposed to types), we provide
the same APIs for each of them.

## General APIs

In what follows, `x` stands for an instance of the appropriate monovalue type.  For example, it
might be `Meters{}` (a unit), `mag<18>()` (a magnitude), or some other type.

### `pow<N>(x)`

Raise the input `x` to the `N`th power.

**Example signature:**

```cpp
// T is an appropriate monovalue type (a unit, a magnitude, ...).
template<std::intmax_t N, typename T>
constexpr auto pow(T);
```

**Result:** An instance of the Nth power of the type of `x`.

!!! example
    `pow<3>(mag<4>())` yields `mag<64>()`.

### `root<N>(x)`

Take the Nth root of the input.

**Example signature:**

```cpp
// T is an appropriate monovalue type (a unit, a magnitude, ...).
template<std::intmax_t N, typename T>
constexpr auto root(T);
```

**Result:** An instance of the Nth root of the type of `x`.

!!! example
    `root<2>(mag<49>())` yields `mag<7>()`.

## Helpers (`inverse`, `squared`, `cubed`, `sqrt`, `cbrt`) {#helpers}

Some powers and roots are very common.  It's useful to have shortcuts for these to make the code
more readable.  The following helpers are available to operate on an instance, `x`, of any
compatible monovalue type (a unit, a magnitude, ...):

| Helper | Result |
|--------|--------|
| `inverse(x)` | `pow<-1>(x)` |
| `squared(x)` | `pow<2>(x)` |
| `cubed(x)` | `pow<3>(x)` |
| `sqrt(x)` | `root<2>(x)` |
| `cbrt(x)` | `root<3>(x)` |

### Type-based versions (`Inverse`, `Squared`, `Cubed`, `Sqrt`, `Cbrt`) {#type-based}

We provide type-based versions of the above helpers, to make it easier to concisely form readable
type names.  Here are the following helpers as applied to a unit `U`, and the equivalent result as
expressed using more general unit power APIs.

| Helper | Result |
|--------|--------|
| `Inverse<U>` | `UnitPower<U, -1>` |
| `Squared<U>` | `UnitPower<U, 2>` |
| `Cubed<U>` | `UnitPower<U, 3>` |
| `Sqrt<U>` | `UnitPower<U, 1, 2>` |
| `Cbrt<U>` | `UnitPower<U, 1, 3>` |
