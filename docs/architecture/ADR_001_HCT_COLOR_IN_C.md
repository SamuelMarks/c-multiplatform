# ADR 001: Native C Implementation of HCT Color Space

## Status
Accepted

## Context
Google's Material Design 3 relies heavily on dynamic color generation derived from a single seed color. This system uses the HCT (Hue, Chroma, Tone) color space, which is based on CAM16 and CIELAB, to ensure perceptually accurate contrast ratios and color matching across a UI.
For `c-multiplatform`, we need this functionality to build the Material 3 Catalog demo and support M3 theming natively. The official Google implementation is `material-color-utilities`, which offers C++, Dart, Java, and TypeScript versions, but no strict C89 version.

We have two options:
1. **Wrap the C++ library:** Compile the C++ implementation and expose a C API via `extern "C"`.
2. **Native C89 implementation:** Port the essential HCT and CAM16 algorithms into pure C89.

## Decision
We will implement the HCT color space natively in **C89**. 

## Rationale
1. **Compiler Compatibility & Binary Bloat:** Our target environments include very old compilers (MSVC 2005) and highly constrained embedded systems. Including a C++ standard library dependency drastically increases binary size and complicates cross-compilation (e.g., using specific C++ runtimes for MSVC vs MinGW).
2. **Performance & Memory Control:** A native C implementation allows us to route all temporary mathematical arrays and allocations through our custom `cmp_arena_t` and `cmp_pool_t` allocators, guaranteeing zero leaks and avoiding C++ `std::vector` or `std::string` heap allocations.
3. **Strict C89 Compliance Mandate:** The repository's architecture strictly dictates ISO C90 (C89) compliance without C++ or C99 features for the core engine.
4. **Focused Scope:** We only need the core color conversion (sRGB <-> HCT), tonal palette generation, and dynamic scheme mappings. We do not need the entirety of the `material-color-utilities` feature set (like image quantization or complex JSON deserialization).

## Consequences
- **Effort:** Porting CAM16 and HCT math from C++ to C89 requires significant effort, particularly translating complex floating-point math and matrix multiplications without standard library helpers.
- **Testing:** We must ensure sub-pixel accuracy compared to the official C++ implementation to avoid perceptual discrepancies. A dedicated test suite checking delta-E < 1.0 against known color vectors will be required.
