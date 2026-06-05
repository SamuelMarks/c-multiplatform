# Flex Layout System Overhaul & Responsive Expansion Plan

This document outlines the step-by-step execution plan to overhaul the LibCMPC Flexbox layout engine. The new engine will transition to a strict constraint-based architecture (Constraints go down, sizes go up, parent sets position) to guarantee elements remain within window bounds. Furthermore, it explicitly handles responsive scaling across all form factors from smartwatches to TVs.

## 1. W3C CSS Flexible Box Layout Module Level 1 Compliance
This section outlines the exhaustive checklist for full standard compliance based on the official W3C CSS Flexbox specification.

### 1.1 Property Definitions & State Management
- [x] **Flex Container Properties**
    - [x] `display`: Support `flex` (block-level) and `inline-flex` (inline-level).
    - [x] `flex-direction`: Implement `row`, `row-reverse`, `column`, `column-reverse`.
    - [x] `flex-wrap`: Implement `nowrap`, `wrap`, `wrap-reverse`.
    - [x] `justify-content`: Implement `flex-start`, `flex-end`, `center`, `space-between`, `space-around`.
    - [x] `align-items`: Implement `flex-start`, `flex-end`, `center`, `baseline`, `stretch`.
    - [x] `align-content`: Implement `flex-start`, `flex-end`, `center`, `space-between`, `space-around`, `stretch`.
- [x] **Flex Item Properties**
    - [x] `order`: Integer value (default 0) for visual reordering.
    - [x] `flex-grow`: Non-negative number (default 0).
    - [x] `flex-shrink`: Non-negative number (default 1).
    - [x] `flex-basis`: Support `auto`, `content`, and explicit `<length>` or `<percentage>`.
    - [x] `align-self`: Overrides container's `align-items` for individual items.

### 1.2 Core Layout Algorithm (W3C Section 9)
- [x] **Step 1: Initial Setup**
    - [x] Generate flex items from in-flow children (blockify items, wrap anonymous text).
    - [x] Determine main and cross axes based on `flex-direction` and `writing-mode`.
- [x] **Step 2: Line Length Determination**
    - [x] Determine the available main and cross space of the flex container.
    - [x] Resolve the `flex-basis` and "hypothetical main size" for each item.
- [x] **Step 3: Main Size Determination (Line Breaking)**
    - [x] Collect items into flex lines based on `flex-wrap` and container width.
    - [x] Handle `visibility: collapse` by performing an initial layout to determine "strut" sizes.
- [x] **Step 4: Main Size Determination (Flexing)**
    - [x] **Resolve Flexible Lengths (Section 9.7):**
        - [x] Calculate the "flex factor sum" (grow or shrink) for each line.
        - [x] If sum < 1, distribute space proportionally; if sum > 1, distribute all space.
        - [x] Handle "frozen" items that reach min/max constraints and iterate until all lengths are resolved.
- [x] **Step 5: Cross Size Determination**
    - [x] Calculate the cross size of each flex line (based on max item height or `align-content`).
    - [x] Handle `align-items: stretch` by calculating used cross sizes for items.
- [x] **Step 6: Main-Axis Alignment**
    - [x] Distribute remaining free space using `justify-content`.
    - [x] Resolve `auto` margins (they take precedence and "soak up" available space).
- [x] **Step 7: Cross-Axis Alignment**
    - [x] Align items within lines using `align-self`.
    - [x] Align lines within the container using `align-content`.

### 1.3 Critical Edge Cases & Sizing
- [x] **Automatic Minimum Size (W3C Section 4.5)**
    - [x] Implement `min-height: auto` and `min-width: auto` logic.
    - [x] For non-scrollable items, the minimum size is the "content-based minimum size."
    - [x] Calculate "transferred size suggestion" using aspect ratios if available.
- [x] **Baseline Alignment**
    - [x] Calculate the baseline of flex items (first line of text or synthesized from margin box).
    - [x] Align items in a line such that their baselines match.
- [x] **Absolute Positioning (W3C Section 4.1)**
    - [x] Exclude `position: absolute` children from the flex flow.
    - [x] Calculate "static position" based on the container's content edges and alignment properties.
- [x] **Intrinsic Sizing (W3C Section 9.9)**
    - [x] Implement `min-content` and `max-content` sizing for the container itself.
    - [x] Handle multi-line containers where the cross-size depends on the main-size (and vice versa).
- [x] **Z-Ordering**
    - [x] Ensure `z-index` (other than `auto`) creates a stacking context even for `static` items.

## 2. Core Architecture & Data Structures
- [x] Define `cmp_layout_constraints_t` struct in `include/cmp.h` to hold `min_width`, `max_width`, `min_height`, and `max_height`.
- [x] Update `cmp_layout_node_t` to include a `flex_basis` property (separating it from `width`/`height` overrides).
- [x] Update `cmp_layout_node_t` to include internal engine states: `measured_width` and `measured_height`.
- [x] Add a `dirty` flag to `cmp_layout_node_t` to optimize layout recalculations during rapid window resizing.
- [x] Ensure `cmp_layout_node_create` initializes new constraint and sizing fields correctly.

## 3. Multi-Pass Layout Algorithm (The Engine)
- [x] **Phase 1: Intrinsic Measurement (Bottom-Up)**
  - [x] Implement `cmp_layout_measure_pass(node)`.
  - [x] Recursively measure children's intrinsic sizes before applying flex rules.
  - [x] Calculate intrinsic text bounds and intrinsic content dimensions.
- [x] **Phase 2: Constraint & Flex Resolution (Top-Down)**
  - [x] Implement `cmp_layout_resolve_flex_pass(node, constraints)`.
  - [x] Distribute `flex_grow` strictly within the parent's `max` constraints.
  - [x] Distribute `flex_shrink` aggressively. If a child's intrinsic size exceeds the parent's `max_width`/`max_height`, forcefully shrink the child (even to 0) to prevent bounds overflow.
  - [x] Clamp all final node sizes to the provided `max` bounds before returning.
- [x] **Phase 3: Final Positioning (Top-Down)**
  - [x] Implement `cmp_layout_position_pass(node, parent_x, parent_y)`.
  - [x] Apply `justify_content` spacing (Start, Center, End, Space Between, Space Around, Baseline) based on the *clamped* sizes.
  - [x] Apply `align_items` and `align_self` to position items on the cross-axis.
  - [x] Resolve `CMP_POSITION_ABSOLUTE` nodes relative to the nearest positioned ancestor, bounded by the screen.
- [x] Wire up `cmp_layout_calculate` to execute Phase 1, Phase 2, and Phase 3 in sequence.

## 4. Strict Overflow & Scroll Isolation
- [x] Ensure the Root node's initial constraint is exactly `max_width = window_width` and `max_height = window_height`.
- [x] Modify scroll view logic: The scroll container's `computed_rect` must be strictly clamped to parent constraints.
- [x] Allow `scroll_content_size` (the internal virtual canvas) to exceed constraints *only* internally, without bleeding outward and breaking the parent UI bounds.
- [x] Implement clipping `overflow: hidden` mapping to rendering system to ensure visuals do not draw outside the computed bounding box.

## 5. Responsive Form Factors & Window Resizing
- [x] **Watch Size (< 300px):**
  - [x] Ensure UI gracefully shrinks to minimum readable bounds.
  - [x] Force `flex_direction = column` overrides for tight spaces.
- [x] **Phone Size (300px - 600px):**
  - [x] Implement standard single-column mobile view constraints.
  - [x] Ensure sidebars gracefully collapse or shift to off-screen drawers (via z-index and absolute positioning).
- [x] **Fold-out / Mini-Tablet (600px - 900px):**
  - [x] Handle real-time layout recalculation when a device folds/unfolds.
  - [x] Support flexible grid wrapping (`flex_wrap = wrap`) that seamlessly upgrades from 1 column to 2 columns.
- [x] **Tablet Size (900px - 1200px):**
  - [x] Support multi-pane layouts (e.g., Master-Detail views).
  - [x] Ensure `flex_grow` correctly partitions screen real-estate (e.g., `flex_grow: 1` for navigation rail, `flex_grow: 3` for content).
- [x] **Desktop Size (1200px - 1920px):**
  - [x] Ensure `max_width` limits can be set to prevent text lines from becoming unreadably long (e.g., capping reading containers at 80ch/800px while centering them via flex).
- [x] **TV Size (> 1920px):**
  - [x] Implement safe-area overscan margins dynamically at the root node.
  - [x] Scale intrinsic font measurements smoothly.
- [x] **Window Resize Handling:**
  - [x] Connect the OS window resize event directly to `cmp_layout_calculate` using the new dirty-flag system to guarantee 60fps/120fps fluid resizing without UI snapping or overflow artifacts.

## 6. Testing & Validation
- [x] **Unit Tests (Bounds Checking):**
  - [x] Write a test where a container of size 800x600 holds a child requesting 5000x5000. Assert the child's `computed_rect` is clamped to 800x600.
  - [x] Write a test for `flex_shrink` where 3 children requesting 400px each in an 800px container correctly shrink to 266.6px each.
- [x] **Unit Tests (Responsive):**
  - [x] Simulate a resize from 400px (Phone) to 1024px (Tablet) and assert that `flex_wrap` re-flows items from 4 rows to 1 row.
- [x] **Unit Tests (W3C Compliance):**
  - [x] Implement tests verifying flex factor sums logic (Section 9.7).
  - [x] Validate `min-height: auto` default behavior for standard items vs scrollable items.
  - [x] Validate absolute positioning inside flex containers with implicit/explicit bounds.
- [x] **Visual Verification:**
  - [x] Run all existing UI tests (e.g., `tests/core/test_cmp_layout.c`).
  - [x] Ensure existing examples (Apple Cupertino, Google Material, Microsoft Fluent) build and render perfectly under the strict constraint engine.

## 7. C89 & Code Quality Mandates
- [x] **Strict C89 Compliance:** All code must strictly conform to ISO C90 (C89). No `//` comments. All variables must be declared at the top of their respective scopes.
- [x] **Error Handling & Signatures:** Every non-void, non-math function MUST return an `int` (0 for success) or an error `enum` representing an exit code.
- [x] **Pointer Outputs:** Output values must be passed via pointer arguments.
- [x] **Assertive Percolation:** Use the `int rc; rc = f(); if (rc != 0) { ... }` pattern relentlessly to pass errors up the stack.
- [x] **Include Formatting:** Wrap all `#include` blocks in `/* clang-format off */` and `/* clang-format on */` (max one time per file) to preserve strict dependency ordering.
- [x] **No `<windows.h>`:** Never include `windows.h`. Forward-declare what is needed or use specific minimal headers to prevent binary bloat.
- [x] **Safe CRT:** Use MSVC "Safe CRT" functions (`sprintf_s`, `strcpy_s`) guarded by `#if defined(_MSC_VER)`, seamlessly falling back to standard C89 functions for GCC/Clang/MinGW.
- [x] **Format Specifiers:** Abstract formatting strings (e.g., `#define NUM_FORMAT "%I64d"` vs `"%lld"`) behind compiler checks.
- [x] **C++ Header Guards:** Wrap every public header in `#ifdef __cplusplus extern "C" { #endif` and `#ifdef __cplusplus } #endif`.
- [x] **Coverage:** Guarantee 100% test coverage and 100% Doxygen-style documentation coverage for all newly added logic.
