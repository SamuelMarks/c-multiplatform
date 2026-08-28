/**
 * @file PLAN_CORE.md
 * @brief PLAN_CORE.md implementation.
 */
/**
 * @file PLAN_CORE.md
 * @brief PLAN_CORE.md implementation.
 */
# Architecture Plan: Core, Execution & Platform

## 1. Abstract Execution & Concurrency
- [x] Define `struct ui_execution_context` abstract interface for FFI and internal scheduling.
- [x] Implement Single-Threaded Tick Engine (for Emscripten/WASM).
- [x] Implement Thread-Pool Task Executor (for background layout and asset decoding).
- [x] Implement Multi-Reactor Event Loop (epoll/kqueue/IOCP integration for headless/server-driven UI).
- [x] Implement Async/Continuation state machines in strict C89 (simulated futures/promises via callbacks and user-data pointers).
- [x] Create lock-free ring buffers and atomic operation wrappers for cross-thread DOM manipulation/diffing.
- [x] Abstract Timer API (slaved to OS monotonic clocks, or A/V presentation timestamps).

## 2. Core Foundations & FFI
- [x] **Strict C89 & MSVC Safe Extensions:** Configure CMake matrix for `-std=c89 -pedantic -Wall -Werror`.
- [x] **No Typedefs:** Use explicit `struct name` and `enum name` globally.
- [x] **Error Enums & Percolation:** Every function MUST return `ui_error_t` (or a subsystem-specific enum discriminant). All actual return data must use out-parameters.
  - *Strict Percolation:* Errors must be checked and percolated up the call stack immediately.
  - *Cleanup on Failure:* If an error occurs midway through a function (e.g., a `malloc` failure), the function MUST clean up all locally acquired resources before returning the failure discriminant (typically utilizing a `goto cleanup;` pattern to avoid duplicate free logic). Never leak memory or leave dangling file handles on the error path.
- [x] Define platform-independent integer types natively (without `<stdint.h>`).
- [x] Implement Custom Memory Allocator (Arena/Pool).
- [x] Create core opaque FFI header (`ui_engine.h`) with standard C linkage.
- [x] Implement thread-safe Handle Manager.

## 3. Platform Abstraction Layer (Windowing & Inputs)
- [x] Define `struct ui_window_backend` abstract vtable.
- [x] Implement Win32 HWND + WGL context creation.
- [x] Implement macOS Cocoa NSWindow + CGL context.
- [x] Implement iOS UIKit UIWindow + EAGL context.
- [x] Implement Android NativeActivity + EGL context.
- [x] Implement Linux/FreeBSD X11 (GLX) and Wayland (EGL) backends.
- [x] Implement Web Emscripten HTML5 Canvas integration.
- [x] Abstract Input Routing (Mouse, Touch, Pen, Keyboard, Gamepad) into a unified engine Event struct.

## 4. Networking, I/O & Persistence (CMake FetchContent Dependencies)
The engine relies on external, modular C libraries to handle network calls, filesystem access, and disk persistence. These MUST be integrated via modern CMake using `FetchContent_Declare` and `FetchContent_MakeAvailable` (strictly avoiding `FetchContent_Populate`), fetching directly from the `master` branches of the respective GitHub repositories (no pinned commits).

- [x] **Network / HTTP(S):** Integrate `c-abstract-http`.
  - Fetch URL: `https://github.com/SamuelMarks/c-abstract-http.git`
  - Use case: Fetching remote CSS stylesheets, downloading fonts (`@font-face`), loading remote images/media, and exposing XHR/Fetch equivalents to the FFI.
- [x] **Filesystem Operations:** Integrate `c-fs`.
  - Fetch URL: `https://github.com/SamuelMarks/c-fs.git`
  - Use case: Abstracting OS-level file handles, directory traversal, and safely reading local assets without violating cross-platform constraints.
- [x] **Database / Persistence:** Integrate `c-orm`.
  - Fetch URL: `https://github.com/SamuelMarks/c-orm.git`
  - Use case: Caching layout trees, persisting user preferences, storing downloaded assets locally, and maintaining UI state across application restarts.
