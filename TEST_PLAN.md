# C-Multiplatform UI Testing Framework Plan (Maximum Exhaustion Edition)

## 1. Memory, Allocators, and Core Lifecycle

### 1.1 `ui_arena.h` (Arena Allocator)
- [x] Test exact 4-byte alignment on 32-bit systems.
- [x] Test exact 8-byte alignment on 64-bit systems.
- [x] Test exact 16-byte alignment for SIMD vector types.
- [x] Test allocation larger than default block size (triggers large-block fallback).
- [x] Test nested `ui_arena_save` and `ui_arena_restore` up to 100 levels deep.
- [x] Test `ui_arena_reset` strictly zeros out memory (if in debug mode).
- [x] Test OOM handling when system `malloc` fails during block expansion.

### 1.2 `ui_pool.h` (Object Pools)
- [x] Test pool initialization with variable element sizes (16, 32, 64, 128 bytes).
- [x] Test complete exhaustion of pool capacity.
- [x] Test dynamic chunk allocation when capacity is exceeded.
- [x] Test object recycling (alloc, free, alloc ensures same memory address is reused).
- [x] Test fragmentation metrics (verify pool does not permanently fragment on random frees).

### 1.3 `ui_ring_buffer.h` & `ui_atomic.h`
- [x] Test single-producer, single-consumer thread safety.
- [x] Test multi-producer, single-consumer spinlock safety.
- [x] Test ring buffer wrap-around math at exact byte boundaries.
- [x] Test `ui_atomic_compare_and_swap` with 32-bit integers.
- [x] Test `ui_atomic_compare_and_swap` with 64-bit pointers.
- [x] Test `ui_atomic_fetch_add` across 100 threads concurrently.

### 1.4 `ui_promise.h` & `ui_execution_context.h`
- [x] Test promise resolution synchronously.
- [x] Test promise resolution asynchronously (next tick).
- [x] Test promise rejection bubbling up the `.catch` chain.
- [x] Test `.finally` executes regardless of resolution/rejection.
- [x] Test memory bounds: 10,000 chained promises do not stack overflow.
- [x] Test execution context cancellation (cancelling pending timers/promises).
- [x] Test Execution Paradigm: Simulate Single-Threaded environment.
- [x] Test Execution Paradigm: Simulate Thread-Pool environment.
- [x] Test Execution Paradigm: Simulate Multi-Reactor environment.

## 2. CSSOM, Styling, and the DOM Engine

### 2.1 CSS Parsing (`ui_css_parser.h`)
- [x] Parse universal selector `*`.
- [x] Parse tag selector `div`.
- [x] Parse class selector `.btn`.
- [x] Parse ID selector `#main`.
- [x] Parse attribute selector `[type="text"]`, `[href^="https"]`, `[data-active]`.
- [x] Parse pseudo-classes: `:hover`, `:active`, `:focus`, `:focus-visible`, `:disabled`.
- [x] Parse structural pseudo-classes: `:first-child`, `:last-child`, `:nth-child(even)`.
- [x] Parse combinators: descendant (space), child (`>`), next-sibling (`+`), subsequent-sibling (`~`).
- [x] Test parser recovery on missing semicolons.
- [x] Test parser recovery on invalid property names (ignored).
- [x] Test parser recovery on unclosed brackets.

### 2.2 Values and Units (`ui_css_values.h`)
- [x] Evaluate `px` (exact device independent pixels).
- [x] Evaluate `em` (relative to parent font size).
- [x] Evaluate `rem` (relative to root font size).
- [x] Evaluate `vw` / `vh` (relative to viewport dimensions).
- [x] Evaluate `vmin` / `vmax`.
- [x] Evaluate `%` (relative to containing block).
- [x] Evaluate colors: `#RGB`, `#RRGGBB`, `#RRGGBBAA`.
- [x] Evaluate colors: `rgb(r, g, b)`, `rgba(r, g, b, a)`.
- [x] Evaluate colors: `hsl(h, s, l)`, `hsla(h, s, l, a)`.
- [x] Evaluate `calc(100% - 20px)`.
- [x] Evaluate nested `calc(min(50%, 200px) + 1rem)`.
- [x] Evaluate `clamp(100px, 50%, 500px)`.

### 2.3 Layout Module: Box Model & Display
- [x] Geometry Assertions: Trigger layout pass and assert exact `x, y, width, height` mathematically for responsive scaling (smartwatch to TV dimensions).
- [x] Test `display: none` (element and children removed from layout tree).
- [x] Test `display: block` (takes full width, starts new line).
- [x] Test `display: inline` (flows with text, ignores width/height).
- [x] Test `display: inline-block` (flows with text, respects width/height).
- [x] Test `box-sizing: content-box` (padding/border expands size).
- [x] Test `box-sizing: border-box` (padding/border inset into size).
- [x] Test margin collapsing between adjacent siblings.
- [x] Test margin collapsing between parent and first/last child.
- [x] Test negative margins overlapping adjacent elements.

### 2.4 Layout Module: Flexbox
- [x] Test `flex-direction: row`, `row-reverse`, `column`, `column-reverse`.
- [x] Test `flex-wrap: nowrap`, `wrap`, `wrap-reverse`.
- [x] Test `justify-content: flex-start`, `flex-end`, `center`, `space-between`, `space-around`, `space-evenly`.
- [x] Test `align-items: stretch`, `flex-start`, `flex-end`, `center`, `baseline`.
- [x] Test `align-content: flex-start`, `flex-end`, `center`, `space-between`, `stretch`.
- [x] Test `align-self` overriding parent `align-items`.
- [x] Test `flex-grow` proportional free space distribution.
- [x] Test `flex-shrink` proportional overflow resolution.
- [x] Test `flex-basis: auto` vs explicit lengths.
- [x] Test `order` modifying visual rendering order vs DOM order.

### 2.5 Layout Module: Grid
- [x] Test `grid-template-columns` explicit pixel tracks.
- [x] Test `grid-template-columns` fractional `fr` units.
- [x] Test `grid-template-columns: repeat(3, 1fr)`.
- [x] Test `grid-template-columns: repeat(auto-fit, minmax(100px, 1fr))`.
- [x] Test `grid-row-start` and `grid-row-end` placement.
- [x] Test `grid-column-start` and `grid-column-end` placement.
- [x] Test overlapping grid items and `z-index` resolution.
- [x] Test `gap`, `row-gap`, `column-gap`.

### 2.6 Positioning & Z-Index
- [x] Test `position: static`.
- [x] Test `position: relative` (offsets without altering layout flow).
- [x] Test `position: absolute` (relative to nearest positioned ancestor).
- [x] Test `position: fixed` (relative to viewport).
- [x] Test `position: sticky` (switches from relative to fixed at scroll thresholds).
- [x] Test stacking context creation (via `opacity < 1`, `z-index`, `transform`).
- [x] Test z-index sorting within the same stacking context.
- [x] Test z-index isolation (nested contexts cannot interleave).

## 3. Interaction & Input Events (`ui_event.h`)

### 3.1 Mouse Vectors
- [x] Event: `mouseenter` (fires once on boundary cross).
- [x] Event: `mouseleave` (fires once on boundary exit).
- [x] Event: `mousemove` (fires continuously).
- [x] Event: `mousedown` (left, middle, right buttons).
- [x] Event: `mouseup` (left, middle, right buttons).
- [x] Event: `click` (requires mousedown + mouseup on same element).
- [x] Event: `dblclick` (requires two clicks within OS time threshold).
- [x] Event: `contextmenu` (right click or specific keyboard key).
- [x] Event: `wheel` (deltaX, deltaY, deltaZ, pixel mode, line mode, page mode).

### 3.2 Keyboard Vectors
- [x] Event: `keydown` (fires continuously if held).
- [x] Event: `keyup`.
- [x] Event: `keypress` (legacy, for character insertion).
- [x] Modifier: `Shift` held detection.
- [x] Modifier: `Ctrl` / `Cmd` held detection.
- [x] Modifier: `Alt` / `Option` held detection.
- [x] Test `Tab` key routing focus sequentially.
- [x] Test `Shift+Tab` key routing focus backwards.

### 3.3 Touch & Gesture Vectors (`ui_gesture.h`)
- [x] Event: `touchstart`, `touchmove`, `touchend`, `touchcancel`.
- [x] Gesture: `Pan` (1 finger, directional constraints).
- [x] Gesture: `Pinch` (2 fingers, scale center point calculation).
- [x] Gesture: `Rotation` (2 fingers, angle calculation).
- [x] Gesture: `Swipe` (velocity exceeding minimum dp/ms).
- [x] Gesture: `LongPress` (duration exceeding 500ms without movement).

### 3.4 Drag and Drop (`ui_drag_drop.h`)
- [x] Test `draggable="true"` attribute parsing.
- [x] Event: `dragstart` (attaching payload).
- [x] Event: `drag` (firing continuously during move).
- [x] Event: `dragenter` (crossing dropzone boundary).
- [x] Event: `dragover` (continuous over dropzone, `preventDefault()` required to allow drop).
- [x] Event: `dragleave` (exiting dropzone).
- [x] Event: `drop` (payload delivery).
- [x] Event: `dragend` (cleanup).
- [x] Visual: Test drag proxy bitmap rendering under cursor.

## 4. Comprehensive Component Verification

*Each component must be tested for Default, Hover, Active, Focus, Disabled states, LTR/RTL, and Memory Leaks.*

### 4.1 Buttons & Indicators
- [x] `ui_button_base`: Text-only, Icon-only, Text+Icon alignments.
- [x] `ui_button_base`: Ripple effect expansion math and clipping.
- [x] `ui_fab_base`: Floating offset, scroll-to-hide animation.
- [x] `ui_badge_base`: Top-right anchor, Top-left anchor (RTL), value truncation (`999` -> `99+`).
- [x] `ui_progress_base`: Determinate % bar filling.
- [x] `ui_progress_base`: Indeterminate CSS animation looping.
- [x] `ui_skeleton_base`: Shimmer gradient CSS mask animation.

### 4.2 Inputs & Controls
- [x] `ui_input_base`: Caret blinking timing, caret positioning between characters.
- [x] `ui_input_base`: Text selection highlighting (mouse drag).
- [x] `ui_input_base`: Text selection highlighting (Shift+Arrow Keys).
- [x] `ui_input_base`: Copy/Paste/Cut clipboard integration.
- [x] `ui_input_mask`: Phone number masking `(XXX) XXX-XXXX`.
- [x] `ui_checkbox_base`: Checked animation, indeterminate (dash) rendering.
- [x] `ui_radio_group_base`: Mutual exclusivity logic.
- [x] `ui_slider_base`: Drag thumb, click track to seek, Arrow key increments.
- [x] `ui_range_slider_base`: Min/Max thumbs pushing each other.
- [x] `ui_switch_base`: Thumb translation animation, track color transition.

### 4.3 Menus & Navigation
- [x] `ui_tabs_base`: Ink bar translation between variable-width tabs.
- [x] `ui_tabs_base`: Content pane swipe physics.
- [x] `ui_menu_base`: Escape to close, click outside to close.
- [x] `ui_menu_base`: Screen edge collision (flip from dropping down to dropping up).
- [x] `ui_context_menu_base`: Spawn at absolute pointer (X,Y).
- [x] `ui_bottom_nav_base`: Icon shift animation on active state.
- [x] `ui_breadcrumbs_base`: Ellipsis injection when container width shrinks.
- [x] `ui_tree_base`: Expand/collapse state management, dynamic child loading.

### 4.4 Data & Content
- [x] `ui_datagrid_base`: Column resize dragging.
- [x] `ui_datagrid_base`: Ascending/Descending sort indicator logic.
- [x] `ui_datagrid_base`: Sticky column headers during vertical scroll.
- [x] `ui_datagrid_base`: Sticky row headers during horizontal scroll.
- [x] `ui_avatar_base`: Initials extraction algorithm (e.g., "John Doe" -> "JD").
- [x] `ui_carousel_base`: Snap-to-center physics, pagination dot sync.

### 4.5 Overlays & Windows
- [x] `ui_dialog_base`: Z-index stacking (dialog over dialog).
- [x] `ui_dialog_base`: Focus trap (tab loops within dialog).
- [x] `ui_dialog_base`: Backdrop blurring effect (if supported by renderer).
- [x] `ui_bottom_sheet_base`: Drag down to dismiss, velocity threshold.
- [x] `ui_toast_manager_base`: Multi-toast queueing, auto-dismiss timeout reset on hover.
- [x] `ui_tooltip_base`: Mouse hover entry delay (e.g., 300ms), immediate exit.

## 5. Typography, I18n, and A11y

### 5.1 Fonts & Text Layout (`ui_text_layout.h`)
- [x] Test glyph caching efficiency (measure texture atlas fragmentation).
- [x] Test line wrapping on explicit `\n`.
- [x] Test line wrapping on spaces (standard).
- [x] Test line wrapping on CJK characters (break anywhere without spaces).
- [x] Test `text-overflow: ellipsis`.
- [x] Test `word-break: break-all`.
- [x] Test HarfBuzz ligature resolution (e.g., `f` + `i` -> `ﬁ`).
- [x] Test Emoji rendering and fallback font scaling.

### 5.2 Accessibility (`ui_aria.h`)
- [x] Test `aria-hidden="true"` removes node from semantic tree.
- [x] Test `aria-label` overrides visual text in semantic tree.
- [x] Test `aria-expanded` toggles report correctly to OS (UIAutomation/NSAccessibility).
- [x] Test `role="button"`, `role="checkbox"`, `role="dialog"`, `role="alert"`.
- [x] Test focus rings draw above all other content.
- [x] Test contrast ratio validator reports errors for `< 4.5:1` (AA standard).

### 5.3 Internationalization (`ui_i18n.h`, `ui_bidi_manager.h`)
- [x] LTR Layout mathematical verification (X starts at 0, grows right).
- [x] RTL Layout mathematical verification (X starts at width, grows left).
- [x] Number formatting: USA `1,000.50`, Germany `1.000,50`.
- [x] Date formatting: USA `MM/DD/YYYY`, UK `DD/MM/YYYY`, ISO `YYYY-MM-DD`.
- [x] Translation interpolation: `Hello, {name}`.

## 6. Graphics, Media, and Rendering Pipeline

### 6.1 Rendering Backends
- [x] `ui_renderer_gles2.h`: Validate Vertex Buffer Object (VBO) batching (merging quads).
- [x] `ui_renderer_gles2.h`: Validate clipping via `glScissor`.
- [x] `ui_renderer_gles2.h`: Validate complex clipping via Stencil buffer (border-radius).
- [x] `ui_renderer_gles_fallback.h`: Validate software rasterizer alpha blending math.
- [x] `ui_shader_manager.h`: Test uniform injection (Matrices, Colors, Floats).

### 6.2 Visual Effects
- [x] Test drop-shadow rendering, opacity fading, and offset limits.
- [x] Test `filter: blur()` Gaussian kernel performance on large rects.
- [x] Test `filter: invert()`, `grayscale()`, `sepia()`.
- [x] Test SVG (`ui_svg.h`) path drawing (bezier curve flattening algorithms).

### 6.3 Media
- [x] `ui_image_decoder.h`: Validate PNG transparency.
- [x] `ui_image_decoder.h`: Validate JPEG EXIF orientation (rotation/flipping).
- [x] `ui_image_decoder.h`: Validate WEBP animated decoding loop.
- [x] `ui_video_player_base.h`: Validate A/V sync timestamps (`ui_av_sync.h`).
- [x] `ui_camera_base.h`: Validate YUV to RGB shader conversion.

## 7. Platform & Compilation Quality Matrix

### 7.1 Cross-Compiler C89 Compatibility Checks
- [x] Validate POSIX/C99 headers (`<stdint.h>`, `<stdbool.h>`, `<unistd.h>`) are properly `#ifdef` guarded for older MSVC targets.
- [x] Validate every public header is wrapped in `#ifdef __cplusplus extern "C" { #endif` (Max 1 per file).
- [x] Validate format specifiers use cross-platform macros (e.g., `#define NUM_FORMAT "%I64d"` for MSVC vs `"%lld"`).

### 7.2 Windows & MSVC Optimizations
- [x] Validate NO occurrences of `#include <windows.h>` in core cross-platform headers to prevent bloat.
- [x] Validate MSVC Safe CRT fallbacks (`sprintf_s`, `strcpy_s`) are properly `#if defined(_MSC_VER)` guarded against C89 fallbacks.

### 7.3 Formatting Constraints
- [x] Validate every `#include` block is wrapped in `/* clang-format off */` and `/* clang-format on */` to preserve strict dependencies. (Max 1 occurrence per file).

### 7.4 CMake Configuration Matrix
- [x] Test CMake Build: MSVC 2005, 2022, 2026.
- [x] Test CMake Build: MinGW, Cygwin, Clang, GCC (Windows, macOS, Linux).
- [x] Test CMake Toggles: CRT Linkage (Static `/MT` vs Shared `/MD`).
- [x] Test CMake Toggles: Charsets (UNICODE vs ANSI).
- [x] Test CMake Toggles: Threading (Multi vs Single-threaded).
- [x] Test CMake Toggles: Link-Time Optimization (LTO).
- [x] Test CMake Toggles: MSVC Runtime Checks (`/RTC1`, `/RTCs`, `/RTCu`).

### 7.5 FFI Bindings & Documentation Automation
- [x] Run Documentation generator: Ensure 100% of `struct`, `enum`, and exported FFI functions have Doxygen/kernel-doc comments.
- [x] Run Rust Binding generator: Validate `sys` crate wrapper output.
- [x] Run C# (.NET) Binding generator: Validate `DllImport` P/Invoke output.
- [x] Run Python/Go Binding generator: Validate C-extension scaffolding.

## 8. Continuous Integration & Automation Framework

### 8.1 Headless Execution Matrix
- [x] Run 100% of Layout Box Model tests in `ui_e2e_headless_ctx`.
- [x] Run 100% of Component State tests in `ui_e2e_headless_ctx`.
- [x] Execute WebDriver-style scripted flows (`input(invalid) -> assert -> input(valid) -> submit -> assert`).
- [x] Run `ui_e2e_advance_time` logic to verify CSS animations finish at exact bounds.

### 8.2 Headful E2E Automation
- [x] Run automated suite launching actual OS Window (OpenGL/WebGL).
- [x] Inject synthetic OS-level mouse clicks and keyboard events to verify physical graphics/input pipeline.

### 8.3 Snapshot & Regression (Pixel-Perfect)
- [x] Generate 500+ component snapshot PNGs via `stb_image_write.h`.
- [x] Run C-based per-pixel Diff Engine against baseline folder.
- [x] Force failure on `Delta E > 2.0`.
- [x] Automatically write `diff_mask_XYZ.png` for all failures.

### 8.4 Fuzzing, Pre-commits & Memory Profiling
- [x] Pre-commit hook: Validate Layout assertions, linters, and `clang-format` on every commit.
- [x] Fuzz CSS Parser: 10,000,000 randomized string injections via `libFuzzer`.
- [x] Fuzz HTML/DOM Builder: 10,000,000 randomized malformed node hierarchies.
- [x] Fuzz Image Decoder: Feed random byte streams to JPEG/PNG parsers.
- [x] CI Job: Linux Valgrind Leak Check (Must exit with 0 leaks in Arena/Pool).
- [x] CI Job: Linux AddressSanitizer (ASan) Run.
- [x] CI Job: Linux ThreadSanitizer (TSan) Run.
- [x] CI Job: Linux UndefinedBehaviorSanitizer (UBSan) Run.
