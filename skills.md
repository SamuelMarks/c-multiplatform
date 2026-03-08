# Skills for LibCMPC Development

## Widget Development
### Create a New Widget
Create a new UI component adhering to the strict C89 LibCMPC widget lifecycle.
1.  **Header (`include/cmpc/`, `include/m3/`, `include/cupertino/`, or `include/f2/`)**:
    *   Define the `Style` struct representing the component's visual properties.
    *   Define the `Widget` struct. The very first member MUST be `CMPWidget widget;` to allow struct casting.
    *   Declare `_<system>_<widget>_style_init`, `_init`, setters, and API functions.
2.  **Source (`src/core/`, `src/m3/`, `src/cupertino/`, or `src/f2/`)**:
    *   Implement static V-Table functions: `measure`, `layout`, `paint`, `event`, `semantics`, and `destroy`.
    *   Implement `_init`: Assign `widget.vtable`, initialize the style, helpers (animators/scroll), and internal state.
    *   **Constraint**: All variable declarations MUST be at the very top of their respective block/function scope (Strict C89). Use `/* */` for comments.

### Implement Layout Logic
Implement the `measure` and `layout` V-Table functions for a widget.
*   **Measure**: Calculate the desired `CMPSize` based on `CMPMeasureSpec` constraints (Unspecified, Exactly, AtMost). Be graceful and compute intrinsic bounds if specs are `CMP_MEASURE_UNSPECIFIED`.
*   **Layout**: Accept new `CMPRect bounds` from the parent. Update internal geometry (e.g., child positions, text metrics) based on these exact bounds.

### Implement Paint Logic
Implement the `paint` V-Table function to emit rendering commands.
*   Use `ctx->gfx` to draw primitives (rects, paths, textures).
*   Use `ctx->gfx->text_vtable` for text rendering.
*   Respect `ctx->clip`. Push/Pop clips if drawing outside bounds is possible.
*   Check for backend capabilities gracefully (e.g., `if (ctx->gfx->draw_path == NULL) return CMP_ERR_UNSUPPORTED;`).

### Implement Semantics (Accessibility)
Implement the `semantics` V-Table function to populate `CMPSemantics`.
*   Map widget state to accessibility roles (button, slider, text field).
*   Provide ARIA-like labels, values, and traits for screen readers.

## Backend Implementation
Backends reside in `src/backend/<platform>/`. If a feature is unavailable on the target OS, return `CMP_ERR_UNSUPPORTED`.

### Implement Graphics Interface (`CMPGfx`)
Map LibCMPC rendering commands to native graphics APIs (GDI, Cairo, CoreGraphics, WebGL).
*   Translate `CMPRect`, `CMPColor`, `CMPPoint` to platform native types.
*   LibCMPC uses a top-left origin coordinate system; apply transforms if the native API differs.

### Implement Window System (`CMPWS`)
Handle OS-level windowing and input.
*   Manage window creation, destruction, DPI scaling, and resize events.
*   Translate platform events (Win32 messages, NSEvents, SDL_Events) into `CMPInputEvent` structs and dispatch them.
*   Handle Predictive Back gestures for edge-swipe navigation (`cmp_predictive`).

### Implement Environment (`CMPEnv`)
Handle OS-level capabilities and I/O.
*   Implement `CMPTasks` for the main event loop and background threads.
*   Map native APIs for storage, camera, network, and clipboard.
*   Integrate native image, audio, and video decoders where applicable.

## Design Systems
### Add a Design System Component
Port a component from Material 3 (`m3`), Apple Cupertino (`cupertino`), or Microsoft Fluent 2 (`f2`).
*   Create a dedicated style struct strictly matching the system's design tokens (e.g., `container_color`, `tonal_elevation`, `typography`).
*   Implement interaction state (pressed, hovered, focused, disabled) update logic in the `event` handler, driving internal animators.
*   Keep logic completely platform-agnostic.

## Visual Documentation & Tooling
### Add to Visual Documentation (`cmp_docgen`)
Update the headless vector documentation generator in `tools/docgen/`.
*   When a new widget is created, write a render test in `tools/docgen/` that initializes the widget and pumps mock events (e.g., hover, press).
*   `cmp_docgen` uses a Null/Mock `CMPGfx` V-Table to intercept drawing commands and output pure SVG strings.

## Testing & Quality
### Write Phase Tests
Create a test executable in `tests/phaseX/` to validate specific logic modules.
*   Use `#ifdef CMP_TESTING` hooks to inject error states (e.g., specific `malloc` failures, backend stub failures).
*   Verify memory leak freedom by checking custom `CMPAllocator` stats if available.
*   Validate behavior strictly against the `cmp_api_*` contract, not the implementation details.