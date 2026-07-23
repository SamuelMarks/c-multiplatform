# UI Engine Architecture & Philosophy: Deep Dive

This document is an exhaustive exploration of the C-Multiplatform UI engine's internal mechanics, constraints, and structural design.

## 1. Core Principles & Build Constraints

The engine forces a highly restrictive subset of C to guarantee portability and longevity.

*   **Strict C89 Compilation:** The CMake matrix enforces `-std=c89 -pedantic -Wall -Werror`. We do not rely on C99 or later. This guarantees the engine compiles on embedded systems, ancient toolchains, and modern compilers alike (MSVC, GCC, Clang, Emscripten).
*   **No Typedefs:** We strictly avoid hiding structures and pointers behind `typedef`s (except where required by platform APIs). You will always see `struct ui_window_backend*` and `enum ui_error` explicitly.
*   **No `<stdint.h>`:** To guarantee C89 compliance without requiring compiler extensions, platform-independent integer types are defined natively within the core headers.
*   **Zero UI-Layer Mutexes:** Locking DOM nodes individually destroys 60 FPS rendering. The UI state and DOM are mutated *exclusively* on the main thread.

## 2. Memory Ownership Model

The UI Engine runs in environments where a garbage collector is either unavailable or unacceptable (due to unpredictable frame pauses).

### A. Arena Allocators (`ui_arena`)
Most UI data is highly transient (e.g., CSS layout calculations, string concatenations for text rendering, or anonymous block generation).
*   **Mechanism:** Memory is bumped linearly within large pre-allocated blocks.
*   **Lifecycle:** Individual frees are prohibited. Instead, the entire arena is reset at the end of the frame tick (`ui_arena_reset(&engine->frame_arena)`).

### B. Object Pools
For long-lived entities (e.g., Window handles, Image decoders, OpenGL context handles), object pools are utilized. They offer constant-time `O(1)` allocation and deallocation without fragmenting the system heap.

### C. Reference Counting (Sparse Usage)
Atomic reference counting is limited strictly to cross-thread primitives—specifically `ui_promise` handles. This ensures that a background network request completing after a UI component is destroyed does not write to a dangling pointer.

## 3. Error Handling & The `goto cleanup` Pattern

Errors are first-class citizens. Functions never return void if they can fail, and out-parameters are used for successful payload delivery.

```c
/* Standard Engine Pattern */
enum ui_error process_image(struct ui_engine *engine, struct ui_image **out_image) {
    struct ui_image *img = NULL;
    void *decode_buffer = NULL;
    enum ui_error err = UI_ERROR_NONE;

    if (!engine || !out_image) return UI_ERROR_INVALID_ARGUMENT;

    img = ui_pool_alloc(engine->image_pool);
    if (!img) { err = UI_ERROR_OUT_OF_MEMORY; goto cleanup; }

    decode_buffer = malloc(1024 * 1024);
    if (!decode_buffer) { err = UI_ERROR_OUT_OF_MEMORY; goto cleanup; }

    err = parse_headers(img, decode_buffer);
    if (err != UI_ERROR_NONE) goto cleanup;

    *out_image = img;
    free(decode_buffer);
    return UI_ERROR_NONE;

cleanup:
    /* Deterministic teardown on any failure */
    if (img) ui_pool_free(engine->image_pool, img);
    if (decode_buffer) free(decode_buffer);
    return err;
}
```
**Strict Percolation:** Errors must be checked (`if (err != UI_ERROR_NONE)`) and immediately percolated up the stack.

## 4. Execution & Threading Model

The application architecture is event-driven and utilizes a distinct multi-threaded approach.

*   **Main Execution Context (`ui_execution_context`):** A single-threaded Tick Engine. All DOM diffing, CSS layout passes, and signal effect executions happen here.
*   **Thread-Pool Task Executor (`ui_thread_pool`):** Offloads heavy CPU work (YUV-to-RGB video decoding, JSON parsing, disk I/O, WebP decoding).
*   **Multi-Reactor Event Loop:** Implements Epoll (Linux), Kqueue (macOS/FreeBSD), or IOCP (Windows) integrations for server-driven UI or headless processing. Network socket polling happens here. Socket readiness generates events that are pushed into the main thread's lock-free ring buffer for processing.
*   **Cross-Thread Communication:** Thread pools push `ui_promise` continuations back to the main thread via atomic lock-free ring buffers to execute callbacks safely.

## 5. Reactive State: The Signal Graph

The engine's reactivity eliminates the need for virtual DOM diffing by pushing data precisely where it's needed.

1.  **`ui_signal`:** A primitive wrapper around a value (integer, string, pointer). Modifying it tags it as dirty.
2.  **`ui_computed`:** An algebraic derivation of one or more signals. Automatically caches its value and only recalculates if the underlying signals mutate.
3.  **`ui_effect`:** An anonymous function that runs immediately, tracking which signals it accesses. If those signals change, the effect re-runs. This is how the DOM updates.

### Widget Binding Archetypes
*   **Presentational Widgets:** Use one-way binding. A `ui_badge_base` exposes `ui_badge_base_bind_text(badge, signal)`. Internally, the badge wraps its `ui_dom_node_set_text` call inside a `ui_effect`.
*   **Interactive Controls (CVA):** Components like `ui_slider_base` implement a **Control Value Accessor** (`struct ui_control_value_accessor`). This vtable hooks into the engine's form architecture (`ui_form_group`, `ui_form_control`). It translates underlying model changes (signals) into visual representations, and user interactions (click, drag) into underlying model patches—safely breaking infinite update cycles.

## 6. Layout & Geometry Constraints

Responsive design requires strict algebraic guarantees to prevent elements from bleeding off-screen.

*   **Viewport Root Solver:** Forces `<html>` and `<body>` to strictly match the OS window width/height at all times.
*   **Overflow Resolution:** The CSS `overflow` property (`hidden`, `scroll`, `auto`) is explicitly enforced. If child dimensions exceed parental constraints, the engine computes virtual scrollbars and clipping boxes instead of allowing bounds violations.
*   **Intrinsic Sizing:** Computes `min-content`, `max-content`, and `fit-content` so text dynamically wraps without pushing siblings off the screen edge.
*   **Algebraic Clamping:** Before percentage layouts are solved, `min-width` and `max-width` constraints are strictly applied.

## 7. Media & A/V Sync

*   **Clock Slaving:** To ensure flawless video playback, the engine's main clock is slaved to the Audio Presentation Timestamp (PTS). Audio sinks (WASAPI, CoreAudio, ALSA) govern the heartbeat.
*   **Decoders:** Incorporates C89-compliant image decoders (PNG, JPEG, WebP) and handles video pipeline hardware fallback.
*   **Vector Graphics:** SVG paths are parsed from XML strings to Bezier/Line commands. A CPU tessellator flattens these paths into triangles for the GPU.

## 8. Rendering, Compositing, and FFI Modular Dependencies

*   **Renderer:** A unified OpenGL ES 2.0 / WebGL 1.0 backend utilizing batched rendering. It avoids excessive draw calls by batching rects, text glyphs (via `stb_truetype` or FreeType), and SVG triangles.
*   **Compositor Pass:** Offscreen Framebuffers (FBOs) are used for z-index sorting, Gaussian Blurs (backdrop filters, shadows), and blending complex opacity layers.
*   **FFI FetchContent:** Dependencies like `c-abstract-http` (Network), `c-fs` (Filesystem), and `c-orm` (Persistence) are strictly integrated via CMake `FetchContent`. They act as abstract interfaces protecting the engine from OS-specific I/O quirks.
