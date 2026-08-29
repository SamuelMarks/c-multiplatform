# Exhaustive Multipass Layout Architecture Plan (Strict C89 & Core Mandate Compliant)

## 1. Executive Summary & Architecture Paradigm
Currently, `c-multiplatform` utilizes a simplified, single-pass conceptual model for flex layout. Without an intrinsic measurement pass, leaf nodes (Text, Buttons, Sliders) with `width = -1.0f` and `height = -1.0f` (auto-size) evaluate to `0x0`. 

To solve this, we are migrating to a **Multipass Intrinsic Measurement Architecture**. This plan ensures that all changes rigidly adhere to the project's Core Workflow Mandates, specifically strict ISO C90 (C89) compliance, assertive error percolation, 100% Doxygen/Test coverage, and MSVC/cross-platform build stability.

### The Three Layout Passes:
1. **Measurement Pass (Bottom-Up):** Traverses the tree. If a node has a registered `measure_cb` (leaf node), the engine queries its content bounds given a maximum available width.
2. **Flex Resolution Pass (Top-Down):** The engine distributes remaining space (handling `flex_grow`, `flex_shrink`), wraps flex lines, and determines the final `width` and `height` of every node.
3. **Positioning Pass (Top-Down):** Alignment properties (`justify_content`, `align_items`, `align_self`) determine absolute `x` and `y` coordinates.

---

## 2. Phase 1: Struct & Header Definitions (`cmp.h` & `cmp_theme_dispatch.h`)
All header modifications must enforce C89 compliance, strict include guarding, and C++ interop wrapping.

### 2.1 Header Integrity & Formatting
- [x] Verify `include/cmp.h` has exactly one `/* clang-format off */` and `/* clang-format on */` block around all `#include` statements.
- [x] Verify POSIX/C99 headers (`<stdint.h>`, `<stdbool.h>`, `<unistd.h>`) are properly guarded for older MSVC 2005 compatibility if touched.
- [x] Verify the file is wrapped exactly once in `#ifdef __cplusplus` `extern "C" {` and `#endif /* __cplusplus */`.

### 2.2 Adding the Callback Typedef (`cmp.h`)
- [x] Define the measurement callback signature. Per mandates, it **MUST** return `int` (error code), and output measurements via pointers (`out_width`, `out_height`).
  ```c
  /**
   * @brief Callback to measure a layout node's intrinsic dimensions.
   * @param context Opaque pointer to the UI node or related context.
   * @param max_width The maximum available horizontal space for wrapping.
   * @param out_width Pointer to receive the calculated intrinsic width.
   * @param out_height Pointer to receive the calculated intrinsic height.
   * @return 0 (CMP_SUCCESS) on success, or a non-zero error code.
   */
  typedef int (*cmp_layout_measure_cb_t)(void *context, float max_width, float *out_width, float *out_height);
  ```
- [x] Attain 100% Doxygen coverage for the new typedef.

### 2.3 Modifying `cmp_layout_node_t` (`cmp.h`)
- [x] Add `cmp_layout_measure_cb_t measure_cb;` to `cmp_layout_node_t`.
- [x] Add `void *measure_ctx;` to `cmp_layout_node_t`.
- [x] Add 100% Doxygen coverage `/* ... */` block comments for the new fields.

---

## 3. Phase 2: Core Layout Engine Implementation (`src/core/cmp_layout.c`)
Modifications to the layout engine must follow strict C89 scope declarations and assertive error percolation.

### 3.1 Strict C89 Declarations
- [x] In `cmp_layout_measure_pass`, ensure all local variables (`int rc;`, `float intrinsic_w;`, `float intrinsic_h;`, `size_t i;`) are declared strictly at the top of the function scope. No inline declarations.
- [x] Verify no `//` comments exist; strictly use `/* ... */`.

### 3.2 Updating `cmp_layout_measure_pass`
- [x] Update function to process measurements. If `node->measure_cb != NULL`:
  - [x] Execute callback: `rc = node->measure_cb(node->measure_ctx, max_available_width, &intrinsic_w, &intrinsic_h);`
  - [x] Assertive percolation: `if (rc != CMP_SUCCESS) { LOG_DEBUG("measure_cb failed: %d\n", rc); return rc; }`
- [x] Calculate `max_available_width` accurately by subtracting parent padding and margins.
- [x] Enforce intrinsic constraint clamping: Clamp the returned `intrinsic_w` against `node->max_width` and `node->min_width`.
- [x] For containers (no `measure_cb`), recursively call `cmp_layout_measure_pass` on children and aggregate dimensions to form intrinsic sizes.
- [x] Ensure the overall `cmp_layout_calculate` orchestrator checks the `rc` of `cmp_layout_measure_pass` and returns it upward.

---

## 4. Phase 3: The UI Layer Dispatcher (`src/cmp_ui.c`)
We must bridge the generic layout engine to theme-aware UI measuring via a central dispatcher.

### 4.1 Dispatch Implementation
- [x] Create `int cmp_ui_layout_measure_dispatch(void *ctx, float max_width, float *out_w, float *out_h)`.
- [x] Attain 100% Doxygen coverage for the function.
- [x] C89 Compliance: Declare `int rc = CMP_SUCCESS; cmp_ui_node_t *node;` at the top of the scope.
- [x] Safe casting: `if (!ctx || !out_w || !out_h) return CMP_ERROR_INVALID_ARG; node = (cmp_ui_node_t *)ctx;`
- [x] Implement a `switch (node->type)` block routing to the correct `cmp_theme_measure_*` function:
  - [x] `case CMP_UI_TYPE_TEXT:` Route to theme text measurement (ensure text wrapping handles `max_width`).
  - [x] `case CMP_UI_TYPE_BUTTON:` `rc = cmp_theme_measure_button(node, out_w, out_h);`
  - [x] `case CMP_UI_TYPE_CHECKBOX:` `rc = cmp_theme_measure_checkbox(node, out_w, out_h);`
  - [x] `case CMP_UI_TYPE_RADIO:` `rc = cmp_theme_measure_radio(node, out_w, out_h);`
  - [x] `case CMP_UI_TYPE_TOGGLE:` `rc = cmp_theme_measure_toggle(node, out_w, out_h);`
  - [x] `case CMP_UI_TYPE_SLIDER:` `rc = cmp_theme_measure_slider(node, out_w, out_h);`
  - [x] `case CMP_UI_TYPE_PROGRESS:` `rc = cmp_theme_measure_progress(node, out_w, out_h);`
  - [x] `case CMP_UI_TYPE_DROPDOWN:` `rc = cmp_theme_measure_dropdown(node, out_w, out_h);`
- [x] Assertive Percolation: `if (rc != CMP_SUCCESS) { *out_w = 0.0f; *out_h = 0.0f; LOG_DEBUG("Theme measure failed: %d\n", rc); return rc; }`

### 4.2 Logging & Safe CRT Verification
- [x] Ensure that if logging occurs inside `cmp_ui.c` or string formatting is needed, `#if defined(_MSC_VER)` wrappers are used.
- [x] Utilize `sprintf_s` for MSVC and gracefully degrade to standard C89 `sprintf` for GCC/Clang/MinGW.
- [x] Abstract format strings (`NUM_FORMAT`) if integers need formatting in debugging logs.

---

## 5. Phase 4: Component Hooking & Wiring (`src/cmp_ui_*.c`)
Every UI component constructor must map its `layout` callback.

- [x] `cmp_ui_text.c`: Inject `node->layout->measure_ctx = node; node->layout->measure_cb = cmp_ui_layout_measure_dispatch;` inside `cmp_ui_text_create`.
- [x] `cmp_ui_button.c` (or equivalent like `cmp_ui.c`): Wire up context/callback.
- [x] `cmp_ui_checkbox.c`: Wire up context/callback.
- [x] `cmp_ui_radio.c`: Wire up context/callback.
- [x] `cmp_ui_switch.c` (Toggle): Wire up context/callback.
- [x] `cmp_ui_slider.c`: Wire up context/callback.
- [x] `cmp_ui_progress_bar.c`: Wire up context/callback.
- [x] `cmp_ui_text_field.c`: Wire up context/callback.
- [x] Ensure strict error checking on any operations before and after these assignments.

---

## 6. Phase 5: Removing Hacks & Legacy Fallbacks
With intrinsic measurement reliably working, hardcoded constraints must be purged.

### 6.1 Layout Engine Purge (`cmp_layout.c`)
- [x] Audit `cmp_layout_resolve_flex_pass`.
- [x] Remove the legacy "fallback size logic" block that blindly overwrites `final_main` with `child->measured_width` outside of standard flex calculations.
- [x] Ensure components correctly react to `flex_grow = 0.0f` vs `flex_grow = 1.0f` combined with their new accurate intrinsic constraints.

### 6.2 Example Cleanup (`examples/multi-design-toolbar/app.c`)
- [x] Strip explicitly assigned `layout->width = X.Xf * REM;` overrides.
- [x] Strip explicitly assigned `layout->height = X.Xf * REM;` overrides.
- [x] Revert `title`, `btn_lang`, `btn_theme`, `btn_palette`, `btn_design` widths and heights back to `-1.0f` (auto-size).

---

## 7. Phase 6: Exhaustive Build, Coverage & Platform Validation
The solution MUST undergo rigorous 100% CI-compliant testing across supported configurations.

### 7.1 CMake & Compiler Options Check
- [x] Verify build succeeds with MSVC 2022/2026.
- [x] Verify build succeeds with MinGW/Cygwin (GCC) and Clang.
- [x] Validate CMake toggle support:
  - [x] Static CRT (`/MT`, `/MTd`) and Shared CRT (`/MD`, `/MDd`).
  - [x] UNICODE and ANSI charsets.
  - [x] LTO enabled/disabled.
  - [x] MSVC `/RTC1`, `/RTCs`, `/RTCu` enabled builds without runtime access violations in the new measurement code.

### 7.2 100% Coverage Mandate Check
- [x] Update `tests/core/test_cmp_layout.c` to add a new suite `test_cmp_layout_multipass`.
- [x] Write a test asserting that a mock `measure_cb` successfully influences `node->computed_rect`.
- [x] Write a test proving that if `measure_cb` returns an error, `cmp_layout_calculate` correctly percolates the `CMP_ERROR_*` up to the caller.
- [x] Verify existing visual regression tests (`cmp_material3_visual_regression_test`, etc.) continue to pass, proving 100% coverage stability.

### 7.3 Final Clang-Format Hook
- [x] Execute `fd -eh -ec -x clang-format -i --style=LLVM` over the entire repository before concluding the loop to guarantee pure header ordering and whitespace integrity.
- [x] Re-run CMake configure/build to verify `clang-format` did not break include dependencies via the `<windows.h>` strictures or MSVC CRT block checks.