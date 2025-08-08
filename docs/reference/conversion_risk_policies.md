# Conversion Risk Policies

A conversion risk policy is a [monovalue type](./detail/monovalue_types.md) that indicates how
a unit conversion function should handle [conversion
risks](../discussion/concepts/conversion_risks.md).  Au tracks two kinds of conversion risk:
overflow, and truncation.  Each conversion risk policy will either enable or disable checking for
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
