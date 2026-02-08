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
*   **Modular Backends:**
    *   **Null Backend:** For headless testing and logic verification.
    *   **SDL3 Backend:** For desktop development and debugging.
    *   *(Planned)*: Native Connectors for GTK3, Win32 GDI, Cocoa, Android, and Web/WASM.
*   **Interface-Driven:** Pure virtual table architecture separates API definitions from execution logic.

## 📂 Architecture Strategy

The project structure is designed to separate the **Contract** (Phase 0) from the **Implementation**.

*   **`include/m3/`**: Contains only header-only definitions, types, and V-Tables. These files represent the total API surface area. When extending the library, an LLM only needs to see the specific `*_interface.h` relevant to the task, keeping context utilization low.
*   **`src/core/`**: Pure logic implementations (Math, Color, Layout, Router, Redux). These have no platform dependencies.
*   **`src/backend/`**: Platform-specific glue code implementation of the V-Tables defined in `m3_api_*.h`.

## 🛠️ Build Instructions

LibM3C uses **CMake**. Ensure you have CMake 3.16+ installed.

### Prerequisites for SDL3 Backend (Optional)
If you wish to run the visual backend, ensure SDL3 is installed on your system. If not found, the build will proceed with only the Null backend and Core library.

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
| `M3_REQUIRE_DOXYGEN` | `ON` | Fail configuration if Doxygen is missing. |
| `M3_ENABLE_COVERAGE` | `OFF` | Enable code coverage (GCC/Clang only). |

### Running Tests

The project includes a comprehensive test suite covering all phases of development.

```bash
cd build
ctest --output-on-failure
```

To run a specific backend test manually:
```bash
./m3_phase4_sdl3_backend
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
| **1. Infra** | Build system, Allocators, Logging, Redux Store | ✅ Complete |
| **2. Core Logic** | Math, HCT Color, Flex Layout, Springs, Router | ✅ Complete |
| **3. Engine** | Render Tree Builders, Event Dispatching, Task Runner | ✅ Complete |
| **4. Backends** | Null Backend, SDL3 Debug Backend | ✅ Complete |
| **5. Components** | Buttons, Text Fields, Lists (Native Widgets) | 🚧 Planned |
| **6. Plugins** | Camera, Network, Storage implementations | 🚧 Planned |
| **7. Release** | Packaging logic for Mobile/Web/Desktop | 🚧 Planned |
