# Geometry Safeguards & Layout Containment Plan

This document outlines the strategy for extending the Geometry Checker and the Layout Engine to enforce strict container bounds. The goal is to prevent elements (buttons, text, widgets) from overflowing their parent containers and to ensure the root layout respects the window bounds, specifically addressing issues like the text overflow seen in `cmp_example_multi_design_toolbar`.

## Phase 1: Extend the Geometry Checker (Testing API)

The current geometry checker validates alignment and basic touch targets but lacks strict recursive containment checks. We will introduce new assertion macros to `tests/cmp_geometry_tester.h` to automatically fail tests if bounds are violated.

- [x] **`ASSERT_STRICT_CONTAINMENT(parent)`**
  Recursively verify that all child node bounds (margin-box) strictly fall within the parent node's bounds (content-box). Exceptions will be explicitly granted only for nodes with `position: absolute` or `position: fixed` relative to a different stacking context.
- [x] **`ASSERT_WITHIN_VIEWPORT(node, w, h)`**
  Verify that the absolute computed bounds of a node (and its descendants) do not have negative coordinates and do not exceed the given window width and height.
- [x] **`ASSERT_TEXT_OVERFLOW_HANDLED(text_node)`**
  Verify that a text node's calculated intrinsic width does not exceed its parent's width unless the parent explicitly has `overflow: visible` or `overflow: scroll`.

## Phase 2: Core Layout Engine Fixes (Containment)

The root cause of elements pushing out of the window is usually a failure in resolving intrinsic constraints (e.g., text refusing to wrap) combined with missing flex-shrink behaviors.

- [x] **Flexbox (`cmp_layout_ffc`) Shrink & Wrap Safeguards:**
  - Enforce `flex-shrink: 1` as the default for flex items. When the flex container reaches the viewport's `max_width`, child elements must compress down to their `min-content` size.
  - If a flex row's children combined `min-content` exceeds the container width, and `flex-wrap` is `nowrap`, trigger overflow clipping logic or force wrap (depending on strict CSS compliance mode).
- [x] **Block Layout (`cmp_layout_bfc`) Width Clamping:**
  - Automatically cap `width: auto` and `max-content` widths to the parent's available constraints (`constraints->max_width`). Block elements must never expand beyond their parent's width.
- [x] **Text Wrapping and Intrinsic Size Resolution:**
  - Update `cmp_layout_get_min_content()` and `cmp_layout_get_max_content()`. Text elements currently return their un-wrapped string length as their width. They must evaluate the available width and calculate the bounding box based on word-wrapping.
  - Introduce an upper bound for `min-content` to prevent long unbreakable strings (e.g., URLs) from blowing out the layout.

## Phase 3: CSS Feature & Property Implementation

To gracefully handle elements that *must* shrink, we need support for clipping and truncating.

- [x] **`text-overflow: ellipsis`**
  Implement support in the text layout resolver. If a text node's `max-width` is clamped by its parent, append an ellipsis (`...`) instead of drawing off-screen or expanding the container.
- [x] **`overflow: hidden` and `overflow: clip`**
  Implement hard clipping bounds during the layout calculate pass so that child geometry bounds are intersected and truncated against the parent's padding box.
- [x] **`box-sizing: border-box` Strict Enforcement:**
  Audit the box-model logic in `cmp_layout_calculate_node()` to ensure that when a percentage width or `max-width` is applied, padding and borders subtract from the available space rather than adding to it.

## Phase 4: Widget-Level Adjustments

Specific UI components must be updated to utilize the new layout safeguards.

- [x] **`cmp_ui_app_bar` (Toolbar Fix):**
  - Update the App Bar internal flex container to either use `flex-wrap: wrap` or apply `overflow-x: auto` / `overflow: hidden` to its action items.
  - Apply `text-overflow: ellipsis` and `white-space: nowrap` to the App Bar title so long translations (like Arabic/Hebrew in `cmp_example_multi_design_toolbar`) shrink gracefully.
- [x] **Buttons (`cmp_ui_action_button`, `cmp_ui_button`):**
  - Set a default `min-width` and explicitly allow the text inside the button to wrap or truncate if the button is squished by a flex container.
- [x] **Root Window Containers:**
  - In `app.c` wrappers (like the multi-design toolbar), ensure the root node (`g_ui_tree`) is configured with `max_width = 100%` and `overflow = hidden` or `scroll` to explicitly signal to the layout engine that it represents the hard window boundary.
