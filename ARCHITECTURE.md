ARCHITECTURE
============

LibM3C is organized around a strict contract-first design that keeps interfaces (Phase 0) decoupled from
implementations. This enables small context windows and modular development.

Directory Layout
----------------

- `include/m3/` — Public ABI: types, V-Tables, and widget contracts.
- `src/core/` — Pure logic (math, layout, animation, widgets). No platform dependencies.
- `src/backend/` — Platform glue for window systems, graphics, and text.
- `tests/` — Phase-scoped tests validating behavior and error handling.

Render + Event Flow
-------------------

1. **Input**: Backends emit `M3InputEvent` instances.
2. **Dispatch**: `m3_event_dispatch` routes events to widgets via their V-Tables.
3. **Layout**: Widgets are measured and laid out using `M3MeasureSpec` and `M3Rect`.
4. **Paint**: Widgets draw via `M3Gfx` and `M3TextVTable`.

Widget Composition
------------------

Widgets are plain C structs that embed an `M3Widget` header with a function V-Table. Widgets can be combined
into render trees and traversed by the render builder.

Text Fields
-----------

Text fields are implemented in `src/core/m3_text_field.c` and expose:

- Decoration container (fill + outline).
- Cursor rendering and blink timing.
- Selection highlight and handles.
- Floating label animation with explicit stepping.
