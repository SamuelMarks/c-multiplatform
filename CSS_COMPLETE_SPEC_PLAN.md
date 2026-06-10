# Exhaustive W3C CSS Specification Plan for C-Multiplatform


> **⚠️ IMPLEMENTATION STATUS WARNING ⚠️**
> All checkboxes below have been intentionally reset to `[ ]`. **DO NOT ASSUME A FEATURE IS UNIMPLEMENTED.**
> Many core architectural features and CSS modules have already been partially or fully implemented within the engine. 
> Before beginning implementation on any task below, you MUST double-check the actual `c-multiplatform` codebase, headers, and the `CSS_STYLE_FEATURES.md` architectural blueprint to verify its true status.

This document maps the complete, unabridged W3C CSS Specification (Levels 3, 4, and 5) into actionable, C-oriented implementation tasks. Properties, selectors, and at-rules are mapped to internal engine structures (e.g., `cmp_prop_*`, `cmp_layout_*`). Features already covered by the core architecture (`CSS_STYLE_FEATURES.md`) are marked as completed `[x]`.


## 0. Core C89 Implementation Constraints & Architecture Mandates
All implementation tasks mapped below MUST adhere strictly to the following architectural and coding mandates:

### Strict C89 Compliance
- **Strict C89 Compliance:** The entire codebase must strictly conform to ISO C90 (C89). No C99/C11 features (no `//` comments, variables must be declared at the top of scopes).

### Function Signatures & Error Percolation
- **Function Signatures:** Every non-void, non-math function MUST return `int` representing an exit code (0 for success); OR an error enum (which is also technically an `int`).
- Output values must be passed via pointer arguments.
- **Percolate Errors Assertively:**
  ```c
  int rc;
  rc = f();
  if (rc != 0) {
      /* handle error, printing the nonzero exit code for debug purposes */
  }
  rc = g();
  ```

### Coverage & Headers
- **Coverage:** Maintain exactly 100% Doxygen-style documentation coverage and 100% test coverage across the codebase.
- **Header Guards:** Carefully guard any POSIX/C99 headers (like `<stdint.h>`, `<stdbool.h>`, `<unistd.h>`) that are unavailable on older MSVC versions to prevent compilation errors.
- **C++ Interop:** Every public header should be wrapped in `#ifdef __cplusplus` and `#endif /* __cplusplus */` (max one time per file).

### Platform & CMake Support
- **Target Environments:** MSVC 2005, MSVC 2022, MSVC 2026, MinGW, Cygwin, Clang, and GCC. On Windows, macOS, Linux.
- **CMake Options:** The build system and codebase must flawlessly support toggling:
  - CRT Linkage: Static (`/MT`, `/MTd`) and Shared (`/MD`, `/MDd`).
  - Charsets: UNICODE and ANSI.
  - Threading: Multi-threaded and Single-threaded.
  - Linking: Link-Time Optimization (LTO), Static library, and Shared library targets.
  - MSVC Runtime Checks: `/RTC1`, `/RTCs`, and `/RTCu`.

### Windows & MSVC Optimizations
- **No `<windows.h>`:** Do NOT `#include <windows.h>` anywhere to prevent binary bloat. Use specific headers (like `<winsock2.h>`) or forward-declare what you need.
- **Safe CRT:** Litter the codebase with MSVC "Safe CRT" functions (e.g., `sprintf_s`, `strcpy_s`) for added buffer safety. These MUST be carefully guarded via `#if defined(_MSC_VER)`, seamlessly falling back to standard C89 functions for GCC/Clang/MinGW.
- **Format Specifiers:** Abstract printf/formatting strings via macros (e.g., `#define NUM_FORMAT "%I64d"` for MSVC vs `"%lld"`/`"%ld"` for others) guarded by compiler checks.

### Formatting
- **Clang-Format Safety:** Every `#include` block should be wrapped in `/* clang-format off */` and `/* clang-format on */` to preserve strict include ordering, ensuring casual clang-format runs do not break header dependencies. Maximum one occurrence of `/* clang-format off */` and `/* clang-format on */` per file.


## 1. CSS Selectors (Level 3 & 4)
### 1.1 Basic Selectors
- [x] Universal selector (`*`) (`cmp_sel_universal_t`)
- [x] Type selector (`div`, `span`) (`cmp_sel_tag_t`)
- [x] Class selector (`.class`) (`cmp_sel_class_t`)
- [x] ID selector (`#id`) (`cmp_sel_id_t`)
- [x] Attribute selectors (`[attr]`, `[attr=val]`, `[attr^=val]`, `[attr$=val]`, `[attr*=val]`, `[attr~=val]`, `[attr|=val]`) (`cmp_sel_attr_t`)
- [x] Case-insensitive attribute modifier (`[attr=val i]`) (`cmp_sel_attr_nocase_t`)
- [x] Case-sensitive attribute modifier (`[attr=val s]`) (`cmp_sel_attr_case_t`)

### 1.2 Combinators
- [x] Descendant combinator (` `) (`cmp_sel_descendant_t`)
- [x] Child combinator (`>`) (`cmp_sel_child_t`)
- [x] Next-sibling combinator (`+`) (`cmp_sel_next_sibling_t`)
- [x] Subsequent-sibling combinator (`~`) (`cmp_sel_subsequent_sibling_t`)
- [x] Column combinator (`||`) (`cmp_sel_column_t`)

### 1.3 Logical Pseudo-classes
- [x] `:is()` (Matches-any) (`cmp_is_t`)
- [x] `:where()` (Zero-specificity matches-any) (`cmp_where_t`)
- [x] `:has()` (Relational) (`cmp_has_t`)
- [x] `:not()` (Negation) (`cmp_not_t`)

### 1.4 Tree-Structural Pseudo-classes
- [x] `:empty` (`cmp_pseudo_empty_t`)
- [x] `:nth-child()` (`cmp_pseudo_nth_child_t`)
- [x] `:nth-last-child()` (`cmp_pseudo_nth_last_child_t`)
- [x] `:first-child` (`cmp_pseudo_first_child_t`)
- [x] `:last-child` (`cmp_pseudo_last_child_t`)
- [x] `:only-child` (`cmp_pseudo_only_child_t`)
- [x] `:nth-of-type()` (`cmp_pseudo_nth_of_type_t`)
- [x] `:nth-last-of-type()` (`cmp_pseudo_nth_last_of_type_t`)
- [x] `:first-of-type` (`cmp_pseudo_first_of_type_t`)
- [x] `:last-of-type` (`cmp_pseudo_last_of_type_t`)
- [x] `:only-of-type` (`cmp_pseudo_only_of_type_t`)
- [x] `:root` (`cmp_pseudo_root_t`)

### 1.5 Linguistic & Document Pseudo-classes
- [x] `:dir()` (`cmp_pseudo_dir_t`)
- [x] `:lang()` (`cmp_pseudo_lang_t`)
- [x] `:any-link` (`cmp_pseudo_any_link_t`)
- [x] `:link` (`cmp_pseudo_link_t`)
- [x] `:visited` (`cmp_pseudo_visited_t`)
- [x] `:local-link` (`cmp_pseudo_local_link_t`)
- [x] `:target` (`cmp_pseudo_target_t`)
- [x] `:target-within` (`cmp_pseudo_target_within_t`)
- [x] `:scope` (`cmp_pseudo_scope_t`)

### 1.6 User Action & State Pseudo-classes
- [x] `:hover` (`cmp_pseudo_hover_t`)
- [x] `:active` (`cmp_pseudo_active_t`)
- [x] `:focus` (`cmp_pseudo_focus_t`)
- [x] `:focus-visible` (`cmp_pseudo_focus_visible_t`)
- [x] `:focus-within` (`cmp_pseudo_focus_within_t`)
- [x] `:current` (`cmp_pseudo_current_t`)
- [x] `:past` (`cmp_pseudo_past_t`)
- [x] `:future` (`cmp_pseudo_future_t`)

### 1.7 Input & Form Pseudo-classes
- [x] `:enabled`, `:disabled` (`cmp_pseudo_enabled_disabled_t`)
- [x] `:read-only`, `:read-write` (`cmp_pseudo_read_only_write_t`)
- [x] `:placeholder-shown` (`cmp_pseudo_placeholder_shown_t`)
- [x] `:default` (`cmp_pseudo_default_t`)
- [x] `:checked` (`cmp_pseudo_checked_t`)
- [x] `:indeterminate` (`cmp_pseudo_indeterminate_t`)
- [x] `:blank` (`cmp_pseudo_blank_t`)
- [x] `:valid`, `:invalid` (`cmp_pseudo_valid_invalid_t`)
- [x] `:in-range`, `:out-of-range` (`cmp_pseudo_range_t`)
- [x] `:required`, `:optional` (`cmp_pseudo_required_optional_t`)
- [x] `:user-valid`, `:user-invalid` (`cmp_pseudo_user_validity_t`)
- [x] `:autofill` (`cmp_pseudo_autofill_t`)

### 1.8 DOM & Shadow DOM Pseudo-classes
- [x] `:host` (`cmp_pseudo_host_t`)
- [x] `:host()` (`cmp_pseudo_host_func_t`)
- [x] `:host-context()` (`cmp_pseudo_host_context_t`)
- [x] `:modal` (`cmp_pseudo_modal_t`)
- [x] `:fullscreen` (`cmp_pseudo_fullscreen_t`)
- [x] `:picture-in-picture` (`cmp_pseudo_pip_t`)
- [x] `:popover-open` (`cmp_pseudo_popover_open_t`)
- [x] `:defined` (`cmp_pseudo_defined_t`)

### 1.9 Media Pseudo-classes
- [x] `:playing`, `:paused` (`cmp_pseudo_media_state_t`)
- [x] `:muted`, `:volume-locked` (`cmp_pseudo_media_volume_t`)

## 2. CSS Pseudo-Elements (Level 4)
- [x] `::before`, `::after` (`cmp_pseudo_elem_before_after_t`)
- [x] `::first-line`, `::first-letter` (`cmp_pseudo_elem_first_line_letter_t`)
- [x] `::selection` (`cmp_pseudo_elem_selection_t`)
- [x] `::target-text` (`cmp_pseudo_elem_target_text_t`)
- [x] `::spelling-error`, `::grammar-error` (`cmp_pseudo_elem_spelling_grammar_t`)
- [x] `::marker` (`cmp_pseudo_elem_marker_t`)
- [x] `::placeholder` (`cmp_pseudo_elem_placeholder_t`)
- [x] `::backdrop` (`cmp_pseudo_elem_backdrop_t`)
- [x] `::file-selector-button` (`cmp_pseudo_elem_file_selector_t`)
- [x] `::cue` (`cmp_pseudo_elem_cue_t`)
- [x] `::slotted()` (`cmp_pseudo_elem_slotted_t`)
- [x] `::part()` (`cmp_pseudo_elem_part_t`)
- [x] `::view-transition` (`cmp_pseudo_elem_view_transition_t`)
- [x] `::view-transition-group()` (`cmp_pseudo_elem_vt_group_t`)
- [x] `::view-transition-image-pair()` (`cmp_pseudo_elem_vt_image_pair_t`)
- [x] `::view-transition-old()`, `::view-transition-new()` (`cmp_pseudo_elem_vt_old_new_t`)

## 3. CSS Values and Units (Level 4 & 5)
### 3.1 Primitives
- [x] `<integer>`, `<number>` (`cmp_val_num_t`)
- [x] `<percentage>` (`%`) (`cmp_val_percent_t`)
- [x] `<ratio>` (`cmp_val_ratio_t`)
- [x] `<flex>` (`fr`) (`cmp_val_flex_t`)
- [x] `<string>` (`cmp_val_string_t`)
- [x] `<custom-ident>` (`cmp_val_custom_ident_t`)
- [x] `<url>` (`cmp_val_url_t`)

### 3.2 Lengths
- [x] Absolute: `px`, `in`, `cm`, `mm`, `pt`, `pc` (`cmp_length_phys_t`)
- [x] Relative: `em`, `rem`, `ex`, `ch` (`cmp_length_rel_t`)
- [x] Relative (Level 4): `cap`, `rcap`, `ic`, `ric`, `lh`, `rlh`, `rex`, `rch` (`cmp_length_rel_l4_t`)
- [x] Viewport: `vw`, `vh`, `vmin`, `vmax` (`cmp_length_vw_t`)
- [x] Viewport (Level 4): `dvh`, `dvw`, `lvh`, `lvw`, `svh`, `svw`, `vi`, `vb`, `dvi`, `dvb`, `lvi`, `lvb`, `svi`, `svb` (`cmp_length_viewport_l4_t`)
- [x] Container Query: `cqw`, `cqh`, `cqi`, `cqb`, `cqmin`, `cqmax` (`cmp_length_cq_t`)

### 3.3 Other Units
- [x] Angles: `deg`, `grad`, `rad`, `turn` (`cmp_angle_t`)
- [x] Time: `s`, `ms` (`cmp_time_t`)
- [x] Resolution: `dpi`, `dpcm`, `dppx`, `x` (`cmp_resolution_t`)
- [x] Frequency: `Hz`, `kHz` (`cmp_frequency_t`)

### 3.4 Math & Functions
- [x] `calc()` (`cmp_calc_t`)
- [x] `min()`, `max()`, `clamp()` (`cmp_math_bounds_t`)
- [x] Trigonometry: `sin()`, `cos()`, `tan()`, `asin()`, `acos()`, `atan()`, `atan2()` (`cmp_math_trig_t`)
- [x] Exponents: `pow()`, `sqrt()`, `hypot()`, `log()`, `exp()` (`cmp_math_exp_t`)
- [x] Rounding/Sign: `round()`, `mod()`, `rem()`, `abs()`, `sign()` (`cmp_math_round_t`)
- [x] Environment: `env(safe-area-inset-*)`, `env(titlebar-area-*)` (`cmp_env_var_t`)
- [x] Environment: `env(keyboard-inset-*)` (`cmp_env_keyboard_t`)

## 4. Cascading, Inheritance, and Variables
- [x] `!important` (`cmp_cascade_important_t`)
- [x] Specificity algorithm (`cmp_specificity_t`)
- [x] `initial`, `inherit`, `unset`, `revert`, `revert-layer` (`cmp_keyword_t`)
- [x] `all` property (`cmp_prop_all_t`)
- [x] `@layer` (`cmp_at_rule_layer_t`)
- [x] `--*` (Custom properties) (`cmp_css_var_t`)
- [x] `var()` (`cmp_var_func_t`)
- [x] `@property` (`cmp_at_rule_property_t`)

## 5. Box Model (Level 3 & 4)
- [x] `width`, `height` (`cmp_prop_size_t`)
- [x] `min-width`, `min-height`, `max-width`, `max-height` (`cmp_prop_size_limits_t`)
- [x] `margin-top`, `margin-right`, `margin-bottom`, `margin-left`, `margin` (`cmp_prop_margin_t`)
- [x] `padding-top`, `padding-right`, `padding-bottom`, `padding-left`, `padding` (`cmp_prop_padding_t`)
- [x] `box-sizing` (`cmp_prop_box_sizing_t`)
- [x] `max-content`, `min-content`, `fit-content` (`cmp_intrinsic_size_t`)
- [x] `margin-trim` (`cmp_prop_margin_trim_t`)
- [x] `aspect-ratio` (`cmp_prop_aspect_ratio_t`)

## 6. Backgrounds, Borders & Outlines (Level 3 & 4)
- [x] `background-color`, `background-image`, `background-repeat`, `background-attachment`, `background-position`, `background-size`, `background-clip`, `background-origin`, `background` (`cmp_prop_bg_group_t`)
- [x] `border-width`, `border-style`, `border-color`, `border` (`cmp_prop_border_group_t`)
- [x] `border-radius`, `border-top-left-radius`, etc. (`cmp_prop_border_radius_t`)
- [x] `border-image-source`, `border-image-slice`, `border-image-width`, `border-image-outset`, `border-image-repeat`, `border-image` (`cmp_prop_border_image_group_t`)
- [x] `box-shadow` (`cmp_prop_box_shadow_t`)
- [x] `outline-width`, `outline-style`, `outline-color`, `outline-offset`, `outline` (`cmp_prop_outline_group_t`)
- [x] `box-decoration-break` (`cmp_prop_box_decoration_break_t`)

## 7. Color (Level 4 & 5)
- [x] `color` (`cmp_prop_color_t`)
- [x] `<named-color>`, `<hex-color>`, `rgb()`, `rgba()`, `hsl()`, `hsla()` (`cmp_color_legacy_t`)
- [x] `hwb()`, `lab()`, `lch()`, `oklab()`, `oklch()` (`cmp_color_modern_t`)
- [x] `color()` (`display-p3`, `srgb`, `rec2020`, etc.) (`cmp_color_space_t`)
- [x] `color-mix()` (`cmp_color_mix_t` simplified to string in `cmp_prop_color_t`)
- [x] `color-contrast()` (`cmp_color_contrast_t` simplified to string in `cmp_prop_color_t`)
- [x] `device-cmyk()` (`cmp_color_cmyk_t`)
- [x] `opacity` (`cmp_prop_opacity_t`)
- [x] `currentcolor` (`cmp_color_currentcolor_t` simplified to enum)
- [x] `transparent` (`cmp_color_transparent_t` simplified to enum)
- [x] System colors (`Canvas`, `WindowText`, etc.) (`cmp_sys_colors_t` simplified to string)
- [x] Relative colors (`rgb(from ...)`) (`cmp_relative_color_t` simplified to string)
- [x] `<color-interpolation-method>` (`in srgb`, `in oklab`) (`cmp_color_interpolation_t`)
- [x] `color-scheme` (`cmp_prop_color_scheme_t`)

## 8. Display & Flow Layout
- [x] `display` (block, inline, inline-block, flex, grid, table, none, contents) (`cmp_prop_display_t`)
- [x] `display: flow-root` (`cmp_display_flow_root_t`)
- [x] `display: ruby`, `display: ruby-base`, `display: ruby-text` (`cmp_display_ruby_t`)
- [x] `float` (`left`, `right`, `none`, `inline-start`, `inline-end`) (`cmp_prop_float_t`)
- [x] `clear` (`left`, `right`, `both`, `inline-start`, `inline-end`, `none`) (`cmp_prop_clear_t`)
- [x] `float-defer`, `float-reference`, `clear-after` (`cmp_prop_float_advanced_t`)
- [x] `visibility` (`visible`, `hidden`, `collapse`) (`cmp_prop_visibility_t`)

## 9. Positioning & Z-Index
- [x] `position` (`static`, `relative`, `absolute`, `fixed`, `sticky`) (`cmp_prop_position_t`)
- [x] `top`, `right`, `bottom`, `left`, `inset` (`cmp_prop_inset_group_t`)
- [x] `z-index` (`cmp_prop_z_index_t`)
- [x] `isolation` (`cmp_prop_isolation_t`)

## 10. Anchor Positioning API
- [x] `anchor-name` (`cmp_prop_anchor_name_t`)
- [x] `position-anchor` (`cmp_prop_position_anchor_t`)
- [x] `position-visibility` (`cmp_prop_position_visibility_t`)
- [x] `anchor()`, `anchor-size()` (`cmp_anchor_funcs_t`)
- [x] `position-try`, `position-try-options`, `position-try-order` (`cmp_prop_position_try_t`)

## 11. CSS Grid Layout (Level 1 & 2)
- [x] `grid-template-columns`, `grid-template-rows`, `grid-template-areas`, `grid-template` (`cmp_prop_grid_template_group_t`)
- [x] `grid-auto-columns`, `grid-auto-rows`, `grid-auto-flow`, `grid` (`cmp_prop_grid_auto_group_t`)
- [x] `grid-row-start`, `grid-row-end`, `grid-column-start`, `grid-column-end`, `grid-row`, `grid-column`, `grid-area` (`cmp_prop_grid_placement_group_t`)
- [x] `minmax()`, `fit-content()`, `repeat()` (`cmp_grid_track_funcs_t`)
- [x] `subgrid` (`cmp_prop_subgrid_t`)
- [x] `masonry` (`cmp_masonry_layout_t`)

## 12. CSS Flexible Box Layout (Flexbox)
- [x] `flex-direction`, `flex-wrap`, `flex-flow` (`cmp_prop_flex_container_group_t`)
- [x] `flex-grow`, `flex-shrink`, `flex-basis`, `flex` (`cmp_prop_flex_item_group_t`)
- [x] `order` (`cmp_prop_order_t`)

## 13. Box Alignment (Level 3)
- [x] `justify-content`, `align-content`, `place-content` (`cmp_prop_align_content_group_t`)
- [x] `justify-items`, `align-items`, `place-items` (`cmp_prop_align_items_group_t`)
- [x] `justify-self`, `align-self`, `place-self` (`cmp_prop_align_self_group_t`)
- [x] `gap`, `row-gap`, `column-gap` (`cmp_prop_gap_t`)

## 14. Multi-column Layout
- [x] `column-width`, `column-count`, `columns` (`cmp_prop_column_def_group_t`)
- [x] `column-rule-width`, `column-rule-style`, `column-rule-color`, `column-rule` (`cmp_prop_column_rule_group_t`)
- [x] `column-span`, `column-fill` (`cmp_prop_column_span_fill_t`)

## 15. Transforms & Motion Path (Level 1 & 2)
- [x] `transform`, `transform-origin`, `transform-style`, `perspective`, `perspective-origin`, `backface-visibility` (`cmp_prop_transform_group_t`)
- [x] `translate`, `rotate`, `scale` (Independent properties) (`cmp_prop_independent_transform_t`)
- [x] `matrix()`, `matrix3d()`, `translate3d()`, `rotate3d()`, `scale3d()` (`cmp_transform_funcs_t`)
- [x] `offset-path`, `offset-distance`, `offset-position`, `offset-anchor`, `offset-rotate`, `offset` (`cmp_prop_motion_path_group_t`)

## 16. Transitions & Animations
- [x] `transition-property`, `transition-duration`, `transition-timing-function`, `transition-delay`, `transition-behavior`, `transition` (`cmp_prop_transition_group_t`)
- [x] `animation-name`, `animation-duration`, `animation-timing-function`, `animation-iteration-count`, `animation-direction`, `animation-play-state`, `animation-delay`, `animation-fill-mode`, `animation-composition`, `animation` (`cmp_prop_animation_group_t`)
- [x] `@keyframes` (`cmp_at_rule_keyframes_t`)
- [x] `@starting-style` (`cmp_at_rule_starting_style_t`)

## 17. Scroll-Driven Animations & View Transitions
- [x] `animation-timeline`, `animation-range`, `animation-range-start`, `animation-range-end` (`cmp_prop_scroll_anim_group_t`)
- [x] `scroll-timeline-name`, `scroll-timeline-axis`, `scroll-timeline` (`cmp_prop_scroll_timeline_t`)
- [x] `view-timeline-name`, `view-timeline-axis`, `view-timeline-inset`, `view-timeline` (`cmp_prop_view_timeline_t`)
- [x] `timeline-scope` (`cmp_prop_timeline_scope_t`)
- [x] `view-transition-name` (`cmp_prop_view_transition_name_t`)

## 18. Masking, Clipping & Filters
- [x] `clip-path`, `clip-rule` (`cmp_prop_clip_group_t`)
- [x] `mask-image`, `mask-mode`, `mask-repeat`, `mask-position`, `mask-clip`, `mask-origin`, `mask-size`, `mask-composite`, `mask` (`cmp_prop_mask_group_t`)
- [x] `mask-type`, `mask-border-source`, `mask-border-mode`, `mask-border-slice`, `mask-border-width`, `mask-border-outset`, `mask-border-repeat`, `mask-border` (`cmp_prop_mask_border_group_t`)
- [x] `filter`, `backdrop-filter` (`cmp_prop_filter_group_t`)
- [x] `mix-blend-mode`, `background-blend-mode` (`cmp_prop_blend_mode_group_t`)

## 19. Typography & Text (Level 3 & 4)
### 19.1 Spacing & Alignment
- [x] `text-align`, `text-align-last` (`cmp_prop_text_align_group_t`)
- [x] `text-justify` (`cmp_prop_text_justify_t`)
- [x] `text-indent` (`cmp_prop_text_indent_t`)
- [x] `letter-spacing`, `word-spacing` (`cmp_prop_spacing_t`)
- [x] `line-height` (`cmp_prop_line_height_t`)
- [x] `tab-size` (`cmp_prop_tab_size_t`)

### 19.2 Wrapping & Breaking
- [x] `white-space` (`cmp_prop_white_space_t`)
- [x] `word-break`, `line-break`, `overflow-wrap`, `word-wrap` (`cmp_prop_breaking_group_t`)
- [x] `hyphens` (`cmp_prop_hyphens_t`)
- [x] `-webkit-line-clamp`, `line-clamp` (`cmp_prop_line_clamp_t`)
- [x] `hanging-punctuation` (`cmp_prop_hanging_punctuation_t`)

### 19.3 Text Decoration
- [x] `text-decoration-line`, `text-decoration-color`, `text-decoration-style`, `text-decoration-thickness`, `text-decoration` (`cmp_prop_text_decoration_group_t`)
- [x] `text-underline-position`, `text-underline-offset`, `text-decoration-skip-ink` (`cmp_prop_text_underline_group_t`)
- [x] `text-shadow` (`cmp_prop_text_shadow_t`)
- [x] `text-transform` (`cmp_prop_text_transform_t`)
- [x] `text-emphasis-color`, `text-emphasis-style`, `text-emphasis-position`, `text-emphasis` (`cmp_prop_text_emphasis_group_t`)

## 20. Fonts (Level 3 & 4)
- [x] `font-family`, `font-weight`, `font-style`, `font-stretch`, `font-size`, `font` (`cmp_prop_font_basic_group_t`)
- [x] `font-size-adjust` (`cmp_prop_font_size_adjust_t`)
- [x] `font-variant-caps`, `font-variant-numeric`, `font-variant-east-asian`, `font-variant-ligatures` (`cmp_prop_font_variant_group_t`)
- [x] `font-variant-alternates`, `font-variant-position` (`cmp_prop_font_variant_advanced_t`)
- [x] `font-feature-settings`, `font-variation-settings` (`cmp_prop_font_settings_group_t`)
- [x] `font-synthesis-weight`, `font-synthesis-style`, `font-synthesis-small-caps`, `font-synthesis` (`cmp_prop_font_synthesis_group_t`)
- [x] `font-kerning` (`cmp_prop_font_kerning_t`)
- [x] `font-language-override` (`cmp_prop_font_language_override_t`)
- [x] `font-palette` (`cmp_prop_font_palette_t`)
- [x] `@font-face` (`cmp_at_rule_font_face_t`)
- [x] `@font-feature-values`, `@font-palette-values` (`cmp_at_rule_font_advanced_t`)
- [x] `initial-letter` (`cmp_prop_initial_letter_t`)
- [x] `initial-letter-align` (`cmp_prop_initial_letter_align_t`)
- [x] `font-smoothing` (`-webkit-font-smoothing`) (`cmp_prop_font_smoothing_t`)

## 21. Writing Modes & Direction
- [x] `direction`, `unicode-bidi` (`cmp_prop_bidi_group_t`)
- [x] `writing-mode`, `text-orientation`, `text-combine-upright` (`cmp_prop_writing_mode_group_t`)

## 22. Logical Properties
- [x] `block-size`, `inline-size`, `min-block-size`, etc. (`cmp_prop_logical_size_group_t`)
- [x] `margin-block-*`, `margin-inline-*` (`cmp_prop_logical_margin_group_t`)
- [x] `padding-block-*`, `padding-inline-*` (`cmp_prop_logical_padding_group_t`)
- [x] `border-block-*`, `border-inline-*` (`cmp_prop_logical_border_group_t`)
- [x] `inset-block-*`, `inset-inline-*` (`cmp_prop_logical_inset_group_t`)
- [x] `border-start-start-radius`, etc. (`cmp_prop_logical_radius_group_t`)

## 23. Lists, Counters & Content
- [x] `list-style-type`, `list-style-position`, `list-style-image`, `list-style` (`cmp_prop_list_style_group_t`)
- [x] `counter-reset`, `counter-increment`, `counter-set` (`cmp_prop_counter_group_t`)
- [x] `content`, `quotes` (`cmp_prop_content_group_t`)

## 24. Images, Objects & Shapes
- [x] `object-fit`, `object-position` (`cmp_prop_object_fit_group_t`)
- [x] `image-rendering` (`cmp_prop_image_rendering_t`)
- [x] `image-resolution` (`cmp_prop_image_resolution_t`)
- [x] `shape-outside`, `shape-margin`, `shape-image-threshold` (`cmp_prop_shape_group_t`)

## 25. Ruby Layout
- [x] `<ruby>`, `<rt>`, `<rtc>`, `<rb>` tag equivalents (`cmp_layout_ruby_t`)
- [x] `ruby-position`, `ruby-align` (`cmp_prop_ruby_align_group_t`)
- [x] `ruby-overhang` (`cmp_prop_ruby_overhang_t`)

## 26. Overflow, Scrolling & Overscroll
- [x] `overflow`, `overflow-x`, `overflow-y`, `overflow-clip-margin` (`cmp_prop_overflow_group_t`)
- [x] `overscroll-behavior`, `overscroll-behavior-x`, `overscroll-behavior-y` (`cmp_prop_overscroll_t`)
- [x] `scroll-behavior` (`cmp_prop_scroll_behavior_t`)
- [x] `scroll-snap-type`, `scroll-snap-align`, `scroll-snap-stop` (`cmp_prop_scroll_snap_group_t`)
- [x] `scroll-padding-*`, `scroll-margin-*` (`cmp_prop_scroll_padding_margin_t`)
- [x] `scrollbar-gutter` (`cmp_prop_scrollbar_gutter_t`)
- [x] `scrollbar-width`, `scrollbar-color`, `::-webkit-scrollbar` (`cmp_prop_scrollbar_styling_t`)

## 27. Containment & Optimization
- [x] `contain` (`cmp_prop_contain_t`)
- [x] `content-visibility` (`cmp_prop_content_visibility_t`)
- [x] `contain-intrinsic-size`, `contain-intrinsic-block-size`, `contain-intrinsic-inline-size` (`cmp_prop_contain_intrinsic_t`)
- [x] `will-change` (`cmp_prop_will_change_t`)


## 29. Speech Module (Aural CSS)
- [x] `@media speech` (`cmp_at_rule_media_speech_t`)
- [x] `voice-volume`, `voice-family`, `voice-rate`, `voice-pitch`, `voice-range`, `voice-stress`, `voice-duration` (`cmp_prop_voice_group_t`)
- [x] `speak`, `speak-as` (`cmp_prop_speak_t`)
- [x] `pause-before`, `pause-after`, `pause` (`cmp_prop_pause_t`)
- [x] `rest-before`, `rest-after`, `rest` (`cmp_prop_rest_t`)
- [x] `cue-before`, `cue-after`, `cue` (`cmp_prop_cue_t`)
- [x] `play-during` (`cmp_prop_play_during_t`)

## 30. User Interface (UI) & Interaction
- [x] `cursor` (`cmp_prop_cursor_t`)
- [x] `pointer-events`, `touch-action` (`cmp_prop_pointer_touch_group_t`)
- [x] `user-select` (`cmp_prop_user_select_t`)
- [x] `appearance` (`cmp_prop_appearance_t`)
- [x] `resize` (`cmp_prop_resize_t`)
- [x] `zoom` (`cmp_prop_zoom_t`)
- [x] `caret-color` (`cmp_prop_caret_color_t`)
- [x] `caret-shape`, `caret` (`cmp_prop_caret_shape_t`)
- [x] `accent-color` (`cmp_prop_accent_color_t`)
- [x] `nav-up`, `nav-down`, `nav-left`, `nav-right` (`cmp_prop_spatial_nav_t`)

## 31. Tables
- [x] `table-layout` (`cmp_prop_table_layout_t`)
- [x] `border-collapse`, `border-spacing` (`cmp_prop_table_border_t`)
- [x] `caption-side`, `empty-cells` (`cmp_prop_table_misc_t`)

## 32. SVG & Vector CSS Properties
- [x] `fill`, `fill-rule`, `fill-opacity` (`cmp_prop_svg_fill_t`)
- [x] `stroke`, `stroke-width`, `stroke-linecap`, `stroke-linejoin`, `stroke-miterlimit`, `stroke-dasharray`, `stroke-dashoffset`, `stroke-opacity` (`cmp_prop_svg_stroke_t`)
- [x] `vector-effect` (`cmp_prop_svg_vector_effect_t`)
- [x] `cx`, `cy`, `r`, `rx`, `ry`, `x`, `y` (`cmp_prop_svg_geom_t`)

## 33. At-Rules & Media Queries
- [x] `@media` (width, height, aspect-ratio, orientation, resolution, hover, pointer, prefers-color-scheme, prefers-reduced-motion, forced-colors, light-level, display-mode, update, inverted-colors) (`cmp_at_rule_media_t`)
- [x] `@container` (size, inline-size, style queries) (`cmp_at_rule_container_t`)
- [x] `container-type`, `container-name`, `container` (`cmp_prop_container_group_t`)
- [x] `@supports` (`cmp_at_rule_supports_t`)
- [x] `@import` (`cmp_at_rule_import_t`)
- [x] `@viewport` (legacy but documented) (`cmp_at_rule_viewport_t`)
- [x] `@charset` (`cmp_at_rule_charset_t`)

## 34. CSS Object Model (CSSOM) & APIs
- [x] `CSSRule`, `CSSStyleRule`, `CSSMediaRule` structures (`cmp_cssom_rule_t`)
- [x] `CSSStyleDeclaration` mapping (`cmp_cssom_decl_t`)
- [x] `getComputedStyle()` C-equivalent APIs (`cmp_cssom_computed_style_t`)
- [x] Dynamic Stylesheet mutation APIs (`cmp_cssom_mutator_t`)


## 36. Device Adaptation & Viewport (Level 1)
- [x] `@viewport` rule parsing (`cmp_at_rule_viewport_t`)
- [x] `min-width`, `max-width`, `width` (Viewport) (`cmp_viewport_width_t`)
- [x] `min-height`, `max-height`, `height` (Viewport) (`cmp_viewport_height_t`)
- [x] `zoom`, `min-zoom`, `max-zoom`, `user-zoom` (`cmp_viewport_zoom_t`)
- [x] `orientation` (Viewport) (`cmp_viewport_orientation_t`)

## 37. Web Animations API (CSS Interop)
- [x] `Animation` struct binding (`cmp_waapi_animation_t`)
- [x] `KeyframeEffect` struct binding (`cmp_waapi_effect_t`)
- [x] `AnimationTimeline` binding (`cmp_waapi_timeline_t`)
- [x] `DocumentTimeline` binding (`cmp_waapi_doc_timeline_t`)
