LibCMPC
======

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![ci](https://github.com/SamuelMarks/c-multiplatform/actions/workflows/ci.yml/badge.svg)](https://github.com/SamuelMarks/c-multiplatform/actions/workflows/ci.yml)
![Test Coverage](docs/badges/test-coverage.svg)
![Doc Coverage](docs/badges/doc-coverage.svg)

**LibCMPC** is a strict C89 (ANSI C) cross-platform application framework with a design-system-agnostic core and a Material 3 component library. It targets robust state management, layout/animation primitives, and backend-agnostic rendering in a highly portable, zero-dependency core. Cupertino and Fluent design systems are on the roadmap (see below).

This project is architected specifically for **Context-Window Scalability**, decoupling Interfaces (Contracts) from Implementations. This allows for modular development and makes the codebase distinctively friendly for LLM-assisted coding and maintenance.

## 🚀 Key Features

*   **Strict C89 ABI:** Compiles on virtually any C compiler (MSVC, GCC, Clang, TCC).
*   **Design-System Core + Material 3 Library:** `cmpc` is design-system-agnostic; `m3` provides Material 3 widgets and styling. Cupertino and Fluent are planned.
*   **HCT Color Science:** Hue-Chroma-Tone utilities for dynamic theming.
*   **Layout + Animation:** Flex-style layout, springs, gesture/scroll helpers, predictive back events.
*   **Redux-style State Store:** Deterministic state management with undo/redo snapshots.
*   **Accessibility Semantics:** A11y primitives and widget semantics metadata.
*   **Widgets (Core + M3):** visuals, text, icons, lists, text fields, selection controls, buttons, cards, chips, menus, tabs, app bars, navigation, dialogs, sheets, progress, scaffold.
*   **Media + IO Helpers:** storage, network, camera sessions, image/audio/video decoders (fallback), i18n utilities.
*   **Modular Backends (platform-gated):**
    *   **Null Backend:** For headless testing and logic verification.
    *   **SDL3 Backend:** Desktop debug backend (optional SDL3_ttf text rendering).
    *   **GTK4 Backend:** Linux desktop backend using GTK4/Cairo.
    *   **Cocoa Backend:** macOS backend using Cocoa/CoreGraphics/CoreText.
    *   **Win32 Backend:** Windows backend using GDI + WinHTTP.
    *   **Web Backend:** Emscripten backend (WebGL; optional WebGPU).
    *   **iOS Backend:** UIKit-based backend with AVFoundation integration.
    *   **Android Backend:** NDK backend with Camera2 integration (API 24+).
*   **Interface-Driven:** Pure virtual table architecture separates API definitions from execution logic. Backend feature coverage varies; unsupported calls return `CMP_ERR_UNSUPPORTED`.

## 📂 Architecture Strategy

The project structure is designed to separate the **Contract** (Phase 0) from the **Implementation**.

*   **`include/cmpc/`**: ABI contracts (`cmp_api_*`) plus public headers for core systems, widgets, accessibility, media, and helpers.
*   **`include/m3/`**: Material 3 design-system headers (widgets, palettes, styling defaults).
*   **`src/core/`**: Core logic (allocators, utf8/log/store/i18n, math/layout/anim/predictive, render/event/gesture/scroll/tasks, a11y, widgets, image/audio/video decoders, storage/network/camera helpers).
*   **`src/m3/`**: Material 3 implementations (color/HCT and widgets: app bar, navigation, tabs, dialogs, sheets, scaffold, menus, buttons, cards, chips, selection, progress, date/time pickers).
*   **`src/backend/`**: Platform-specific implementations of the `cmp_api_*` V-Tables (per backend subdirectory).
*   **`packaging/`**: Packaging scripts and stubs for desktop, mobile, web, and SDL3 builds.
*   **CMake targets**: `cmpc` (alias `cmp::cmpc`) for the core library and `m3` (alias `m3::m3`) for the Material 3 library (depends on `cmpc`).

## 📚 Documentation

*   **`USAGE.md`**: Build and widget usage walkthroughs.
*   **`ARCHITECTURE.md`**: Contract/implementation split and system flow.
*   **`DESIGN_PRINCIPLES.md`**: C89 and API design conventions.
*   **`docs/examples/`**: Source snippets referenced by documentation (currently empty).
*   **Doxygen HTML**: `cmp_docs` emits API docs into `build/docs`.

## 🛠️ Build Instructions

LibCMPC uses **CMake**. Ensure you have CMake 3.16+ installed.

### Backend Prerequisites (Optional)
Backends are opt-in and will compile as stubs when their platform or dependencies are missing. Use `cmp_*_backend_is_available` at runtime to check support.

* **SDL3:** Install SDL3; enable SDL3_ttf for text rendering if needed.
* **GTK4:** Install GTK4 dev packages and ensure `pkg-config` can locate `gtk4`.
* **Cocoa/iOS:** Requires Apple toolchains with Objective-C and system frameworks.
* **Win32:** Requires a Windows toolchain with user32/gdi32/winhttp.
* **Web:** Requires Emscripten; WebGPU is optional.
* **Android:** Requires the Android NDK; Camera2 integration targets API 24+.

### Compile

```bash
mkdir build
cd build
cmake .. -D CMAKE_BUILD_TYPE=Debug
cmake --build .
```

### Build Options

| Option | Default | Description |
| :--- | :--- | :--- |
| `CMP_WARNINGS_AS_ERRORS` | `ON` | Treat compiler warnings as errors. |
| `CMP_ENABLE_SDL3` | `OFF` | Enable the SDL3 debug backend. |
| `CMP_ENABLE_SDL3_TTF` | `OFF` | Enable SDL3_ttf text rendering for the SDL3 backend. |
| `CMP_ENABLE_GTK4` | `OFF` | Enable the GTK4 backend (Linux). |
| `CMP_ENABLE_COCOA` | `OFF` | Enable the Cocoa backend (macOS). |
| `CMP_ENABLE_WIN32` | `ON` | Enable the Win32 backend (Windows). |
| `CMP_ENABLE_WEB` | `OFF` | Enable the Web (Emscripten) backend. |
| `CMP_ENABLE_WEBGPU` | `OFF` | Enable WebGPU for the Web backend. |
| `CMP_ENABLE_IOS` | `OFF` | Enable the iOS backend. |
| `CMP_ENABLE_ANDROID` | `ON (Android), OFF otherwise` | Enable the Android backend. |
| `CMP_ENABLE_LIBCURL` | `ON` | Enable libcurl-backed network support. |
| `CMP_APPLE_USE_CFNETWORK_C` | `ON` | Use CFNetwork C APIs for Apple network backends (OFF uses Foundation). |
| `CMP_REQUIRE_DOXYGEN` | `OFF` | Fail configuration if Doxygen is missing. |
| `CMP_ENABLE_COVERAGE` | `OFF` | Enable code coverage (GCC/Clang only). |
| `CMP_COVERAGE_LINE_THRESHOLD` | `100` | Minimum line coverage percentage enforced by `cmp_coverage`. |
| `CMP_COVERAGE_FUNCTION_THRESHOLD` | `100` | Minimum function coverage percentage enforced by `cmp_coverage`. |
| `CMP_COVERAGE_BRANCH_THRESHOLD` | `100` | Minimum branch coverage percentage enforced by `cmp_coverage`. |
| `CMP_ENABLE_LCOV` | `ON` | Enable `cmp_coverage_lcov` output when `lcov`/`genhtml` are available. |
| `CMP_ENABLE_PACKAGING` | `OFF` | Enable packaging targets (Android, iOS, Web, Desktop, SDL3). |
| `CMP_BUILD_WEB_APP` | `OFF` | Build a minimal web app target for Emscripten packaging. |
| `CMP_BUILD_PACKAGING_STUB` | `OFF` | Build a minimal desktop stub executable for installer packaging. |

### Running Tests

The project includes a comprehensive test suite covering all phases of development.

```bash
cd build
ctest --output-on-failure
```

To run a specific backend test manually (platform-dependent):
```bash
./cmp_phase4_sdl3_backend
./cmp_phase4_gtk4_backend
./cmp_phase4_cocoa_backend
./cmp_phase4_win32_backend
./cmp_phase4_web_backend
./cmp_phase4_ios_backend
./cmp_phase4_android_backend
```

### Coverage

Coverage requires a dedicated build configured with `CMP_ENABLE_COVERAGE=ON`.

```bash
cmake -S . -B build-coverage -DCMP_ENABLE_COVERAGE=ON -DCMP_REQUIRE_DOXYGEN=ON
cmake --build build-coverage --target cmp_coverage
cmake --build build-coverage --target cmp_docs
```

If `lcov`/`genhtml` are available, you can also generate an HTML report:

```bash
cmake --build build-coverage --target cmp_coverage_lcov
```

To enforce coverage and badge updates on every commit, enable the repository
hooks path:

```bash
git config core.hooksPath .githooks
```

### Packaging

Packaging helpers live under `packaging/`. Enable the packaging targets and invoke
the desired platform target:

```bash
cmake -S . -B build-packaging -DCMP_ENABLE_PACKAGING=ON \
  -DCMP_BUILD_PACKAGING_STUB=ON \
  -DCMP_BUILD_WEB_APP=ON
cmake --build build-packaging --target cmp_package_android
cmake --build build-packaging --target cmp_package_ios_xcframework
cmake --build build-packaging --target cmp_package_web
cmake --build build-packaging --target cmp_package_desktop
cmake --build build-packaging --target cmp_package_sdl3
```

Each platform folder contains a README with prerequisites and output locations.

## 📦 Usage Example

Below is a conceptual example of initializing the backend and creating a window using the C89 ABI.

```c
#include "cmpc/cmp_backend_sdl3.h"
#include <stdio.h>

int main(void) {
    CMPSDL3BackendConfig config;
    CMPSDL3Backend *backend = NULL;
    CMPWS *ws; /* Window System Interface */
    CMPWSWindowConfig wincfg;
    CMPHandle window;
    CMPBool available;
    int rc;

    /* Check availability */
    cmp_sdl3_backend_is_available(&available);
    if (!available) {
        printf("SDL3 not available.\n");
        return 1;
    }

    /* Initialize Config */
    cmp_sdl3_backend_config_init(&config);
    
    /* Create Backend */
    rc = cmp_sdl3_backend_create(&config, &backend);
    if (rc != 0) return rc;

    /* Get Interfaces */
    struct CMPWS ws_interface;
    cmp_sdl3_backend_get_ws(backend, &ws_interface);
    ws = &ws_interface;

    /* Configure Application */
    CMPWSConfig app_config;
    app_config.utf8_app_name = "My CMPC App";
    app_config.utf8_app_id = "com.example.cmpc";
    ws->vtable->init(ws->ctx, &app_config);

    /* Create Window */
    wincfg.width = 800;
    wincfg.height = 600;
    wincfg.utf8_title = "LibCMPC Demo";
    wincfg.flags = CMP_WS_WINDOW_RESIZABLE;
    
    ws->vtable->create_window(ws->ctx, &wincfg, &window);
    ws->vtable->show_window(ws->ctx, window);

    /* Event Loop (Simplified) */
    CMPInputEvent event;
    CMPBool has_event;
    CMPBool running = CMP_TRUE;

    while (running) {
        ws->vtable->poll_event(ws->ctx, &event, &has_event);
        if (has_event && event.type == CMP_INPUT_WINDOW_CLOSE) {
            running = CMP_FALSE;
        }
        /* Render loop logic goes here... */
    }

    /* Cleanup */
    ws->vtable->destroy_window(ws->ctx, window);
    ws->vtable->shutdown(ws->ctx);
    cmp_sdl3_backend_destroy(backend);

    return 0;
}
```

## 🗺️ Roadmap & Status

| Phase | Description | Status |
| :--- | :--- | :--- |
| **0. Protocols** | Core, GFX, Env, WS, and widget interface definitions | ✅ Complete |
| **1. Infra** | Build system, allocators, logging, handles, store, i18n | ✅ Complete |
| **2. Core Logic** | Math, HCT color, layout, animation, router, path | ✅ Complete |
| **3. Engine** | Render lists, event dispatching, gesture/scroll, task runner | ✅ Complete |
| **4. Backends** | Null, SDL3, GTK4, Cocoa, Win32, Web, iOS, Android (platform-gated; coverage varies) | ✅ Implemented (platform-gated) |
| **5. Components** | Core widgets + M3 widgets (buttons, navigation, dialogs, sheets, scaffold, etc.) | ✅ Complete |
| **6. Media + Helpers** | Storage, camera, network, image/audio/video decoders, a11y, predictive back | ✅ Core APIs complete (backend support varies) |
| **7. Packaging** | Android/iOS/Web/Desktop/SDL3 packaging targets and scripts | 🚧 In progress |
| **8. Design Systems** | Apple Cupertino + Microsoft Fluent libraries | 🗓️ Planned |
| **9. Docs + Tutorials** | Interactive API docs and guided tutorials | 🗓️ Planned |
| **10. Example Apps** | Port official sample apps (Material/Compose, Cupertino, Fluent) | 🗓️ Planned |
