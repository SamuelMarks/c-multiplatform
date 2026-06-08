# The 50% W3C CSS Standard Layout Architecture Plan

## Executive Summary
To achieve true parity with modern UI frameworks, fixing Flexbox is insufficient. The `c-multiplatform` layout engine must be elevated to implement approximately 50% of the W3C CSS Layout Standard. This requires shifting from a monolithic layout loop to a **Formatting Context Router**—an architecture where layout algorithms are modularized based on `display` types, and intrinsic sizing dictates document flow. 

This plan rigidly adheres to C89 standards and strict error percolation while introducing CSS Grid, Block/Inline flows, absolute positioning, margin collapsing, and Stacking Contexts.

---

## Phase 1: Core CSS State & Struct Definitions (`cmp.h`)
To support the W3C standard, `cmp_layout_node_t` must be expanded to hold standard CSS properties.

- [x] **Display & Flow Types:** Expand `cmp_display_t` to include `CMP_DISPLAY_BLOCK`, `CMP_DISPLAY_INLINE`, `CMP_DISPLAY_INLINE_BLOCK`, `CMP_DISPLAY_FLEX`, `CMP_DISPLAY_GRID`, and `CMP_DISPLAY_NONE`.
- [x] **Positioning Types:** Expand `cmp_position_type_t` to include `CMP_POSITION_STATIC`, `CMP_POSITION_RELATIVE`, `CMP_POSITION_ABSOLUTE`, `CMP_POSITION_FIXED`, and `CMP_POSITION_STICKY`.
- [x] **Box Sizing:** Implement `cmp_box_sizing_t` (`CMP_BOX_SIZING_CONTENT_BOX`, `CMP_BOX_SIZING_BORDER_BOX`).
- [x] **Intrinsic Keywords:** Allow width/height properties to accept enumerated float constants representing `min-content`, `max-content`, `fit-content`, and `auto`.
- [x] **Z-Index:** Add `int z_index;` and a boolean `is_stacking_context` to track Z-axis rendering priority.

---

## Phase 2: The Formatting Context Router
Instead of one massive `measure_pass` and `position_pass`, layout must be routed dynamically based on the CSS Formatting Context established by the container.

- [x] **`cmp_layout_calculate_node(node, constraints)`**: The new entry point. It evaluates the `display` property and dispatches to the correct context algorithm.
- [x] **Block Formatting Context (BFC)**: Implement `cmp_layout_bfc(...)`. Elements stack vertically. Width defaults to 100% of parent. Implement W3C **Vertical Margin Collapsing** rules between siblings and parent/child boundaries.
- [x] **Inline Formatting Context (IFC)**: Implement `cmp_layout_ifc(...)`. Elements flow horizontally and wrap. Implement **Line-Box** generation, calculating the tallest element per line to determine line-height, and align items via `vertical-align` (baseline, top, middle, bottom).
- [x] **Flex Formatting Context (FFC)**: Implement `cmp_layout_ffc(...)`. (See Phase 4 for Flexbox specifics).
- [x] **Grid Formatting Context (GFC)**: Implement `cmp_layout_gfc(...)`. (See Phase 5 for Grid specifics).

---

## Phase 3: Intrinsic & Extrinsic Sizing Algorithms
W3C standard dictates that layout constraints flow *down*, and intrinsic sizes flow *up*.

- [x] **Min-Content Calculation**: Implement `cmp_layout_get_min_content(...)`. Calculates the narrowest possible width before overflow (e.g., the length of the longest unbreakable word in a text node).
- [x] **Max-Content Calculation**: Implement `cmp_layout_get_max_content(...)`. Calculates the ideal width assuming infinite horizontal space and no forced line breaks.
- [x] **Fit-Content Algorithm**: Resolve W3C `fit-content` logic, clamping width dynamically between `min-content` and the available space without exceeding `max-content`.
- [x] **Percentage Resolution**: Ensure all percentage-based margins, paddings, and widths strictly resolve against the parent's *inline* size (width), even for `margin-top` / `padding-bottom`, per W3C specification.

---

## Phase 4: W3C Flexbox Compliance (The Core Fix)
Resolve the overlapping and unexpected wrapping bugs using the official W3C Flexbox algorithm (Section 9).

- [x] **Step 1: Line Breaking (`flex-wrap`)**: Generate persistent flex lines. Store them in a `cmp_layout_lines_t` struct attached to the context. Do not recalculate lines during positioning.
- [x] **Step 2: Main-Size Resolution**: Distribute `flex-grow` and `flex-shrink` with strict float-epsilon snapping. Calculate the "flex factor sum" and handle "frozen" items that hit their min/max constraints.
- [x] **Step 3: Cross-Size Re-measurement**: **Crucial Fix.** If a child's width changes due to grow/shrink, recursively call `cmp_layout_calculate_node` on it with the new width to accurately reflow its height.
- [x] **Step 4: Cross-Size Alignment**: Calculate the final container height by summing all wrapped flex line heights (fixing the collapsed container bug). Evaluate `align-content` and `align-items`.

---

## Phase 5: CSS Grid Engine Foundation
Implementing a lightweight version of the CSS Grid module for complex 2D layouts.

- [x] **Track Sizing Definitions**: Support explicit grids via `grid-template-columns` and `grid-template-rows`.
- [x] **Fractional Units (`fr`)**: Distribute remaining free space proportionally among flexible tracks after non-flexible (px, %, min-content) tracks have been sized.
- [x] **Grid Line Placement**: Support `grid-column-start`, `grid-column-end`, `grid-row-start`, `grid-row-end` for absolute cell placement.
- [x] **Implicit Tracks**: Automatically generate and size implicit tracks (`grid-auto-rows`, `grid-auto-columns`) for items placed outside the explicit template.

---

## Phase 6: Positioning, Stacking & Viewport Geometry
Decouple absolutely positioned elements from the document flow.

- [x] **Out-of-Flow Deferral**: Elements with `position: absolute` or `fixed` must be removed from normal BFC/FFC/GFC calculations. Their parent context just ignores them.
- [x] **Absolute Positioning Solver**: Run a separate top-down pass after all flow elements are sized. Calculate `top`/`right`/`bottom`/`left` relative to the nearest ancestor with `position != static`.
- [x] **Sticky Positioning**: Implement `position: sticky`. It acts as `relative` until its bounding box intersects a scroll container's edge, at which point it dynamically acts as `fixed`.
- [x] **Stacking Contexts (Painter's Algorithm)**: Implement a stable sort based on `z-index`. `z-index` only applies if `position != static` or `display == flex/grid`. Create strict boundaries so children of a low z-index element can never paint over a high z-index element, regardless of their own z-index.

---

## Phase 7: Responsive Math & `calc()`
- [x] **Viewport Units**: Dynamically resolve `vw`, `vh`, `vmin`, and `vmax` based on the root window constraints passed to the master layout loop.
- [x] **`calc()` Expression Support**: Create a layout-time evaluator to handle mixed-unit arithmetic (e.g., `calc(100% - 20px)`) precisely at the moment of constraint resolution.
- [x] **Clamp / Min / Max**: Native evaluation of `clamp(min, val, max)`, `min()`, and `max()` to ensure fluid typography and responsive scaling bounds.

---

## Execution Strategy
To safely implement this massive paradigm shift without breaking the existing framework:
1. **Define Structs First**: Update `cmp.h` to hold all new CSS layout states and ensure C89/C++ header compilation passes.
2. **Abstract the Router**: Rename `cmp_layout_calculate` to `cmp_layout_ffc` (making it flex-specific), and create a new master `cmp_layout_calculate_node` that routes to it.
3. **Fix Flexbox First**: Implement Phase 4 to immediately resolve the clipping and overlapping bugs affecting the current Fluent/Material UI examples.
4. **Iterative Feature Rollout**: Incrementally implement Block context (margin collapse), Absolute positioning, and eventually CSS Grid, backing every addition with strict math unit tests.