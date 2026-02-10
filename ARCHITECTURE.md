ARCHITECTURE
============

LibM3C is organized around a strict contract-first design that keeps interfaces (Phase 0) decoupled from
implementations. This enables small context windows and modular development.

Directory Layout
----------------

- `include/m3/` — ABI contracts (`m3_api_*`) plus public module headers (core systems, widgets, plugins).
- `src/core/` — Pure logic: allocators/arena/utf8/log/store, math/color/layout/anim/router, render/event/tasks,
  widgets (visuals/text/buttons/selection/text_field/list/navigation/dialogs/progress), and plugin helpers
  (storage/camera/network).
- `src/backend/<platform>/` — Platform implementations of the `m3_api_*` V-Tables (null, sdl3, linux/gtk4, cocoa,
  win32, web, ios, android).
- `tests/` — Phase-scoped tests validating behavior and error handling.

Render + Event Flow
-------------------

1. **Input**: Backends emit `M3InputEvent` instances via `M3WS`.
2. **Dispatch**: `m3_event_dispatch` routes events to widgets (and navigation/router helpers).
3. **Layout**: Widgets are measured and laid out using `M3MeasureSpec` and `M3Rect`.
4. **Render**: Render lists are built and executed against `M3Gfx` (rects, text, textures).
5. **Tasks**: Time-based updates (animations, delayed work) run via `M3Tasks`.

Backend Composition
-------------------

Each backend exposes a consistent bundle of interfaces:

- **Window system (`M3WS`)** for windows, input, and DPI.
- **Graphics (`M3Gfx`)** for rendering primitives and text.
- **Environment (`M3Env`)** for IO, sensors, camera, network, tasks, and timing.

Backends are compiled in behind build flags; missing platform dependencies produce stubs that report unavailable
at runtime via `m3_*_backend_is_available`.

Widget Composition
------------------

Widgets are plain C structs that embed an `M3Widget` header with a function V-Table. Widgets can be combined
into render trees and traversed by the render builder. Semantics are carried via `M3Semantics` for accessibility.

Text Fields
-----------

Text fields are implemented in `src/core/m3_text_field.c` and build on the text/visuals modules, exposing:

- Decoration container (fill + outline).
- Cursor rendering and blink timing.
- Selection highlight and handles.
- Floating label animation with explicit stepping.
