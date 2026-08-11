!!! warning "Lifetime risk"
    If the result of this operation is an Eigen expression template, it refers to its operands
    instead of owning its own copy of the data.  The result is valid only as long as its operands
    are alive.  If it needs to outlive them, materialize it with [`eval()`](./eigen.md#eval).  See
    our [Eigen safety guide](../discussion/concepts/eigen_safety.md) to learn more.
