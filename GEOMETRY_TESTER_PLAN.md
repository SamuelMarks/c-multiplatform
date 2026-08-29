# Geometry Overlay & Tester Implementation Plan

This document outlines the fundamentally exhaustive, step-by-step implementation plan for building a visual geometry debugger and an automated layout testing suite. This serves as Phase 1 of the ultimate test suite designed to ensure **pixel-perfect implementation** across supported design systems (Apple HIG, Material 3, Fluent 2).

## Phase 1: Engine Infrastructure & Precise Data Exposure
*Goal: Ensure the layout engine exposes exhaustive, sub-pixel accurate metrics, stacking contexts, and typographic baselines needed for pixel-perfect validation.*

- [x] Define `cmp_rect_t` (integer bounds) and `cmp_rect_f_t` (sub-pixel float bounds) for precision handling.  
- [x] Implement `cmp_node_get_margin_box(node)` to fetch outer bounds.
- [x] Implement `cmp_node_get_border_box(node)` to fetch border bounds (standard layout bounds).
- [x] Implement `cmp_node_get_padding_box(node)` to fetch inner border bounds.
- [x] Implement `cmp_node_get_content_box(node)` to fetch innermost content bounds.
- [x] Implement `cmp_node_get_scroll_bounds(node)` to fetch the total scrollable overflow dimensions.
- [x] Implement `cmp_node_get_baseline_y(node)` to expose the primary text baseline for typography alignment.   
- [x] Implement `cmp_node_get_transform_matrix(node)` to expose 2D/3D transformed geometry states.
- [x] Implement `cmp_node_get_z_index(node)` and expose stacking context IDs for depth sorting validation.      
- [x] Expose global mock environment modifiers: `cmp_env_set_dpi_scale(scale)`, `cmp_env_set_safe_areas(top, right, bottom, left)`.
- [x] Add event hooks for `cmp_on_layout_pass_start()` and `cmp_on_layout_pass_complete()` to measure layout thrashing/performance.
- [x] Ensure the entire layout pipeline can execute in a strict headless mode without window creation, OS font-linking, or GPU context overhead.

## Phase 2: Visual Geometry Overlay (Real-time Debugger)
*Goal: Build an advanced on-screen visualizer to spot layout, typography, and accessibility anomalies in real-time.*

- [x] Create a dedicated debug render layer that executes entirely independent of the main UI layer.
- [x] Implement a global developer menu or hotkey toggle (e.g., `Ctrl+Shift+D`).
- [x] **Box Model Visualization:**
  - [x] Render 1px solid blue outline for content area.
  - [x] Render translucent green fill for padding zones.
  - [x] Render translucent yellow outline for border zones.
  - [x] Render translucent orange fill/hatching for margin zones.
- [x] **Typography Rulers:**
  - [x] Render a dashed pink line across elements representing their text `baseline`.
  - [x] Render guides for font `x-height` and `cap-height` to verify typographic centering.
- [x] **Overlap & Clash Detection:**
  - [x] Implement strict sibling bounds intersection checks within the same stacking context.
  - [x] Filter out explicit `position: absolute`/`fixed`, or intentionally negative-margined nodes.
  - [x] Render a bold, pulsating red hatched rectangle over any unintentional geometric collisions.
- [x] **Grid & Flex Deep-Dive Visualizers:**
  - [x] Render dashed lines for Flex main-axis and cross-axis alignment.
  - [x] Render CSS Grid track lines (rows and columns) extending beyond the container.
  - [x] Render hatched purple patterns specifically in flex/grid `gap` spaces.
  - [x] Overlay named grid areas with text labels.
- [x] **Accessibility & UX Visualizers:**
  - [x] **Touch Targets:** Draw a translucent red box around interactable elements highlighting their actual clickable area.
  - [x] **Target Warnings:** Flag elements where the clickable area is smaller than the recommended standard (e.g., < 44x44pt).
  - [x] Visualize `overflow: hidden` boundaries by faintly dimming clipped content.
- [x] **Interactive Metrics Tooltips:**
  - [x] On mouse hover, display: Node Tag, Classes, ID.
  - [x] Display computed X/Y, W/H, and subpixel fractional variants.
  - [x] Display Min/Max width and height constraints.
  - [x] Display Z-index and Stacking Context origin.

## Phase 3: Exhaustive Automated Tester API (Headless Macros)
*Goal: Create an unbreakable assertion API to programmatically validate geometric states, responsivness, and design system constraints.*

- [x] `ASSERT_GEOMETRY_EQ(node, x, y, w, h)`: Strict integer boundary match.
- [x] `ASSERT_GEOMETRY_APPROX(node, x, y, w, h, tolerance)`: Fuzzy match for sub-pixel text rendering differences.
- [x] `ASSERT_NO_OVERLAPS(container)`: Recursively fails if any static siblings intersect.
- [x] `ASSERT_NO_OVERLAPS_STRICT(container)`: Includes negatively margined elements in overlap checks.
- [x] `ASSERT_CONTAINED_BY(child, parent)`: Fails if the child's border-box exceeds the parent's content-box.
- [x] `ASSERT_ALIGNED_Y(node_a, node_b, alignment)`: Tests Top, Center, Bottom alignment axes.
- [x] `ASSERT_ALIGNED_X(node_a, node_b, alignment)`: Tests Left, Center, Right alignment axes.
- [x] `ASSERT_ALIGNED_BASELINE(node_a, node_b)`: Fails if text baselines do not share the exact same Y coordinate.
- [x] `ASSERT_TOUCH_TARGET_MIN(node, min_w, min_h)`: Validates physical/logical hit areas for A11y.
- [x] `ASSERT_Z_ORDER_ABOVE(node_above, node_below)`: Validates absolute visual stacking regardless of DOM order.
- [x] `ASSERT_SCROLLABLE(node, axis)`: Validates that overflow content correctly triggers scroll bounds without clipping.
- [x] `ASSERT_ASPECT_RATIO(node, ratio, tolerance)`: Ensures images/media maintain intrinsic proportions under stress.
- [x] `ASSERT_LAYOUT_PASSES_LESS_THAN(count)`: Performance assertion to prevent layout thrashing (e.g., infinite resize loops).

## Phase 4: CSS Layout Edge Case & Stress Testing
*Goal: Prevent regressions on the most complex CSS specifications (Flex, Grid, Positioning).*

- [x] **Flexbox - Wrap & Flow:**
  - [x] Assert row-to-column wrapping triggers at exact subpixel container thresholds.
  - [x] Assert `reverse` layouts correctly mirror geometry coordinates.
- [x] **Flexbox - Alignment:**
  - [x] Assert `align-items: baseline` works correctly across 3 distinct font sizes in the same row.
  - [x] Assert `flex-grow` distributes free space proportionally down to the fractional pixel.
  - [x] Assert `flex-shrink` limits size reduction to min-content to protect toolbars/icons.
- [x] **Grid - Responsiveness:**
  - [x] Assert `auto-fit` vs `auto-fill` behaviors dynamically reduce/maintain column counts as viewport shrinks.
  - [x] Assert explicitly positioned grid items overlap correctly when assigned to the same cell.
- [x] **Positioning - Constraints:**
  - [x] Assert `position: sticky` elements anchor at precise scroll offsets and release correctly.
  - [x] Assert `position: absolute` calculates bounds relative to the nearest positioned ancestor's *padding box*.
- [x] **Text & Inline Layout:**
  - [x] Assert container height expands exactly to accommodate text wrapping on word boundaries.
  - [x] Assert `text-overflow: ellipsis` reduces bounding boxes and doesn't push adjacent siblings.
- [x] **Transforms:**
  - [x] Assert 2D Rotations calculate correct expanded bounding boxes to prevent clipping.

## Phase 5: Design System Compliance Testing (Pixel-Perfect Specs)
*Goal: Use the API to enforce exact geometric rules dictated by platform design specifications.*

- [x] **Apple HIG Compliance:**
  - [x] Assert all interactive components (buttons, sliders) meet the `ASSERT_TOUCH_TARGET_MIN(node, 44, 44)` standard.
  - [x] Assert safe area padding is automatically applied and respects notches/dynamic islands.
  - [x] Assert system typography baseline rhythms match iOS standard tracking/leading.
- [x] **Google Material 3 Compliance:**
  - [x] Assert interactive elements meet the `ASSERT_TOUCH_TARGET_MIN(node, 48, 48)` (dp/pt) standard.
  - [x] Assert component alignments adhere strictly to the Material 8dp/16dp spatial grid.
  - [x] Assert floating action buttons (FABs) maintain correct z-index elevation bounds.
- [x] **Microsoft Fluent 2 Compliance:**
  - [x] Assert specific border-radius metrics correctly scale relative to component control heights.
  - [x] Assert standard Fluent layout gaps (e.g., 4px, 8px, 12px) are mathematically exact between flex siblings.

## Phase 6: CI/CD & Automated Reporting
*Goal: Integrate into the test runner and provide actionable debugging data upon failure.*

- [x] Scaffold `test_cmp_layout_geometry.c` containing all headless layout tests.
- [x] Integrate into standard CTest/CMake build process.
- [x] On assertion failure, automatically generate a JSON dump of the layout tree (DOM + computed styles + metrics).
- [x] On assertion failure, print an ASCII-art bounding box representation to standard output to visualize the overlapping/misaligned nodes instantly in CI logs.
- [x] Add performance guardrails: fail CI if a standard viewport resize triggers more than 2 full layout reflow passes.