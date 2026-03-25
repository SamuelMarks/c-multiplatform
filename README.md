C multiplatform (libcmp)
========================

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)

**LibCMPC** is a strict C89 (ANSI C) cross-platform application framework. Following a strategic rewrite, it provides a highly cohesive, monolithic core for building modality-agnostic GUI applications in native C. It integrates best-in-class sibling libraries for networking, file systems, and databases, while maintaining zero external dependencies for its core rendering and layout engines.

This project is architected specifically for **Context-Window Scalability**, utilizing an amalgamated header design (`cmp.h`) that makes the codebase distinctively friendly for LLM-assisted coding and maintenance.

## 🚀 Key Features

*   **Strict C89 ABI:** Compiles on virtually any C compiler (MSVC, GCC, Clang, TCC).
*   **Unified Modality Engine:** Run your application in single-threaded, multi-threaded worker-pool, or fully asynchronous (epoll/IOCP) modes without changing your business logic.
*   **Coroutines & Async/Await:** Native C coroutine support (`cmp_coroutine_t`) for complex asynchronous task orchestration.
*   **Flexbox Layout Engine:** A fully featured C implementation of Flexbox (`cmp_layout_node_t`) for responsive, dynamic user interfaces.
*   **Native UI Primitives:** Core unstyled widgets (`cmp_ui_box_create`, `cmp_ui_text_input_create`, etc.) ready to be themed via `cmp_theme_t`.
*   **Deep Ecosystem Integration:**
    *   **VFS (`c-fs`):** Transparent virtual file system mounting and asynchronous reads.
    *   **HTTP/WebSockets (`c-abstract-http`):** Native networking integration directly into the modality event loop.
    *   **ORM (`c-orm`):** Local SQLite persistence with data-binding observables (`cmp_orm_observable_t`) tied directly to UI nodes.

## 📂 Architecture Strategy

The framework is organized into a cohesive, amalgamated structure:

*   **`include/cmp.h`**: The single source of truth for all API contracts, structs, and interfaces.
*   **`src/`**: Modular implementations of the core systems (`cmp_memory`, `cmp_layout`, `cmp_ui`, `cmp_window`, `cmp_modality`).
*   **`tests/`**: Comprehensive test suites using the `greatest` framework, ensuring memory safety and behavioral correctness.
*   **`examples/`**: Standalone GUI applications demonstrating real-world usage (e.g., OAuth2 Login flows).

## 🛠️ Build Instructions

LibCMPC uses **CMake** and manages its internal ecosystem dependencies automatically via `FetchContent`.

### Compile

```bash
$ cmake -S . -B build -D CMAKE_BUILD_TYPE=Debug
$ cmake --build build
```

### Running Tests

```bash
$ cd build
$ ctest --output-on-failure
```

---

## License

Licensed under either of

- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE) or <https://www.apache.org/licenses/LICENSE-2.0>)
- MIT license ([LICENSE-MIT](LICENSE-MIT) or <https://opensource.org/licenses/MIT>)

at your option.

### Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.
