# The Exhaustive Google Material 3, Material You & M3 Expressive Compliance Matrix

This document is the absolute, exhaustive, token-level compliance checklist for implementing Google Material Design 3 (M3), Material You (Dynamic Color), and Material 3 Expressive. It maps directly to the official Material Design token structures (`md.ref`, `md.sys`, `md.comp`), algorithmic physics, platform-specific overrides, and accessibility standards.

*Note for LLMs and Contributors:* This checklist is designed to track progress at the deepest architectural level. Check off items (`- [x]`) only when they are fully implemented, algorithmically verified, visually tested, and strictly adhere to the official Material Design 3 guidelines.

---

## 1. Core Foundations & Reference Tokens (`md.ref`)

### 1.1. Color Physics & HCT Space (CAM16)
- [ ] Implement CAM16 viewing conditions & chromatic adaptation.
- [ ] Implement HCT (Hue, Chroma, Tone) space conversion to/from sRGB.
- [ ] Implement HCT space conversion to/from Display P3.
- [ ] Implement HCT space conversion to/from Linear RGB.
- [ ] Ensure CIEDE2000 / L*a*b* perceptual uniformity in tone generation.

### 1.2. Tonal Palette Generation
*(Generate the following tones for each palette: 0, 10, 20, 25, 30, 35, 40, 50, 60, 70, 80, 90, 95, 98, 99, 100)*
- [ ] `md.ref.palette.primary`
- [ ] `md.ref.palette.secondary`
- [ ] `md.ref.palette.tertiary`
- [ ] `md.ref.palette.error`
- [ ] `md.ref.palette.neutral`
- [ ] `md.ref.palette.neutral-variant`

### 1.3. Material Color Utilities (MCU) Scheme Variants
- [ ] `SchemeTonalSpot` (Default M3 pastel/muted harmony)
- [ ] `SchemeSpritz` (Desaturated, monochromatic-leaning)
- [ ] `SchemeVibrant` (High chroma expansion)
- [ ] `SchemeExpressive` (Hue rotation for tertiary/secondary)
- [ ] `SchemeRainbow` (Hue-shifted vibrant tones)
- [ ] `SchemeFruitSalad` (High contrast, multi-hue)
- [ ] `SchemeMonochrome` (Tone-only, 0 chroma)
- [ ] `SchemeFidelity` (Exact source color preservation for brand seeds)
- [ ] `SchemeContent` (Media-driven extraction)

### 1.4. Typography Reference (`md.ref.typeface`)
- [ ] **Weight Mapping**
  - [ ] Regular (400)
  - [ ] Medium (500)
  - [ ] Semi-bold (600)
  - [ ] Bold (700)
- [ ] **Font Family Bridges**
  - [ ] Brand (`md.ref.typeface.brand`)
  - [ ] Plain (`md.ref.typeface.plain`)

---

## 2. System Tokens (`md.sys`)

### 2.1. System Color Roles (`md.sys.color`) - Light Theme
- [ ] Primary / On Primary
- [ ] Primary Container / On Primary Container
- [ ] Secondary / On Secondary
- [ ] Secondary Container / On Secondary Container
- [ ] Tertiary / On Tertiary
- [ ] Tertiary Container / On Tertiary Container
- [ ] Error / On Error
- [ ] Error Container / On Error Container
- [ ] Surface Dim / Surface / Surface Bright
- [ ] Surface Container Lowest / Low / Standard / High / Highest
- [ ] On Surface / On Surface Variant
- [ ] Outline / Outline Variant
- [ ] Inverse Surface / Inverse On Surface / Inverse Primary
- [ ] Scrim / Shadow / Surface Tint
- [ ] Primary Fixed / Primary Fixed Dim / On Primary Fixed / On Primary Fixed Variant
- [ ] Secondary Fixed / Secondary Fixed Dim / On Secondary Fixed / On Secondary Fixed Variant
- [ ] Tertiary Fixed / Tertiary Fixed Dim / On Tertiary Fixed / On Tertiary Fixed Variant

### 2.2. System Color Roles (`md.sys.color`) - Dark Theme
- [ ] Primary / On Primary
- [ ] Primary Container / On Primary Container
- [ ] Secondary / On Secondary
- [ ] Secondary Container / On Secondary Container
- [ ] Tertiary / On Tertiary
- [ ] Tertiary Container / On Tertiary Container
- [ ] Error / On Error
- [ ] Error Container / On Error Container
- [ ] Surface Dim / Surface / Surface Bright
- [ ] Surface Container Lowest / Low / Standard / High / Highest
- [ ] On Surface / On Surface Variant
- [ ] Outline / Outline Variant
- [ ] Inverse Surface / Inverse On Surface / Inverse Primary
- [ ] Scrim / Shadow / Surface Tint
- [ ] Primary Fixed / Primary Fixed Dim / On Primary Fixed / On Primary Fixed Variant
- [ ] Secondary Fixed / Secondary Fixed Dim / On Secondary Fixed / On Secondary Fixed Variant
- [ ] Tertiary Fixed / Tertiary Fixed Dim / On Tertiary Fixed / On Tertiary Fixed Variant

### 2.3. Dynamic Contrast Profiles
- [ ] **Standard:** Baseline M3 specs (3:1 UI, 4.5:1 Text).
- [ ] **Medium:** Increased contrast container tones.
- [ ] **High:** Maximum WCAG AAA compliance.

### 2.4. System Typography Scale (`md.sys.typescale`)
*(Mapping: Font Size / Line Height / Tracking)*
- [ ] **Display**
  - [ ] Large (57/64/-0.25)
  - [ ] Medium (45/52/0)
  - [ ] Small (36/44/0)
- [ ] **Headline**
  - [ ] Large (32/40/0)
  - [ ] Medium (28/36/0)
  - [ ] Small (24/32/0)
- [ ] **Title**
  - [ ] Large (22/28/0)
  - [ ] Medium (16/24/0.15)
  - [ ] Small (14/20/0.1)
- [ ] **Body**
  - [ ] Large (16/24/0.5)
  - [ ] Medium (14/20/0.25)
  - [ ] Small (12/16/0.4)
- [ ] **Label**
  - [ ] Large (14/20/0.1)
  - [ ] Medium (12/16/0.5)
  - [ ] Small (11/16/0.5)

### 2.5. System Shape (`md.sys.shape`)
- [ ] None (0dp)
- [ ] Extra Small (4dp)
- [ ] Small (8dp)
- [ ] Medium (12dp)
- [ ] Large (16dp)
- [ ] Extra Large (28dp)
- [ ] Full (Pill/Circular)
- [ ] Asymmetric Top-Left manipulation
- [ ] Asymmetric Top-Right manipulation
- [ ] Asymmetric Bottom-Left manipulation
- [ ] Asymmetric Bottom-Right manipulation

### 2.6. Elevation & State Layers (`md.sys.elevation` / `md.sys.state`)
- [ ] **Tonal Elevation (Tint Opacity on Surface)**
  - [ ] Level 0 (0%)
  - [ ] Level 1 (5%)
  - [ ] Level 2 (8%)
  - [ ] Level 3 (11%)
  - [ ] Level 4 (12%)
  - [ ] Level 5 (14%)
- [ ] **Shadow Elevation (Y-offset & Blur)**
  - [ ] Level 0 (0dp)
  - [ ] Level 1 (1dp)
  - [ ] Level 2 (3dp)
  - [ ] Level 3 (6dp)
  - [ ] Level 4 (8dp)
  - [ ] Level 5 (12dp)
  - [ ] Ambient shadow layering
  - [ ] Spot shadow layering
- [ ] **Interactive States (Overlay applied to `On-*` colors)**
  - [ ] Hover: 8%
  - [ ] Focus: 10%
  - [ ] Pressed: 10%
  - [ ] Dragged: 16%
  - [ ] Disabled Container: 12% (on Surface)
  - [ ] Disabled Content: 38% (on Surface)

---

## 3. Material 3 Expressive & Motion (`md.sys.motion`)

### 3.1. Easing Curves
- [ ] Emphasized Easing `(0.2, 0.0, 0, 1.0)`
- [ ] Emphasized Accelerate `(0.3, 0.0, 0.8, 0.15)`
- [ ] Emphasized Decelerate `(0.05, 0.7, 0.1, 1.0)`
- [ ] Standard Easing `(0.2, 0.0, 0, 1.0)`
- [ ] Standard Accelerate `(0.3, 0.0, 1.0, 1.0)`
- [ ] Standard Decelerate `(0.0, 0.0, 0.0, 1.0)`
- [ ] Linear `(0.0, 0.0, 1.0, 1.0)`

### 3.2. Durations
- [ ] Short 1 (50ms)
- [ ] Short 2 (100ms)
- [ ] Short 3 (150ms)
- [ ] Short 4 (200ms)
- [ ] Medium 1 (250ms)
- [ ] Medium 2 (300ms)
- [ ] Medium 3 (350ms)
- [ ] Medium 4 (400ms)
- [ ] Long 1 (450ms)
- [ ] Long 2 (500ms)
- [ ] Long 3 (550ms)
- [ ] Long 4 (600ms)
- [ ] Extra Long 1 (700ms)
- [ ] Extra Long 2 (800ms)
- [ ] Extra Long 3 (900ms)
- [ ] Extra Long 4 (1000ms)

### 3.3. Complex Pattern Animations
- [ ] **Container Transform**
  - [ ] Cross-fade content
  - [ ] Bounds morphing
  - [ ] Shape morphing
- [ ] **Shared Axis**
  - [ ] X translation + fade
  - [ ] Y translation + fade
  - [ ] Z translation + fade
- [ ] **Fade Through**
  - [ ] Swap boundaries
- [ ] **Predictive Back**
  - [ ] Root window scaling (90%)
  - [ ] Corner rounding
  - [ ] Edge-pull translation

---

## 4. Canonical Layouts & Adaptive Grid

### 4.1. Window Size Classes
- [ ] **Compact (< 600dp):** 4 Columns, 16dp Margins, 16dp Gutters.
- [ ] **Medium (600 - 839dp):** 8 Columns, 24dp Margins, 24dp Gutters.
- [ ] **Expanded (840 - 1199dp):** 12 Columns, 24dp Margins, 24dp Gutters.
- [ ] **Large (1200+ dp):** 12 Columns, 24dp Margins, Max Body Width logic.

### 4.2. Layout Patterns
- [ ] **List-Detail:** Split pane Stack in Compact
- [ ] **List-Detail:** Split pane Side-by-Side in Expanded
- [ ] **Supporting Pane:** Focus pane + secondary pane.
- [ ] **Feed:** Staggered / masonry flow based on grid.

### 4.3. Foldable / Hinge Physics
- [ ] Hardware hinge occlusion avoidance.
- [ ] Posture adaptation: Tabletop push-up
- [ ] Posture adaptation: Book mode split

---

## 5. Exhaustive Component Specifications (`md.comp`)

### 5.1. Buttons
- [ ] **Elevated Button**
  - [ ] Container: 40dp height, Full shape (Pill).
  - [ ] Container Color: Surface Container Low
  - [ ] Elevation: 1 (Hover 2, Pressed 1)
  - [ ] Padding (No Icon): 24dp L/R
  - [ ] Padding (With Leading Icon): 16dp L / 8dp Gap / 24dp R
  - [ ] Icon Size: 18dp
  - [ ] Label: Label Large
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Filled Button**
  - [ ] Container: 40dp height, Full shape (Pill).
  - [ ] Container Color: Primary
  - [ ] Label Color: On Primary
  - [ ] Elevation: 0
  - [ ] Padding (No Icon): 24dp L/R
  - [ ] Padding (With Leading Icon): 16dp L / 8dp Gap / 24dp R
  - [ ] Icon Size: 18dp
  - [ ] Label: Label Large
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Filled Tonal Button**
  - [ ] Container: 40dp height, Full shape (Pill).
  - [ ] Container Color: Secondary Container
  - [ ] Label Color: On Secondary Container
  - [ ] Elevation: 0
  - [ ] Padding (No Icon): 24dp L/R
  - [ ] Padding (With Leading Icon): 16dp L / 8dp Gap / 24dp R
  - [ ] Icon Size: 18dp
  - [ ] Label: Label Large
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Outlined Button**
  - [ ] Container: 40dp height, Full shape (Pill).
  - [ ] Container Color: Transparent
  - [ ] Border Color: Outline (1dp)
  - [ ] Label Color: Primary
  - [ ] Elevation: 0
  - [ ] Padding (No Icon): 24dp L/R
  - [ ] Padding (With Leading Icon): 16dp L / 8dp Gap / 24dp R
  - [ ] Icon Size: 18dp
  - [ ] Label: Label Large
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Text Button**
  - [ ] Container: 40dp height, Full shape (Pill).
  - [ ] Container Color: Transparent
  - [ ] Label Color: Primary
  - [ ] Elevation: 0
  - [ ] Padding (No Icon): 12dp L/R
  - [ ] Padding (With Leading Icon): 12dp L / 8dp Gap / 16dp R
  - [ ] Icon Size: 18dp
  - [ ] Label: Label Large
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Floating Action Button (FAB)**
  - [ ] Standard: 56dp container, 24dp icon, Elev 3.
  - [ ] Small: 40dp container, 24dp icon, Elev 3.
  - [ ] Large: 96dp container, 36dp icon, Elev 3.
  - [ ] Extended: 56dp height, 16dp padding L/R, dynamic width morphing.
  - [ ] Lowered FAB variant (Elev 1 default)
  - [ ] Color Variant: Surface
  - [ ] Color Variant: Primary
  - [ ] Color Variant: Secondary
  - [ ] Color Variant: Tertiary
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Segmented Button**
  - [ ] 40dp height
  - [ ] Outline color border (1dp)
  - [ ] Single-select variant
  - [ ] Multi-select variant
  - [ ] Checkmark animation pushing text horizontally (18dp checkmark)
  - [ ] Active segment container: Secondary Container
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed, Selected
- [ ] **Icon Button**
  - [ ] Target size 48x48dp, Visual footprint 40x40dp.
  - [ ] Standard variant
  - [ ] Filled variant
  - [ ] Filled Tonal variant
  - [ ] Outlined variant
  - [ ] Toggleable states (Selected vs Unselected variants)
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed

### 5.2. Communication
- [ ] **Badges**
  - [ ] Small Dot: 6x6dp, Color: Error.
  - [ ] Large Label: 16dp height, min width 16dp.
  - [ ] Large Label Padding: 4dp L/R.
  - [ ] Large Label Font: Label Small.
  - [ ] Automatic overflow handling (e.g., "99+")
- [ ] **Progress Indicators**
  - [ ] Linear Determinate (Stop/resume interpolation)
  - [ ] Linear Indeterminate (Dual bar racing)
  - [ ] Circular Determinate (4dp stroke)
  - [ ] Circular Indeterminate (Expressive expanding arc)
  - [ ] Track Color: Surface Variant
  - [ ] Indicator Color: Primary
- [ ] **Snackbars**
  - [ ] Container Color: Inverse Surface
  - [ ] Label Color: Inverse On Surface
  - [ ] Action Button Color: Inverse Primary
  - [ ] Elevation: 3
  - [ ] Shape: Extra Small
  - [ ] Single-line padding: 16dp L/R.
  - [ ] Multi-line padding handling.
  - [ ] Close icon support
  - [ ] Stacking / queue behavior
- [ ] **Pull to Refresh**
  - [ ] Circular indicator in pill container.
  - [ ] Stretch overscroll physics.
  - [ ] Spinning refresh state.

### 5.3. Containment
- [ ] **Cards**
  - [ ] Elevated Card: Surface Container Low, Elev 1, Shape Medium (12dp).
  - [ ] Filled Card: Surface Container Highest, Elev 0, Shape Medium (12dp).
  - [ ] Outlined Card: Surface, Outline Variant border (1dp), Elev 0, Shape Medium (12dp).
  - [ ] Draggable / Swipeable states
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed, Dragged
- [ ] **Dialogs**
  - [ ] Container Color: Surface Container High
  - [ ] Elevation: 3
  - [ ] Shape: Extra Large (28dp)
  - [ ] Padding: 24dp (Title, Body, Actions)
  - [ ] Actions right-aligned
  - [ ] Full-screen Dialog: Elev 0, App bar integration
  - [ ] Full-screen Dialog: Slide-up animation
  - [ ] Scrollable content areas with visual dividers
  - [ ] Hero transitions (Expressive)
- [ ] **Dividers**
  - [ ] Thickness: 1dp
  - [ ] Color: Outline Variant
  - [ ] Full-bleed variant
  - [ ] Inset variant (16dp/72dp start)
  - [ ] Middle inset variant
- [ ] **Tooltips**
  - [ ] Plain Tooltip: Height 24dp, Padding 8dp, Container Inverse Surface.
  - [ ] Plain Tooltip: Transient fade in/out
  - [ ] Rich Tooltip: Contains Headline, Body, and Actions.
  - [ ] Rich Tooltip: Elevation 2.
  - [ ] Rich Tooltip: Persistent

### 5.4. Navigation
- [ ] **Bottom App Bar**
  - [ ] Height: 80dp
  - [ ] Container Color: Surface Container
  - [ ] FAB Cutout radius + spacing (symmetric)
  - [ ] Action item slots (max 4)
  - [ ] Hide on scroll behavior
- [ ] **Navigation Bar (Bottom Nav)**
  - [ ] Height: 80dp
  - [ ] Active Indicator: Pill shape (32x64dp)
  - [ ] Active Indicator Color: Secondary Container
  - [ ] Label visibility: Always
  - [ ] Label visibility: Selected
  - [ ] Label visibility: Never
  - [ ] Badge integration
- [ ] **Navigation Drawer**
  - [ ] Modal Drawer: Surface Container Low, 360dp max width, Scrim 32%.
  - [ ] Standard Drawer: Persistent, inline with layout.
  - [ ] Active Item: Pill shape, Secondary Container.
  - [ ] Active Item Padding: 16dp L/R.
  - [ ] Header / Title slots
- [ ] **Navigation Rail**
  - [ ] Width: 80dp
  - [ ] FAB integration slot at top
  - [ ] Center aligned items
  - [ ] Active Indicator: Pill shape
  - [ ] Label visibility behavior
  - [ ] Badge integration
- [ ] **Top App Bar**
  - [ ] Center-aligned: 64dp height, Title centered.
  - [ ] Small: 64dp height, Title left-aligned.
  - [ ] Medium: 112dp height, scrolls up to 64dp, Title expands.
  - [ ] Large: 152dp height, Expressive multi-line, Title scales down on scroll.
  - [ ] Scroll behavior: Pinned
  - [ ] Scroll behavior: Enter Always
  - [ ] Scroll behavior: Enter Always Collapsed
  - [ ] Scroll behavior: Exit Until Collapsed
- [ ] **Tabs**
  - [ ] Primary Tabs: 48dp height (icon+text)
  - [ ] Primary Tabs Active Indicator: Pill shape matching Primary color.
  - [ ] Secondary Tabs: 48dp height (text only)
  - [ ] Secondary Tabs Active Indicator: 2dp bottom line.
  - [ ] Fixed variant
  - [ ] Scrollable variant (Fading edges)
  - [ ] Indicator transitions

### 5.5. Selection Controls
- [ ] **Checkboxes**
  - [ ] Box Size: 18x18dp
  - [ ] Touch Target: 48x48dp
  - [ ] Checked state: Primary container, On Primary checkmark.
  - [ ] Unchecked state: Outline colored border (2dp).
  - [ ] Indeterminate state: Dash icon.
  - [ ] Error state: Error color override.
  - [ ] Morphing animations between states.
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Radio Buttons**
  - [ ] Outer ring: 20x20dp
  - [ ] Inner dot: 10x10dp
  - [ ] Touch Target: 48x48dp
  - [ ] Selected state
  - [ ] Unselected state
  - [ ] Concentric ring pop animation
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Switches**
  - [ ] Track: 52x32dp
  - [ ] Touch Target: 48x48dp
  - [ ] Thumb Unselected: 16x16dp (1dp Outline)
  - [ ] Thumb Selected: 24x24dp (Primary)
  - [ ] Thumb expands to 28x28dp on press.
  - [ ] Optional 16dp icon inside thumb (Check / X).
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Sliders**
  - [ ] Track height: 4dp to 16dp (Expressive)
  - [ ] Handle: 20x20dp
  - [ ] Touch Target: 48x48dp
  - [ ] Continuous Slider
  - [ ] Discrete Slider (with tick marks)
  - [ ] Value tooltip bubble
  - [ ] Range Slider (Dual thumb)
  - [ ] Range Slider overlapping tooltip collision resolution
  - [ ] Expressive Media Slider (Squiggly track representing waveform/playing state)
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed

### 5.6. Text Inputs
- [ ] **Filled Text Field**
  - [ ] Container: Surface Variant
  - [ ] Shape: Extra Small (Top corners only)
  - [ ] Bottom line unfocused: 1dp (On Surface Variant)
  - [ ] Bottom line focused: 2dp (Primary)
- [ ] **Outlined Text Field**
  - [ ] Container: Transparent
  - [ ] Border unfocused: 1dp (Outline)
  - [ ] Border focused: 2dp (Primary)
  - [ ] Label Cutout: Dynamic SVG/Path breaking for floating label.
- [ ] **Features (Both variants)**
  - [ ] Single-line
  - [ ] Multi-line (Text Area)
  - [ ] Floating label animation (Body Large -> Label Small)
  - [ ] Leading icon (24dp)
  - [ ] Trailing icon (24dp)
  - [ ] Prefix text
  - [ ] Suffix text
  - [ ] Helper text
  - [ ] Error messages (Error color overrides)
  - [ ] Character counter (e.g., "0 / 100")
  - [ ] States: Enabled, Disabled, Hover, Focus, Error

### 5.7. Information & Data
- [ ] **Chips**
  - [ ] Container Height: 32dp
  - [ ] Shape: Small (8dp)
  - [ ] Icon Size: 18dp
  - [ ] Avatar Size: 24dp
  - [ ] Padding adjusts dynamically based on leading elements.
  - [ ] Assist Chip (Elevated & Flat)
  - [ ] Filter Chip (Elevated & Flat)
  - [ ] Filter Chip: Leading checkmark animation shifts label.
  - [ ] Input Chip
  - [ ] Input Chip: Contains trailing 'X' to delete.
  - [ ] Suggestion Chip (Elevated & Flat)
  - [ ] States: Enabled, Disabled, Hover, Focus, Pressed
- [ ] **Lists**
  - [ ] One-line item (56dp height)
  - [ ] Two-line item (72dp height)
  - [ ] Three-line item (88dp height)
  - [ ] Leading visuals (Icons, Avatars, Images, Checkboxes)
  - [ ] Trailing visuals (Icons, Text, Meta)
- [ ] **Data Tables**
  - [ ] Header row (Label Small text)
  - [ ] Content rows (Body Medium text)
  - [ ] Sorting arrows (Ascending/Descending)
  - [ ] Checkbox row selection
  - [ ] Pagination controls (Rows per page, Previous/Next)
- [ ] **Carousels (Expressive)**
  - [ ] Multi-browse carousel (Peeking cards, parallax clipping)
  - [ ] Hero carousel (Large primary item, smaller peeking items)
  - [ ] Uncontained carousel (Free-scrolling image strips)
  - [ ] Fluid parallax morphing animations

### 5.8. Pickers & Menus
- [ ] **Date Pickers**
  - [ ] Modal calendar picker (Surface Container High, Elev 3)
  - [ ] Docked calendar picker
  - [ ] Day selection circles
  - [ ] Date range picker (Selection pill stretching across multiple days)
  - [ ] Keyboard input mode parsing
- [ ] **Time Pickers**
  - [ ] Dial picker (Analog clock hands, dragging)
  - [ ] Input picker (Digital text fields)
  - [ ] AM/PM segmented button selector
  - [ ] AM/PM vertical/horizontal flip based on orientation
- [ ] **Menus**
  - [ ] Dropdown Menu (Surface Container, Elev 2, Shape Extra Small)
  - [ ] Cascading / Sub-menus (Positioning logic to prevent clipping)
  - [ ] Exposed Dropdown Menu (Combobox)
- [ ] **Sheets (Bottom, Side)**
  - [ ] Standard Bottom Sheet (Persistent, Co-exists with UI, elevated)
  - [ ] Modal Bottom Sheet (Scrim 32% opacity)
  - [ ] Drag handle (4x32dp, Outline color)
  - [ ] Standard Side Sheet
  - [ ] Modal Side Sheet
  - [ ] Detached Side Sheet (Expressive, floats above UI with margins, fully rounded)

---

## 6. Edge Cases, Haptics & Accessibility

### 6.1. Haptics & Physical Interactions
- [ ] System vibration hook for Slider snapping
- [ ] System vibration hook for Switch toggling
- [ ] System vibration hook for Long-press triggers
- [ ] System vibration hook for Swipe-to-dismiss

### 6.2. Edge-to-Edge System Bar Insets
- [ ] Transparent Navigation Bars
- [ ] Transparent Status Bars
- [ ] Safe Area Inset padding (Prevents FABs drawing under iOS/Android home indicator)

### 6.3. Semantic Accessibility (A11y)
- [ ] Touch Targets: Enforce strict 48x48dp minimums (expanding hitboxes without expanding visual footprint).
- [ ] Focus Traversal: Strict TAB-key ring rendering (Outline color, 2dp thickness, 2dp offset from element bounds).
- [ ] Minimum contrast ratios (WCAG AA/AAA) across all dynamic color palettes.
- [ ] Screen Reader Mapping: Role
- [ ] Screen Reader Mapping: Name/Label
- [ ] Screen Reader Mapping: Value
- [ ] Screen Reader Mapping: State (Checked/Disabled)
- [ ] Screen Reader Mapping: Actions
- [ ] `Prefers-Reduced-Motion` OS override: Disables container transforms.
- [ ] `Prefers-Reduced-Motion` OS override: Limits animations to opacity crossfades.

### 6.4. Internationalization (i18n)
- [ ] RTL (Right-to-Left) Matrix Reversal: Mirror layouts
- [ ] RTL (Right-to-Left) Matrix Reversal: Mirror paddings
- [ ] RTL (Right-to-Left) Matrix Reversal: Mirror navigation drawer docking
- [ ] RTL (Right-to-Left) Matrix Reversal: Mirror tab directions
- [ ] Bi-Directional (BiDi) Text Shaping: Arabic/Hebrew script rendering within LTR components
- [ ] Locale-aware date formatting
- [ ] Locale-aware time formatting
- [ ] Locale-aware number formatting

---
**Status:** In Progress
**Loop Condition:** This matrix will be repeated and refined until every micro-interaction, token value, and platform-specific behavior is 100% compliant with the complete Google Material 3 Expressive spec.