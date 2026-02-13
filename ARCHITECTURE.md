ARCHITECTURE
============

LibCMPC is organized around a strict contract-first design that keeps interfaces (Phase 0) decoupled from
implementations. This enables small context windows and modular development.

Directory Layout
----------------

- `include/cmpc/` — ABI contracts (`cmp_api_*`) plus design-system-agnostic public headers (core systems, widgets, a11y,
  media helpers).
- `include/m3/` — Material 3 design-system public headers (widgets, palettes, styling defaults).
- `src/core/` — Core logic: allocators/arena/utf8/log/store/i18n, math/layout/anim/router/path, render/event/gesture/
  scroll/tasks, a11y and predictive back helpers, widgets (visuals/text/icon/text_field/list/extras), and media/helpers
  (image/audio/video decoders, storage/camera/network).
- `src/m3/` — Material 3 implementations: HCT color science, widget styling and components (buttons, navigation, app bars,
  tabs, dialogs, sheets, scaffold, menus, cards, chips, selection controls, progress, date/time pickers).
- `src/backend/<platform>/` — Platform implementations of the `cmp_api_*` V-Tables (null, sdl3, linux/gtk4, cocoa,
  win32, web, ios, android).
- `tests/` — Phase-scoped tests validating behavior and error handling.
- `packaging/` — CMake-driven packaging scripts for desktop, mobile, web, and SDL3 builds.

Render + Event Flow
-------------------

1. **Input**: Backends emit `CMPInputEvent` instances via `CMPWS`.
2. **Dispatch**: `cmp_event_dispatch` routes events to widgets (and navigation/router helpers).
3. **Layout**: Widgets are measured and laid out using `CMPMeasureSpec` and `CMPRect`.
4. **Render**: Render lists are built and executed against `CMPGfx` (rects, text, textures).
5. **Tasks**: Time-based updates (animations, delayed work) run via `CMPTasks`.

Backend Composition
-------------------

Each backend exposes a consistent bundle of interfaces:

- **Window system (`CMPWS`)** for windows, input, and DPI.
- **Graphics (`CMPGfx`)** for rendering primitives and text.
- **Environment (`CMPEnv`)** for IO, sensors, camera, image/audio/video decoders, network, tasks, and timing.

Backends are compiled in behind build flags; missing platform dependencies produce stubs that report unavailable
at runtime via `cmp_*_backend_is_available`.

Widget Composition
------------------

Widgets are plain C structs that embed an `CMPWidget` header with a function V-Table. Widgets can be combined
into render trees and traversed by the render builder. Semantics are carried via `CMPSemantics` for accessibility.

Accessibility + Predictive Back
-------------------------------

- `cmp_a11y` provides a small semantics surface that backends can map to native accessibility APIs.
- `cmp_predictive` reports predictive back gestures (start/progress/commit/cancel) for edge-swipe navigation.

Text Fields
-----------

Text fields are implemented in `src/core/cmp_text_field.c` and build on the text/visuals modules, exposing:

- Decoration container (fill + outline).
- Cursor rendering and blink timing.
- Selection highlight and handles.
- Floating label animation with explicit stepping.
