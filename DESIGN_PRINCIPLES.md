DESIGN PRINCIPLES
=================

**Purpose:** This document outlines the foundational engineering principles and constraints that guide the development of LibCMPC.
**Current State:** The framework rigorously adheres to strict C89 standards, maintains zero external dependencies for its core rendering/layout, uses explicit memory ownership, and enforces high test coverage.
**Upcoming Features:** These principles are actively being applied to the implementation of the upcoming native design systems (**Material 3, Cupertino / HIG, and Fluent 2**), ensuring they are delivered with the same performance, zero-dependency philosophy, and architectural purity as the core engine.

- **Strict C89 ABI**: No C99/C11 features; portable across compilers (MSVC, GCC, Clang, Emscripten).
- **Zero Dependencies**: The core avoids linking external libraries, implementing layout, routing, color science (HCT), rendering queues, and math natively.
- **Explicit Error Codes**: All public APIs return `int` with `CMP_OK` or a specific failure code.
- **Contract-First**: `cmp_api_*` headers define the ABI; implementations can change without breaking the contract.
- **Explicit Ownership**: Handle-based objects with reference counting; widgets are POD structs with inspectable state.
- **Allocator Injection**: Callers can supply allocators; core avoids hidden global state.
- **Deterministic Core**: Math/layout/animation logic is platform-agnostic; backends isolate OS dependencies.   
- **Feature Gating**: Backends and optional integrations are controlled via build flags and runtime availability checks.
- **Ecosystem Integration**: Designed to work seamlessly with sibling C89 libraries (`c-abstract-http`, `c-orm`, `cdd-c`, `c-fs`) for a complete full-stack application experience, including networking, ORM persistence, and OpenAPI GUI generation.
- **Graceful Degradation**: Unsupported backend features return `CMP_ERR_UNSUPPORTED`; core provides small fallback decoders for basic media formats.
- **Test + Doc Coverage**: Phase-scoped tests and Doxygen annotations keep behavior and APIs verifiable.        
- **Visual Verification**: CI pipelines use `cmp_docgen` to intercept rendering calls and output interactive SVG-based documentation to ensure components behave correctly without requiring pixel-perfect headless browser automation.
