# Material Catalog Implementation Plan (C-Multiplatform)

This document contains an exhaustive, low-level plan to build a pixel and behavior-perfect clone of the official Android Jetpack Compose Material Catalog app, ported to C using the `c-multiplatform` (CMP) framework. 

This plan accounts for the fundamental differences between Kotlin/Compose and C, outlining the necessary state management, memory handling, rendering pipelines, and exact component logic required to achieve parity.

## Phase 1: Core Architecture & Memory Management
- [x] **State Management & Reactivity (C-Equivalent)**
  - [x] Implement `catalog_state_t` global context (holds navigation stack, current theme, window dimensions).
  - [x] Build a UI invalidation system (equivalent to Compose's Recomposition trigger) to selectively redraw nodes when state changes.
  - [x] Define memory ownership semantics (arena allocators for UI nodes per frame/screen vs. persistent state allocations).
- [x] **Data Models (`Components.kt` & `Examples.kt`)**
  - [x] Define `material_component_t` struct (id, name, description, icon resource ID, tint icon flag, URLs).
  - [x] Define `material_example_t` struct (name, description, source URL, function pointer `void (*render)(cmp_ui_node_t*)`).
  - [x] Statically allocate the global component registry (24 core components).
  - [x] Statically allocate the 60+ example metadata definitions.
- [ ] **Navigation System (`NavGraph.kt`)**
  - [x] Implement `catalog_router_t` for stack-based navigation.
  - [x] Route handler for `Home` (Screen ID: 0).
  - [x] Route handler for `Component Details` (Screen ID: 1, arg: `component_id`).
  - [x] Route handler for `Example Viewer` (Screen ID: 2, args: `component_id`, `example_index`).
  - [x] Implement Back Stack management (handling OS back button/Esc key).
  - [ ] Cross-fade transition animations between route changes.

## Phase 2: Theming & Styling Engine (`Themes.kt`, `Theme.kt`, `Color.kt`)
- [x] **Color System**
  - [x] Implement 10 core Material color palettes (Blue, Brown, Green, Indigo, Orange, Pink, Purple, Red, Teal, Yellow).
  - [x] Implement Light/Dark variants for each palette (Primary, PrimaryVariant, Secondary, SecondaryVariant, Background, Surface, Error, On* variants).
  - [x] Implement color math utilities (`color_contrast_ratio`, `is_light_color`, `variant_color`, `on_color` generation).
- [x] **Typography System**
  - [x] Define `theme_font_family_t` enum (Default, SansSerif, Serif, Monospace, Cursive).
  - [x] Implement font loading via `c-fs` for the different typeface files.
  - [x] Map Material Typography scales (H1-H6, Subtitle1-2, Body1-2, Button, Caption, Overline) to font sizes, weights, and letter spacings.
- [x] **Shape System**
  - [x] Define `theme_shape_corner_family_t` (Rounded, Cut).
  - [x] Implement path generators for Rounded rectangles vs. Cut corner (beveled) rectangles.
  - [x] Global shape size constraints (Small max 16dp, Medium max 32dp, Large max 48dp).
- [x] **Theme Persistence (`c-orm` / `c-fs` integration)**
  - [x] Define `theme_saver_t` to serialize `material_theme_t` to a local config file.
  - [x] Load saved theme preferences on application startup.

## Phase 3: Foundation UI Components & Layouts
- [x] **Window Insets & Safe Areas**
  - [x] Integrate OS-level status bar and navigation bar padding.
  - [x] Scaffold `contentWindowInsets` mapping to prevent drawing under system UI.
- [x] **Material Ripple & Interaction State**
  - [x] Implement touch/click coordinate tracking.
  - [x] Build the `RippleDrawable` equivalent (expanding circle animation clipped to bounds, fading out on release).
  - [x] State layer overlays (Hover: 4% alpha, Focus: 12% alpha, Pressed: 12% alpha).
- [x] **Catalog Scaffold (`CatalogScaffold.kt`)**
  - [x] Main layout constraints.
  - [x] Top App Bar slot integration.
  - [x] Modal Bottom Sheet layout wrapper (for the Theme Picker overlay).
  - [x] `SheetScrim` rendering (Color.Black at 32% alpha, blocking underlying input, dismiss-on-tap).
- [x] **Catalog Top App Bar (`CatalogTopAppBar.kt`)**
  - [x] Title rendering with text overflow ellipsis calculation.
  - [x] Dynamic Back navigation icon (only shown if Nav Stack > 1).
  - [x] Theme palette toggle button.
  - [x] 'More' vertical dropdown menu (anchored popup, off-click dismissal).
  - [x] URL launching abstraction (`Url.kt` -> `c-abstract-http` or OS-native `ShellExecute`/`open`).
- [x] **Theme Picker UI (`ThemePicker.kt`)**
  - [x] Drag-to-dismiss gesture for the Bottom Sheet.
  - [x] `LazyColumn` equivalent layout for scrollable theme options.
  - [x] Horizontal `LazyRow` for Color selection RadioButtons.
  - [x] Font family grid selection.
  - [x] Shape corner size continuous sliders (0f to Max).
  - [x] "Apply" (diff check to enable/disable) and "Reset to Default" logic.

## Phase 4: Primary Application Screens
- [x] **Home Screen (`Home.kt`)**
  - [x] Responsive `LazyVerticalGrid` (calculate `cellsCount = maxOf((maxWidth / 180dp), 1)`).
  - [x] Implement inner border rendering (`gridItemBorder`) for bottom/end cell separation.
  - [x] `ComponentItem` click handlers to trigger Router navigation.
- [x] **Component Details Screen (`Component.kt`)**
  - [x] Vertical scroll view implementation.
  - [x] Header: 108dp centered SVG icon rendering (with conditional disabled-alpha tinting).
  - [x] Body: Description typography rendering.
  - [x] List: Loop through component examples and render `ExampleItem` cards (1dp border, content alpha medium, chevron icon).
- [x] **Example Viewer Screen (`Example.kt`)**
  - [x] Fullscreen Scaffold with injected example title.
  - [x] Box layout with `Alignment.Center` mapping.
  - [x] Isolation of example state (ensure exiting an example cleans up its specific memory/animations).

## Phase 5: Material Component Implementation & Samples
*This phase requires building the actual C-Multiplatform UI controls that match Material specifications, followed by implementing the specific catalog sample views.*

- [x] **App Bars**
  - [x] *Implementation:* `cmp_top_app_bar`, `cmp_bottom_app_bar` (elevation shadows, FAB cutouts).
  - [x] *Samples:* `SimpleBottomAppBar`, `SimpleTopAppBar`.
- [ ] **Backdrop**
  - [ ] *Implementation:* `cmp_backdrop_scaffold` (front layer, back layer, reveal/conceal animations).
  - [ ] *Samples:* `BackdropScaffoldSample`.
- [x] **Badges**
  - [x] *Implementation:* `cmp_badge_box` (dot size, text size constraints, top-end anchor positioning).
  - [x] *Samples:* `BottomNavigationItemWithBadge`.
- [x] **Navigation & Bottom Navigation**
  - [x] *Implementation:* `cmp_bottom_navigation`, `cmp_navigation_rail` (label animations, active indicators).
  - [x] *Samples:* `BottomNavigationSample`, `BottomNavigationWithOnlySelectedLabelsSample`, `NavigationRailSample`, `NavigationRailWithOnlySelectedLabelsSample`, `CompactNavigationRailSample`, `NavigationRailBottomAlignSample`.
- [x] **Buttons & FABs**
  - [x] *Implementation:* `cmp_button`, `cmp_outlined_button`, `cmp_text_button`, `cmp_fab`, `cmp_extended_fab` (elevation physics, bounds layout).
  - [x] *Samples:* `ButtonSample`, `OutlinedButtonSample`, `TextButtonSample`, `ButtonWithIconSample`, `SimpleFab`, `SimpleExtendedFabNoIcon`, `SimpleExtendedFabWithIcon`, `FluidExtendedFab`.
- [x] **Cards**
  - [x] *Implementation:* `cmp_card` (shadow drawing, corner clipping).
  - [x] *Samples:* `CardSample`, `ClickableCardSample`.
- [x] **Selection Controls (Checkboxes, Radio, Switch)**
  - [x] *Implementation:* `cmp_checkbox` (checkmark path interpolation, tri-state logic), `cmp_radio_button` (inner circle scale animation), `cmp_switch` (thumb sliding, track color morphing).
  - [x] *Samples:* `CheckboxSample`, `TriStateCheckboxSample`, `RadioButtonSample`, `RadioGroupSample`, `SwitchSample`.
- [x] **Chips**
  - [x] *Implementation:* `cmp_chip`, `cmp_chip_group` (horizontal reflow layouts, leading icon logic).
  - [x] *Samples:* `ChipSample`, `OutlinedChipWithIconSample`, `ChipGroupSingleLineSample`, `ChipGroupReflowSample`.
- [x] **Dialogs**
  - [x] *Implementation:* `cmp_alert_dialog` (focus trapping, modal window overlay).
  - [x] *Samples:* `AlertDialogSample`, `CustomAlertDialogSample`.
- [x] **Lists & Dividers**
  - [x] *Implementation:* `cmp_list_item` (1, 2, and 3 line variants, leading/trailing slot alignment), `cmp_divider` (1dp thickness logic).
  - [x] *Samples:* `ClickableListItems`, `OneLineListItems`, `TwoLineListItems`, `ThreeLineListItems`, `OneLineRtlLtrListItems`, `TwoLineRtlLtrListItems`, `ThreeLineRtlLtrListItems`.
- [x] **Menus**
  - [x] *Implementation:* `cmp_dropdown_menu` (window boundary detection to prevent clipping, scrollable bounds).
  - [x] *Samples:* `MenuSample`, `MenuWithScrollStateSample`, `ExposedDropdownMenuSample`.
- [x] **Drawers & Sheets**
  - [x] *Implementation:* `cmp_modal_drawer`, `cmp_bottom_sheet_scaffold` (velocity tracking for swipe gestures, snap thresholds).
  - [x] *Samples:* `ModalDrawerSample`, `BottomDrawerSample`, `BottomSheetScaffoldSample`, `ModalBottomSheetSample`.
- [x] **Progress Indicators**
  - [x] *Implementation:* `cmp_linear_progress`, `cmp_circular_progress` (indeterminate rotation + sweep angle math, determinate tweening).
  - [x] *Samples:* `LinearProgressIndicatorSample`, `CircularProgressIndicatorSample`.
- [x] **Sliders**
  - [x] *Implementation:* `cmp_slider`, `cmp_range_slider` (touch slop detection, discrete tick marks, tooltip popups).
  - [x] *Samples:* `SliderSample`, `StepsSliderSample`, `RangeSliderSample`, `StepRangeSliderSample`.
- [x] **Snackbars**
  - [x] *Implementation:* `cmp_snackbar_host` (queueing mechanism, slide-in/out animations, duration timeouts).
  - [x] *Samples:* `ScaffoldWithSimpleSnackbar`, `ScaffoldWithCustomSnackbar`, `ScaffoldWithCoroutinesSnackbar` (Async task simulation in C).
- [x] **Tabs**
  - [x] *Implementation:* `cmp_tab_row` (dynamic indicator width calculation, horizontal scroll physics).
  - [x] *Samples:* `TextTabs`, `IconTabs`, `TextAndIconTabs`, `LeadingIconTabs`, `ScrollingTextTabs`, `FancyTabs`, `FancyIndicatorTabs`, `FancyIndicatorContainerTabs`, `ScrollingFancyIndicatorContainerTabs`.
- [x] **Text Fields**
  - [x] *Implementation:* `cmp_text_field`, `cmp_outlined_text_field` (IME keyboard integration, floating label animation, cursor blinking, text selection/highlighting, password masking `*`).
  - [x] *Samples:* `SimpleTextFieldSample`, `TextFieldWithInitialValueAndSelection`, `SimpleOutlinedTextFieldSample`, `OutlinedTextFieldWithInitialValueAndSelection`, `TextFieldWithIcons`, `TextFieldWithPlaceholder`, `TextFieldWithErrorState`, `TextFieldWithHelperMessage`, `PasswordTextField`, `TextFieldWithHideKeyboardOnImeAction`, `TextArea` (with 280dp explicit width constraint as per CatalogApp logic).

## Phase 6: Accessibility (A11y) & Internationalization (i18n)
- [x] **Semantic Tree**
  - [x] Ensure all custom CMP components populate a semantic tree for screen readers.
  - [x] Map component descriptions to `contentDescription` equivalents.
- [x] **Focus Management**
  - [x] Implement D-Pad / Keyboard Tab navigation across grids, lists, and form elements.
  - [x] Render visual focus rings when navigating via keyboard.
- [x] **Right-To-Left (RTL) Support**
  - [x] Implement `LayoutDirection` awareness in `cmp_ui_node_t`.
  - [x] Mirror Padding, Margins, and alignment for RTL locales (specifically testing `*RtlLtrListItems` samples).

## Phase 7: Optimization & Polish
- [x] **Rendering Performance**
  - [ ] Batch draw calls for Grid and List views.
  - [x] Implement Viewport culling (don't draw or animate elements currently outside the scroll view).
- [ ] **Asset Pipeline**
  - [ ] Create a script to convert Android Vector Drawables (`.xml`) or Material SVGs into CMP-native vector drawing commands (or embed as rasterized bitmaps).
  - [ ] Include high-res illustrations for the App Icon and Catalog cover images.

## Phase 8: Testing & Tooling (`greatest` & CTest)
- [x] **Unit Testing**
  - [x] Test color math (`isLightColor`, contrast ratios).
  - [x] Test theme serialization/deserialization.
  - [x] Test routing stack push/pop logic.
- [x] **Visual Regression & Automation**
  - [x] Hook `screenshot_generator.c` to iterate through *every* Example Route automatically.
  - [x] Capture the bitmap buffer of every component sample.
  - [x] Implement pixel-diffing against a golden set of reference images to ensure UI stability across C codebase changes.
  - [x] Ensure `generate_screenshots.ps1` / `.sh` handles the full matrix of components.
