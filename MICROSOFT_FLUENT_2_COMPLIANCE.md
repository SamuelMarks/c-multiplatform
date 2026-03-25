# Microsoft Fluent 2 Comprehensive Compliance Checklist

This document serves as the exhaustive, highly-granular compliance and implementation tracker for the Microsoft Fluent 2 Design System within the `c-multiplatform` framework. Achieving 100% completion of this checklist guarantees full adherence to Microsoft's Fluent 2 specifications across all supported platforms (Windows, Web, iOS, Android, macOS, Linux).

---

## 1. Design Tokens & Foundations

### 1.1. Color System (Global & Semantic)
- [ ] **Global Neutral Palette:** Implement 100-step neutral ramps (`Neutral10` through `Neutral100`, plus intermediate steps like `Neutral12`, `Neutral14`, `Neutral16`).
- [ ] **Global Brand Palette:** Implement 100-step brand ramps based on system accent color or custom brand seed.
- [ ] **Global Shared Palettes:** Implement 100-step ramps for Red, Dark Red, Cranberry, Pumpkin, Peach, Marigold, Yellow, Gold, Brass, Brown, Forest, Seafoam, Dark Green, Light Teal, Teal, Steel, Blue, Royal Blue, Cornflower, Navy, Lavender, Purple, Grape, Berry, Pink, Magenta, Plum.
- [ ] **Layering Tokens:** Map backgrounds to `colorNeutralBackground1`, `colorNeutralBackground2`, `colorNeutralBackground3`, `colorNeutralBackground4`, `colorNeutralBackground5`, `colorNeutralBackground6`.
- [ ] **Layering Hover/Pressed:** Implement corresponding hover and pressed states (e.g., `colorNeutralBackground1Hover`).
- [ ] **Surface Tokens:** Map floating surfaces to `colorTransparentBackground`, `colorTransparentBackgroundHover`, etc.
- [ ] **Foreground Tokens (Text):** `colorNeutralForeground1` (Primary text), `colorNeutralForeground2` (Secondary), `colorNeutralForeground3` (Tertiary), `colorNeutralForeground4` (Disabled).
- [ ] **Foreground Tokens (Static):** `colorNeutralForegroundStaticInverted` (White text on dark/brand backgrounds).
- [ ] **Brand Tokens:** `colorBrandBackground`, `colorBrandBackgroundHover`, `colorBrandBackgroundPressed`, `colorBrandBackgroundSelected`.
- [ ] **Brand Foreground Tokens:** `colorBrandForeground1`, `colorBrandForeground2`.
- [ ] **Stroke Tokens:** `colorNeutralStroke1`, `colorNeutralStroke2`, `colorNeutralStrokeAccessible`.
- [ ] **Focus Tokens:** `colorStrokeFocus1` (Outer ring), `colorStrokeFocus2` (Inner ring).
- [ ] **Status Tokens - Success:** Backgrounds, Foregrounds, and Strokes for Green/Success states.
- [ ] **Status Tokens - Warning:** Backgrounds, Foregrounds, and Strokes for Yellow/Warning states.
- [ ] **Status Tokens - Danger/Error:** Backgrounds, Foregrounds, and Strokes for Red/Danger states.
- [ ] **Status Tokens - Severe Warning:** Backgrounds, Foregrounds, and Strokes for Dark Orange/Severe states.
- [ ] **Status Tokens - Information:** Backgrounds, Foregrounds, and Strokes for Blue/Info states.
- [ ] **Alpha/Transparent Colors:** Map exact RGBA values for states requiring transparency (e.g., `colorNeutralBackgroundAlpha`, `colorBrandBackgroundAlpha`).
- [ ] **Theme Switching Algorithm:** Real-time generation and invalidation of token maps when switching Light <-> Dark modes.
- [ ] **High Contrast Mapping:** Strict mapping of all tokens to OS High Contrast equivalents (`Window`, `WindowText`, `Highlight`, `HighlightText`, `ButtonFace`, `ButtonText`, `GrayText`).

### 1.2. Typography & Text Shaping
- [ ] **Font Family Stacks:** 
  - [ ] Windows: `Segoe UI Variable`, `Segoe UI`, `sans-serif`.
  - [ ] macOS/iOS: `system-ui`, `-apple-system`, `BlinkMacSystemFont`, `sans-serif`.
  - [ ] Android: `Roboto`, `sans-serif`.
  - [ ] Web: `Segoe UI Variable`, `Segoe UI`, `-apple-system`, `BlinkMacSystemFont`, `Roboto`, `Helvetica Neue`, `sans-serif`.
- [ ] **Optical Sizing:** Support for optical sizing features in `Segoe UI Variable` (`Display`, `Text`, `Small`).
- [ ] **Font Weights:** Regular (400), Medium (500), Semibold (600), Bold (700).
- [ ] **Type Ramp - Caption:** Caption 2 (10px, 14px LH), Caption 1 (12px, 16px LH), Caption 1 Strong.
- [ ] **Type Ramp - Body:** Body 1 (14px, 20px LH), Body 1 Strong, Body 2 (16px, 22px LH), Body 2 Strong.
- [ ] **Type Ramp - Subtitle:** Subtitle 1 (20px, 28px LH), Subtitle 2 (24px, 32px LH), Subtitle 2 Strong.
- [ ] **Type Ramp - Title:** Title 1 (28px, 36px LH), Title 2 (32px, 40px LH), Title 3 (40px, 52px LH).
- [ ] **Type Ramp - Display:** Display (68px, 92px LH).
- [ ] **Letter Spacing (Tracking):** Precise negative and positive letter spacing corresponding to size (e.g., tighter on Display, looser on Caption).
- [ ] **Font Rendering (C specific):** Integration with FreeType/HarfBuzz for precise subpixel antialiasing and kerning pairs matching Windows DWrite standards.

### 1.3. Elevation, Shadows & Materials
- [ ] **Elevation 2:** Used for temporary, hover states, or subtle cards. (`box-shadow: 0 1px 2px rgba(...)`).
- [ ] **Elevation 4:** Used for actionable cards, simple flyouts.
- [ ] **Elevation 8:** Used for command bars, dropdowns, contextual menus.
- [ ] **Elevation 16:** Used for dialogs, tooltips, critical transient UI.
- [ ] **Elevation 28:** Used for highly elevated temporary surfaces (rare).
- [ ] **Elevation 64:** Used for top-level windows, major panes.
- [ ] **Dark Mode Elevation Mapping:** In dark mode, shadows are suppressed, and elevation is represented by progressively lighter surface background tints.
- [ ] **Materials - Mica:** Integration of the opaque, dynamic backdrop material that incorporates theme and desktop wallpaper.
- [ ] **Materials - Mica Alt:** Integration of the variant Mica material with enhanced background tinting for tabbed applications.
- [ ] **Materials - Acrylic:** Integration of the translucent, blurred material for transient surfaces (menus, flyouts, tooltips).

### 1.4. Spacing, Layout & Grid
- [ ] **Grid Base:** 4px baseline grid.
- [ ] **Spacing Tokens:** `None` (0px), `XXS` (2px), `XS` (4px), `S` (8px), `M` (12px), `L` (16px), `XL` (20px), `XXL` (24px), `XXXL` (32px), `Max` (40px).
- [ ] **Stack Layout:** Component to stack items horizontally or vertically with uniform spacing tokens.
- [ ] **Wrap Layout:** Component to flow items horizontally and wrap to the next line with specified gap tokens.
- [ ] **Grid Layout:** 2D grid component with definable row/column tracks, gaps, and alignments.

### 1.5. Shape, Borders & Corner Radius
- [ ] **Radius Tokens:**
  - [ ] `None` (0px)
  - [ ] `Small` (2px) - Used for checkboxes, tooltips.
  - [ ] `Medium` (4px) - Used for buttons, text inputs, flyouts.
  - [ ] `Large` (8px) - Used for cards, dialogs.
  - [ ] `XLarge` (12px) - Used for large surfaces.
  - [ ] `XXLarge` (16px) - Used for window rounding.
  - [ ] `Circular` (50% / Pill) - Used for badges, avatars, radio buttons.
- [ ] **Stroke Alignment:** All structural borders must be drawn strictly *inside* the bounds of the element (Inner Stroke) to prevent layout shifting.
- [ ] **Focus Rings:** Complex two-part focus rings (inner and outer strokes) with specified gap, rendered outside the component boundary.

### 1.6. Motion & Animation
- [ ] **Duration Tokens:** `UltraFast` (50ms), `Faster` (100ms), `Fast` (150ms), `Normal` (250ms), `Gentle` (300ms), `Slow` (400ms), `Slower` (500ms).
- [ ] **Easing Curves (Cubic Bezier):**
  - [ ] `Linear` (0, 0, 1, 1)
  - [ ] `AccelerateMax` (1, 0, 1, 1)
  - [ ] `AccelerateMid` (0.7, 0, 1, 0.5)
  - [ ] `AccelerateMin` (0.8, 0, 1, 1)
  - [ ] `DecelerateMax` (0, 0, 0, 1)
  - [ ] `DecelerateMid` (0.1, 0.9, 0.2, 1)
  - [ ] `DecelerateMin` (0.33, 0, 0.1, 1)
  - [ ] `MaxStandard` (0, 0, 0, 1)
  - [ ] `Standard` (0.33, 0, 0.67, 1)
- [ ] **Micro-interactions - Press:** Scale transform to 0.96 or 0.98 on pointer down/touch start.
- [ ] **Micro-interactions - Hover:** Subtle color transitions, elevation increases, and icon translations.
- [ ] **Page Transitions:** Slide up, slide left/right, fade in/out configurations.

### 1.7. Iconography (Fluent System Icons)
- [ ] **Icon Sizes:** 10, 12, 16, 20, 24, 28, 32, 48.
- [ ] **Icon Styles:** `Regular` (default, outlined) and `Filled` (active/selected states).
- [ ] **State Swapping:** Logic to automatically swap an icon from Regular to Filled when its parent container becomes `active` or `selected`.

---

## 2. Basic Inputs & Controls

### 2.1. Button
- [ ] **Variants:** Primary, Secondary (Default), Subtle, Outline, Transparent.
- [ ] **Sizes:** Small (24px height), Medium (32px height), Large (40px height).
- [ ] **Shapes:** Rounded, Circular, Square.
- [ ] **Content Layouts:** Text only, Icon + Text, Text + Icon, Icon only.
- [ ] **States:** Rest, Hover, Pressed, Focus, Disabled.
- [ ] **Animation:** Background color transition, scale-down on press.
- [ ] **Focus Ring:** Proper `colorStrokeFocus1` + `colorStrokeFocus2` implementation.

### 2.2. CompoundButton
- [ ] **Features:** Button containing primary text and secondary descriptive text.
- [ ] **Variants & States:** Mirrors standard Button variants and states.

### 2.3. MenuButton
- [ ] **Features:** Button that opens a Dropdown/Menu. Contains trailing chevron icon.
- [ ] **States:** Chevron rotates 180 degrees when menu is open. Button remains in "Pressed/Active" state while menu is open.

### 2.4. SplitButton
- [ ] **Features:** Two distinct hit targets (Primary action, Secondary menu action).
- [ ] **Divider:** Vertical separator between the two hit targets.
- [ ] **Focus:** Independent focus states for the primary action and the menu dropdown arrow.

### 2.5. ToggleButton
- [ ] **Features:** Button with an on/off state.
- [ ] **Visuals:** Uses `colorNeutralBackground1Selected` when toggled on.

### 2.6. Checkbox
- [ ] **Sizes:** Medium (16px box), Large (20px box).
- [ ] **States:** Unchecked, Checked, Indeterminate (Mixed).
- [ ] **Interactions:** Rest, Hover, Pressed, Focus, Disabled.
- [ ] **Animations:** Checkmark draws in/out. Indeterminate dash draws in/out. Background fill transition.
- [ ] **Label:** Text layout to the right or left of the box.

### 2.7. RadioGroup & RadioButton
- [ ] **Sizes:** Medium, Large.
- [ ] **Layout:** Vertical or Horizontal stacking.
- [ ] **States:** Unselected, Selected.
- [ ] **Interactions:** Rest, Hover, Pressed, Focus, Disabled.
- [ ] **Animations:** Inner circle scales up from 0 to target size.

### 2.8. Switch / Toggle
- [ ] **Features:** Horizontal pill-shaped toggle.
- [ ] **States:** Off, On.
- [ ] **Interactions:** Rest, Hover, Pressed, Focus, Disabled.
- [ ] **Animations:** Thumb slides left/right. Thumb elongates dynamically during drag/press.
- [ ] **Accessibility:** Must map to standard checkbox or toggle switch ARIA/UIA roles.

### 2.9. Slider
- [ ] **Orientation:** Horizontal, Vertical.
- [ ] **Variants:** Continuous, Stepped (with optional tick marks).
- [ ] **Features:** Single thumb, Range (two thumbs).
- [ ] **Interactions:** Hover (thumb grows), Pressed (thumb scales down slightly), Dragging.
- [ ] **Visuals:** Filled track (left of thumb), unfilled track (right of thumb).
- [ ] **Tooltip:** Option to show value tooltip above thumb while dragging.

### 2.10. SpinButton (Number Input)
- [ ] **Features:** Numeric text input with integrated Up/Down chevron buttons.
- [ ] **Validation:** Min, Max, Step increments.
- [ ] **Layout:** Buttons can be stacked on the right, or placed on either side.
- [ ] **Interactions:** Mouse scroll wheel support, Up/Down arrow key support. Long-press to rapidly spin.

---

## 3. Text Inputs & Forms

### 3.1. Input / TextField
- [ ] **Variants:** Outline (default), Underline, Filled.
- [ ] **Sizes:** Small, Medium, Large.
- [ ] **States:** Rest, Hover, Focus (Bottom border accentuates), Disabled, Invalid/Error.
- [ ] **Content Additions:** Prefix text, Suffix text, Leading Icon, Trailing Icon.
- [ ] **Clear Button:** Integrated trailing 'X' button that appears when text is entered, clears input on click.
- [ ] **Password Mode:** Toggle trailing 'Eye' icon to mask/unmask characters.

### 3.2. Textarea
- [ ] **Sizes:** Small, Medium, Large.
- [ ] **States:** Rest, Hover, Focus, Disabled, Invalid.
- [ ] **Resizing:** None, Vertical, Horizontal, Both. Render grab-handle in bottom right.
- [ ] **Auto-grow:** Option to automatically expand height based on line count, up to a `max-height`.

### 3.3. Field & Label (Form Wrappers)
- [ ] **Label Placement:** Top (stacked), Left (inline).
- [ ] **Required Indicator:** Asterisk `*` appended to label, colored with `colorDangerForeground1`.
- [ ] **Help/Description Text:** Secondary text placed below the input.
- [ ] **Validation Message:** Error text placed below the input (replaces or sits alongside Help text), colored red, with an error icon.
- [ ] **State Linking:** Automatically pushes `disabled` and `invalid` states to the child input component.

### 3.4. SearchBox
- [ ] **Features:** TextField specialized for searching.
- [ ] **Visuals:** Leading magnifying glass icon. Trailing clear button.
- [ ] **Interactions:** Optional "collapse" mode where it appears as an icon until clicked, expanding into a full field.

---

## 4. Dropdowns, Pickers & Selections

### 4.1. Select / Dropdown
- [ ] **Variants:** Outline, Underline, Filled.
- [ ] **Features:** Triggers a popup/flyout containing a Listbox.
- [ ] **Visuals:** Trailing chevron down icon.
- [ ] **States:** Rest, Hover, Focus, Disabled, Open (Chevron flips).
- [ ] **Selection Mode:** Single selection (updates trigger text), Multi-selection (updates trigger text to "X items selected", uses checkboxes in list).

### 4.2. ComboBox
- [ ] **Features:** Editable text input combined with a dropdown list.
- [ ] **Behaviors:** Auto-complete (inline text selection matching closest result), Filtering (dropdown list reduces based on input).
- [ ] **Freeform Mode:** Allows submitting text that does not exist in the preset list.

### 4.3. Listbox (Dropdown Menu Content)
- [ ] **Items:** `Option`, `OptionGroup` (with headers).
- [ ] **Visuals:** Hover highlighting, Checkmark icon for selected items.
- [ ] **Virtualization:** Support for thousands of items via recycling DOM/render nodes.

### 4.4. ColorPicker
- [ ] **Features:** Complex composite component.
- [ ] **Visuals:** Hue slider, Saturation/Value 2D draggable area, Alpha slider.
- [ ] **Inputs:** Hex input, RGB/RGBA inputs.
- [ ] **Swatches:** Grid of preset color squares.

### 4.5. Calendar & DatePicker
- [ ] **Views:** Month (Days grid), Year (Months grid), Decade (Years grid).
- [ ] **Interactions:** Swipe/Click arrows to change month/year.
- [ ] **States:** Today (highlighted), Selected, Hover, Disabled dates, Out-of-month dates.
- [ ] **DatePicker:** Input field with calendar icon that opens the Calendar in a Flyout.

### 4.6. TimePicker
- [ ] **Features:** Dropdown or spin button set to select hours, minutes, and AM/PM.

---

## 5. Navigation & Menus

### 5.1. Menu (Context & Dropdown)
- [ ] **Features:** Transient floating surface.
- [ ] **Items:** `MenuItem`, `MenuCheckbox`, `MenuRadio`, `MenuDivider`, `MenuGroupHeader`.
- [ ] **Sub-menus:** Items with trailing chevrons that open nested Menus on hover/click.
- [ ] **Keyboarding:** Arrow keys (Up/Down for item, Left/Right for submenus), Escape to close, Enter to select. Type-ahead to jump to item.
- [ ] **Icons & Shortcuts:** Leading icon slot, trailing keyboard shortcut text slot (e.g., "Ctrl+C").

### 5.2. NavigationView (App Navigation)
- [ ] **Placement:** Left sidebar, Top header.
- [ ] **States:** Expanded (Icon + Text), Collapsed (Icon only), Overlay (slides over content).
- [ ] **Items:** `NavItem`, `NavCategory` (expandable group), `NavDivider`.
- [ ] **Visuals:** Active item indicated by a distinct background and a vertical pill/accent line on the left edge.
- [ ] **Footer:** Dedicated slot for bottom-aligned items (e.g., Settings, User Profile).

### 5.3. Tabs / Pivot
- [ ] **Variants:** Transparent, Subtle, Solid.
- [ ] **Sizes:** Small, Medium, Large.
- [ ] **Layout:** Horizontal (default), Vertical.
- [ ] **Interactions:** Click to select.
- [ ] **Animations:** The active indicator (underline or background pill) must fluidly animate its width and X-position when switching tabs.
- [ ] **Overflow:** If tabs exceed container width, show a scrolling button or trailing overflow `...` menu.

### 5.4. Breadcrumb
- [ ] **Features:** Trail of links showing hierarchy.
- [ ] **Separators:** Chevrons between items.
- [ ] **Overflow:** Collapse middle items into an ellipsis `...` dropdown when width is constrained. Ensure first and last items are always visible.
- [ ] **States:** Last item is visually distinct (non-link, bold, or different color) representing the current page.

### 5.5. Toolbar / CommandBar
- [ ] **Features:** Surface for contextual commands.
- [ ] **Layout:** Primary commands on left, Secondary commands on right.
- [ ] **Overflow:** Responsive auto-collapse. Commands that don't fit are moved into a trailing ellipsis `...` menu.
- [ ] **Items:** Buttons, ToggleButtons, SplitButtons, Dividers.

### 5.6. Link
- [ ] **Variants:** Inline (within text paragraph), Standalone.
- [ ] **States:** Rest, Hover (underline appears), Pressed, Focus, Visited.
- [ ] **Icons:** Support for trailing "External Link" icon.

### 5.7. Stepper / Wizard
- [ ] **Features:** Linear progression through multiple steps.
- [ ] **States:** Completed (Checkmark), Current (Active ring), Upcoming (Inactive).

---

## 6. Surfaces & Layout Containers

### 6.1. Card
- [ ] **Variants:** Filled (Background tint), Outline (Border), Subtle (No border/bg until hover).
- [ ] **Orientation:** Vertical, Horizontal.
- [ ] **Interactions:** Static (display only), Clickable (behaves as a button, elevates on hover).
- [ ] **Anatomy:** Header, Preview (Image/Media), Body, Footer (Actions).

### 6.2. Accordion
- [ ] **Variants:** Flush, Contained.
- [ ] **Features:** Expandable/collapsible panels.
- [ ] **Anatomy:** `AccordionHeader` (with expand/collapse chevron), `AccordionPanel` (content body).
- [ ] **Behavior:** Single-expand (opening one closes others) or Multi-expand.

### 6.3. Divider
- [ ] **Orientation:** Horizontal, Vertical.
- [ ] **Appearance:** Default, Subtle, Brand.
- [ ] **Spacing:** Inset (indented from edges) or full-width.

### 6.4. SplitView / Resizer
- [ ] **Features:** Draggable separator between two layout panes.
- [ ] **Interactions:** Hover (separator highlights or thickens), Drag (resizes panes), Double-click (reset to default or collapse).

### 6.5. ScrollView / Scrollbar
- [ ] **Visuals:** Fluent 2 specific thin scrollbars that expand on hover.
- [ ] **Behavior:** Auto-hide when inactive. Overlay vs Inline modes.

---

## 7. Overlays, Modals & Feedback

### 7.1. Dialog / Modal
- [ ] **Variants:** Standard, Alert (requires explicit action, non-light-dismiss), Fullscreen.
- [ ] **Anatomy:** Title, Body, Footer (Primary/Secondary buttons).
- [ ] **Backdrop:** Dark overlay or blurred overlay behind the dialog.
- [ ] **Behavior:** Traps keyboard focus. Disables scrolling on the underlying page/body.

### 7.2. Flyout / Popover
- [ ] **Features:** Contextual popup anchored to a specific UI element.
- [ ] **Positioning:** Auto-placement logic (Top, Bottom, Left, Right, TopStart, BottomEnd, etc.).
- [ ] **Collision:** Automatic flipping or shifting if the flyout would render outside the viewport bounds.
- [ ] **Dismissal:** Light-dismiss (clicking outside), Escape key.

### 7.3. Tooltip
- [ ] **Features:** Small text label appearing on hover/focus.
- [ ] **Visuals:** Inverted theme (Dark in Light mode, Light in Dark mode).
- [ ] **Timing:** Delay on hover (e.g., 500ms), immediate dismiss. Zero delay if another tooltip was recently shown.

### 7.4. Toast / Notification
- [ ] **Intents:** Info, Success, Warning, Error.
- [ ] **Anatomy:** Icon, Title, Body text, Actions (Buttons), Close Button.
- [ ] **Animations:** Slide in from bottom-right or top-right. Slide out on dismiss.
- [ ] **Lifespan:** Auto-dismiss after X seconds. Pause timer on hover.

### 7.5. MessageBar (Inline Alert)
- [ ] **Variants:** Multiline, Singleline.
- [ ] **Intents:** Info, Success, Warning, Error.
- [ ] **Layout:** Embedded in standard page flow (pushes content down).
- [ ] **Actions:** Inline buttons or links.

### 7.6. ProgressBar & ProgressRing
- [ ] **Variants:** Indeterminate (animated loading), Determinate (value-based fill).
- [ ] **Sizes:** XSmall, Small, Medium, Large.
- [ ] **Thickness:** Standard (2px), Large (4px).
- [ ] **States:** Active, Paused, Error (turns red).

### 7.7. Skeleton / Shimmer
- [ ] **Features:** Placeholder for loading content.
- [ ] **Shapes:** Circle (Avatar), Rectangle (Image), Text Line.
- [ ] **Animation:** Smooth left-to-right gradient sweep (shimmer).

---

## 8. Data Display & Status

### 8.1. Avatar
- [ ] **Sizes:** 16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 72, 96, 120.
- [ ] **Shapes:** Circular (People), Square with rounded corners (Groups/Entities).
- [ ] **Display Priority:** 1. Image, 2. Initials (1 or 2 letters), 3. Fallback Icon.
- [ ] **Colors:** Automatic background color assignment based on name hash (for initials).

### 8.2. AvatarGroup
- [ ] **Layout:** Stacked/Overlapping, Pie (split circle).
- [ ] **Overflow:** If count > max, show an overflow avatar with text like "+3".

### 8.3. PresenceBadge
- [ ] **Features:** Status indicator attached to bottom-right of an Avatar.
- [ ] **Statuses:** Available (Green check/circle), Busy (Red), Do Not Disturb (Red with minus), Away (Yellow clock), Offline (Gray circle), Out of Office (Arrow).

### 8.4. Badge & InfoLabel
- [ ] **Variants:** Filled, Outline, Tint, Ghost.
- [ ] **Shapes:** Rounded, Circular.
- [ ] **Usage:** Notification counts, unread status, tagging.

### 8.5. Tag / TagGroup
- [ ] **Features:** Interactive labels.
- [ ] **Interactions:** Dismissible (trailing X button), Selectable (behaves like a toggle).

### 8.6. DataGrid / Table
- [ ] **Anatomy:** Header Row, Data Rows, Columns, Cells.
- [ ] **Features:** Column resizing (draggable headers).
- [ ] **Features:** Column sorting (Ascending/Descending/Unsorted states with icons).
- [ ] **Features:** Row selection (Single, Multiple via checkboxes).
- [ ] **Features:** Sticky/Frozen headers and columns.
- [ ] **Virtualization:** High-performance rendering of thousands of rows.

### 8.7. Tree / TreeView
- [ ] **Features:** Hierarchical list of items.
- [ ] **Interactions:** Expand/Collapse nodes, Selection, Drag and drop reordering.

---

## 9. Accessibility (A11y) & Interaction

### 9.1. Keyboard Navigation Rules
- [ ] **Tab Flow:** Sequential progression through interactive elements.
- [ ] **Arrow Key Flow:** Spatial/Logical navigation within composite widgets (e.g., RadioGroups, Toolbars, DataGrids, Menus).
- [ ] **Focus Trapping:** Modal dialogs must prevent focus from escaping to the background document.
- [ ] **Focus Restoration:** Closing a dialog or flyout must return focus to the element that triggered it.

### 9.2. Screen Reader Integration
- [ ] **ARIA/UIA Roles:** Correct assignment of roles (e.g., `button`, `dialog`, `listbox`, `option`, `slider`).
- [ ] **Properties/States:** Real-time updates to properties like `aria-expanded`, `aria-checked`, `aria-valuenow`, `aria-disabled`.
- [ ] **Live Regions:** Dynamic announcements for Toast notifications and validation errors.

### 9.3. Visual Accessibility
- [ ] **Contrast Ratios:** Ensure WCAG AA compliance (4.5:1 for text, 3:1 for graphical objects/UI bounds).
- [ ] **Reduced Motion:** Detect OS reduced motion preference and disable/minimize layout animations and transitions.

---

## 10. Deep Platform / OS Integrations (c-multiplatform core)

### 10.1. Windows (Win32 / UWP)
- [ ] **DWM Backdrop:** API calls to enable `DWMWA_SYSTEMBACKDROP_TYPE` for Mica/MicaAlt, and `DWMWA_USE_IMMERSIVE_DARK_MODE` for correct titlebar coloring.
- [ ] **Windowing:** Custom TitleBar rendering that perfectly aligns with Windows 11 caption buttons (Min, Max, Close).
- [ ] **Snap Layouts:** `WM_NCHITTEST` handling to trigger the Windows 11 Snap Assist flyout on the custom maximize button.
- [ ] **UI Automation:** COM implementation of `IRawElementProviderSimple` for full Narrator support.

### 10.2. macOS / iOS (Cocoa / UIKit)
- [ ] **Materials:** Map Fluent Acrylic/Mica to `NSVisualEffectView` or `UIVisualEffectView` with appropriate blending modes (Vibrancy).
- [ ] **Typography:** CoreText integration to map to `SF Pro` with correct optical tracking.
- [ ] **Touch Targets (iOS):** Automatic inflation of component hit-boxes to a minimum of 44x44 points.

### 10.3. Android (NDK / JNI)
- [ ] **Touch Targets:** Automatic inflation of hit-boxes to a minimum of 48x48 dp.
- [ ] **Feedback:** Map Fluent pressed states to Android Ripple effects where native feeling is preferred.
- [ ] **System Bars:** Edge-to-edge drawing under transparent status/navigation bars.

### 10.4. Web (Emscripten / WebAssembly)
- [ ] **Canvas/WebGL Parity:** Pixel-perfect rendering matching HTML/CSS Fluent UI Web React components.
- [ ] **DOM Mapping:** Creating an invisible, synchronized DOM overlay for screen readers, text selection, and native password managers.
- [ ] **Fallbacks:** Utilizing CSS `backdrop-filter` for blur effects.

---

## 11. Code Architecture & C Framework Conventions
- [ ] **Namespace Prefixing:** All Fluent 2 components, types, and functions must be prefixed with `cmp_f2_` (e.g., `cmp_f2_button_create()`).
- [ ] **Theme Generator:** A core C function `cmp_f2_theme_generate(uint32_t brand_color, bool is_dark, cmp_f2_theme_t* out_theme)` that mathematically derives all 100+ tokens.
- [ ] **Memory Management:** Zero-leak destruction patterns for complex composite components like DataGrid and ColorPicker.
- [ ] **Asset Pipeline:** Build scripts to compile Fluent System Icons SVG/TTF files into a raw binary C array or texture atlas.
- [ ] **State Machine:** Robust internal state machines preventing invalid UI states (e.g., a button being simultaneously `Hover` and `Disabled` shouldn't render the hover color).

---

## 12. Documentation (Docs)

### 12.1. API Reference
- [ ] **Doxygen / Sphinx Integration:** Generate HTML/PDF API reference docs from inline C comments for all `cmp_f2_` headers.
- [ ] **Component Function Signatures:** Fully document instantiation (`cmp_f2_button_create`), property setters (`cmp_f2_button_set_text`), and event listeners.
- [ ] **Struct/Enum Documentation:** Explicit documentation for all configuration structs (e.g., `cmp_f2_dialog_config_t`) and state enums (`CMP_F2_STATE_HOVER`).

### 12.2. Developer Guides & Tutorials
- [ ] **"Getting Started with Fluent 2":** A comprehensive guide on initializing the `f2` module within the broader `c-multiplatform` context.
- [ ] **Migration Guide:** Documentation on moving from the Material 3 (`m3/`) implementation or base components to the Fluent 2 system.
- [ ] **Theming Guide:** Tutorial on overriding the core brand seed color and intercepting the dynamic palette generator.
- [ ] **Layout System Guide:** How to use the `f2` specific Grid, Stack, and Wrap components effectively.

### 12.3. Component Gallery / Demo App
- [ ] **Interactive Executable:** Build a cross-platform "F2 Component Gallery" sample application (similar to WinUI 3 Gallery).
- [ ] **Live State Editing:** Allow users to toggle states (Disabled, Invalid, Hover) and variants (Primary, Outline, Subtle) directly in the demo.
- [ ] **Live Theme Switching:** Real-time toggles for Light Mode, Dark Mode, High Contrast, and dynamically changing the brand color via a ColorPicker.
- [ ] **Source Code Snippets:** Provide C code snippets alongside each component preview in the gallery.

### 12.4. Design Alignment & Specifications
- [ ] **Figma Mapping:** Provide links matching the C implementation to Microsoft's official Fluent 2 Figma community files.
- [ ] **Metric Transparency:** Document the exact mathematical conversions used (e.g., tracking ratios -> C letter-spacing) to prove 1-to-1 visual fidelity.

---

## 13. Testing & Quality Assurance (QA)

### 13.1. Unit Testing
- [ ] **Token Math Verification:** Write Cest/Greatest unit tests to mathematically verify the 100-step color ramp generator outputs exactly matching the official Fluent 2 brand ramps.
- [ ] **State Machine Tests:** Verify that components properly transition between Rest -> Hover -> Pressed -> Focus without getting stuck in invalid overlapping states.
- [ ] **Contrast Assertions:** Automated checks verifying that the generated `Foreground` vs `Background` tokens mathematically meet WCAG 4.5:1 ratios in both light and dark themes.
- [ ] **API Memory Tests:** Unit tests validating the lifecycle (create -> update -> destroy) of composite widgets without faults.

### 13.2. Visual Regression Testing
- [ ] **Golden Image Baselines:** Establish a set of baseline PNGs for every component variant and state.
- [ ] **Pixel-Diff Automation:** Run headless CI renders using the WebGL or software renderer, failing the build if a component's visual output deviates by >0.1% from the golden baseline.
- [ ] **Cross-OS Visual Parity Check:** Verify that the software-rendered output of a button looks identical on Linux vs macOS vs Windows.

### 13.3. Accessibility (A11y) Testing
- [ ] **UIA Tree Validation (Windows):** Automated test scripts leveraging `Inspect.exe` / UIA APIs to verify the accessibility tree is correctly populated (e.g., Dialog traps focus, names are populated).
- [ ] **DOM ARIA Validation (Web):** Run Emscripten test builds through standard web accessibility checkers (e.g., Axe, Lighthouse) to verify the shadow DOM ARIA output.
- [ ] **Keyboard Flow Tests:** Headless integration tests sending virtual `TAB`, `SHIFT+TAB`, `ENTER`, `SPACE`, and `ESC` inputs to verify focus moves correctly through complex widgets (e.g., RadioGroups, Toolbars).

### 13.4. Memory & Performance Profiling
- [ ] **Valgrind / ASAN Runs:** Ensure the comprehensive destruction of massive DataGrids and deeply nested TreeViews leaks 0 bytes in C.
- [ ] **60fps / 120fps Benchmarks:** Profile the tick-based animation system (`cmp_anim.h`) to ensure cubic-bezier easing evaluations do not drop frames during heavy layout updates.
- [ ] **Virtualization Stress Test:** Benchmark the DataGrid component scrolling through 1,000,000 rows at 60fps to verify layout recycling efficiency.

### 13.5. Platform-Specific Integration Tests
- [ ] **Windows Windowing Test:** Verify that creating an `f2` window dynamically triggers the DWM Mica backdrop API without crashing on Windows 10 vs Windows 11.
- [ ] **Web Resize & DPI Test:** Verify that the Canvas/WebGL surface flawlessly scales when moved between a 1x (1080p) monitor and a 2x (Retina/4K) display.