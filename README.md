c-multiplatform
===============

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Doc Coverage](https://img.shields.io/badge/docs-100%25-brightgreen.svg)](#)
[![Test Coverage](https://img.shields.io/badge/coverage-100%25-brightgreen.svg)](#)

A high-performance, strictly-compliant **ANSI C (C89 / ISO C90)** multiplatform UI engine and headless Component Development Kit (CDK). Featuring a full **W3C CSS layout engine**, fine-grained **Signals reactivity**, **universal FFI** interoperability with zero bridge overhead, an **inspectable real DOM** for WebAssembly, and **dynamic runtime-defined widgets with Ahead-of-Time (AoT) C code ejection**.

---

## Table of Contents

- [0. Project Overview & Competitor Landscape](#0-project-overview--competitor-landscape)
  - [The Lingua Franca of FFI](#the-lingua-franca-of-ffi)
  - [Competitor Comparison Matrix](#competitor-comparison-matrix)
- [1. Target Platforms](#1-target-platforms)
- [2. Multi-Language FFI Support](#2-multi-language-ffi-support)
- [3. Example Code: Reactive UI in Pure C89](#3-example-code-reactive-ui-in-pure-c89)
- [4. Runtime-Defined Widgets & AoT Code Ejection](#4-runtime-defined-widgets--aot-code-ejection)
  - [Dynamic Schema Interpretation (Live Interpreter)](#dynamic-schema-interpretation-live-interpreter)
  - [Zero-Overhead Ahead-of-Time (AoT) C Code Ejection](#zero-overhead-ahead-of-time-aot-c-code-ejection)
- [5. Design Languages & Headless CDK](#5-design-languages--headless-cdk)
  - [The Headless Architecture](#the-headless-architecture)
  - [First-Class Design Systems](#first-class-design-systems)
  - [Design Tokens & Dynamic Theming](#design-tokens--dynamic-theming)
- [6. Accessibility (a11y) & Internationalization (i18n)](#6-accessibility-a11y--internationalization-i18n)
  - [First-Class Accessibility (a11y)](#first-class-accessibility-a11y)
  - [First-Class Internationalization (i18n) & Bidirectional Text](#first-class-internationalization-i18n--bidirectional-text)
- [7. Compiler & Toolchain Support Matrix](#7-compiler--toolchain-support-matrix)
  - [Tested Toolchains](#tested-toolchains)
  - [Build Matrix Configuration](#build-matrix-configuration)
- [8. Execution Modalities & Concurrency Architecture](#8-execution-modalities--concurrency-architecture)
  - [Multi-Threaded Multi-Reactor](#multi-threaded-multi-reactor)
  - [Single-Threaded Cooperative Mode](#single-threaded-cooperative-mode)
  - [Headless & Automation Mode](#headless--automation-mode)
  - [Deterministic Memory Management](#deterministic-memory-management)
- [Building from Source](#building-from-source)
- [License](#license)

---

## 0. Project Overview & Competitor Landscape

`c-multiplatform` is a ground-up reimagining of cross-platform user interface engineering. Rather than inventing a bespoke language or bundling a virtual machine, it is implemented entirely in **strict, portable ANSI C (C89 / ISO C90)**.

The engine provides:
- A full **W3C CSS layout engine** (Flexbox, CSS Grid, CSS Selectors, CSSOM View, Box Sizing, Transitions, and Logical Properties).
- A fine-grained, push-based **Signals reactive state engine** (`ui_signal`, `ui_computed`, `ui_effect`) and a reactive Forms framework using the **Control Value Accessor (CVA)** pattern—completely bypassing Virtual DOM diffing.
- A comprehensive, unstyled **Component Development Kit (CDK)** delivering accessible behavioral primitives (Dialogs, Sliders, Dropdowns, Focus Traps, Virtual Grids).
- An **Inspectable Real DOM** for the Web target, ensuring accessibility and browser integration that canvas-based frameworks destroy.
- **Runtime-Defined Widgets with AoT Ejection**, enabling applications to load dynamic schemas at runtime and compile them into raw C89 code with zero interpretation overhead.

### The Lingua Franca of FFI

C is the universal ABI of modern computing. Every significant language—Rust, Go, C#, Python, Zig, Swift, JavaScript, Kotlin, Java—features native C Foreign Function Interface (FFI) capabilities.

By implementing the entire engine behind a pristine, opaque C ABI (`extern "C"`, scalar `ui_error_t` status codes, opaque pointer handles), `c-multiplatform` eliminates the serialization and bridging overhead that plagues cross-platform ecosystems. Developers write high-performance rendering and layout in C, while writing application workflows and business logic in their language of choice.

### Competitor Comparison Matrix

| Feature / Metric | `c-multiplatform` | Flutter | React Native | Kotlin Multiplatform (KMP) | Electron | Qt / QML |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Language Requirements** | **Any language via native C FFI** (C, Rust, Go, C#, Python, etc.) | Dart only | JavaScript / TypeScript | Kotlin only | JavaScript / TypeScript | C++ or Python |
| **Bridge Overhead** | **Zero (Native C ABI call)** | Dart VM isolate crossing | Asynchronous JSON/JSI bridge | JNI / Kotlin/Native runtime | IPC serialization | C++/C wrapper boundary |
| **Binary Footprint** | **Microscopic (< 2 MB typical static link)** | Heavy (30–80 MB+ runtime) | Heavy (engine + JS VM) | Moderate-to-Heavy | Massive (150–300 MB+) | Heavy (dynamic Qt DLLs) |
| **Web Target Architecture** | **Inspectable Real DOM** (Real HTML elements) | `<canvas>` / CanvasKit (breaks a11y, copy/paste) | React DOM (heavy overhead) | Canvas / Compose HTML | N/A (runs on desktop only) | WebAssembly Canvas |
| **Screen Readers & a11y** | **Native OS ARIA tree & inspectable DOM** | Emulated / brittle accessibility bridge | Platform accessible elements | Platform-dependent | Web standard a11y | Qt Accessibility API |
| **Layout Standards** | **100% W3C CSS** (Flexbox, Grid, CSSOM) | Bespoke widget constraints | CSS Flexbox subset (Yoga) | Compose layout DSL | Full CSS (Chromium) | QML Layouts / QLayout |
| **Memory Model** | **Hierarchical Arenas & Object Pools (O(1), zero GC)** | Garbage Collected (Dart) | Garbage Collected (JS) | Garbage Collected (JVM/Kotlin) | Garbage Collected (V8) | Reference counting / Parent-child heap |
| **Dynamic UI to AoT** | **JSON Schema Interpreter &rarr; AoT C89 Ejection** | Not supported (recompilation required) | Server-driven UI (slow JSON render) | Not supported | Server-driven UI | QML interpretation |
| **Oldest Toolchain** | **MSVC 2005 (Visual Studio 8.0) & C89** | Modern Clang only | Modern Clang/LLVM | Modern JVM / LLVM | Modern Node / Clang | Modern C++17/20 |

---

## 1. Target Platforms

`c-multiplatform` targets every major tier of consumer and enterprise hardware through modular platform backends:

- **Windows Desktop:** Native Win32 API windowing (`ui_window_backend_win32.h`), GDI and OpenGL ES 2.0 / Direct3D render paths. Implemented with **zero `<windows.h>` header bloat** using forward declarations and selective subsystem headers.
- **Linux & BSD:** Native **Wayland** support (`libwayland-client`, `wayland-protocols`, `libxkbcommon`) and **X11** fallback (`libX11`, `libGL`). Supports headless rendering via DRM/KMS.
- **macOS:** Native Cocoa / AppKit window backend (`ui_window_backend_macos.h`), Quartz/CoreGraphics events, and AppleClang / Metal-compatible GL context management.
- **iOS:** UIKit native view backend, Touch event dispatch, and Retina-aware scaling.
- **Android:** Android NDK NativeActivity backend, EGL surface binding, and Android input event translation.
- **Web (WebAssembly):** Compiled via Emscripten (`ui_window_backend_web.h`). Unlike frameworks that force a single WebGL `<canvas>`, the Web target leverages an **Inspectable Real DOM** architecture where widgets construct native HTML elements, maintaining complete fidelity with browser password managers, assistive technologies, and native text selection.
- **Embedded & Real-Time:** Headless framebuffers and single-threaded cooperative execution paths capable of running on resource-constrained microcontrollers and RTOS environments with sub-megabyte RAM allocations.

---

## 2. Multi-Language FFI Support

Because every engine structure is opaque and every function follows the standard C calling convention (`extern "C"`, scalar `ui_error_t` returns, explicit out-pointer parameters), binding to `c-multiplatform` requires **zero intermediate proxy code**:

- **Rust:** Seamlessly bind using `bindgen` or the pre-generated `c-multiplatform-sys` and idiomatic safe wrapper crates:
  ```rust
  let mut app_state: *mut ui_app_state_registry = std::ptr::null_mut();
  let rc = unsafe { ui_app_state_registry_create(arena, &mut app_state) };
  assert_eq!(rc, UI_ERROR_NONE);
  ```
- **C# / .NET:** Direct P/Invoke integration with no C++/CLI glue:
  ```csharp
  [DllImport("c_multiplatform", CallingConvention = CallingConvention.Cdecl)]
  public static extern ui_error_t ui_ffi_app_state_set_string(string key, string val);

  ui_error_t rc = ui_ffi_app_state_set_string("session_id", "tok_9934");
  ```
- **Python:** Direct interop via `ctypes` or `cffi`:
  ```python
  import ctypes
  lib = ctypes.CDLL("libc_multiplatform.so")
  lib.ui_ffi_app_state_set_int.argtypes = [ctypes.c_char_p, ctypes.c_int]
  lib.ui_ffi_app_state_set_int.restype = ctypes.c_int
  rc = lib.ui_ffi_app_state_set_int(b"user_score", 100)
  assert rc == 0  # UI_ERROR_NONE
  ```
- **Go:** Zero-overhead integration via `cgo`:
  ```go
  // #include "ui_ffi_runtime.h"
  import "C"
  rc := C.ui_ffi_app_state_set_string(C.CString("session_id"), C.CString("tok_9934"))
  if rc != C.UI_ERROR_NONE {
      /* handle error */
  }
  ```
- **Swift:** Native import without bridging headers via Clang C-module maps.
- **Zig:** First-class interop using `@cImport` and `@cInclude("ui_engine.h")`.
- **JavaScript / TypeScript (Node.js, Bun):** High-speed native add-ons via N-API, Bun FFI, or direct WebAssembly instantiation in browsers.
- **Kotlin / Java:** Modern Project Panama Foreign Function & Memory (FFM) API or classical JNI.
- **C++:** Public headers include `#ifdef __cplusplus extern "C"` guards, ensuring seamless integration into C++98 through C++26 codebases.

---

## 3. Example Code: Reactive UI in Pure C89

The following example demonstrates declarative layout, strict error percolation (`ui_error_t`), and reactive state using the **Control Value Accessor (CVA)** pattern and **Signals** in strict ANSI C (C89):

```c
#include "ui_engine.h"
#include "ui_arena.h"
#include "ui_dom_node.h"
#include "ui_component.h"
#include "ui_input_base.h"
#include "ui_button_base.h"
#include "ui_form_builder.h"
#include "ui_form_group.h"
#include "ui_form_control.h"
#include "ui_control_value_accessor.h"
#include "ui_error.h"
#include <stdio.h>

/* Constructs a reactive sign-up form adhering strictly to C89 and ui_error_t */
ui_error_t build_signup_screen(struct ui_arena *arena,
                               struct ui_dom_node *container,
                               ui_form_group_t **out_form) {
  ui_error_t rc;
  struct ui_form_builder *fb = NULL;
  struct ui_input_base *email_input = NULL;
  struct ui_control_value_accessor email_cva;
  struct ui_button_base *submit_btn = NULL;
  struct ui_component *email_comp = NULL;
  struct ui_component *submit_comp = NULL;
  ui_form_control_t *email_ctrl = NULL;
  union ui_signal_payload default_email;

  if (arena == NULL || container == NULL || out_form == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* 1. Initialize reactive form group */
  rc = ui_form_builder_create(arena, &fb);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_form_builder_group_start(fb, "auth");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  default_email.ptr_val = (void *)"";
  rc = ui_form_builder_control(fb, "email", default_email,
                               UI_SIGNAL_TYPE_POINTER, NULL, NULL);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_form_builder_group_end(fb);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_form_builder_build(fb, out_form);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* 2. Instantiate CDK Input and extract its Control Value Accessor (CVA) */
  rc = ui_input_base_create(&email_input);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_input_base_get_cva(email_input, &email_cva);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* 3. Bind CVA to form control: two-way reactive bridge */
  rc = ui_form_group_get_control(*out_form, "email", &email_ctrl);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_form_control_bind_cva(email_ctrl, &email_cva);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* 4. Instantiate Submit Button */
  rc = ui_button_base_create(&submit_btn);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_button_base_set_text(submit_btn, "Submit");
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* 5. Mount components into host DOM container */
  rc = ui_input_base_get_component(email_input, &email_comp);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_mount(email_comp, container);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_button_base_get_component(submit_btn, &submit_comp);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_component_mount(submit_comp, container);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

cleanup:
  if (fb != NULL) {
    ui_form_builder_destroy(fb);
  }
  if (rc != UI_ERROR_NONE) {
    if (email_input != NULL) {
      ui_input_base_destroy(email_input);
    }
    if (submit_btn != NULL) {
      ui_button_base_destroy(submit_btn);
    }
  }
  return rc;
}
```

---

## 4. Runtime-Defined Widgets & AoT Code Ejection

`c-multiplatform` introduces an **isomorphic execution model** allowing complete user interfaces to be designed dynamically in a serialized JSON Schema format, interpreted live, and then ejected directly to native C89 source code.

### Dynamic Schema Interpretation (Live Interpreter)

The engine can consume a JSON UI schema (aligned with OpenAPI 3.1 Schema Objects and Arazzo workflows) at runtime. The runtime interpreter validates the schema, resolves components in the `ui_component_registry`, attaches Control Value Accessors (CVAs) to localized `ui_dynamic_context` scopes, and binds global signals:

```c
#include "ui_runtime_schema.h"
#include "ui_runtime_builder.h"
#include "ui_component_registry.h"
#include "ui_dynamic_context.h"
#include "ui_app_state_registry.h"
#include "ui_arena.h"
#include "ui_error.h"

ui_error_t render_dynamic_form(struct ui_arena *arena, const char *schema_json,
                               struct ui_dom_node **out_dom) {
  ui_error_t rc;
  struct ui_component_registry *registry = NULL;
  struct ui_dynamic_context *ctx = NULL;
  struct ui_app_state_registry *app_state = NULL;
  struct ui_runtime_node *ast_root = NULL;

  if (arena == NULL || schema_json == NULL || out_dom == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Acquire component registry and execution scopes */
  rc = ui_component_registry_get_default(&registry);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_dynamic_context_create(arena, &ctx);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  rc = ui_app_state_registry_create(arena, &app_state);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Parse JSON into transient AST */
  rc = ui_runtime_schema_parse_node(arena, schema_json, &ast_root);
  if (rc != UI_ERROR_NONE) {
    goto cleanup;
  }

  /* Build live DOM tree */
  rc = ui_runtime_build_tree(ast_root, registry, ctx, app_state,
                             NULL, NULL, out_dom);

cleanup:
  if (ctx != NULL) {
    ui_dynamic_context_destroy(ctx);
  }
  if (app_state != NULL) {
    ui_app_state_registry_destroy(app_state);
  }
  return rc;
}
```

### Zero-Overhead Ahead-of-Time (AoT) C Code Ejection

Once a dynamically designed interface (e.g. from a visual form builder or survey studio) is validated, you can "eject" the schema to raw C code using `ui_runtime_eject`:

```c
#include "ui_runtime_eject.h"
#include "ui_error.h"
#include <stdio.h>

ui_error_t eject_schema_to_disk(const struct ui_runtime_node *ast_root,
                                const char *form_prefix) {
  ui_error_t rc;
  char c_filename[256];
  char h_filename[256];

  if (ast_root == NULL || form_prefix == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

#if defined(_MSC_VER)
  sprintf_s(c_filename, sizeof(c_filename), "%s.c", form_prefix);
  sprintf_s(h_filename, sizeof(h_filename), "%s.h", form_prefix);
#else
  sprintf(c_filename, "%s.c", form_prefix);
  sprintf(h_filename, "%s.h", form_prefix);
#endif

  /* Eject AST directly into compiled C89 source and header files */
  rc = ui_runtime_eject_node_to_c(ast_root, form_prefix, c_filename, h_filename);
  if (rc != UI_ERROR_NONE) {
    return rc;
  }
  return UI_ERROR_NONE;
}
```

**The Ejection Guarantee:**
- The generated C code uses the **exact same public C APIs** as manual handwritten code.
- Enforces strict `goto cleanup;` error percolation with zero memory leaks.
- Strips all JSON parsers, string lookups, and runtime hash maps from your production binary, yielding instant execution and microscopic memory overhead.

---

## 5. Design Languages & Headless CDK

### The Headless Architecture

`c-multiplatform` separates **component behavior and accessibility** from **visual presentation**. The Component Development Kit (CDK) provides headless base components (e.g., `ui_button_base`, `ui_slider_base`, `ui_select_base`, `ui_dialog_base`, `ui_datepicker_base`):
- Base widgets manage ARIA attributes (`role`, `aria-checked`, `aria-expanded`), keyboard navigation (`Tab`, `Arrows`, `Enter`), and reactive signal bindings.
- Visual layers provide styling, typography, spacing, and micro-interactions.

### First-Class Design Systems

Developers can build or layer any visual design system on top of the CDK:
- **Material Design 3 (Material You):** Full support for Material 3 tokens, elevation filters, dynamic color palette generation from seed tones, and hardware-accelerated ripple primitives (`ui_ripple_base`).
- **Fluent Design System (Microsoft):** Support for Acrylic translucency, Reveal Highlight cursor illumination, connected animations, and rounded modern geometry.
- **Cupertino (Apple Human Interface Guidelines):** Native macOS/iOS visual fidelity, SF-style typography hierarchies, spring physics, and segmented controls.
- **Bespoke / Unstyled Themes:** Completely unstyled primitives ready for corporate design systems.

### Design Tokens & Dynamic Theming

The engine uses standard CSS Custom Properties (`var(--...)`) for design tokens (`ui_design_tokens.h`). Swapping between Light, Dark, or High-Contrast themes requires a single CSSOM call, automatically invalidating and repainting without recreating a single C struct:

```c
#include "ui_design_tokens.h"
#include "ui_component.h"
#include "ui_error.h"

ui_error_t apply_dark_theme(struct ui_component *root_comp) {
  ui_error_t rc;

  if (root_comp == NULL) {
    return UI_ERROR_INVALID_ARGUMENT;
  }

  /* Dynamic CSS token updates cascade through the layout and render pipeline */
  rc = ui_component_set_property(root_comp, "--color-surface", "#121212");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_component_set_property(root_comp, "--color-on-surface", "#FFFFFF");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  rc = ui_component_set_property(root_comp, "--color-primary", "#BB86FC");
  if (rc != UI_ERROR_NONE) {
    return rc;
  }

  return UI_ERROR_NONE;
}
```

---

## 6. Accessibility (a11y) & Internationalization (i18n)

Accessibility and Internationalization are treated as foundational requirements at the lowest ABI level, not afterthoughts.

### First-Class Accessibility (a11y)

- **W3C ARIA Node Graph (`ui_aria.h`):** Every UI node exposes semantic roles (`UI_ARIA_ROLE_BUTTON`, `DIALOG`, `SLIDER`, `ALERT`), states (`aria-disabled`, `aria-checked`, `aria-busy`), and properties (`aria-valuemin`, `aria-controls`).
- **Focus Management & Trapping (`ui_focus_manager.h`, `ui_focus_trap.h`, `ui_focus_ring.h`):** Deterministic keyboard focus cycling, directional spatial navigation, and modal focus traps to keep assistive technologies contained within active overlays.
- **Live Announcer (`ui_live_announcer.h`):** Directly interfaces with host screen readers (Windows Narrator/NVDA, macOS VoiceOver, Linux Orca) to announce dynamic state changes and validation errors.
- **Inspectable Web DOM:** Unlike canvas-based frameworks that isolate the user from browser assistive technologies, our WASM backend generates true semantic HTML nodes so native browser screen readers and extensions work out of the box.

### First-Class Internationalization (i18n) & Bidirectional Text

- **Bidirectional (BiDi) Engine (`ui_bidi_manager.h`):** Full Unicode Annex #9 Bidirectional algorithm support. Handles Right-to-Left (RTL) scripts (Arabic, Hebrew, Persian) seamlessly alongside embedded Left-to-Right (LTR) segments.
- **W3C CSS Logical Properties (`ui_css_logical.h`):** Layouts use flow-relative rules (`margin-inline-start`, `padding-block-end`, `border-inline-start`) that mirror automatically when `dir="rtl"` is set, eliminating the need for duplicate stylesheets.
- **Complex Text Shaping via HarfBuzz:** When configured with `-DUI_USE_HARFBUZZ=ON`, the text pipeline properly shapes complex scripts, handles ligatures, and executes font fallback chains (`ui_font_manager.h`).
- **String Translation Registries (`ui_i18n.h`):** Thread-safe localization dictionaries with parameterized string formatting and locale-aware number and date formats.

---

## 7. Compiler & Toolchain Support Matrix

`c-multiplatform` enforces strict **ISO C90 (C89)** compliance with zero compiler extensions (`-std=c89 -pedantic`). It compiles cleanly without warnings across decades of compiler toolchains:

### Tested Toolchains

- **MSVC 2005 (Visual Studio 8.0):** Verified under native Windows and Wine. Adheres strictly to MSVC 2005 C89 limits (declarations at start of block, Safe CRT functions like `sprintf_s`, no `<stdint.h>`, no `<windows.h>` header pollution).
- **MSVC 2022 (Visual Studio 17.0):** Modern MSVC toolset with static CRT (`/MT`, `/MTd`) and dynamic CRT (`/MD`, `/MDd`), supporting runtime checks (`/RTC1`, `/RTCs`, `/RTCu`).
- **MSVC 2026 (Next-Gen Visual Studio):** Continually verified against upcoming Microsoft toolchains for forward compatibility.
- **AppleClang:** Native compilation for macOS (Intel & Apple Silicon) and iOS with maximum warning flags (`-Wall -Wextra -Wpedantic -Werror`).
- **GCC (GNU Compiler Collection):** Linux, MinGW-w64, and Cygwin targets, tested across GCC 4.8 through GCC 14+.
- **LLVM / Clang:** AddressSanitizer (ASAN), UndefinedBehaviorSanitizer (UBSAN), and ThreadSanitizer (TSAN) verification in CI.

### Build Matrix Configuration

CMake options allow granular control over linkage, character sets, and threading:

```bash
# Example: Static CRT, Unicode, Release build with Wayland
cmake -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DUI_CRT_STATIC=ON \
  -DUI_ENABLE_UNICODE=ON \
  -DUI_ENABLE_WAYLAND=ON \
  -DUI_ENABLE_LTO=ON
```

| CMake Option | Values | Description |
| :--- | :--- | :--- |
| `UI_CRT_STATIC` | `ON` / `OFF` (default) | Windows CRT linkage: Static (`/MT`, `/MTd`) vs Shared (`/MD`, `/MDd`). |
| `UI_ENABLE_UNICODE` | `ON` (default) / `OFF` | Enables native UNICODE wide-character handling vs ANSI. |
| `UI_SINGLE_THREADED` | `OFF` (default) / `ON` | Disables multi-threading for single-core microcontrollers and WASM. |
| `UI_SHARED_LIB` | `OFF` (default) / `ON` | Builds engine as a Shared Library (`.dll`, `.so`, `.dylib`) vs Static archive. |
| `UI_ENABLE_LTO` | `OFF` (default) / `ON` | Enables Interprocedural / Link-Time Optimization. |
| `UI_ENABLE_WAYLAND` | `ON` (default) / `OFF` | Enables native Wayland client backend on Linux. |
| `UI_USE_HARFBUZZ` | `OFF` (default) / `ON` | Enables HarfBuzz integration for complex text shaping. |
| `UI_MSVC_RUNTIME_CHECKS` | `OFF` (default) / `ON` | Enables MSVC `/RTC1`, `/RTCs`, or `/RTCu` stack frame checks. |
| `UI_ENABLE_ASAN` | `OFF` (default) / `ON` | Compiles with AddressSanitizer memory safety instrumentation. |

---

## 8. Execution Modalities & Concurrency Architecture

The engine is architected to dynamically adapt its concurrency and execution modality to the host environment:

### Multi-Threaded Multi-Reactor

On desktop and mobile platforms, the engine separates tasks cleanly across threads:
1. **Main UI Thread (`ui_execution_context`):** Exclusively owns the DOM and CSSOM trees. This eliminates mutex locking in the visual graph, guaranteeing smooth 60/120 FPS rendering.
2. **Worker Pool (`ui_thread_pool`):** Background worker threads handle file I/O, texture decoding, and computational tasks, returning results via thread-safe promises (`ui_promise`).
3. **Reactor Thread (`ui_reactor`):** A dedicated multi-reactor event loop multiplexes OS handles and network sockets via `epoll` (Linux), `kqueue` (macOS/BSD), or `select` (Windows), dispatching I/O events back to the main thread tick loop.

### Single-Threaded Cooperative Mode

When compiling with `-DUI_SINGLE_THREADED=ON`, all threading constructs, mutexes, and atomics are replaced with zero-overhead synchronous shims. Background workers and reactor tasks execute cooperatively within `ui_execution_context_tick()`. This mode is optimized for:
- WebAssembly execution on browser main threads without Web Workers or SharedArrayBuffer requirements.
- Single-core embedded microcontrollers (ARM Cortex-M, RISC-V, ESP32).

### Headless & Automation Mode

The engine includes headless window and renderer backends (`ui_e2e_headless.h`):
- Run layout computations, CSS validation, and user interaction flows in CI/CD environments with zero GPU or display server dependencies.
- Perfect for automated visual regression testing, geometry assertions, and server-side UI pre-rendering.

### Deterministic Memory Management

`c-multiplatform` avoids standard heap fragmentation and garbage collection pauses:
- **Per-Frame Transient Arenas (`ui_arena`):** Layout passes, CSS token resolutions, and temporary string views are allocated in arenas and freed in $O(1)$ time via `ui_arena_reset()`.
- **Component Object Pools:** Long-lived nodes (windows, persistent widgets, shaders) use pre-allocated pools.
- **Strict Error Percolation:** Every allocation failure safely cascades via structured `goto cleanup;` handlers, mathematically preventing memory leaks under low-memory conditions.

---

## Building from Source

### Prerequisites

- CMake (version 3.15 or newer)
- A compliant C89/C90 compiler (MSVC, GCC, Clang, or AppleClang)

### Standard Build Instructions

```bash
# Clone the repository
git clone https://github.com/SamuelMarks/c-multiplatform.git
cd c-multiplatform

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run the test suite (100% test coverage target)
ctest --test-dir build --output-on-failure
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
