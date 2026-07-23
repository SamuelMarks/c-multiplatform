# C Multiplatform UI Engine & Media Framework

The universal, ultra-lightweight UI and media engine for the next generation of cross-platform applications.

Built entirely in **strictly-compliant C89**, this multi-threaded engine delivers a complete W3C CSS layout system, modern reactive state (Signals), and a robust Component Development Kit (CDK) across Web, Mobile, and Desktop.

## The Ground-Up Rewrite: A Paradigm Shift

This project (`tmp/c-multiplatform`) represents a complete, ground-up reimplementation of the original `c-multiplatform` architecture. The initial prototype revealed architectural constraints in supporting highly dynamic, state-driven user interfaces. We have moved away from a rigid "Design System in C" towards a **Component Dev Kit (CDK)** approach. 

Developers can compose these CDK primitives to build their *own* design systems without fighting the framework, while retaining the extreme performance of a native C core. Accessibility (a11y) and Internationalization (i18n) are integrated foundationally at the lowest ABI level, rather than being tightly coupled as an afterthought.

## The Vision: Beyond Flutter, React Native, and KMP

The current landscape of cross-platform development forces compromises: heavy JavaScript bridges (React Native), proprietary layout systems and massive bundle sizes (Flutter), or language-ecosystem lock-in (Kotlin Multiplatform).

**This framework takes a different approach:**
By leveraging the universal C ABI as a fully opaque FFI boundary, it acts as a hyper-fast, invisible UI runtime that can be seamlessly bound to **any modern language** (Rust, Go, C#, Python, Zig, Swift, JS) with **zero bridge overhead**.

*   **No "Widget Soup":** Instead of proprietary layout algorithms, we implemented a 100% W3C CSS Engine (Flexbox, Grid, Cascading). Design your layouts using the web standards you already know, but render them via native OpenGL ES 2.0 or native DOM.
*   **Zero Language Lock-in:** Write your business logic in Rust, Python, or Go. The engine handles the heavy lifting of DOM generation, text shaping, A/V synchronization, and composition. The reactive signal graph safely traverses the FFI boundary.
*   **Insane Performance & Footprint:** Written in C89 with custom memory pools and multi-reactor architecture. It bypasses massive VM/JS engine initialization overhead, ensuring near-instantaneous Time-To-Interactive (TTI).
*   **Modern Paradigms, Native Speeds:** Brings modern concepts like Reactive Signals and Headless UI to a bare-metal execution environment.

---

## Core Differentiators

- ⚡ **Universal C ABI:** Bind to anything. Truly native cross-platform without the ecosystem constraints of KMP or the JS payload of React Native.
- 🎨 **100% W3C CSS Engine:** Full support for CSS Selectors, Grid, Flexbox, Color Level 5, Math functions, and Animations. Features **zero-copy lexing** directly from memory-mapped files and enables dynamic hot-reloading without recompiling C code.
- 🧠 **Reactive State (Signals):** Built-in core Signal engine (`ui_signal`, `ui_computed`, `ui_effect`) replacing imperative updates. Includes a reactive Forms framework for complex validation and dependency graphs.
- 🏗️ **Headless CDK:** Purely structural, unstyled behavioral components guaranteeing robust focus management, ARIA accessibility, and geometric tethering before any visual themes are applied.
- 🌐 **Web Target Focus — Beyond the Canvas Trap:** Unlike traditional cross-platform frameworks that draw to a single WebGL `<canvas>` (breaking a11y and text selection), our WASM target uses an **Inspectable DOM** approach. UI primitives map to real HTML DOM nodes with native inputs, ensuring flawless screen reader and password manager support.
- 🛡️ **Military-Grade Robustness:** Valgrind/ASAN memory leak testing, strict C89 compliance, and an uncompromising push for **100% Test Coverage** (unit and property-based testing) and **100% Documentation Coverage** to provide mathematical API guarantees.

---

## Upcoming Flagship Use Cases

The new architecture enables zero-overhead isomorphic execution, bridging dynamic interpretation with AoT C code ejection:

1. **Survey Maker & Production Lifecycle:** Rapid iteration via remote over-the-air schema updates, ejecting to native C for perfect 60fps performance and zero parsing in production.
2. **Full Low-Code App Builder:** Visual IDE rendering directly to canvas, with an "Export App" function generating statically-linked, zero-dependency native binaries.
3. **API-Driven Auto-Generated Interfaces:** UI components, validation, and layout derived natively from **OpenAPI 3.1**, with dynamic event orchestration via **Arazzo** specs.

---

## Architecture & Master Plans

Due to the immense scope and robustness constraints, the architectural plan is split into domain-specific modules:

* **[Core & Platform (PLAN_CORE.md)](PLAN_CORE.md):** Abstract execution, memory pools, strict C89 standards, FFI boundaries, and OS windowing/input abstractions.
* **[Engine (PLAN_ENGINE.md)](PLAN_ENGINE.md):** DOM generation, Media, text shaping, and GLES 2.0 Compositor.
* **[CSS Engine (PLAN_CSS_W3C.md)](PLAN_CSS_W3C.md):** 100% W3C CSS Implementation.
* **[Runtime Widgets (PLAN_RUNTIME_WIDGETS.md)](PLAN_RUNTIME_WIDGETS.md):** Component implementation plans and specifications.
* **[Signal & State (SIGNAL_TODO_PLAN.md)](SIGNAL_TODO_PLAN.md):** The core Signal engine and reactive Forms framework.
* **[Quality Assurance (PLAN_QUALITY.md)](PLAN_QUALITY.md):** Coverage constraints, leak testing, and documentation mandates.
* **[Wayland (WAYLAND_PLAN.md)](WAYLAND_PLAN.md):** Native Wayland display server integration plan.
* **[Test Plan (TEST_PLAN.md)](TEST_PLAN.md):** Testing methodology and tracking.

---

## Developer Guides

Detailed guides can be found in the [`guides/`](guides/) directory. Key highlights include:

* [Getting Started](guides/getting-started/)
* [Architecture Overview](guides/architecture/)
* [DOM, Components & Event System](guides/dom-and-events/)
* [State Management](guides/state-management/)
* [Layout & Styling](guides/layout-and-styling/)
* [Advanced CSS Engine](guides/css-engine/)

[**View all Guides & Documentation in the Guides Index**](guides/README.md)

---

## Getting Started: Building from Source

This project is built using CMake (minimum version 3.15) and relies on standard C toolchains (GCC, Clang, MSVC).

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Configuration Options

You can customize the build using the following CMake options:
- `-DUI_ENABLE_WAYLAND=ON/OFF`: Enable Wayland Support on Linux (default: ON)
- `-DUI_USE_HARFBUZZ=ON/OFF`: Enable HarfBuzz integration for complex text shaping (default: OFF)
- `-DUI_ENABLE_UNICODE=ON/OFF`: Enable UNICODE charset (default: ON)
- `-DUI_SINGLE_THREADED=ON/OFF`: Disable Multi-threading (default: OFF)
- `-DUI_CRT_STATIC=ON/OFF`: Use Static CRT linkage (default: OFF)
- `-DUI_ENABLE_LTO=ON/OFF`: Enable Link-Time Optimization (default: OFF)
