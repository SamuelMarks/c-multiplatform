# Architecture Plan: DOM, Media & Rendering Engine

## 1. DOM & Layout Integration
- [x] Build the DOM Tree structure (handling nodes, text content, attributes).
- [x] Connect the DOM to the CSSOM (from PLAN_CSS_W3C.md) for style resolution.
- [x] Build Layout Tree generator (ignoring `display: none`, expanding anonymous boxes).
- [x] Link to the CSS Layout algorithms (Flexbox, Grid, Block) to compute final geometry.

## 2. Geometry Constraints & Sanity Checking (Window Resize Safety)
To strictly prevent UI elements from bleeding off-screen and ensure true CSS responsiveness during window resizes:
- [x] **Viewport Root Solver:** Force the root `<html>` / `<body>` elements to strictly match the OS window dimensions upon `ui_window_resize` events.
- [x] **Overflow Resolution Pipeline:** Implement the CSS Standard overflow logic (`overflow: hidden|scroll|auto`). If child geometry exceeds parental constraints, explicitly compute scrollbar tracks rather than breaking bounds.
- [x] **Min/Max Clamping Algorithms:** Ensure every layout pass enforces `min-width`, `min-height`, `max-width`, and `max-height` algebraically before computing final `width`/`height` percentages.
- [x] **Containment Sanity Checks:** Inject a debug render pass (e.g., drawing red wireframes) that asserts `child_rect.x + child_rect.width <= parent_rect.width` (accounting for margins/overflow) to mathematically prove the layout constraint engine holds under stress testing.
- [x] **Intrinsic Sizing Adjustments:** Calculate `min-content`, `max-content`, and `fit-content` accurately so shrink-to-fit containers dynamically wrap without pushing siblings off the window edge.

## 3. Component Architecture & Styling API
Before implementing Design Systems, define a rigid, modular approach to creating reusable UI components (conceptually aligning with Angular's encapsulation or Compose's Modifier patterns):
- [x] **Opaque Component State:** Define a `struct ui_component` that holds isolated DOM subtrees to prevent state leakage.
- [x] **Style Injection Pipeline:** Implement an API for passing style payloads into components. Components must support:
  - Default internal stylesheets.
  - User-provided style overrides (via an FFI equivalent of a `Modifier` or injected CSS strings).
- [x] **Scoped CSS (Encapsulation):** Implement logic to generate unique CSS classes (e.g., `.btn-a7f2`) or utilize the CSS `@scope` module so internal widget styling does not pollute the global CSSOM.
- [x] **Property Binding:** Map internal component state to CSS Custom Properties (Variables) so users can pass in theme parameters (e.g., `ui_component_set_property(btn, "--primary-color", "#f00")`).

## 4. Media & Asset Pipeline
- [x] Define `struct ui_media_source` abstract vtable (demuxers, decoders).
- [x] **Audio:** Implement WASAPI, CoreAudio, ALSA, OpenSL ES, WebAudio sinks.
- [x] **Video:** Implement hardware video decoding pipelines (MediaFoundation, AVFoundation, MediaCodec, FFmpeg fallback).
- [x] **A/V Sync:** Slaved execution paradigm—drive renderer clock using Audio PTS.
- [x] **Images:** Implement C89 PNG, JPEG, WebP decoders.
- [x] **SVG:** Write SVG path parser (XML to Bezier/Line commands).
- [x] **SVG:** Implement CPU path flattening/tessellation to emit triangulated geometry.
- [x] Build asynchronous asset streaming queue.

## 5. Rendering & Compositing
- [x] Implement unified OpenGL ES 2.0 / WebGL 1.0 backend.
- [x] Implement Shader compilation manager and caching.
- [x] **Geometry Batching:** Batch UI rects, borders, and SVG triangles.
- [x] **Text:** Integrate `stb_truetype` or FreeType, build dynamic glyph atlas generation.
- [x] **Text:** Implement complex text shaping and layout (BiDi, ligatures).
- [x] **Video:** Implement YUV-to-RGB conversion in fragment shaders.
- [x] **Compositor Pass:** Implement Offscreen Framebuffers (FBOs) for blending and z-index sorting.
- [x] **Effects:** Implement Gaussian Blur shaders (shadows, backdrop-filters).
