# Multi-Paradigm C89 UI Engine & Media Framework (Master Plan)

This project is a multi-threaded, strictly C89-compliant cross-platform engine driving a CSS layout system and robust UI design systems. It supports complex media (video, audio, SVG) and targets web, mobile, and desktop environments utilizing a fully opaque FFI boundary.

Due to the immense scope and robustness constraints (100% test coverage, rigid typography mappings, physics engines, and strict memory standards), the architectural plan is split into domain-specific modules.

## Architecture Modules

### 1. Engine Core & Platform
*   **[PLAN_CORE.md](PLAN_CORE.md):** Abstract execution (async/multireactor), memory pools, strict C89 standards, FFI boundaries, and OS windowing/input abstractions.
*   **[PLAN_ENGINE.md](PLAN_ENGINE.md):** DOM generation, Media (A/V sync, decoders), text shaping, and GLES 2.0 Compositor.
*   **[PLAN_CSS_W3C.md](PLAN_CSS_W3C.md):** 100% W3C CSS Implementation (Selectors, Grid, Flexbox, Cascading, Color Level 5, Math functions, Animations).

### 2. Headless Component Development Kit (CDK) & Reactive State
Implementations of purely structural, unstyled behavioral components (analogous to Angular CDK or Headless UI). This guarantees robust focus management, accessibility, and geometric tethering before visual themes are applied.
*   **[SIGNAL_TODO_PLAN.md](SIGNAL_TODO_PLAN.md):** The core Signal engine (`ui_signal`, `ui_computed`, `ui_effect`) and reactive Forms framework (Control Value Accessor, Validators). All CDK widgets must conform to this state architecture.
*   **[CDK_TODO_PLAN.md](CDK_TODO_PLAN.md):** Focus routing, ARIA mappings, Portal/Overlay managers, tethering logic, and the unstyled widget baseline (virtual scrolling, trees, text editors).

### 3. Quality & Interoperability
*   **[PLAN_QUALITY.md](PLAN_QUALITY.md):** 100% coverage constraints, Valgrind/ASAN memory leak testing, documentation mandates, and auto-generated FFI bindings (Rust, C#, etc.).
