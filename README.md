c-multiplatform
===============

[![License](https://img.shields.io/badge/license-Apache--2.0%20OR%20MIT-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Doc Coverage](https://img.shields.io/badge/docs-29%25-red.svg)](#)
[![Test Coverage](https://img.shields.io/badge/coverage-47%25-red.svg)](#)

The universal, ultra-lightweight UI and media engine for the next generation of cross-platform applications.

---

`c-multiplatform` is a ground-up reimagining of cross-platform UI development. Built entirely in **strictly-compliant C89**, this engine delivers a complete W3C CSS layout system, modern reactive state (Signals), and a robust Component Development Kit (CDK) across **iOS, Android, Web, macOS, Linux, and Windows**.

## The Lingua Franca of FFI

Why C? Because **C is the lowest common denominator of the world of FFI (Foreign Function Interface)**.

By leveraging the universal C ABI as a fully opaque FFI boundary, `c-multiplatform` acts as a hyper-fast, invisible UI runtime that can be seamlessly bound to **any modern language**—Rust, Go, C#, Python, Zig, Swift, or JavaScript—with **zero bridge overhead**. Write the core UI rendering in lightning-fast native C, and write the business logic in whatever language your team is most productive in.

## A True Alternative to Flutter, React Native, and Kotlin Multiplatform

The current landscape of cross-platform development forces compromises:
- **React Native** relies on heavy, asynchronous JavaScript bridges that degrade performance.
- **Flutter** forces developers into the proprietary Dart ecosystem, ships massive bundle sizes, and its Web target draws to a `<canvas>`, breaking native text selection and accessibility.
- **Kotlin Multiplatform (KMP)** ties projects deeply into the JVM and Kotlin compiler ecosystem.

**`c-multiplatform` introduces a fundamentally different approach:**
- **Zero Language Lock-in:** Use the engine via FFI from your language of choice.
- **Insane Performance & Footprint:** Bypasses massive VM/JS engine initialization overhead, ensuring near-instantaneous Time-To-Interactive (TTI).
- **Web Target Focus (Real DOM):** Unlike traditional frameworks that draw to a single WebGL `<canvas>` (breaking a11y), the WASM target uses an **Inspectable DOM** approach. UI primitives map to real HTML DOM nodes with native inputs, ensuring flawless screen reader and password manager support.

## Uncompromising Portability & Toolchains

Because `c-multiplatform` adheres to strict C89 standards, it offers unprecedented compiler and platform support out of the gate. Whether compiling with the latest **LLVM/Clang**, modern **GCC**, or spanning over two decades of Microsoft tooling (from legacy **MSVC 2005** to the upcoming **MSVC 2026**), this engine is designed to build reliably.

It is built to run literally anywhere: from high-end desktop environments and modern smartphones, down to legacy Windows machines, resource-constrained embedded systems, and modern WebAssembly environments.

## Architecture & Computational Models

The engine’s architecture is highly adaptable to its target environment, capable of shifting its execution model to suit the host platform:

- **Flexible Threading Models:** By default, the engine utilizes a robust multi-threaded multi-reactor pattern (separating UI, rendering, and I/O threads) for maximum throughput on desktop and mobile targets. However, it can be compiled to operate entirely single-threaded (`-DUI_SINGLE_THREADED=ON`) for single-core embedded devices or strict cooperative environments like WASM.
- **Custom Memory Management:** Bypasses standard `malloc`/`free` overhead using custom memory pools and arena allocators. This ensures zero memory fragmentation, deterministic performance, and O(1) allocation strategies which are critical for sustaining 60/120fps rendering.
- **Reactive Execution:** The UI doesn't rely on expensive virtual DOM diffing. Instead, it uses a granular, push-based reactive Signal graph (`ui_signal`, `ui_computed`) that propagates state changes deterministically, synchronizing perfectly with the display refresh rate (VSync).

## Core Features

### 🏗️ Component Dev Kit (CDK) Approach
Rather than dictating a rigid visual design system, `c-multiplatform` introduces a Headless CDK approach to C. It provides purely structural, unstyled behavioral components (Alerts, Dropdowns, Datepickers, Focus Traps). Developers can compose these primitives to build bespoke design systems from scratch without friction, all while retaining native C speeds.

### 🌍 Universal Accessibility (a11y) & Internationalization (i18n)
Accessibility and Internationalization are treated as foundational pillars at the lowest ABI level.
- **a11y:** Built-in ARIA support, robust focus management, and geometric tethering exist natively before any visual themes are applied.
- **i18n:** Natively handled text shaping (via HarfBuzz integration), bidirectional (RTL) text support, and seamless font fallback mechanisms are built directly into the text rendering pipeline.

### 🎨 100% W3C CSS Engine
Design layouts using familiar web standards. Full support for CSS Selectors, Flexbox, Grid, Color Level 5, Math functions, and Animations. Render them via native OpenGL ES 2.0 on desktop/mobile or native DOM on the web.

### 🧠 Reactive State (Signals)
A built-in core Signal engine (`ui_signal`, `ui_computed`, `ui_effect`) brings modern, fine-grained reactivity and a robust Forms framework without virtual DOM diffing overhead.

---

## Building from Source

This project is built using CMake (minimum version 3.15).

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Configuration Options

Customize the computational model and platform features via CMake:
- `-DUI_ENABLE_WAYLAND=ON/OFF`: Enable Wayland Support on Linux (default: ON)
- `-DUI_USE_HARFBUZZ=ON/OFF`: Enable HarfBuzz integration for complex text shaping (default: OFF)
- `-DUI_ENABLE_UNICODE=ON/OFF`: Enable UNICODE charset (default: ON)
- `-DUI_SINGLE_THREADED=ON/OFF`: Disable multi-threading for cooperative/embedded environments (default: OFF)

---

## Getting Started

Writing UI in pure C is straightforward using the CDK and Signal APIs. Here is a simple example of creating a declarative "Sign up" input form directly in native C:

```c
#include "ui_engine.h"
#include "ui_label_base.h"
#include "ui_input_base.h"
#include "ui_button_base.h"
#include "ui_layout.h"

/* Form submission handler */
void on_signup_click(ui_event_t* event) {
    /* Handle form data */
}

int main(int argc, char** argv) {
    ui_window_t* window;
    ui_node_t* form_container;
    ui_node_t* title_label;
    ui_node_t* first_name_input;
    ui_node_t* last_name_input;
    ui_node_t* signup_button;

    /* Initialize the engine (automatically selects optimal threading/memory model) */
    ui_engine_init();

    /* Create a new window */
    window = ui_window_create("Sign Up", 800, 600);

    /* Create a container with Flexbox layout */
    form_container = ui_node_create();
    ui_node_set_style(form_container,
        "display: flex; flex-direction: column; "
        "justify-content: center; align-items: stretch; "
        "padding: 40px; gap: 16px; max-width: 400px; margin: auto;");

    /* Create form components from the CDK */
    title_label = ui_label_create("Sign up");
    ui_node_set_style(title_label, "font-size: 24px; font-weight: bold; margin-bottom: 24px;");

    first_name_input = ui_input_create("First name");
    last_name_input = ui_input_create("Last name");

    signup_button = ui_button_create("Sign up");
    ui_node_set_style(signup_button, "background-color: #007AFF; color: white; padding: 12px;");

    /* Attach event listener */
    ui_node_on(signup_button, "click", on_signup_click);

    /* Assemble the tree */
    ui_node_append_child(form_container, title_label);
    ui_node_append_child(form_container, first_name_input);
    ui_node_append_child(form_container, last_name_input);
    ui_node_append_child(form_container, signup_button);

    ui_window_set_root(window, form_container);

    /* Start the main event loop */
    ui_engine_run(window);

    /* Cleanup */
    ui_engine_cleanup();
    return 0;
}
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
