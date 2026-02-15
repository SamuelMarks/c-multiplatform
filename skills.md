# Skills for LibCMPC Development

## Widget Development
### Create a New Widget
Create a new UI component adhering to the C89 LibCMPC widget lifecycle.
1.  **Header (`include/cmpc/` or `include/m3/`)**:
    *   Define the `Style` struct.
    *   Define the `Widget` struct (first member must be `CMPWidget widget;`).
    *   Declare `_style_init`, `_init`, setters, and API functions.
2.  **Source (`src/core/` or `src/m3/`)**:
    *   Implement static V-Table functions: `measure`, `layout`, `paint`, `event`, `get_semantics`, `destroy`.
    *   Implement `_init`: Assign `widget.vtable`, initialize style, helpers (animators/scroll), and state.
    *   **Constraint**: All variable declarations must be at the very top of functions.

### Implement Layout Logic
Implement the `measure` and `layout` V-Table functions for a widget.
*   **Measure**: Calculate `CMPSize` based on `CMPMeasureSpec` width/height and internal content/padding. Be graceful if specs are `CMP_MEASURE_UNSPECIFIED`.
*   **Layout**: Accept new `CMPRect bounds`. Update internal geometry (e.g., child positions, text metrics). Don't perform heavy logic here if possible.

### Implement Paint Logic
Implement the `paint` V-Table function.
*   Use `ctx->gfx` to draw primitives (rects, lines, textures).
*   Use `ctx->gfx->text_vtable` for text rendering.
*   Respect `ctx->clip`. Push/Pop clips if drawing outside bounds is possible.
*   Check for backend capabilities (e.g., `if (vtable->draw_rect == NULL) return CMP_ERR_UNSUPPORTED;`).

## Backend Implementation
### Implement Graphics Interface (`CMPGfx`)
Implement a specific function in a backend graphics V-Table (e.g., `src/backend/win32/cmp_backend_win32.c`).
*   Map LibCMPC `CMPRect`, `CMPColor`, `CMPPoint` to platform native types (GDI, Cairo, SDL).
*   Handle coordinate system transformations if necessary (LibCMPC uses top-left origin).
*   Return `CMP_OK` on success or platform-mapped errors.

### Implement Window System (`CMPWS`)
Implement windowing operations.
*   Handle window creation, destruction, resize events, and input polling.
*   Translate platform events (Win32 messages, NSEvents, SDL_Events) into `CMPInputEvent` structs.

## Material 3 System
### Add Design System Component
Port a Material 3 component (from spec/Android Compose) to C.
*   Create a dedicated style struct matching M3 tokens (e.g., `container_color`, `tonal_elevation`).
*   Implement `m3_<widget>_style_init` to set specification defaults.
*   Implement interaction state (pressed, hovered, disabled) update logic in the `event` handler.

## Testing & Quality
### Write Phase Tests
Create a test executable in `tests/phaseX/` to validate specific logic.
*   Use `#ifdef CMP_TESTING` hooks to inject error states (e.g., specific `malloc` failures, backend stub failures).
*   Verify memory leak freedom by checking allocator stats if available.
*   Validate behavior against the API contract, not the implementation details.