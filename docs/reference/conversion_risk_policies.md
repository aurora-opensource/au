# Conversion Risk Policies

A conversion risk policy is a [monovalue type](./detail/monovalue_types.md) that indicates how
a unit conversion function should handle [conversion
risks](../discussion/concepts/conversion_risks.md).  Au tracks two kinds of conversion risk:
overflow and truncation.  Each conversion risk policy will either enable or disable checking for
each of these risks.

## Conversion Risk Sets

A _conversion risk set_ represents a set of risks.  It may be empty, refer to an individual risk, or
refer to multiple risks.  The only supported way to form a risk set is to use a built-in risk set
constant, or else to combine such constants using the bitwise-or operator (`|`).  Here are the risk
set constants:

- `OVERFLOW_RISK`
- `TRUNCATION_RISK`
- `ALL_RISKS`
    - Currently, this is equivalent to `OVERFLOW_RISK | TRUNCATION_RISK`.  However, if we later
      discover new conversion risks to guard against, then this constant would change to include
      them too.

## Forming Policies From Sets

To turn a conversion risk _set_, `RISK_SET`, into a _conversion risk policy_, use one of two
functions:

- `ignore(RISK_SET)`
    - This produces a policy that _ignores_ all the risks in `RISK_SET`, and _checks for_ all other
      risks.
- `check_for(RISK_SET)`
    - This produces a policy that _checks for_ all the risks in `RISK_SET`, and _ignores_ all other
      risks.

In practice, `ignore()` is used very commonly, and `check_for()` is used very rarely, mostly
internally to the library.

## Modifying Existing Policies

Sometimes you have an existing policy and need to adjust which risks it checks for.  For example,
suppose you wrote a conversion function that handled overflow via clamping, and also supported
arbitrary policy parameter inputs.  You would want to remove `OVERFLOW_RISK` from the set of checks
in that policy, because you'll be handling overflow manually.

Two member functions support modifying an existing policy, `policy`, by adding or removing risks
from it:

- `policy.but_ignoring(RISK_SET)`
    - Returns a new policy that no longer checks for any risks in `RISK_SET`, but otherwise checks
      for the same risks as `policy`.
- `policy.but_also_checking_for(RISK_SET)`
    - Returns a new policy that additionally checks for all risks in `RISK_SET`, on top of whatever
      risks `policy` already checks.

??? example "Example: clamped conversion"
    Suppose we want a "clamped" unit conversion: one that automatically clamps out-of-range values
    to the nearest representable value, instead of overflowing.  We want to let the user pass their
    own policy to control truncation checking, but whatever that policy is, we need to tweak it and
    disable overflow checking since we're handling overflow ourselves.  It might look something like
    this[^1]:

    [^1]: For clarity, note that this function is for illustration purposes only.  It would not
    handle cases where **all** of the following are true: (a) the initial rep is an integral type,
    (b) the final rep is _also_ integral, _and_ &#40;c) the conversion factor is rational, but neither
    integer nor inverse-integer (such as between feet and meters).  In these cases, the clamped
    answer wouldn't be at the limits of the type, because of the subsequent division by the
    denominator.  However, note that after [#453] is resolved, this caveat goes away, and the
    function would work correctly in all cases.

    ```cpp
    template <class T, class UnitSlot, class U, class R, class Policy = decltype(check_for(ALL_RISKS))>
    constexpr auto clamped_convert_to(UnitSlot unit, Quantity<U, R> q, Policy policy = {}) {
        using ResultLimits = std::numeric_limits<Quantity<AssociatedUnitT<UnitSlot>, T>>;
        return will_conversion_overflow<T>(q, unit)
                   ? (q > ZERO ? ResultLimits::max() : ResultLimits::lowest())
                   : q.template as<T>(unit, policy.but_ignoring(OVERFLOW_RISK))
        //                                  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    }
    ```

    Consider this example callsite:

    ```
    auto length = clamped_convert_to<int>(inches, yards(3.14), ignore(TRUNCATION_RISK));
    ```

    The user opted out of truncation risk checking at the _callsite_, because they wanted to
    represent the `double` input in `int`, which requires truncation.  Additionally, _inside_ the
    function, we further opt out of overflow risk, because we have already handled any inputs that
    would actually overflow.  The member functions that modify existing policies are the only way to
    robustly support this kind of use case.

[#453]: https://github.com/aurora-opensource/au/issues/453
