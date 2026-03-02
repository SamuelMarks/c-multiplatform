C multiplatform (libcmp)
========================

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![ci](https://github.com/SamuelMarks/c-multiplatform/actions/workflows/ci.yml/badge.svg)](https://github.com/SamuelMarks/c-multiplatform/actions/workflows/ci.yml)
[![visual-docs](https://github.com/SamuelMarks/c-multiplatform/actions/workflows/visual_docs.yml/badge.svg)](https://github.com/SamuelMarks/c-multiplatform/actions/workflows/visual_docs.yml)
![Test Coverage](docs/badges/test-coverage.svg)
![Doc Coverage](docs/badges/doc-coverage.svg)

**LibCMPC** is a strict C89 (ANSI C) cross-platform application framework with a design-system-agnostic core and a fully-featured Material Design 3 component library. It targets robust state management, layout/animation primitives, and backend-agnostic rendering in a highly portable, zero-dependency core. 

This project is architected specifically for **Context-Window Scalability**, decoupling Interfaces (Contracts) from Implementations. This allows for modular development and makes the codebase distinctively friendly for LLM-assisted coding and maintenance.

## 🚀 Key Features

*   **Strict C89 ABI:** Compiles on virtually any C compiler (MSVC, GCC, Clang, TCC).
*   **Zero Dependencies:** Core layout, math, text bounds, rendering queues, and data structures are completely self-contained.
*   **Material 3 Library:** A comprehensive suite of native M3 widgets (Buttons, Cards, Chips, Selection, Progress, Dialogs, Sheets, App Bars, Navigation, and Scaffold).
*   **Visual Documentation Generator:** A fully automated headless vector renderer that translates layout & drawing commands into SVG tables via GitHub Actions, proving the UI logic without needing a screen.
*   **Layout + Animation:** Flex-style layout, springs, gesture/scroll helpers, predictive back events.
*   **Accessibility Semantics:** A11y primitives and widget semantics metadata.
*   **Modular Backends (platform-gated):**
    *   **Null Backend:** For headless testing, doc generation, and logic verification.
    *   **SDL3 Backend:** Desktop debug backend.
    *   **GTK4 Backend:** Linux desktop backend using GTK4/Cairo.
    *   **Cocoa Backend:** macOS backend using Cocoa/CoreGraphics/CoreText.
    *   **Win32 Backend:** Windows backend using GDI + WinHTTP.
    *   **Web Backend:** Emscripten backend (WebGL; optional WebGPU).
    *   **iOS Backend:** UIKit-based backend with AVFoundation integration.
    *   **Android Backend:** NDK backend with Camera2 integration (API 24+).
*   **Interface-Driven:** Pure virtual table architecture separates API definitions from execution logic.

## 📂 Architecture Strategy

The project structure is designed to separate the **Contract** (Phase 0) from the **Implementation**.

*   **`include/cmpc/`**: ABI contracts (`cmp_api_*`) plus public headers for core systems, widgets, accessibility, media, and helpers.
*   **`include/m3/`**: Material 3 design-system headers (widgets, palettes, styling defaults).
*   **`src/core/`**: Core logic (allocators, utf8/log/store/i18n, math/layout/anim/predictive, render/event/gesture/scroll/tasks, a11y, widgets, image/audio/video decoders, storage/network/camera helpers).
*   **`src/m3/`**: Material 3 implementations (color/HCT and widgets: app bar, navigation, tabs, dialogs, sheets, scaffold, menus, buttons, cards, chips, selection, progress, date/time pickers).
*   **`src/backend/`**: Platform-specific implementations of the `cmp_api_*` V-Tables (per backend subdirectory).
*   **`tools/docgen/`**: The headless vector rendering pipeline for visual documentation.
*   **`packaging/`**: Packaging scripts and stubs for desktop, mobile, web, and SDL3 builds.

## 📚 Documentation

The LibCMPC documentation pipeline generates a full static website natively using **Doxygen**. It embeds automatically-generated SVG screenshots of the widgets running in different environments to ensure visual regressions are caught and documented.

*   [**Interactive API Docs & Widget Tour**](https://github.com/SamuelMarks/c-multiplatform/releases) (Check the releases tab for the generated `website.zip`)
*   **`docs/pages/`**: Markdown source for the documentation website landing pages.
*   **`USAGE.md`**: Build and widget usage walkthroughs.
*   **`ARCHITECTURE.md`**: Contract/implementation split and system flow.
*   **`DESIGN_PRINCIPLES.md`**: C89 and API design conventions.

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
| `CMP_ENABLE_GTK4` | `OFF` | Enable the GTK4 backend (Linux). |
| `CMP_ENABLE_COCOA` | `OFF` | Enable the Cocoa backend (macOS). |
| `CMP_ENABLE_WIN32` | `ON` | Enable the Win32 backend (Windows). |
| `CMP_ENABLE_WEB` | `OFF` | Enable the Web (Emscripten) backend. |
| `CMP_ENABLE_IOS` | `OFF` | Enable the iOS backend. |
| `CMP_ENABLE_ANDROID` | `ON` (if Android) | Enable the Android NDK backend. |
| `CMP_BUILD_DOCGEN` | `OFF` | Build the visual documentation generator tool. |
| `CMP_REQUIRE_DOXYGEN` | `OFF` | Fail configuration if Doxygen is missing. |
| `CMP_ENABLE_COVERAGE` | `OFF` | Enable code coverage (GCC/Clang only). |
| `CMP_ENABLE_PACKAGING` | `OFF` | Enable packaging targets (Android, iOS, Web, Desktop). |

### Visual Documentation Pipeline

LibCMPC can visually render its own widgets directly to SVG without a headless browser, avoiding heavy dependencies.

```bash
# Build the doc generator
cmake -S . -B build -DCMP_BUILD_DOCGEN=ON
cmake --build build --target cmp_docgen

# Run it across different targets (adds JSON manifests + SVGs)
./build/cmp_docgen --platform linux --theme material
./build/cmp_docgen --platform web --theme material

# Generate the Markdown matrix and Doxygen
python3 tools/docgen/build_docs.py
cmake -S . -B build-docs -DCMP_REQUIRE_DOXYGEN=ON
cmake --build build-docs --target cmp_docs
```
Open `build-docs/docs/html/index.html` to browse the interactive site.

### Running Tests

The project includes a comprehensive test suite covering all phases of development.

```bash
cd build
ctest --output-on-failure
```

To enforce coverage and badge updates on every commit, enable the repository hooks path:
```bash
git config core.hooksPath .githooks
```

## 🗺️ Roadmap & Status

| Phase | Description | Status |
| :--- | :--- | :--- |
| **0. Protocols** | Core, GFX, Env, WS, and widget interface definitions | ✅ Complete |
| **1. Infra** | Build system, allocators, logging, handles, store, i18n | ✅ Complete |
| **2. Core Logic** | Math, HCT color, layout, animation, router, path | ✅ Complete |
| **3. Engine** | Render lists, event dispatching, gesture/scroll, task runner | ✅ Complete |
| **4. Backends** | Null, SDL3, GTK4, Cocoa, Win32, Web, iOS, Android | ✅ Implemented |
| **5. Components** | Core widgets + Extensive M3 widget library | ✅ Complete |
| **6. Media + Helpers** | Storage, camera, network, image/audio/video, a11y | ✅ Core APIs complete |
| **7. Packaging** | Android/iOS/Web/Desktop/SDL3 packaging targets | 🚧 In progress |
| **8. Docs + Visuals** | Interactive Doxygen APIs, Automated SVG widget tours | ✅ Complete |
| **9. Example Apps** | Port official sample apps (Material/Compose equivalents) | 🗓️ Planned |
| **10. Wasm Viewer** | Interactive widget demos directly in the documentation | 🗓️ Planned |
