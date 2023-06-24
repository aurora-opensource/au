# Prefix

A "prefix" scales a [unit](./unit.md) by some [magnitude](./magnitude.md), and prepends a _prefix
symbol_ to the unit's label.

## Applying to unit types

To apply a prefix to a unit _type_, pass that unit as a _template parameter_ to the prefix.  This
results in a new unit with the following properties:

1. The **label** for the new unit is the label for the input unit, with the **prefix symbol**
   prepended.

2. The **magnitude** of the new unit is the magnitude of the input unit, scaled by the **scaling
   factor**.

!!! example "Example: `Centi` and `Meters`"
    `Centi` is a prefix whose prefix symbol is `c`, and whose scaling factor is $1/100$.

    `Meters` is a unit whose symbol is `m`.

    Therefore, `Centi<Meters>` is a unit whose symbol is `cm`, and whose magnitude is
    $1/100\,\text{m}$.

## Applying to instances: the "prefix applier"

Au uses many kinds of instances, not just types.  These include [QuantityMaker](./quantity.md),
[QuantityPointMaker](./quantity_point.md), `SingularNameFor`, and even [instances of unit
types](./detail/monovalue_types.md).

Instances are naturally used as _function parameters_, not template parameters.  Therefore, for each
prefix, we provide a "prefix applier" which can be called as a function.  For any of the above kinds
of inputs, the prefix applier returns the same kind, but in the prefixed unit.

Let's return to the example of `Centi` and `Meters`.  The prefix applier for `Centi` is spelled
`centi`: note that we use snake_case, rather than CamelCase.  Here are the various ways a prefix
applier can be used.

| Kind of input | Example (meters) | Application | Example use |
|---------------|------------------|-------------|---------------|
| Unit instance | `Meters{}` | `centi(Meters{})` | `length.as(Centi<Meters>{})` |
| `QuantityMaker` | `meters` | `centi(meters)` | `centi(meters)(170)` |
| `QuantityPointMaker` | `meters_pt` | `centi(meters_pt)` | `centi(meters_pt)(1.5)` |
| `SingularNameFor` | `meter` | `centi(meter)` | `curvature.in(radians / centi(meter))` |

Note again that every output here is the same kind of thing as the input.  So, `centi(meters_pt)` is
a `QuantityPointMaker`, and `centi(meters_pt)(1.5)` creates a `QuantityPoint` of $1.5\,\text{cm}$.

## List of supported prefixes

We support every [SI prefix](https://www.nist.gov/pml/owm/metric-si-prefixes) and [binary
prefix](https://en.wikipedia.org/wiki/Binary_prefix).

### SI prefixes

| Prefix | Prefix applier | Prefix symbol | Scaling factor |
|--------|----------------|---------------|----------------|
| `Quetta` | `quetta` | `Q` | $10^{30}$ |
| `Ronna` | `ronna` | `R` | $10^{27}$ |
| `Yotta` | `yotta` | `Y` | $10^{24}$ |
| `Zetta` | `zetta` | `Z` | $10^{21}$ |
| `Exa` | `exa` | `E` | $10^{18}$ |
| `Peta` | `peta` | `P` | $10^{15}$ |
| `Tera` | `tera` | `T` | $10^{12}$ |
| `Giga` | `giga` | `G` | $10^{9}$ |
| `Mega` | `mega` | `M` | $10^{6}$ |
| `Kilo` | `kilo` | `k` | $10^{3}$ |
| `Hecto` | `hecto` | `h` | $10^{2}$ |
| `Deka` | `deka` | `da` | $10^{1}$ |
| `Deci` | `deci` | `d` | $10^{-1}$ |
| `Centi` | `centi` | `c` | $10^{-2}$ |
| `Milli` | `milli` | `m` | $10^{-3}$ |
| `Micro` | `micro` | `u` | $10^{-6}$ |
| `Nano` | `nano` | `n` | $10^{-9}$ |
| `Pico` | `pico` | `p` | $10^{-12}$ |
| `Femto` | `femto` | `f` | $10^{-15}$ |
| `Atto` | `atto` | `a` | $10^{-18}$ |
| `Zepto` | `zepto` | `z` | $10^{-21}$ |
| `Yocto` | `yocto` | `y` | $10^{-24}$ |
| `Ronto` | `ronto` | `r` | $10^{-27}$ |
| `Quecto` | `quecto` | `q` | $10^{-30}$ |

### Binary prefixes

| Prefix | Prefix applier | Prefix symbol | Scaling factor |
|--------|----------------|---------------|----------------|
| `Yobi` | `yobi` | `Yi` | $2^{80} (\approx 10^{24})$ |
| `Zebi` | `zebi` | `Zi` | $2^{70} (\approx 10^{21})$ |
| `Exbi` | `exbi` | `Ei` | $2^{60} (\approx 10^{18})$ |
| `Pebi` | `pebi` | `Pi` | $2^{50} (\approx 10^{15})$ |
| `Tebi` | `tebi` | `Ti` | $2^{40} (\approx 10^{12})$ |
| `Gibi` | `gibi` | `Gi` | $2^{30} (\approx 10^{9})$ |
| `Mebi` | `mebi` | `Mi` | $2^{20} (\approx 10^{6})$ |
| `Kibi` | `kibi` | `Ki` | $2^{10} (\approx 10^{3})$ |
