/**
 * @file PLAN_QUALITY.md
 * @brief PLAN_QUALITY.md implementation.
 */
/**
 * @file PLAN_QUALITY.md
 * @brief PLAN_QUALITY.md implementation.
 */
# Architecture Plan: Quality Assurance, Testing & Bindings

## 1. Automated Testing & Tooling (100% Coverage Mandate)
- [x] **Unit Tests:** C89 testing framework for parsing math, string views, and memory management.
- [x] **Geometry & Layout Assertions:** Tests that parse DOM/CSS, trigger a layout pass, and mathematically assert the exact `x, y, width, height` of resulting boxes (verifying responsiveness across smartwatch to TV dimensions).
- [x] **Headless E2E Automation:** A WebDriver-like testing API that runs the engine without a GPU/Window (using a dummy backend) to rapidly execute scripted user flows (e.g., `input(invalid) -> assert(error_visible) -> input(valid) -> submit -> assert(screen_changed)`).
- [x] **Headful E2E Automation:** A test harness that launches the actual OS Window, renders via OpenGL/WebGL, and injects synthetic OS-level mouse clicks and keyboard events to verify the entire graphics and input pipeline.
- [x] **Execution Paradigm Tests:** Stress tests simulating Single-Threaded, Thread-Pool, and Multi-Reactor environments.
- [x] **Memory Safety (Zero-Leak Policy):** Configure CI with Valgrind and AddressSanitizer (ASAN) to verify no memory leaks occur in the Arena/Pool allocators.
- [x] **Error Path Stress Testing:** Explicitly write tests that mock `malloc` failures and I/O failures to mathematically prove that the error percolation logic and `goto cleanup;` handlers successfully free resources and return the correct enum discriminant without crashing.
- [x] **Pre-commit Hooks:** Enforce strict git pre-commit hooks to ensure layout tests, linters, and formatting run every single time a commit is attempted.

## 2. Cross-Compiler C89 Compatibility & Headers
- [x] **Header Guards:** Carefully guard any POSIX/C99 headers (like `<stdint.h>`, `<stdbool.h>`, `<unistd.h>`) that are unavailable on older MSVC versions to prevent compilation errors.
- [x] **C++ Interop:** Every public header must be wrapped in `#ifdef __cplusplus extern "C" { #endif` and `#ifdef __cplusplus } #endif /* __cplusplus */` (maximum one time per file).
- [x] **Format Specifiers:** Abstract printf/formatting strings via macros (e.g., `#define NUM_FORMAT "%I64d"` for MSVC vs `"%lld"`/`"%ld"` for others) guarded by compiler checks.

## 3. Platform & CMake Support Matrix
- [x] **Target Environments:** MSVC 2005, MSVC 2022, MSVC 2026, MinGW, Cygwin, Clang, and GCC across Windows, macOS, and Linux.
- [x] **CMake Options:** The build system and codebase must flawlessly support toggling:
  - [x] **CRT Linkage:** Static (`/MT`, `/MTd`) and Shared (`/MD`, `/MDd`).
  - [x] **Charsets:** UNICODE and ANSI.
  - [x] **Threading:** Multi-threaded and Single-threaded.
  - [x] **Linking:** Link-Time Optimization (LTO), Static library, and Shared library targets.
  - [x] **MSVC Runtime Checks:** `/RTC1`, `/RTCs`, and `/RTCu`.

## 4. Windows & MSVC Optimizations
- [x] **No `windows.h` Bloat:** Do NOT `#include <windows.h>` anywhere to prevent binary bloat. Use specific headers (like `<winsock2.h>`) or forward-declare what you need.
- [x] **Safe CRT Fallbacks:** Litter the codebase with MSVC "Safe CRT" functions (e.g., `sprintf_s`, `strcpy_s`) for added buffer safety. These MUST be carefully guarded via `#if defined(_MSC_VER)`, seamlessly falling back to standard C89 functions for GCC/Clang/MinGW.

## 5. Formatting Constraints
- [x] **Clang-Format Safety:** Every `#include` block should be wrapped in `/* clang-format off */` and `/* clang-format on */` to preserve strict include ordering. This ensures casual runs of clang-format do not break header dependencies. Maximum one occurrence of `/* clang-format off */` and `/* clang-format on */` per file.

## 6. Documentation Coverage (100% Mandate)
- [x] **API Boundary:** Every `struct`, `enum`, and exported FFI function must be thoroughly documented using Doxygen/kernel-doc standard comments.
- [x] **Internal Architecture:** Write architectural markdown docs for memory ownership and threading models.

## 7. Bindings (FFI Automation)
- [x] **Rust:** Auto-generate safe wrappers (`sys` crate + idiomatic wrapper crate).
- [x] **C# (.NET):** Auto-generate `DllImport` P/Invoke bindings.
- [x] **Python/Go:** Create initial scaffolding for dynamically loaded C-extension modules mapping to the C89 handles.
