# The Ultimate C89 Architectural Blueprint: 500+ Step Microscopic CSS & Rendering Pipeline

This document serves as the infallible, highly-exhaustive foundation for the C-Multiplatform UI framework. Before a single Fluent 2, Material 3, or Cupertino component is implemented, this 500+ step architectural blueprint must be strictly adhered to. It is meticulously designed to support infinite responsive scaling—from a 40mm Apple Watch face up to an 85" 4K TV—by seamlessly revealing or hiding UI complexity. It covers internationalization (i18n), comprehensive accessibility (a11y), and the microscopic rendering behaviors required by modern design languages.

## Phase 1: Memory, State Management & Core Data Structures

- [x] **1. Unified Arena Allocators (`cmp_arena_t`)**: All node allocation occurs within pre-allocated memory arenas to prevent fragmentation and guarantee O(1) allocation time, essential for 60/120fps UI rendering.
- [x] **2. Object Pooling & Slab Allocators (`cmp_pool_t`)**: Dedicated high-speed pools for frequently spawned and destroyed objects like `cmp_event_t`, `cmp_rect_t`, and temporary matrix calculations.
- [x] **3. High-Precision Math Primitives (`cmp_fixed_t` & `cmp_float_t`)**: Platform-agnostic mathematical representations ensuring layout reproducibility across x86, ARM, and WebAssembly, maintaining sub-pixel accuracy.
- [x] **4. 4x4 Transformation Matrices (`cmp_mat4_t`)**: High-performance SIMD-aligned (where available) matrix operations for 3D CSS transforms (`translate3d`, `rotateX`, `perspective`).
- [x] **5. Quaternion Interpolation (`cmp_quat_t`)**: Spherical linear interpolation (Slerp) for complex 3D CSS rotations to prevent gimbal lock during animations.
- [x] **6. Fast-Math Rect Intersections (`cmp_rect_math_t`)**: Vectorized bounds-checking, unions, and intersection algorithms for hit-testing and occlusion culling.
- [x] **7. The Style Dictionary (`cmp_style_dict_t`)**: A highly optimized, interned string dictionary for CSS properties, eliminating duplicate string allocations and allowing fast pointer comparisons.
- [x] **8. Bitfield State Packing (`cmp_state_flags_t`)**: Packs DOM boolean states (hover, active, focus, disabled, dirty layout, dirty paint) into 32-bit/64-bit integers for ultra-fast bitwise evaluation.
- [x] **9. Least-Recently-Used (LRU) Caches (`cmp_lru_t`)**: Configurable memory caps and eviction policies for texture atlases, shaped text runs, and pre-compiled shaders to prevent RAM exhaustion.
- [x] **10. Weak References & Handles (`cmp_handle_t`)**: Safe pointer equivalents that automatically nullify when the underlying DOM node is destroyed, preventing use-after-free crashes in async callbacks.
- [x] **11. Double-Buffered State Trees (`cmp_tree_t`)**: The UI architecture utilizes a double-buffered tree system: the *Current Frame* tree (being rendered) and the *Next Frame* tree (being mutated).
- [x] **12. Fast Path DOM Differ (`cmp_vdom_diff_t`)**: A localized reconciliation algorithm (akin to React's Virtual DOM) that rapidly determines the minimal set of structural mutations needed.
- [x] **13. Granular Dirty Bit Propagation (`cmp_dirty_t`)**: Distinguishes between `DIRTY_LAYOUT` (requires reflow), `DIRTY_PAINT` (requires redraw), and `DIRTY_COMPOSITE` (only requires matrix update).
- [x] **14. Garbage Collection Heuristics (`cmp_gc_t`)**: Incremental sweep-and-clear mechanisms for orphaned DOM nodes and detached stacking contexts that happen outside the main tick.
- [x] **15. Persistent Reactive State (`cmp_store_t`)**: A localized, thread-safe key-value store tied to the component lifecycle, acting as the single source of truth for UI data.
- [x] **16. Event Ring Buffers (`cmp_ring_buffer_t`)**: Lock-free, fixed-size ring buffers for ingesting high-frequency OS inputs (1000Hz mouse polling) without blocking the UI thread.
- [x] **17. Deferred Layout Queue (`cmp_deferred_layout_t`)**: Batches multiple DOM writes and style mutations into a single synchronous layout pass right before the VSync tick.
- [x] **18. Immutable Style Snapshots (`cmp_style_snap_t`)**: Records the exact computed style of a node at a specific frame, required for calculating CSS Transition deltas.
- [x] **19. Thread-Local Contexts (`cmp_tls_ctx_t`)**: Isolates parsing, layout, and string formatting buffers to individual worker threads to prevent mutex lock contention.
- [x] **20. Panic & Recovery Boundaries (`cmp_panic_t`)**: Isolates catastrophic render failures within a single component tree, replacing the component with a fallback error UI instead of crashing the entire application.

## Phase 2: CSS Lexing, Parsing & AST Generation

- [x] **21. Zero-Copy CSS Lexer (`cmp_css_lexer_t`)**: Tokenizes CSS source files directly from memory-mapped files without allocating intermediary strings.
- [x] **22. CSS Syntax Error Recovery (`cmp_css_recovery_t`)**: Implements W3C error-handling rules, gracefully discarding invalid properties or blocks while continuing to parse the rest of the file.
- [x] **23. Abstract Syntax Tree (AST) Generation (`cmp_css_ast_t`)**: Converts token streams into a hierarchical, memory-compact tree representing rulesets, selectors, and declarations.
- [x] **24. `@import` Resolution (`cmp_css_import_t`)**: Fetches and recursively parses external stylesheets, injecting them at the exact rule position of the import statement.
- [x] **25. `@supports` Feature Queries (`cmp_css_supports_t`)**: Conditionally applies style blocks based on the engine's internal capability flags (e.g., `@supports (backdrop-filter: blur(10px))`).
- [x] **26. Shorthand Property Expansion (`cmp_shorthand_t`)**: Expands properties like `background: red url(...)` into their constituent longhands (`background-color`, `background-image`, etc.) during parsing.
- [x] **27. CSS Custom Identifiers (`cmp_custom_ident_t`)**: Interns and validates user-defined names for grid lines, animations, and view-transitions, ensuring they don't collide with CSS keywords.
- [x] **28. Data URI Parsing (`cmp_data_uri_t`)**: Decodes `url(data:image/png;base64,...)` directly into memory buffers during the CSS parse phase.
- [x] **29. String Unescaping (`cmp_css_escape_t`)**: Processes CSS escape sequences (`\0000A0`, `\A`) into their proper Unicode representations for content generation and selector matching.
- [x] **30. CSS Nesting (`&` Selector) (`cmp_nesting_t`)**: Natively unrolls nested CSS structures during the parse phase without requiring external preprocessors like Sass.
- [x] **31. Declarative Shadow DOM Parsing (`cmp_shadow_parse_t`)**: Identifies `<template shadowrootmode="open">` equivalents and automatically attaches them to the parsed element.
- [x] **32. CSS Color Parser (`cmp_color_parse_t`)**: Converts hex (`#RRGGBBAA`), `rgb()`, `hsl()`, `hwb()`, `lab()`, `lch()`, and `oklch()` strings into internal floating-point RGBA representations.
- [x] **33. CSS Variables (Custom Properties) Parse Tree (`cmp_css_var_parse_t`)**: Extracts and stores `--variable` declarations in a separate resolution dictionary attached to the AST.
- [x] **34. AST Caching & Serialization (`cmp_ast_cache_t`)**: Serializes the parsed CSS AST to a binary format on disk, dramatically accelerating startup times for subsequent application launches.
- [x] **35. Inline Style Parsing (`cmp_inline_style_t`)**: Bypasses full selector parsing to rapidly evaluate strings assigned to the `style="..."` attribute.

## Phase 3: Cascade, Selectors & Specificity

- [x] **36. CSS Cascade Resolution (`cmp_cascade_t`)**: The core engine that merges user-agent styles, author styles, and inline styles into a final Computed Style object.
- [x] **37. Specificity Vectors (`cmp_specificity_t`)**: Computes rigorous A, B, C vectors (ID, Class/Attribute/Pseudo-class, Element/Pseudo-element) to resolve conflicting declarations.
- [x] **38. Cascade Layers (`@layer`) Support (`cmp_layer_rule_t`)**: Implements deterministic overriding based on explicit cascade layer declaration order rather than pure selector specificity.
- [x] **39. Style Encapsulation & Shadow DOM (`cmp_shadow_root_t`)**: Implements strict style boundaries. Styles defined within a component cannot leak out, and global styles cannot bleed in.
- [x] **40. Slotted Content (`::slotted()`) (`cmp_slot_t`)**: Enables targeting elements injected into a Shadow DOM slot from the host scope, crucial for composable Material/Fluent components.
- [x] **41. Host Context (`:host`, `:host-context()`) (`cmp_host_ctx_t`)**: Allows a shadow root to style its host element conditionally based on the host's classes or outer document state.
- [x] **42. Universal & Tag Selectors (`cmp_sel_tag_t`)**: Fast-path matching for `*` and element types (`div`, `button`).
- [x] **43. Class & Attribute Selectors (`cmp_sel_attr_t`)**: Matches `.class` and `[attr^="val"]`, utilizing bitmasks or bloom filters for rapid rejection.
- [x] **44. Combinators (`cmp_sel_combinator_t`)**: Traverses the DOM to evaluate Descendant (space), Child (`>`), Next Sibling (`+`), and Subsequent Sibling (`~`) relationships.
- [x] **45. Logical Selectors (`:is()`, `:where()`) (`cmp_is_where_t`)**: Specificity-forgiving and specificity-adopting selector lists for concise UI theme definitions.
- [x] **46. Relational Selectors (`:has()`) (`cmp_has_selector_t`)**: Advanced backtracking matching engine to evaluate parent/ancestor inclusion based on child properties or state.
- [x] **47. Structural Pseudo-classes (`cmp_structural_pseudo_t`)**: Engine support for `:nth-child()`, `:nth-of-type()`, `:first-child`, `:last-child`, `:only-child`, and `:empty`.
- [x] **48. State Pseudo-classes (`cmp_state_pseudo_t`)**: Dynamically applies styles upon internal state changes (`:hover`, `:active`, `:focus`, `:disabled`, `:checked`, `:indeterminate`).
- [x] **49. Language & Direction Selectors (`cmp_lang_dir_sel_t`)**: Evaluates `:lang()` and `:dir()` to dynamically apply locale-specific fonts and adjustments without structural changes.
- [x] **50. Target & Focus-Within Selectors (`cmp_target_focus_t`)**: Evaluates `:target` (matching URL hash fragments) and `:focus-within` (triggering styles when any descendant receives focus).
- [x] **51. Selector Bloom Filters (`cmp_bloom_filter_t`)**: Generates hashed signatures of ancestor classes to instantly skip evaluating complex selectors that are mathematically guaranteed to fail.
- [x] **52. Style Sharing & Memoization (`cmp_style_share_t`)**: Detects sibling elements with identical classes/attributes and points them to a single shared Computed Style object in memory.
- [x] **53. User-Agent Default Stylesheets (`cmp_ua_style_t`)**: Bakes an infallible fallback stylesheet that acts as the bedrock for the cascade, initializing root font sizes and default margins.
- [x] **54. CSS Keyword Resolution (`cmp_keyword_t`)**: Robust propagation for `inherit`, `initial`, `unset`, and `revert` across the layout tree.

## Phase 4: Values, Units & Math Functions

- [x] **55. CSS Variables Resolution (`cmp_css_var_resolve_t`)**: Resolves `--var-name` properties dynamically at layout time, detecting and breaking infinite recursive loops (`var(--a) -> var(--b) -> var(--a)`).
- [x] **56. Typed Custom Properties (`@property`) (`cmp_typed_prop_t`)**: Enables animation of custom properties by defining their syntax (e.g., `<color>`, `<length>`, `<angle>`), initial value, and inheritance rules.
- [x] **57. CSS `calc()` Expression Evaluator (`cmp_calc_t`)**: A micro-parser for evaluating dynamic arithmetic expressions mixing fixed pixels, percentages, `em`s, and custom properties.
- [x] **58. Math: Min, Max, Clamp (`cmp_math_bounds_t`)**: Native evaluation of `min()`, `max()`, and `clamp()`, automatically resolving mixed unit types dynamically based on their parent container sizes.
- [x] **59. Math: Trigonometry (`cmp_math_trig_t`)**: Evaluates `sin()`, `cos()`, `tan()`, `asin()`, `acos()`, `atan()`, and `atan2()` for complex radial UI layouts and animations.
- [x] **60. Math: Exponential & Logarithmic (`cmp_math_exp_t`)**: Evaluates `pow()`, `sqrt()`, `hypot()`, `log()`, and `exp()`.
- [x] **61. Math: Rounding & Sign (`cmp_math_round_t`)**: Evaluates `round()`, `mod()`, `rem()`, `abs()`, and `sign()`.
- [x] **62. Relative Length Units (`cmp_length_rel_t`)**: Computes `em` (font-size), `rem` (root font-size), `ex` (x-height), `ch` (zero-width), and `ic` (CJK ideograph advance) relative to the resolved font metrics.
- [x] **63. Viewport Length Units (`cmp_length_vw_t`)**: Calculates `vw`, `vh`, `vmin`, and `vmax` directly tied to the OS window dimensions.
- [x] **64. Dynamic/Logical Viewport Units (`cmp_length_dvh_t`)**: Accounts for mobile browser/OS expanding/collapsing toolbars (`dvh`, `lvh`, `svh`).
- [x] **65. Physical Absolute Units (`cmp_length_phys_t`)**: Translates `in`, `cm`, `mm`, `pt`, and `pc` to device pixels based on the active monitor's DPI scaling factor.
- [x] **66. Container Query Units (`cmp_length_cq_t`)**: Evaluates `cqw`, `cqh`, `cqi`, `cqb`, `cqmin`, and `cqmax` relative to the nearest ancestor with a defined containment context.
- [x] **67. Angle Units (`cmp_angle_t`)**: Normalization of `deg`, `grad`, `rad`, and `turn` into a uniform internal radian representation.
- [x] **68. Time & Frequency Units (`cmp_time_t`)**: Parsing and uniform conversion of `s`, `ms`, and `Hz` for animations and auditory events.
- [x] **69. Resolution Units (`cmp_resolution_t`)**: Parsing `dpi`, `dpcm`, and `dppx` for high-density display media queries.
- [x] **70. Safe Area Environment Variables (`cmp_env_safe_area_t`)**: Evaluates `env(safe-area-inset-top/bottom/left/right)`, dynamically padding content away from iPhone notches, TV overscan, and Watch circular bezels.

## Phase 5: I18n: Writing Modes & Text Directions

- [x] **71. Unicode Bidirectional Algorithm (UBA) (`cmp_bidi_t`)**: Full implementation of LTR and RTL text mixing, handling neutral characters and explicit directional formatting codes (`&lrm;`, `&rlm;`).
- [x] **72. Bidi Isolate & Override (`cmp_bidi_isolate_t`)**: Evaluates `unicode-bidi: isolate`, `bidi-override`, and `plaintext`, managing directionality boundaries for dynamic data injection (e.g., Arabic names in English text).
- [x] **73. Vertical Writing Modes (`cmp_writing_mode_t`)**: Transforms coordinate systems for `writing-mode: vertical-rl` (Japanese/Traditional Chinese) and `vertical-lr` (Mongolian).
- [x] **74. Text Orientation (`cmp_text_orientation_t`)**: Evaluates `text-orientation: mixed` (rotates Latin characters in vertical flows) vs `upright` (keeps Latin characters upright).
- [x] **75. Tate-chu-yoko (`cmp_text_combine_t`)**: Implements `text-combine-upright: all` to squeeze short horizontal text (like "10" or "!!") into a single character space within a vertical flow.
- [x] **76. CJK Line Breaking (Kinsoku Shori) (`cmp_kinsoku_t`)**: Enforces strict Japanese/Chinese typesetting rules, preventing punctuation (like periods or closing brackets) from starting a new line.
- [x] **77. Thai & Khmer Word Breaking (`cmp_dict_break_t`)**: Uses dictionary-based heuristics to calculate valid line breaks for languages written without spaces.
- [x] **78. Arabic Kashida Justification (`cmp_kashida_t`)**: Justifies Arabic text not by stretching spaces, but by injecting curvilinear extensions (Kashidas) into specific letters to flush edges.
- [x] **79. Indic Syllable Shaping (`cmp_indic_shape_t`)**: Manages complex reordering and consonant conjuncts required for Devanagari, Bengali, and Tamil scripts.
- [x] **80. Ruby Text Rendering (`cmp_ruby_t`)**: Positions small phonetic/annotative characters (furigana/pinyin) above or beside base text (`<ruby>`, `<rt>`).
- [x] **81. Ruby Alignment & Spacing (`cmp_ruby_align_t`)**: Evaluates `ruby-align` (space-around, center) and `ruby-position` (over, under, inter-character).
- [x] **82. List Style Types (I18n) (`cmp_list_style_t`)**: Generates localized list markers (e.g., `hiragana`, `cjk-decimal`, `lower-roman`, `armenian`, `georgian`).
- [x] **83. Quotation Mark Localization (`cmp_quotes_t`)**: Evaluates the `quotes` property to automatically inject locale-specific quotation marks (e.g., `« »` for French, `「 」` for Japanese) via `::before { content: open-quote }`.
- [x] **84. Logical Spacing & Borders (`cmp_logical_geom_t`)**: Maps `margin-inline-start`, `padding-block-end`, `border-inline-start-color` dynamically based on the current `writing-mode` and `dir`.

## Phase 6: Typography, Shaping & Advanced Fonts

- [x] **85. Zero-Allocation UTF-8 Decoder (`cmp_utf8_iter_t`)**: A low-level string iterator that decodes UTF-8 into Unicode codepoints on-the-fly without heap allocations.
- [x] **86. Text Run Segmentation (`cmp_text_run_t`)**: Splits contiguous text into discrete runs based on script, language, and bidirectional (Bidi) rules before shaping.
- [x] **87. Complex Text Shaping via HarfBuzz (`cmp_shaper_t`)**: Translates Unicode runs into positioned glyphs, handling complex ligatures, kerning pairs, and Arabic/Indic script shaping rules perfectly.
- [x] **88. Font Fallback Chains (`cmp_font_chain_t`)**: Resolves missing glyphs by seamlessly traversing a prioritized list of fallback typefaces, guaranteeing no "tofu" (missing character) boxes.
- [x] **89. WOFF/WOFF2 Decompression (`cmp_woff2_t`)**: In-memory parsing and Brotli-decompression of modern web fonts for rapid asset loading.
- [x] **90. Font-Face Unicode Range Subsetting (`cmp_unicode_range_t`)**: Only downloads or loads specific font files if a character matching its defined `unicode-range` is present in the DOM.
- [x] **91. System Font Stack Resolution (`cmp_system_fonts_t`)**: Maps keywords like `-apple-system`, `BlinkMacSystemFont`, `Segoe UI`, and `Roboto` directly to the OS-native UI font files.
- [x] **92. Synthetic Font Bolding (`cmp_synthetic_bold_t`)**: Algorithmic bolding (via multi-pass rendering or SDF expansion) when an explicit font weight is unavailable.
- [x] **93. Synthetic Font Italicizing (`cmp_synthetic_italic_t`)**: Algorithmic skewing to mimic italics when true italic glyphs are unavailable.
- [x] **94. Variable Font Axis Resolution (`cmp_var_font_t`)**: Support for OpenType variable fonts, allowing continuous interpolation across weight (wght), width (wdth), optical size (opsz), and slant (slnt) axes.
- [x] **95. OpenType Feature Selection (`cmp_opentype_t`)**: Exposes stylistic sets (ss01-ss20), tabular figures (tnum), old-style numerals (onum), fractions (frac), and ligatures (liga/dlig).
- [x] **96. Font Variant Caps (`cmp_font_caps_t`)**: Evaluates `small-caps`, `all-small-caps`, and `petite-caps`, substituting glyphs dynamically or algorithmically scaling capitals.
- [x] **97. Font Variant Numeric (`cmp_font_numeric_t`)**: Evaluates `lining-nums`, `oldstyle-nums`, `proportional-nums`, `tabular-nums`, `diagonal-fractions`, and `slashed-zero`.
- [x] **98. Font Variant East Asian (`cmp_font_east_asian_t`)**: Evaluates `jis78`, `jis83`, `jis90`, `jis04`, `simplified`, and `traditional` variant glyphs.
- [x] **99. Glyph Atlas Packing Algorithm (`cmp_atlas_t`)**: Utilizes an advanced Skyline/MaxRects bin-packing algorithm to tightly pack requested glyphs into a single dynamic GPU texture atlas.
- [x] **100. Signed Distance Field (SDF) Generation (`cmp_sdf_t`)**: Renders glyphs as SDFs rather than raster bitmaps. This allows infinite, crisp scaling, sub-pixel anti-aliasing, and cheap text-shadows.
- [x] **101. Sub-pixel Anti-Aliasing (ClearType equivalent) (`cmp_subpixel_aa_t`)**: Exploits RGB LCD stripe geometries to triple horizontal text resolution on traditional monitors.
- [x] **102. Text Overflow and Ellipsis (`cmp_text_overflow_t`)**: Calculates precise character truncation metrics and appends localized ellipsis (`…`) when shaped text exceeds its bounding box constraints.
- [x] **103. Multi-line Clamping (`cmp_line_clamp_t`)**: Enforces `-webkit-line-clamp` properties, truncating text at a specific visual line count regardless of the container's physical height.
- [x] **104. Text Transform & Capitalization (`cmp_text_transform_t`)**: Locale-aware evaluation of `uppercase`, `lowercase`, and `capitalize` (accounting for nuances like Turkish 'i').
- [x] **105. Letter Spacing & Word Spacing (`cmp_spacing_t`)**: Adjusts tracking at the shaping level to preserve kerning pairs where possible, while enforcing uniform distance.
- [x] **106. Text Shadows (`cmp_text_shadow_t`)**: Supports multiple layers of GPU-accelerated blurred text shadows, commonly used in Material Design hero headers.
- [x] **107. Text Decorations (`cmp_text_decoration_t`)**: Complex rendering of underlines, overlines, and line-throughs, supporting `style` (wavy, dashed, dotted) and `thickness`.
- [x] **108. Text Decoration Skip Ink (`cmp_skip_ink_t`)**: Calculates precise intersections between text underlines and descending glyphs (like 'g' or 'p'), breaking the line to ensure legibility.
- [x] **109. Drop Caps & Initial Letters (`cmp_initial_letter_t`)**: Sinks the first letter of a paragraph into subsequent lines, adhering to complex baseline alignment rules.
- [x] **110. Font Smoothing Constraints (`cmp_font_smoothing_t`)**: Evaluates `-webkit-font-smoothing: antialiased`, disabling sub-pixel rendering for dark mode texts on macOS to prevent heavy blooming.

## Phase 7: Box Model, Physical Properties & Intrinsic Sizing

- [x] **111. Absolute Box Model Representation (`cmp_box_t`)**: Strict tracking of `content-box` vs `border-box` sizing, storing precise floating-point values for margins, borders, padding, and content dimensions.
- [x] **112. Min-Intrinsic (Min-Content) Sizing (`cmp_min_content_t`)**: Calculates the narrowest possible width of a box before it overflows (e.g., the length of the longest unbreakable word).
- [x] **113. Max-Intrinsic (Max-Content) Sizing (`cmp_max_content_t`)**: Calculates the ideal width of a box assuming infinite horizontal space and no forced line breaks.
- [x] **114. Fit-Content Evaluation (`cmp_fit_content_t`)**: Resolves W3C `fit-content` algorithm, clamping width between the `min-content` and the available space, but never exceeding `max-content`.
- [x] **115. Box Sizing Constraints (`cmp_box_sizing_t`)**: Evaluation of `min-width`, `max-width`, `min-height`, and `max-height` against auto and intrinsic values.
- [x] **116. Collapsing Margins Resolver (`cmp_margin_collapse_t`)**: Implements W3C-compliant vertical margin collapsing rules between siblings, parents, and children, resolving negative margin overlaps.
- [x] **117. Fractional and Sub-pixel Layout (`cmp_subpixel_t`)**: Prevents layout rounding errors that cause 1px gaps by utilizing deterministic floating point accumulation before final rasterization snapping.
- [x] **118. Replaced Elements (`cmp_replaced_t`)**: Specialized handlers for `<image>`, `<video>`, and `<canvas>` equivalents. These elements dictate their own intrinsic sizes to the layout engine.
- [x] **119. Aspect Ratio Constraints (`cmp_aspect_ratio_t`)**: Calculates widths and heights based on `aspect-ratio: X/Y` when only one dimension (width or height) is explicitly constrained.
- [x] **120. Object Fit (`cmp_object_fit_t`)**: Manages how replaced elements fit into their box (`contain`, `cover`, `fill`, `none`, `scale-down`) combined with `object-position`.
- [x] **121. Box Decoration Break (`cmp_box_dec_break_t`)**: Determines how backgrounds, borders, and shadows are drawn when an inline element wraps across multiple lines (`clone` vs `slice`).
- [x] **122. Box Outlines (`cmp_outline_t`)**: Renders outlines (which do not affect layout like borders do), supporting `outline-offset` to draw focus rings outside the element bounds.
- [x] **123. Margin/Padding Percentages (`cmp_box_percentage_t`)**: Resolves percentages for margins and padding based strictly on the parent's *inline* size (width), as per W3C specification.
- [x] **124. Display Property Resolution (`cmp_display_t`)**: Decouples `display: [outside] [inside]`, evaluating block/inline flow against flex/grid formatting contexts.
- [x] **125. Visibility Toggling (`cmp_visibility_t`)**: Distinguishes `visibility: hidden` (reserves layout space) from `display: none` (removes from flow entirely), including `:collapse` for table rows.

## Phase 8: Adaptive Layout Solvers: Flexbox

- [x] **126. Flex Container Instantiation (`cmp_flex_ctx_t`)**: Establishes a new Flex Formatting Context, isolating child layout from parent block rules.
- [x] **127. Flex Direction & Flow (`cmp_flex_flow_t`)**: Maps main-axis and cross-axis dimensions dynamically based on `flex-direction` (row, column, row-reverse, column-reverse).
- [x] **128. Flex Wrapping (`cmp_flex_wrap_t`)**: Manages multi-line flex containers, calculating cross-axis line heights, breaking items into discrete flex lines when `flex-wrap: wrap` is applied.
- [x] **129. Flex Item Intrinsic Calculation (`cmp_flex_intrinsic_t`)**: Resolves flex item base sizes by evaluating `flex-basis` against intrinsic content sizes and applied `width`/`height`.
- [x] **130. Flex Space Distribution (Growing) (`cmp_flex_grow_t`)**: Mathematically distributes positive free space along the main axis proportionally based on `flex-grow` factors.
- [x] **131. Flex Space Resolution (Shrinking) (`cmp_flex_shrink_t`)**: Resolves negative overflow space proportionally based on `flex-shrink` factors, ensuring elements don't shrink below their `min-content` bounds.
- [x] **132. Main-Axis Alignment (`cmp_flex_justify_t`)**: Evaluates `justify-content` (`flex-start`, `center`, `space-between`, `space-around`, `space-evenly`) to position items along the flex line.
- [x] **133. Cross-Axis Alignment (`cmp_flex_align_t`)**: Evaluates `align-items` and `align-self` (`flex-start`, `center`, `stretch`, `baseline`) to position items within their flex line's cross-axis.
- [x] **134. Multi-Line Cross-Axis Alignment (`cmp_flex_align_content_t`)**: Evaluates `align-content` to distribute empty space between flex lines across the entire container's cross-axis.
- [x] **135. Flex Baseline Alignment (`cmp_flex_baseline_t`)**: Synthesizes layout baselines for flex items containing text, aligning diverse UI controls (icons, buttons, labels) precisely along their typographic baseline.
- [x] **136. Flex Order Modification (`cmp_flex_order_t`)**: Alters the visual rendering order of flex items independently from their DOM source order via the `order` property.
- [x] **137. Gap Utilities (Flex) (`cmp_flex_gap_t`)**: Injects uniform empty space (`row-gap`, `column-gap`) between flex items without requiring complex margin selectors or `first-child` pseudo-classes.
- [x] **138. Flex Bug Mitigations (`cmp_flex_quirks_t`)**: Implements necessary fallbacks and fixes for standard flexbox edge-cases (e.g., minimum size of flex items `min-width: auto` implications).

## Phase 9: Adaptive Layout Solvers: CSS Grid & Masonry

- [x] **139. Grid Container Instantiation (`cmp_grid_ctx_t`)**: Establishes a new Grid Formatting Context, parsing explicit template rows, columns, and named areas.
- [x] **140. Grid Track Sizing Algorithm (`cmp_grid_track_t`)**: Calculates optimal column and row dimensions iteratively, handling fixed pixels, percentages, `min-content`, and `max-content`.
- [x] **141. Fractional Tracks (`fr` Unit) (`cmp_grid_fr_t`)**: Distributes remaining free space proportionally among flexible tracks after non-flexible tracks have been sized.
- [x] **142. Grid `minmax()` Resolution (`cmp_grid_minmax_t`)**: Implements the W3C `minmax()` algorithm, guaranteeing responsive tracks that never shrink below constraints but expand gracefully.
- [x] **143. Auto-Fit & Auto-Fill Repetitions (`cmp_grid_repeat_t`)**: Dynamically generates grid tracks based on available width, creating highly responsive grids without media queries (`repeat(auto-fit, minmax(200px, 1fr))`).
- [x] **144. Grid Line Resolution & Placement (`cmp_grid_placement_t`)**: Maps grid items to specific tracks via `grid-column: start / end` and `grid-row`, parsing negative line numbers and span directives.
- [x] **145. Named Grid Areas (`cmp_grid_area_t`)**: Resolves ASCII-art style `grid-template-areas` strings, automatically mapping items with matching `grid-area` properties to their designated zones.
- [x] **146. Grid Auto-Placement (Dense) (`cmp_grid_auto_dense_t`)**: Resolves the `grid-auto-flow: dense` packing algorithm, backfilling smaller grid items into empty gaps left by larger items.
- [x] **147. Grid Auto-Placement (Sparse) (`cmp_grid_auto_sparse_t`)**: Resolves standard row/column auto-flow placement for grid items without explicit coordinates.
- [x] **148. Implicit Grid Tracks (`cmp_grid_implicit_t`)**: Generates and sizes tracks dynamically for grid items placed outside the bounds of the explicitly defined template (`grid-auto-rows`).
- [x] **149. CSS Grid Subgrid (`cmp_subgrid_t`)**: Enables nested grids to adopt the track sizing and names of their parent grid (`grid-template-columns: subgrid`), essential for cohesive UI forms and tables.
- [x] **150. Grid Alignment (`cmp_grid_align_t`)**: Evaluates `justify-items`, `align-items`, `justify-self`, and `align-self` to position content within a specific grid cell area.
- [x] **151. Masonry Layout (Experimental Grid) (`cmp_masonry_t`)**: Implements the `grid-template-rows: masonry` algorithm, packing items tightly into columns based on vertical space rather than strict rows (Pinterest-style).
- [x] **152. Gap Utilities (Grid) (`cmp_grid_gap_t`)**: Evaluates `grid-gap` properties, subtracting gap distances from the available space before resolving track distributions.

## Phase 10: Viewport Adaptation & Container Queries (Watch to TV)

- [x] **153. Media Queries Evaluator (`cmp_media_query_t`)**: Parses viewport dimensions (`width`, `height`, `aspect-ratio`), orientation (`landscape/portrait`), and pixel density to conditionally apply `@media` blocks.
- [x] **154. Hover & Pointer Media Features (`cmp_pointer_media_t`)**: Evaluates `@media (hover: hover)` and `@media (pointer: coarse)` to adapt UI target sizes (Phone vs Desktop) dynamically.
- [x] **155. Display Update Media Features (`cmp_update_media_t`)**: Evaluates `@media (update: fast|slow|none)` to modify animations and UI feedback for e-ink displays or slow-refresh TVs.
- [x] **156. Light Level Media Features (`cmp_light_level_t`)**: Evaluates `@media (light-level: dim|normal|washed)` to adjust contrast based on ambient light sensors.
- [x] **157. Container Query Contexts (`cmp_container_ctx_t`)**: Evaluates `container-type: inline-size` or `size`, establishing isolated layout boundaries for responsive components.
- [x] **158. Container Query Resolution (`cmp_container_query_t`)**: Evaluates `@container (min-width: X)` to reflow components (e.g., transforming a row of tabs into a dropdown) based strictly on parent width, independent of the viewport.
- [x] **159. Container Style Queries (`cmp_style_query_t`)**: Conditionally applies styles based on the computed custom properties of a parent container (`@container style(--theme: dark)`).
- [x] **160. Content Visibility (`cmp_content_visibility_t`)**: Implements `content-visibility: auto`, aggressively stripping rendering, layout, and style recalculation for off-screen subtrees to guarantee 60fps on infinite scrolling TV interfaces.
- [x] **161. Containment Overrides (`cmp_contain_t`)**: Enforces `contain: strict | layout style paint`, guaranteeing mutations inside a Watch component never invalidate the TV dashboard it resides within.
- [x] **162. Viewport Resize Observers (`cmp_resize_observer_t`)**: Triggers specific C-callbacks or JS-events exclusively when designated elements resize, deferring heavy recalcs outside the render tick.

## Phase 11: Flow Layouts, Floats & Multi-Column

- [x] **163. Block Formatting Contexts (BFC) (`cmp_bfc_t`)**: Strict structural rules that dictate how block-level elements stack vertically and contain floats.
- [x] **164. Inline Formatting Contexts (IFC) (`cmp_ifc_t`)**: Rules for how inline-level elements flow horizontally, wrap on boundaries, and align to line-boxes based on line-height and vertical-align.
- [x] **165. Floating Elements (`cmp_float_t`)**: Implements `float: left` and `float: right`, removing elements from normal flow and wrapping inline content around their bounding boxes.
- [x] **166. Clearfix Engine (`cmp_clear_t`)**: Resolves `clear: left/right/both`, ensuring elements drop below preceding floated elements.
- [x] **167. CSS Shapes for Floats (`cmp_shape_outside_t`)**: Wraps inline text around complex non-rectangular boundaries like circles, ellipses, and SVG polygons defined via `shape-outside`.
- [x] **168. Shape Margin & Threshold (`cmp_shape_margin_t`)**: Pads the text-wrapping boundary around a `shape-outside` polygon using `shape-margin` and `shape-image-threshold`.
- [x] **169. Multi-column Layout Engine (`cmp_multicolumn_t`)**: Routes inline content through discrete vertical column boxes via `column-count`, `column-width`, `column-gap`, and `column-rule`.
- [x] **170. Multi-column Balancing (`cmp_column_balance_t`)**: Algorithmically adjusts column heights to ensure content is distributed as evenly as possible across all columns (`column-fill: balance`).
- [x] **171. Column Spanning (`cmp_column_span_t`)**: Allows block-level elements inside a multi-column container to span across all columns, breaking the column flow and establishing a new block context.
- [x] **172. CSS Table Layout Engine (`cmp_table_t`)**: Implements `display: table`, `table-row`, and `table-cell`. Supports both `table-layout: fixed` and `auto` algorithms.
- [x] **173. Table Spanning (`cmp_table_span_t`)**: Calculates complex grid unions for `colspan` and `rowspan` attributes, adjusting sibling cell placements.
- [x] **174. Table Border Collapsing (`cmp_border_collapse_t`)**: Resolves W3C conflict algorithms for `border-collapse: collapse` where adjacent cells share borders of varying thicknesses and colors.

## Phase 12: Positioning, Anchors & Z-Axis Stacking

- [x] **175. Absolute & Relative Positioning (`cmp_pos_absolute_t`)**: Removes elements from flow and positions them via `top`, `right`, `bottom`, `left` relative to their closest positioned ancestor.
- [x] **176. Fixed Positioning (`cmp_pos_fixed_t`)**: Anchors elements to the root viewport context, completely immune to standard document scrolling.
- [x] **177. Sticky Positioning (`cmp_pos_sticky_t`)**: Hybrid model that acts relatively positioned until it crosses a designated scroll threshold, then acts fixed within its container's scrolling boundaries.
- [x] **178. Anchor Positioning API (`cmp_anchor_t`)**: Dynamically tethers floating elements (tooltips, popovers) to reference anchor elements (`anchor(--name bottom)`).
- [x] **179. Anchor Fallback Placements (`cmp_anchor_fallback_t`)**: Evaluates `@position-fallback` to automatically flip a tooltip to the top of an anchor if it collides with the bottom viewport edge.
- [x] **180. Anchor Size Projections (`cmp_anchor_size_t`)**: Allows an absolutely positioned element to inherit the width or height of its anchor target (`width: anchor-size(--name width)`).
- [x] **181. Stacking Context Instantiation (`cmp_stack_ctx_t`)**: Creates discrete Z-axis boundaries. `z-index`, `opacity`, and `transform` properties spawn new contexts, preventing deep children from intertwining.
- [x] **182. Z-Index Sorting Algorithm (`cmp_z_index_t`)**: A stable sort algorithm that orders sibling stacking contexts within the same parent context based on integer weight, determining the painter's algorithm order.
- [x] **183. 3D Transform Contexts (`cmp_transform_3d_t`)**: Evaluates `transform-style: preserve-3d`, allowing children to exist in a shared 3D space rather than being flattened onto the parent's plane.
- [x] **184. Backface Visibility (`cmp_backface_t`)**: Evaluates `backface-visibility: hidden` to cull elements when their 3D rotation turns their back towards the camera (essential for card-flip animations).
- [x] **185. Top Layer Management (`cmp_top_layer_t`)**: A specialized pseudo-Z-axis that ensures `<dialog>` and `popover` elements unconditionally render above all other stacking contexts, bypassing `z-index` wars.
- [x] **186. Popover API State (`cmp_popover_state_t`)**: Manages the `popover` attribute, handling light-dismissal (clicking outside to close) and keyboard `ESC` dismissal automatically.
- [x] **187. Compositor Layer Trees (`cmp_layer_t`)**: Converts the 2D layout tree into a 3D hardware Layer Tree, decoupling elements that can be hardware-composited from those requiring software repainting.

## Phase 13: Visuals: Backgrounds, Gradients & Colors

- [x] **188. Complex Backgrounds (`cmp_background_t`)**: Supports infinite layers of stacked backgrounds, mixing images, solid colors, and gradients on a single element.
- [x] **189. Linear Gradients (`cmp_grad_linear_t`)**: GPU-accelerated generation of `linear-gradient` and `repeating-linear-gradient` with arbitrary color stops and angle resolution.
- [x] **190. Radial Gradients (`cmp_grad_radial_t`)**: GPU-accelerated generation of `radial-gradient` supporting circle, ellipse, `closest-side`, and `farthest-corner` projection.
- [x] **191. Conic Gradients (`cmp_grad_conic_t`)**: GPU-accelerated generation of `conic-gradient` mapping colors around a center point (essential for Cupertino color-picker wheels and pie charts).
- [x] **192. Background Sizing & Attachment (`cmp_bg_size_t`)**: Evaluates `background-size: cover/contain`, `background-position`, `background-repeat: round/space`, and parallax `attachment: fixed`.
- [x] **193. Background Blend Modes (`cmp_bg_blend_mode_t`)**: Supports `background-blend-mode` operations like `multiply`, `screen`, `overlay`, and `color-dodge` using fixed-function hardware blending.
- [x] **194. Modern Perceptual Color Formats (`cmp_oklch_t`)**: Natively understands perceptual color spaces (OKLCH, LCH, LAB) to ensure consistent lightness when generating Cupertino/Material color ramps.
- [x] **195. Wide Color Gamut (Display P3) Support (`cmp_color_space_t`)**: Parses `color(display-p3 r g b)` and maps to wider native color spaces on compatible Apple and HDR TV displays.
- [x] **196. Relative Color Syntax (`cmp_relative_color_t`)**: Evaluates `rgb(from var(--color) r g b / 50%)`, dynamically decomposing and modifying custom property colors in real-time.
- [x] **197. Color Mixing Function (`cmp_color_mix_t`)**: Real-time evaluation of `color-mix(in oklch, red 50%, blue)` to derive intermediate surface tones and hover overlays dynamically without hardcoding variants.
- [x] **198. Color Contrast Function (`cmp_color_contrast_t`)**: Evaluates `color-contrast(var(--bg) vs white, black)` to automatically select the most legible text color based on WCAG luminance math.
- [x] **199. ICC Profile Parsing (`cmp_icc_profile_t`)**: Reads embedded color profiles from JPEG/PNG images, color-matching them to the target monitor's gamut via GPU shader matrices.

## Phase 14: Borders, Shapes, Masks & Filters

- [x] **200. Border Radius & Clipping (`cmp_radius_t`)**: Manages complex elliptical border radii and anti-aliased clipping masks for inner content that spills out of rounded corners.
- [x] **201. Squircles & Continuous Corners (`cmp_corner_shape_t`)**: Enables Apple-style continuous curves (`corner-shape: squircle`) mathematically distinct from standard circular `border-radius`.
- [x] **202. Border Images (`cmp_border_image_t`)**: Slices a single source image into a 9-patch grid via `border-image-slice`, constructing complex ornamental borders that scale without distortion.
- [x] **203. Box Shadows (`cmp_box_shadow_t`)**: Renders hardware-accelerated inset and outset shadows. Includes specialized routines for fusing multiple shadows into a single texture map to reduce draw calls.
- [x] **204. Material Design Elevation Shadows (`cmp_shadow_9patch_t`)**: Renders complex, highly-blurred Material shadows using cheap 9-patch mesh stretching, saving immense fragment shader overhead.
- [x] **205. CSS Filters (`cmp_filter_t`)**: Hardware-accelerated processing of `blur()`, `brightness()`, `contrast()`, `drop-shadow()`, `grayscale()`, `hue-rotate()`, `invert()`, and `saturate()`.
- [x] **206. Backdrop Filtering & Blurring (`cmp_backdrop_t`)**: Calculates region intersections to apply real-time gaussian blurs for Cupertino/Fluent-style acrylic and frosted glass materials (`backdrop-filter`).
- [x] **207. Backdrop Filter Edge Mirroring (`cmp_backdrop_edge_t`)**: Prevents dark/transparent halos around screen edges during backdrop blurs by reflecting edge pixels outward before convolution.
- [x] **208. Advanced Masking (`cmp_mask_image_t`)**: Masks layers using alpha-channels from external textures, SVG files, or generated gradients (`mask-image`, `mask-size`, `mask-position`).
- [x] **209. Clip-Path Engine (`cmp_clip_path_t`)**: Hard-clips layers using mathematically defined boundaries (`polygon()`, `circle()`, `ellipse()`, `inset()`, `path()`).
- [x] **210. Isolate Blending (`cmp_isolation_t`)**: Enforces `isolation: isolate`, ensuring blend modes applied inside a group (`mix-blend-mode`) do not bleed into the backdrop behind the group.

## Phase 15: SVG, Vector Graphics & Paths

- [x] **211. SVG ViewBox Parsing (`cmp_svg_viewbox_t`)**: Maps arbitrary internal SVG coordinate systems into the physical bounds of the layout box, preserving aspect ratios.
- [x] **212. SVG Path Rendering (`cmp_svg_renderer_t`)**: Tessellates complex vector paths (Bezier curves, arcs) into triangle strips for native GPU execution, allowing infinitely scalable iconography.
- [x] **213. SVG Stroke Operations (`cmp_svg_stroke_t`)**: Calculates `stroke-width`, `stroke-linecap`, `stroke-linejoin`, and miter limits for precise path outlines.
- [x] **214. Dashed SVG Strokes (`cmp_svg_dash_t`)**: Renders dashed lines using `stroke-dasharray` and `stroke-dashoffset`, often manipulated via CSS variables for drawing animations.
- [x] **215. SVG Fill Rules (`cmp_svg_fill_t`)**: Implements `nonzero` and `evenodd` winding rules for complex, self-intersecting path fills using GPU stencil buffer techniques.
- [x] **216. CSS Integration with SVG (`cmp_svg_css_t`)**: Exposes SVG internal nodes (rect, circle, path) to the main CSS cascade, allowing styling via standard classes, variables, and hover states.
- [x] **217. SVG `<use>` and Shadow Cloning (`cmp_svg_use_t`)**: Implements SVG symbol instantiation, safely cloning sub-trees into the shadow DOM while sharing vertex data.
- [x] **218. Foreign Object Rendering (`cmp_svg_foreign_t`)**: Allows standard HTML/CSS DOM trees to be embedded within SVG documents, seamlessly merging the layout contexts.
- [x] **219. SVG Filter Primitives (`cmp_svg_filters_t`)**: Supports advanced node-based image processing (`feColorMatrix`, `feDisplacementMap`, `feTurbulence`) required for complex fluid/gooey UI effects.
- [x] **220. SMIL Animation Hooks (`cmp_svg_smil_t`)**: Provides parsing and integration hooks for internal SVG declarative animations (`<animate>`, `<animateTransform>`).

## Phase 16: Rendering Pipelines (GPU & CPU Fallback)

- [x] **221. Graphics Backend Abstraction (`cmp_gpu_t`)**: Defines a uniform API boundary capable of wrapping Vulkan, Metal, DirectX 12, OpenGL ES 3.0, and a `CMP_BACKEND_CPU_SOFTWARE` fallback for legacy devices (Windows 98/old smartphones).
- [x] **222. GPU Vertex Buffer Object (VBO) Batching (`cmp_vbo_t`)**: Aggregates thousands of UI rects and glyphs into massive contiguous memory blocks, uploaded to the GPU (or passed to the CPU rasterizer) in a single transaction.
- [x] **223. Uniform Buffer Objects (UBO) (`cmp_ubo_t`)**: Uploads global UI state (viewport dimensions, current time, scroll offsets) to the GPU simultaneously for all shaders (or shared context memory for CPU).
- [x] **224. Draw Call Reduction Algorithms (`cmp_draw_call_optimizer_t`)**: Sorts render commands by texture map, shader state, and blending mode to minimize expensive CPU-to-GPU context switches.
- [x] **225. Specialized UI Shaders (`cmp_shader_t`)**: Highly optimized vertex/fragment shaders written to handle rounded rects, gradients, and SDF text natively on the GPU without CPU rasterization.
- [x] **226. Shader Pre-compilation & Caching (`cmp_shader_cache_t`)**: Compiles necessary SPIR-V/MSL shaders at startup or retrieves them from disk cache to avoid jank on first-paint.
- [x] **227. Multisample Anti-Aliasing (MSAA) (`cmp_msaa_t`)**: Hardware-level edge smoothing for complex vector paths and un-aligned rotations where SDF generation is inapplicable.
- [x] **228. Linear sRGB Color Space Blending (`cmp_linear_blend_t`)**: Ensures all alpha blending and gradients are calculated in linear color space before being gamma-corrected to sRGB for display, preventing "muddy" midtones.
- [x] **229. Texture Compression Formats (`cmp_tex_compression_t`)**: Decodes and mounts ASTC, BC7, or ETC2 compressed textures directly into GPU memory to drastically reduce VRAM bandwidth on embedded devices.
- [x] **230. Mipmap Generation & Anisotropic Filtering (`cmp_mipmap_t`)**: Generates texture LODs to prevent aliasing and moiré patterns when high-res images are scaled down via 3D CSS transforms.
- [x] **231. VSync & Presentation Modes (`cmp_swapchain_t`)**: Manages double/triple buffered swapchains, negotiating Mailbox vs FIFO presentation modes with the OS window manager to prevent tearing.
- [x] **232. Overdraw Visualizer (`cmp_overdraw_t`)**: A diagnostic render mode that tints the screen based on fragment shader complexity, highlighting areas where too many transparent layers overlap.
- [x] **233. Hardware Layer Limits & Tiling (`cmp_layer_tiling_t`)**: Chunks massively tall hardware layers (like a long scroll view) into 512x512 VRAM tiles to prevent GPU memory allocation failures.

## Phase 17: Interaction, Pointers, Touch & Stylus

- [x] **234. Hit-Testing & Pointer Projection (`cmp_hit_test_t`)**: Projects 2D screen coordinates backwards through the 3D Layer Tree (reversing CSS transforms) to accurately identify the target DOM node.
- [x] **235. Pointer Events State (`cmp_pointer_events_t`)**: Respects `pointer-events: none`, making bounding boxes completely transparent to hit-testing rays, defaulting to underlying layers.
- [x] **236. W3C Event Capturing & Bubbling (`cmp_event_t`)**: Dispatches events from the root down to the target node (capture), then propagates back up (bubble), allowing parent interception or delegation.
- [x] **237. Passive Event Listeners (`cmp_passive_event_t`)**: Implements `{passive: true}` flag, strictly preventing `preventDefault()` calls so the engine never blocks the compositor thread waiting for JS/C-callbacks.
- [x] **238. Pointer Capture & Retargeting (`cmp_pointer_capture_t`)**: Allows an element to exclusively receive all subsequent pointer events until release, essential for custom sliders, drag-and-drop, and scrollbars.
- [x] **239. State-Machine Gesture Recognizers (`cmp_gesture_t`)**: A unified multi-touch gesture system implementing deterministic state machines (Possible -> Began -> Changed -> Ended/Cancelled).
- [x] **240. Discrete Gestures: Pan, Pinch, Rotate (`cmp_complex_gesture_t`)**: Translates raw touch/mouse deltas into high-level semantic intents, managing multi-touch centroid tracking.
- [x] **241. Stylus Pen Pressure (`cmp_pointer_pressure_t`)**: Extracts raw pressure, tilt (altitude/azimuth), and barrel button states from OS tablet APIs for digital ink rendering.
- [x] **242. Touch Action Filtering (`cmp_touch_action_t`)**: Evaluates `touch-action: pan-x pan-y pinch-zoom`, intercepting specific touch gestures to drive native scrolling while passing unhandled gestures to the UI.
- [x] **243. Context Menu & Long Press (`cmp_context_menu_t`)**: Triggers custom event hooks for right-clicks or timed multi-touch holds, interfacing natively with OS context menus if requested.
- [x] **244. Hover Intents & Hysteresis (`cmp_hover_intent_t`)**: Adds micro-delays before opening complex nested menus to prevent accidental triggering when diagonal cursor movement crosses sibling borders.
- [x] **245. Pointer Lock API (`cmp_pointer_lock_t`)**: Captures the mouse unconditionally, hiding the cursor and exposing raw unbounded delta values (useful for 3D viewports or games within the UI).

## Phase 18: Kinematics: Scrolling & Snapping

- [x] **246. Independent Scroll Contexts (`cmp_scroll_ctx_t`)**: Tracks `scrollTop` and `scrollLeft` entirely decoupled from the Layout step.
- [x] **247. Hardware-Accelerated Scroll Offset Injection**: Scroll offsets are injected directly into the GPU shader matrix. The CPU never recalculates layout or vertices during a scroll, achieving perfect 120hz smoothness.
- [x] **248. Scroll Velocity Tracking (`cmp_scroll_velocity_t`)**: Calculates a rolling average of pointer deltas over time to accurately determine the exact pixel-per-second velocity upon touch release.
- [x] **249. Momentum Friction Decay Formulas (`cmp_scroll_momentum_t`)**: Implements platform-specific exponential decay algorithms (Cupertino deceleration vs Material deceleration) for natural kinetic scrolling.
- [x] **250. Overscroll Behavior (`cmp_overscroll_t`)**: Evaluates `overscroll-behavior: contain/none` to prevent scroll chaining (where scrolling a child accidentally scrolls the parent when reaching the boundary).
- [x] **251. Rubber-Banding Physics (`cmp_rubber_band_t`)**: Models a damped harmonic oscillator (spring physics) to calculate the elastic bounce-back effect when scrolling past the boundaries of a container on Apple platforms.
- [x] **252. Scroll Behavior Smooth (`cmp_scroll_smooth_t`)**: Evaluates `scroll-behavior: smooth`, intercepting programmatic scroll jumps and applying cubic-bezier tweening automatically.
- [x] **253. Custom Scrollbars (`cmp_scrollbar_t`)**: Hardware composited scrollbars styled via `::-webkit-scrollbar` equivalents, including thumb sizing, track styles, and hover expansions (Fluent style).
- [x] **254. Scrollbar Gutter (`cmp_scrollbar_gutter_t`)**: Evaluates `scrollbar-gutter: stable`, reserving layout space for the scrollbar even when the content isn't currently overflowing to prevent layout shifts.
- [x] **255. Scroll Snap Points (`cmp_scroll_snap_t`)**: Evaluates `scroll-snap-type` and `scroll-snap-align` to magnetically pull the scroll position to discrete pagination boundaries upon resting.
- [x] **256. Scroll Snap Stop (`cmp_scroll_snap_stop_t`)**: Evaluates `scroll-snap-stop: always`, forcing fast kinetic scrolls to stop at the very next pagination boundary rather than flying past it.
- [x] **257. Scroll Padding & Margin (`cmp_scroll_padding_t`)**: Enforces spatial offsets for scroll snapping and `scrollIntoView()` commands, preventing headers from obscuring scrolled targets.
- [x] **258. Smooth Scroll Anchoring (`cmp_scroll_anchor_t`)**: Prevents layout shifts from altering the user's perceived scroll position when elements dynamically resize or load above the viewport.
- [x] **259. Pull-to-Refresh Primitives (`cmp_ptr_t`)**: Exposes overscroll distance metrics dynamically via events or CSS variables to trigger native or custom loading spinners.

## Phase 19: Motion: Transitions, Keyframes & View Morphing

- [x] **260. The Main Render Loop Tick (`cmp_tick_t`)**: The beating heart of the framework, synchronizing heavily with the OS VSync signal (e.g., `CADisplayLink`, `Choreographer`) to prevent micro-stutters.
- [x] **261. Delta-Time Management (`cmp_dt_t`)**: Measures the exact milliseconds elapsed since the last frame, ensuring all animations progress at a consistent speed regardless of variable screen refresh rates.
- [x] **262. "Compositor-Only" Animations**: Strictly bypasses the CPU Layout and Paint phases for animations mutating opacity or transform, updating GPU uniforms directly.
- [x] **263. CSS Transitions Engine (`cmp_transition_t`)**: Automatically detects style mutations, computes the delta between old/new values, and spawns implicit animations interpolating color, size, and layout properties.
- [x] **264. Keyframe Animations (`cmp_keyframe_t`)**: Parses `@keyframes` at-rules, evaluating complex percentage-based waypoints, property tweens, fill-modes, and play-states (`paused`, `running`).
- [x] **265. Animation Composition (`cmp_anim_compose_t`)**: Evaluates `animation-composition: add/accumulate/replace`, dictating how multiple concurrent animations overlapping the same property combine their values.
- [x] **266. Spring Physics Interpolator (`cmp_spring_ease_t`)**: Implements `linear(spring(...))` or custom hooks evaluating mass, stiffness, damping, and initial velocity for Cupertino/Fluent fluid interfaces.
- [x] **267. Cubic Bezier Interpolator (`cmp_bezier_ease_t`)**: Evaluates standard and custom `cubic-bezier(x1, y1, x2, y2)` easing curves.
- [x] **268. Custom Stepping Functions (`cmp_step_ease_t`)**: Implements `steps(n, start/end)` for discrete frame-by-frame animations (e.g., retro game sprites, typing cursors).
- [x] **269. Motion Path Animations (`cmp_motion_path_t`)**: Evaluates `offset-path`, `offset-distance`, and `offset-rotate` to animate elements along complex geometric SVG curves rather than straight lines.
- [x] **270. Scroll-Driven Animations (`cmp_scroll_timeline_t`)**: Binds animation progress directly to a scrollbar offset rather than time via `animation-timeline: scroll()`, enabling parallax effects.
- [x] **271. View Transitions API (`cmp_view_transition_t`)**: Captures snapshot textures of the "old" and "new" states of a component/page, seamlessly morphing between them using cross-fades and affine transforms.
- [x] **272. View Transition Shared Elements (`cmp_vt_shared_t`)**: Detects `view-transition-name` matches across DOM states, calculating scaling matrices to morph a hero image from a thumbnail to full screen.
- [x] **273. Discrete Property Transitions (`cmp_discrete_transition_t`)**: Supports animating `display: none` to `display: block` via `@starting-style` and `transition-behavior: allow-discrete`.
- [x] **274. FLIP Animation Technique Integration**: First, Last, Invert, Play helper functions built into the layout engine to convert expensive layout changes (e.g., reordering flex items) into cheap transform animations.

## Phase 20: Forms, Text Editing & Input Methods (IME)

- [x] **275. Form Controls & Pseudo-elements (`cmp_form_controls_t`)**: Exposes customizable pseudo-elements (`::placeholder`, `::file-selector-button`, `::-webkit-slider-thumb`) to completely restyle native inputs.
- [x] **276. Input Validation States (`cmp_validation_t`)**: Manages HTML5-equivalent constraint validation, exposing `:valid`, `:invalid`, `:user-invalid`, and `pattern` mismatch states to the styling engine.
- [x] **277. Input Masking & Formatting (`cmp_input_mask_t`)**: Native hooks for restricting inputs to specific formats (e.g., phone numbers, credit cards) while the user types, injecting static separators.
- [x] **278. Checkbox & Radio Indeterminate States (`cmp_indeterminate_t`)**: Manages the partial-selection state (`:indeterminate`) essential for Fluent 2 nested tree-views and checkboxes.
- [x] **279. Dropdown Select Rendering (`cmp_select_ui_t`)**: Renders `<select>` dropdown menus mapping to native OS picker wheels on mobile (Watch/Phone) and popover menus on Desktop.
- [x] **280. Data List Autosuggestions (`cmp_datalist_t`)**: Provides anchored tooltip popovers containing filtered `<datalist>` options while typing into text inputs.
- [x] **281. Range Slider Thumb Intersections (`cmp_range_slider_t`)**: Extends base input structures to allow multiple thumbs on a single range track, evaluating intersections, z-index bumping, and step intervals.
- [x] **282. Color Picker UI Elements (`cmp_color_picker_t`)**: Native component bindings or built-in HSV wheel structures for `<input type="color">`.
- [x] **283. Date & Time Picker UI Elements (`cmp_date_picker_t`)**: Native component bindings or built-in calendar matrices for `<input type="date/time">`.
- [x] **284. Text Input Caret Rendering (`cmp_caret_t`)**: Blinks a hardware-composited text cursor based on the OS default blink rate and width, handling complex bi-directional text caret splitting.
- [x] **285. Text Selection Highlighting (`cmp_selection_t`)**: Renders precise blue/accented selection boxes underneath shaped text runs, merging contiguous rectangles across multiple lines and blocks.
- [x] **286. Contenteditable Equivalent (`cmp_editable_t`)**: Enables rich text editing within structural blocks, updating the underlying node tree symmetrically when a user presses Enter, Backspace, or pastes.
- [x] **287. Virtual Keyboard & IME Integration (`cmp_ime_t`)**: Communicates with the OS Input Method Editor (IME) to correctly handle multi-keystroke compositions (e.g., Pinyin, Hiragana) with inline composition highlighting.
- [x] **288. Spellcheck & Autocorrect Integration (`cmp_spellcheck_t`)**: Native OS integration to display red squiggly underlines on misspelled words and surface OS-level replacement overlays.
- [x] **289. Undo/Redo State Stacks (`cmp_undo_redo_t`)**: Isolates text mutation events into reversible blocks mapped to the standard OS `Ctrl+Z` / `Cmd+Z` behaviors natively.

## Phase 21: Accessibility (A11y) & Screen Readers

- [x] **290. Render Tree to A11y Semantic Tree Translation (`cmp_a11y_tree_t`)**: Parses the layout tree into a highly distilled Accessibility Tree, explicitly hiding decorative elements and merging logical groups.
- [x] **291. Screen Reader OS Integration (`cmp_screen_reader_t`)**: Maps the semantic A11y Tree directly to platform-native APIs (UIAutomation on Windows, VoiceOver on Apple, TalkBack on Android).
- [x] **292. ARIA Role & State Mapping (`cmp_aria_t`)**: Implements standard WAI-ARIA roles (`button`, `dialog`, `tablist`) and states (`aria-expanded`, `aria-checked`), syncing them with native UI protocols.
- [x] **293. ARIA Relationship Graph (`cmp_aria_relations_t`)**: Resolves `aria-owns`, `aria-controls`, and `aria-describedby` ID references, overriding the natural DOM hierarchy in the A11y Tree.
- [x] **294. A11y Live Regions (`cmp_aria_live_t`)**: Supports `aria-live="polite"` and `assertive` to proactively announce dynamic UI changes (like toast notifications) via the screen reader.
- [x] **295. Focus Management & Spatial Navigation (`cmp_focus_manager_t`)**: Maintains keyboard focus state and computes directional (Up/Down/Left/Right) spatial navigation paths using 2D intersection heuristics (critical for TV Remotes).
- [x] **296. Focus Indicators and `focus-visible` (`cmp_focus_ring_t`)**: Tracks whether focus was initiated via keyboard versus pointer, applying high-contrast rings strictly when navigating via D-pad/Tab key.
- [x] **297. Screen Reader Rotor Hooks (`cmp_a11y_rotor_t`)**: Maps heading levels, links, and form elements to Apple VoiceOver Rotor / Android TalkBack local menus for fast scanning.
- [x] **298. A11y Action Execution (`cmp_a11y_action_t`)**: Allows screen readers to programmatically trigger clicks, scrolls, and focus changes, routing them directly into the framework's event system.
- [x] **299. Dynamic Type Scaling (`cmp_dynamic_type_t`)**: Listens to OS-level accessibility text size overrides, scaling root `rem` values symmetrically across the layout tree.
- [x] **300. Reduced Motion Preference (`cmp_prefers_reduced_motion_t`)**: Hooks into the OS settings to detect reduced motion requests, globally overriding `cmp_timeline_t` durations to zero or switching to dissolve fades.
- [x] **301. Reduced Transparency Mode (`cmp_a11y_transparency_t`)**: Detects OS-level requests to disable blurs and acrylic materials, substituting them with solid opaque fallback colors.
- [x] **302. High Contrast & Forced Colors Mode (`cmp_forced_colors_t`)**: Automatically strips non-essential backgrounds, gradients, and box-shadows to comply with Windows High Contrast requirements (`@media (forced-colors: active)`).
- [x] **303. System Color Keywords (`cmp_sys_colors_t`)**: Resolves system keywords (`Canvas`, `CanvasText`, `AccentColor`) strictly adhering to the OS user's defined high-contrast palette.

## Phase 22: OS Integration, Windowing & Native APIs

- [x] **304. Multi-Window Lifecycle Controller (`cmp_window_manager_t`)**: Manages spawning and tracking multiple top-level OS windows from a single unified layout tree context.
- [x] **305. DPI Awareness per Monitor (`cmp_dpi_t`)**: Subscribes to OS display events, dynamically rescaling UI trees when a window is dragged between a 1x 1080p monitor and a 2x 4K monitor.
- [x] **306. System Safe Areas (`cmp_safe_areas_t`)**: Injects dimensions for physical bezels, Apple Watch circles, TV overscan, and smartphone notches into the `env()` variable system.
- [x] **307. Translucent OS Windows (`cmp_window_blur_t`)**: Hooks into Desktop Window Manager (DWM/Mica on Windows, NSVisualEffectView on macOS) to render the UI on a transparent, blurred OS backdrop.
- [x] **308. Frameless Window Drag Regions (`cmp_app_region_t`)**: Evaluates `-webkit-app-region: drag`, intercepting mouse events within the UI and passing them to the OS window manager to move the application.
- [x] **309. System Titlebar Overlay (`cmp_titlebar_env_t`)**: Evaluates `env(titlebar-area-x)` etc. to allow custom UI to safely position itself alongside OS-rendered close/minimize buttons on macOS/Windows.
- [x] **310. Dark Mode & System Appearance (`cmp_prefers_color_scheme_t`)**: Automatically detects OS-level theme changes (Light/Dark) and hot-reloads the active style tree via reactive CSS variable overrides.
- [x] **311. Clipboard API Integration (`cmp_clipboard_t`)**: Interfaces with the OS clipboard to handle complex MIME types, supporting plain text, rich text (HTML), and bitmap image payloads.
- [x] **312. System Drag and Drop API (`cmp_dnd_t`)**: Manages cross-application drag sessions, evaluating drop zones, parsing file URIs, and generating dynamic drag ghost textures.
- [x] **313. Native Dialog Interop (`cmp_native_dialog_t`)**: Fallback wrappers invoking OS-native file pickers, color pickers, and print dialogs instead of rendering bespoke UI.
- [x] **314. System Tray & Menu Bars (`cmp_system_menu_t`)**: Maps defined UI structures to the macOS global menu bar or Windows system tray contextual menus.
- [x] **315. Hardware Haptics Engine (`cmp_haptics_t`)**: Triggers precise linear resonant actuator (LRA) patterns (light impact, selection change) for Cupertino/Material toggles and pull-to-refresh actions.
- [x] **316. Picture-in-Picture (PiP) Hooks (`cmp_pip_t`)**: Promotes specific video or UI sub-trees to floating OS-managed overlays that persist across application minimization.
- [x] **317. Print / PDF Generation Context (`cmp_print_ctx_t`)**: Evaluates `@media print`, respects `page-break-before/after`, and reroutes rendering commands to a PDF generator backend instead of a GPU swapchain.
- [x] **318. System Network Status (`cmp_network_t`)**: Hooks to OS network APIs to automatically adapt asset loading strategies or show "offline" UI components.
- [x] **319. Localization String Interpolation (`cmp_i18n_t`)**: Fast lookup maps for localized string replacement, managing pluralization rules and variable injection natively within text nodes.
- [x] **320. Content Security Policy (CSP) Evaluator (`cmp_csp_t`)**: Enforces strict security boundaries, preventing the arbitrary loading of remote fonts, images, or stylesheets without explicit domain whitelisting.
 
 
 
 
 
 
 
 
