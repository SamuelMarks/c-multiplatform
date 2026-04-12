# Material 3 Widget Catalog Implementation Plan (Exhaustive)

This document contains a comprehensive, low-level architectural plan to build a pixel-perfect, behavior-accurate Material Design 3 (M3) Widget Catalog using the C-Multiplatform (CMP) framework. This plan enforces C-native memory paradigms, M3 adaptive design principles, dynamic theming, and advanced motion physics.

## Phase 1: Core Architecture, Memory, and Event Loop
- [x] **Memory Management System (`m3_memory.h`)**
  - [x] Initialize `m3_state_arena`: Persistent memory for router history, active theme, and cached typography glyphs.
  - [x] Initialize `m3_ui_arena`: Transient memory cleared every frame (or every recomposition) for the UI node tree.
  - [x] Implement arena fallback allocators for unexpected out-of-memory handling.
  - [x] Implement string interning pool for component labels and descriptions to reduce allocations.
- [x] **State & Reactivity Engine (`m3_state.h`)**
  - [x] Define global `m3_catalog_state_t` context.
  - [x] Build dependency tracking graph: map UI nodes to specific state variables (e.g., slider value mapped to text field).
  - [x] Implement `m3_invalidate_ui()` with partial tree rebuilds (dirty flag propagation) instead of full screen clears.
  - [x] Implement debounce mechanism for rapid state changes (e.g., window resize, scroll events).
- [x] **Event Pipeline & Input Handling**
  - [x] Distinguish input modalities: Mouse, Touch, Stylus, Keyboard, and Gamepad.
  - [x] Implement standard hover tracking and focus rings (only visible for keyboard navigation).
  - [x] Implement touch slop detection (minimum drag distance before scrolling begins).
  - [x] Velocity tracking for swipe gestures (calculating pixels/ms for fling physics).
- [x] **Navigation & Routing Router (`m3_router.h`)**
  - [x] Define `m3_route_t` struct (Screen ID, Args pointer, Transition Enum).
  - [x] Build back-stack array with max depth of 32.
  - [x] Integrate OS back button / Escape key trapping to pop the stack.
  - [x] Implement URL-like deep linking parser (e.g., `cmp://catalog/button/filled`).
  - [x] Retain scroll positions per route when popping back from a detail screen.

## Phase 2: M3 Dynamic Theming & Token Engine
- [✓] **Color System & HCT Color Space (`m3_color.h`)**
  - [✓] Implement Hue, Chroma, Tone (HCT) color space conversion math.
  - [✓] Build Dynamic Color Generator: Seed Color -> Tonal Palettes (Primary, Secondary, Tertiary, Neutral, Neutral Variant, Error).
  - [✓] Map all 30+ M3 Semantic Roles:
    - [✓] Accents: `Primary`, `OnPrimary`, `PrimaryContainer`, `OnPrimaryContainer`, `InversePrimary` (Repeat for Secondary/Tertiary/Error).
    - [✓] Surfaces: `Surface`, `SurfaceDim`, `SurfaceBright`, `SurfaceContainerLowest`, `Low`, `Standard`, `High`, `Highest`, `OnSurface`, `OnSurfaceVariant`, `InverseSurface`, `InverseOnSurface`.
    - [✓] Outline: `Outline`, `OutlineVariant`.
  - [✓] Implement smooth color tweening algorithm for Light <-> Dark mode transitions.
- [✓] **Typography System (`m3_typography.h`)**
  - [✓] Map the exact M3 scales: `Display`, `Headline`, `Title`, `Label`, `Body` (Large, Medium, Small variants).
  - [✓] Exact metric enforcement: Font size (sp), Line height (dp), Tracking/Letter spacing (em), Weight.
  - [✓] Integrate `c-fs` and FreeType/HarfBuzz to load standard Roboto and custom fonts.
  - [✓] Support dynamic type scaling (accessibility multiplier up to 2.0x) without breaking layouts.
- [✓] **Shape System & Corner Interpolation (`m3_shapes.h`)**
  - [✓] Define standard scales: `None`, `Extra Small` (4dp), `Small` (8dp), `Medium` (12dp), `Large` (16dp), `Extra Large` (28dp), `Full` (Circle).
  - [✓] Support asymmetric shapes (e.g., top-left/top-right rounded, bottom squared for Bottom Sheets).
  - [✓] Implement shape morphing paths for animated transitions (e.g., FAB expanding into a Bottom Sheet).
- [✓] **Motion & Physics (`m3_motion.h`)**
  - [✓] Easing Curves: `Emphasized`, `Emphasized Decelerate`, `Emphasized Accelerate`, `Standard`, `Standard Decelerate`, `Standard Accelerate`.
  - [✓] Durations: `Short 1-4` (50-200ms), `Medium 1-4` (250-400ms), `Long 1-4` (450-600ms), `Extra Long 1-4` (700-1000ms).
  - [✓] Parallax and Shared Axis transition logic.

## Phase 3: Adaptive Layout & Window Sizing
- [✓] **Window Size Classes (`m3_adaptive.h`)**
  - [✓] Implement logic to detect `Compact` (< 600dp), `Medium` (600dp - 840dp), and `Expanded` (> 840dp).
- [✓] **Canonical Layouts**
  - [✓] List-Detail View: Stacked on Compact, side-by-side split pane on Expanded.
  - [✓] Supporting Pane: Hidden/Bottom Sheet on Compact, visible side panel on Expanded.
  - [✓] Feed Layout: Single column on Compact, responsive grid/masonry on Medium/Expanded.
- [✓] **Dynamic Navigation Scaffolding**
  - [✓] Compact: Use `Bottom Navigation Bar`.
  - [✓] Medium: Use `Navigation Rail` on the left edge.
  - [✓] Expanded: Use `Permanent Navigation Drawer`.
  - [ ] Implement smooth, animated threshold crossings when window is resized across breakpoints.

## Phase 4: Component Implementation (Exhaustive Library)

### 4.1 Actions & Buttons (`m3_buttons.c`)
- [ ] **Common Button Behaviors**: Minimum touch target 48x48dp, hover state layers (8% opacity), focus (10%), pressed (10%), disabled state (12% alpha text, 12% alpha background if filled).
- [ ] **Common Button Layouts**: Optional leading icon (18dp size, 8dp spacing), optional trailing icon.
- [ ] *Elevated Button*: SurfaceContainerLow background, Primary text, 1dp elevation resting, 2dp pressed.
- [ ] *Filled Button*: Primary background, OnPrimary text, 0dp elevation.
- [ ] *Filled Tonal Button*: SecondaryContainer background, OnSecondaryContainer text.
- [ ] *Outlined Button*: Transparent background, Outline border (1dp), Primary text.
- [ ] *Text Button*: Transparent background, Primary text, reduced horizontal padding.
- [ ] **FABs (Floating Action Buttons)**:
  - [ ] *Small FAB* (40x40dp), *Standard FAB* (56x56dp), *Large FAB* (96x96dp).
  - [ ] Color mappings: Primary, Secondary, Tertiary, Surface variants.
  - [ ] *Extended FAB*: Fluid width (wrap content) vs Fixed width. Icon + Text layout.
  - [ ] Scroll behavior: Extended FAB collapses to Standard FAB on scroll down, expands on scroll up.
- [ ] **Icon Buttons**:
  - [ ] Standard, Filled, Filled Tonal, and Outlined variants.
  - [ ] Toggleable states (Selected vs Unselected) with separate icons.
- [ ] **Segmented Buttons**:
  - [ ] Single-select and Multi-select variants.
  - [ ] Checkmark animation pushing the label upon selection.

### 4.2 Communication (`m3_communication.c`)
- [ ] **Badges**:
  - [ ] *Small dot*: 6x6dp, Error color, anchored to top-right of icon.
  - [ ] *Large badge*: 16dp height, pill shape, supports up to 3 digits (e.g., "99+").
- [ ] **Progress Indicators**:
  - [ ] *Linear*: Determinate (animated width), Indeterminate (two shrinking/expanding bars with specific bezier loops). Include Stop/Pause states.
  - [ ] *Circular*: Determinate (animated sweep), Indeterminate (rotating start/end angles, expanding/contracting sweep). Include 4-color morphing variant.
- [ ] **Snackbars**:
  - [ ] Queueing system (FIFO).
  - [ ] Single-line vs Two-line layout. Optional close icon. Optional text action.
  - [ ] InverseSurface background, InverseOnSurface text.
  - [ ] Automatic dismissal timeout (Short vs Long). Swipe to dismiss gesture.

### 4.3 Containment (`m3_containment.c`)
- [ ] **Bottom Sheets**:
  - [ ] Standard (always on screen, scrollable inner content).
  - [ ] Modal (scrim background, slide up animation).
  - [ ] Drag Handle indicator rendering (4x32dp pill).
  - [ ] Fling velocity threshold to dismiss.
- [ ] **Cards**:
  - [ ] *Elevated*: SurfaceContainerLow, 1dp elevation.
  - [ ] *Filled*: SurfaceContainerHighest, 0dp elevation.
  - [ ] *Outlined*: Surface background, Outline border.
  - [ ] Interactive states: Hover lift, drag elevation increase.
- [ ] **Dialogs**:
  - [ ] *Basic Dialog*: Icon, Title, Supporting Text, actions right-aligned.
  - [ ] *Full-screen Dialog*: Used on Compact screens for complex tasks (close 'X' at top left, 'Save' at top right).
- [ ] **Dividers**:
  - [ ] Horizontal and Vertical.
  - [ ] Variants: Full-width, Inset (aligned with text), Middle-inset.

### 4.4 Navigation (`m3_navigation.c`)
- [ ] **App Bars (Top)**:
  - [ ] Variants: *Center-aligned*, *Small*, *Medium*, *Large*.
  - [ ] Scroll integration: Background morphs from Surface to SurfaceContainer upon scroll intersection.
  - [ ] Large/Medium titles animate shrinking and moving to the Small title position on scroll.
- [ ] **App Bars (Bottom)**:
  - [ ] FAB cutout / cradle logic (optional in M3, usually FAB overlays now).
- [ ] **Navigation Bar (Bottom)**:
  - [ ] Label visibility: Always show, show only when selected, hide all.
  - [ ] Active indicator: Pill shape behind the active icon, animated width/position tweening when switching tabs.
- [ ] **Navigation Rail**:
  - [ ] Vertical alignment: Top, Center, Bottom.
  - [ ] FAB integration at the top of the rail.
- [ ] **Navigation Drawer**:
  - [ ] Standard (pushes content) vs Modal (overlays content with scrim).
  - [ ] Active item pill background.
- [ ] **Tabs**:
  - [ ] *Primary Tabs*: Indicator wraps the text width.
  - [ ] *Secondary Tabs*: Indicator fills the full tab width.
  - [ ] Scrollable vs Fixed width distributions.
  - [ ] Swipe integration with ViewPager/carousel content.

### 4.5 Selection & Input (`m3_inputs.c`)
- [ ] **Checkboxes**:
  - [ ] States: Unchecked, Checked, Indeterminate (dash icon).
  - [ ] Checkmark path drawing animation (draws from center to bottom-left to top-right).
- [ ] **Radio Buttons**:
  - [ ] Inner circle scale-up animation. Outer ring color transition.
- [ ] **Switches**:
  - [ ] Large thumb with optional check/X icon inside.
  - [ ] Track expands/contracts when thumb is pressed/dragged.
- [ ] **Chips**:
  - [ ] *Assist Chip*, *Filter Chip*, *Input Chip*, *Suggestion Chip*.
  - [ ] Elevated vs Flat variants.
  - [ ] Filter chip checkmark reveal animation (slides label to the right).
- [ ] **Sliders**:
  - [ ] Continuous vs Discrete.
  - [ ] Tick mark rendering for discrete.
  - [ ] Value indicator tooltip (teardrop/pill shape appearing above thumb on touch).
  - [ ] Range slider variant (two thumbs, track highlighting between them).
- [ ] **Text Fields**:
  - [ ] Variants: *Filled* (bottom border, background), *Outlined* (full border with label cutout).
  - [ ] State layers: Focused (thick primary border), Error (thick error border), Disabled.
  - [ ] Floating label animation (moves from placeholder position to top border position).
  - [ ] Supporting text, character counter limit tracking.
  - [ ] Text selection handlers (drag cursors, copy/paste floating toolbar).
  - [ ] Search Bar / Search View transition (bar expands to fill screen).

## Phase 5: Interaction, Polish, and A11y
- [ ] **State Layers & Ripple Dynamics (`m3_ripple.c`)**
  - [ ] Render the M3 state layer (semi-transparent overlay based on color role).
  - [ ] Implement M3 Ripple: Starts at touch point, expands as a clipped circle, lingers while pressed, fades out on release.
  - [ ] Custom ripple bounding boxes for complex shapes (e.g., pill-shaped buttons).
- [ ] **Accessibility (A11y)**
  - [ ] Map all UI components to `cmp_a11y_tree_t`.
  - [ ] Enforce standard `contentDescription` logic.
  - [ ] Announce state changes (e.g., "Switch, On", "Tab 2 of 4").
  - [ ] Support High Contrast mode overrides (bypass primary colors for deep blacks/whites).
- [ ] **Internationalization (i18n) & RTL**
  - [ ] Support Right-To-Left layout mirroring automatically.
  - [ ] Flip directional icons (e.g., back arrows) while leaving non-directional icons (e.g., checkmarks) intact.

## Phase 6: Catalog App Screens & Tools
- [ ] **Home Screen (Catalog Grid)**
  - [ ] Responsive masonry or grid layout of component category cards.
  - [ ] Filter/Search bar at the top to find components by name.
- [ ] **Component Detail Screen**
  - [ ] Dynamic layout showing variations of the selected component.
  - [ ] "Interactive Playground" pane: sliders to tweak padding, color roles, enabled/disabled states live.
- [ ] **Theme Studio Tool**
  - [ ] A dedicated screen to build custom M3 themes.
  - [ ] Seed color hex input.
  - [ ] Visual preview of the generated tonal palettes.
  - [ ] Export to JSON / save to disk functionality.
- [ ] **Automated Testing Suite**
  - [ ] *Unit Tests*: M3 color math accuracy, layout rect calculation bounds.
  - [ ] *Integration Tests*: Simulated touch events triggering router changes.
  - [ ] *Visual Regression*: `screenshot_generator.c` configured to render all 100+ component variants in Light, Dark, LTR, and RTL modes, diffing against golden bitmaps with a <1% tolerance.
  - [ ] *Performance Profiling*: Ensure deep hierarchies (e.g., nested tabs in expanded dialogs in scroll views) render at 60+ FPS on target hardware.