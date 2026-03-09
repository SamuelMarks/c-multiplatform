# Skills for LibCMPC Development

## 1. Core Principles & Strict C89 Constraints
When executing tasks in this repository, always strictly adhere to these C89 constraints:
*   **Variable Declarations**: All variables MUST be declared at the very beginning of their enclosing block scope. 
*   **Comments**: Use `/* ... */` for comments. Never use C99-style `//` comments.
*   **Dependencies**: Rely only on standard ANSI C libraries. Do not include external dependencies in the core framework.
*   **Error Handling**: Return `int` status codes (`CMP_OK` for success, or specific `CMP_ERR_*` values like `CMP_ERR_UNSUPPORTED`).
*   **Allocators**: Rely on the injected `CMPAllocator` contexts. Avoid hidden global state and ensure leak-free handling.

## 2. Widget Creation Lifecycle
Create a new UI component adhering to the strictly decoupled C89 LibCMPC widget lifecycle.

### A. Header Definition (`include/{cmpc,m3,cupertino,f2}/`)
*   **Style Struct**: Define `CMP<System><Widget>Style` representing visual properties (colors, typography, radii).
*   **Widget Struct**: Define the struct. **Crucial Rule**: The very first member MUST be `CMPWidget widget;` to allow safe generic struct casting.
*   **API Signatures**: Declare initialization functions (`_init`, `_style_init`), setters, and specific API interactions.

### B. Source Implementation (`src/{core,m3,cupertino,f2}/`)
*   **V-Table Setup**: Implement static functions for `measure`, `layout`, `paint`, `event`, `semantics`, and `destroy`.
*   **Measure**: Calculate `CMPSize` based on `CMPMeasureSpec` constraints (Unspecified, Exactly, AtMost).
*   **Layout**: Accept final `CMPRect bounds` from the parent to update internal node geometry and text metrics.
*   **Paint**: 
    *   Use `ctx->gfx` for drawing primitives.
    *   Push/Pop clips (`ctx->clip`) if drawing outside bounds is possible.
    *   Check capabilities gracefully (e.g., `if (!ctx->gfx->draw_path) return CMP_ERR_UNSUPPORTED;`).
*   **Event**: Handle `CMPInputEvent` interactions (hover, press, focus) and drive internal `cmp_anim` animators.
*   **Semantics**: Populate `CMPSemantics` mapping to accessibility roles for screen readers (VoiceOver, UIA).

## 3. Design System Fidelity
When modifying or adding widgets, adhere to the specific requirements of the target design system:

### Material Design 3 (`m3`)
*   **Color & Theming**: Leverage `m3_color.h` HCT (Hue, Chroma, Tone) science for tonal palettes and dynamic colors.
*   **Layouts**: Support Canonical Layouts and Adaptive Window classes where applicable.
*   **Motion**: Use the `m3_motion.h` predefined easing curves (Emphasized, Standard) and integrate predictive back.

### Apple Cupertino (`cupertino`)
*   **Geometry**: Use Apple's continuous curves (`kCACornerCurveContinuous` / squircles) over standard circular arcs.
*   **Motion**: Ensure interactions feel native by using interruptible spring physics (`cmp_spring_init`).
*   **Materials**: Emulate iOS/macOS vibrancy and background blurs, mapping them structurally so backends like macOS can hook into `NSVisualEffectView`.
*   **Accessibility**: Heavily map traits to dynamic type sizing and VoiceOver interactions.

### Microsoft Fluent 2 (`f2`)
*   **Materials**: Integrate with Mica and Acrylic backdrops, mapping deeply to Windows Win32 backends (`DWMWA_SYSTEMBACKDROP_TYPE`).
*   **Elevation**: Follow standard Fluent 2 shadow ramps (Levels 2, 4, 8, 16, 64) and 1px structural borders.
*   **Typography**: Utilize the Fluent specific scale mapped optimally to `Segoe UI Variable`.

## 4. Backend Implementations (`src/backend/<platform>/`)
Backends implement the ABI V-Tables. If an OS feature is missing, return `CMP_ERR_UNSUPPORTED` rather than failing.
*   **Graphics Interface (`CMPGfx`)**: Map rendering commands to native APIs (GDI, Cairo, CoreGraphics, WebGL). Ensure top-left origin coordinates.
*   **Window System (`CMPWS`)**: Manage windowing, DPI scales, input event translation to `CMPInputEvent`, and predictive back edge-swipes.
*   **Environment (`CMPEnv`)**: Implement main event loops (`CMPTasks`) and map native OS capabilities (storage, camera, network, clipboard).

## 5. Visual Documentation (`cmp_docgen`)
*   LibCMPC uses a headless vector renderer for documentation. It mocks the `CMPGfx` V-Table to intercept drawing commands and output pure SVG strings.
*   When a new widget is created, **always write a render test in `tools/docgen/`** that initializes the widget and pumps mock events (e.g., hover, press) to generate the visual documentation.

## 6. Testing & Quality (`tests/phaseX/`)
*   Create test executables for each specific logic module phase.
*   Use `#ifdef CMP_TESTING` hooks to inject error states (e.g., specific `malloc` failures, backend stub failures).
*   Verify memory leak freedom by checking custom `CMPAllocator` stats if available.
*   Validate behavioral correctness strictly against the `cmp_api_*` contract, not the implementation details.