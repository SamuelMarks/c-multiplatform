DESIGN PRINCIPLES
=================

- **Strict C89 ABI**: No C99/C11 features; portable across compilers.
- **Explicit Error Codes**: All public APIs return `int` with `M3_OK` or a specific failure code.
- **Contract-First**: Headers define the ABI; implementations can change without breaking the contract.
- **No Hidden State**: Widgets are POD structs; state is explicit and inspectable.
- **Deterministic Logic**: Core logic avoids nondeterminism and platform dependencies.
- **Test Coverage**: Every behavior path is exercised via phase-scoped tests.
- **Doc Coverage**: Public APIs are fully documented for Doxygen.
- **Minimal Dependencies**: Core has no external dependencies; backends are optional.
