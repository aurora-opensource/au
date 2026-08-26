# Au Idioms

This section covers common idioms and usage patterns you'll encounter in using Au.  These concepts
can make you more effective with this library, but may not help with other units libraries.

- **[Abbreviated quantity construction](./abbreviated-quantity-construction.md)**.  Most C++ units
  libraries provide user-defined literals (UDLs) to make quantities in a concise, readable way ---
  think `3.5_m` instead of `meters(3.5)`.  We'll explain why Au initially rejected this approach,
  what we have provided instead, and finally, the innovative and surprising solution that
  supercharged UDLs and fixed their most glaring problems for quantities.

- **[Namespaces and includes](./namespaces-and-includes.md)**.  Which headers to include, and how to
  bring Au's names into scope --- including why the answer differs between implementation files and
  headers.

- **[Unit slots](./unit-slots.md)**.  Many Au APIs have parameters that accept an explicitly named
  unit.  We call these paremeters "unit slots".  This page will explain what kinds of things can go
  in that slot, and which ones to prefer in different situations.
