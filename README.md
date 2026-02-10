LibM3C
======

**LibM3C** is a strict C89 (ANSI C) cross-platform application framework designed to implement Material 3 design principles, robust state management, and modern UI physics in a highly portable, zero-dependency core.

This project is architected specifically for **Context-Window Scalability**, decoupling Interfaces (Contracts) from Implementations. This allows for modular development and makes the codebase distinctively friendly for LLM-assisted coding and maintenance.

## 🚀 Key Features

*   **Strict C89 ABI:** Compiles on virtually any C compiler (MSVC, GCC, Clang, TCC).
*   **Material 3 Ready:** Includes HCT color science using the sophisticated Hue-Chroma-Tone color space for dynamic theming.
*   **Flex-style Layout:** A constraint-based layout engine similar to Flexbox.
*   **Physics-based Animation:** Spring simulation and timing controllers built-in.
*   **Redux-style State Store:** Deterministic state management with time-travel debugging capabilities (Undo/Redo).
*   **Widget Toolkit:** Visuals/text primitives plus buttons, selection controls, text fields, lists, navigation, dialogs, and progress indicators.
*   **Plugin APIs:** Storage, camera, and network helpers that bind to backend IO/sensors/tasking.
*   **Modular Backends (platform-gated):**
    *   **Null Backend:** For headless testing and logic verification.
    *   **SDL3 Backend:** Desktop debug backend (optional SDL3_ttf text rendering).
    *   **GTK4 Backend:** Linux desktop backend using GTK4/Cairo.
    *   **Cocoa Backend:** macOS backend using Cocoa/CoreGraphics/CoreText.
    *   **Win32 Backend:** Windows backend using GDI + WinHTTP.
    *   **Web Backend:** Emscripten backend (WebGL; optional WebGPU).
    *   **iOS Backend:** UIKit-based backend with AVFoundation integration.
    *   **Android Backend:** NDK backend with Camera2 integration (API 24+).
*   **Interface-Driven:** Pure virtual table architecture separates API definitions from execution logic.

## 📂 Architecture Strategy

The project structure is designed to separate the **Contract** (Phase 0) from the **Implementation**.

*   **`include/m3/`**: ABI contracts (`m3_api_*`) plus public module headers (core systems, widgets, and plugins).
*   **`src/core/`**: Pure logic implementations (math, color, layout, animation, router, render, event, tasks, widgets, plugins).
*   **`src/backend/`**: Platform-specific implementations of the `m3_api_*` V-Tables (per backend subdirectory).

## 📚 Documentation

*   **`USAGE.md`**: Build and widget usage walkthroughs.
*   **`ARCHITECTURE.md`**: Contract/implementation split and system flow.
*   **`DESIGN_PRINCIPLES.md`**: C89 and API design conventions.

## 🛠️ Build Instructions

LibM3C uses **CMake**. Ensure you have CMake 3.16+ installed.

### Backend Prerequisites (Optional)
Backends are opt-in and will compile as stubs when their platform or dependencies are missing. Use `m3_*_backend_is_available` at runtime to check support.

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
| `M3_WARNINGS_AS_ERRORS` | `ON` | Treat compiler warnings as errors. |
| `M3_ENABLE_SDL3` | `OFF` | Enable the SDL3 debug backend. |
| `M3_ENABLE_SDL3_TTF` | `OFF` | Enable SDL3_ttf text rendering for the SDL3 backend. |
| `M3_ENABLE_GTK4` | `OFF` | Enable the GTK4 backend (Linux). |
| `M3_ENABLE_COCOA` | `OFF` | Enable the Cocoa backend (macOS). |
| `M3_ENABLE_WIN32` | `ON` | Enable the Win32 backend (Windows). |
| `M3_ENABLE_WEB` | `OFF` | Enable the Web (Emscripten) backend. |
| `M3_ENABLE_WEBGPU` | `OFF` | Enable WebGPU for the Web backend. |
| `M3_ENABLE_IOS` | `OFF` | Enable the iOS backend. |
| `M3_ENABLE_ANDROID` | `ON (Android), OFF otherwise` | Enable the Android backend. |
| `M3_ENABLE_LIBCURL` | `ON` | Enable libcurl-backed network support. |
| `M3_APPLE_USE_CFNETWORK_C` | `ON` | Use CFNetwork C APIs for Apple network backends (OFF uses Foundation). |
| `M3_REQUIRE_DOXYGEN` | `OFF` | Fail configuration if Doxygen is missing. |
| `M3_ENABLE_COVERAGE` | `OFF` | Enable code coverage (GCC/Clang only). |

### Running Tests

The project includes a comprehensive test suite covering all phases of development.

```bash
cd build
ctest --output-on-failure
```

To run a specific backend test manually (platform-dependent):
```bash
./m3_phase4_sdl3_backend
./m3_phase4_gtk4_backend
./m3_phase4_cocoa_backend
./m3_phase4_win32_backend
./m3_phase4_web_backend
./m3_phase4_ios_backend
./m3_phase4_android_backend
```

## 📦 Usage Example

Below is a conceptual example of initializing the backend and creating a window using the C89 ABI.

```c
#include "m3/m3_backend_sdl3.h"
#include <stdio.h>

int main(void) {
    M3SDL3BackendConfig config;
    M3SDL3Backend *backend = NULL;
    M3WS *ws; /* Window System Interface */
    M3WSWindowConfig wincfg;
    M3Handle window;
    M3Bool available;
    int rc;

    /* Check availability */
    m3_sdl3_backend_is_available(&available);
    if (!available) {
        printf("SDL3 not available.\n");
        return 1;
    }

    /* Initialize Config */
    m3_sdl3_backend_config_init(&config);
    
    /* Create Backend */
    rc = m3_sdl3_backend_create(&config, &backend);
    if (rc != 0) return rc;

    /* Get Interfaces */
    struct M3WS ws_interface;
    m3_sdl3_backend_get_ws(backend, &ws_interface);
    ws = &ws_interface;

    /* Configure Application */
    M3WSConfig app_config;
    app_config.utf8_app_name = "My M3 App";
    app_config.utf8_app_id = "com.example.m3";
    ws->vtable->init(ws->ctx, &app_config);

    /* Create Window */
    wincfg.width = 800;
    wincfg.height = 600;
    wincfg.utf8_title = "LibM3C Demo";
    wincfg.flags = M3_WS_WINDOW_RESIZABLE;
    
    ws->vtable->create_window(ws->ctx, &wincfg, &window);
    ws->vtable->show_window(ws->ctx, window);

    /* Event Loop (Simplified) */
    M3InputEvent event;
    M3Bool has_event;
    M3Bool running = M3_TRUE;

    while (running) {
        ws->vtable->poll_event(ws->ctx, &event, &has_event);
        if (has_event && event.type == M3_INPUT_WINDOW_CLOSE) {
            running = M3_FALSE;
        }
        /* Render loop logic goes here... */
    }

    /* Cleanup */
    ws->vtable->destroy_window(ws->ctx, window);
    ws->vtable->shutdown(ws->ctx);
    m3_sdl3_backend_destroy(backend);

    return 0;
}
```

## 🗺️ Roadmap & Status

| Phase | Description | Status |
| :--- | :--- | :--- |
| **0. Protocols** | Core, GFX, Env, and Widget Interface definitions | ✅ Complete |
| **1. Infra** | Build system, allocators, logging, handles, store | ✅ Complete |
| **2. Core Logic** | Math, HCT color, flex layout, springs, router | ✅ Complete |
| **3. Engine** | Render lists, event dispatching, task runner | ✅ Complete |
| **4. Backends** | Null, SDL3, GTK4, Cocoa, Win32, Web, iOS, Android (platform-gated) | ✅ Implemented (platform-gated) |
| **5. Components** | Visuals, text, buttons, selection, text fields, lists, navigation, dialogs, progress | ✅ Complete |
| **6. Plugins** | Storage, camera, network | ✅ Core APIs complete (backend support varies) |
| **7. Release** | Packaging logic for Mobile/Web/Desktop | 🚧 Planned |
