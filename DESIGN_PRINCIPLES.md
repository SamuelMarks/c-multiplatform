DESIGN PRINCIPLES
=================

- **Strict C89 ABI**: No C99/C11 features; portable across compilers.
- **Explicit Error Codes**: All public APIs return `int` with `CMP_OK` or a specific failure code.
- **Contract-First**: `cmp_api_*` headers define the ABI; implementations can change without breaking the contract.
- **Explicit Ownership**: Handle-based objects with reference counting; widgets are POD structs with inspectable state.
- **Allocator Injection**: Callers can supply allocators; core avoids hidden global state.
- **Deterministic Core**: Math/layout/animation logic is platform-agnostic; backends isolate OS dependencies.
- **Feature Gating**: Backends and optional integrations are controlled via build flags and runtime availability checks.
- **Graceful Degradation**: Unsupported backend features return `CMP_ERR_UNSUPPORTED`; core provides small fallback decoders for basic media formats.
- **Test + Doc Coverage**: Phase-scoped tests and Doxygen annotations keep behavior and APIs verifiable.
- **Minimal Dependencies**: Core has no external dependencies; backends are optional and pull in platform SDKs as needed.
