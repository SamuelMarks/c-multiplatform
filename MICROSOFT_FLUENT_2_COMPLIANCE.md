# Microsoft Fluent 2 Compliance Checklist

This document tracks the compliance and implementation status of the Microsoft Fluent 2 Design System specifically for the `c-multiplatform` framework. 

Fluent 2 is Microsoft's evolution of its design language. While this framework already features a robust cross-platform core and Material 3 (`m3/`) implementation, achieving parity with Microsoft's official Fluent toolkits (like WinUI 3 and Fluent UI Web Components) requires a dedicated `f2/` token and component system, along with deep OS-level integrations.

## 1. Core Primitives & Foundation
*These items track the underlying C framework capabilities required to support a modern design system. Many overlap with the existing M3 implementation and are already present in `cmpc/`.*

- [x] **High-Performance Rendering:** Path and shape drawing system (`cmp_render.h`).
- [x] **Shadow & Elevation Engine:** Base shadow rendering algorithm (`CMPShadow` in `cmp_visuals.h`).
- [x] **Animation Engine:** Tick-based animation system with easing evaluation (`cmp_anim.h`).
- [x] **Accessibility Tree:** Cross-platform semantics tree and focus management (`cmp_a11y.h`).
- [x] **Event Routing:** Input, gesture, and focus event propagation (`cmp_event.h`, `cmp_gesture.h`).
- [x] **Theming Architecture:** Dynamic generation and switching of light/dark color schemes (conceptual overlap with `m3_color.h`).
- [ ] **Fluent 2 Scheme Generator:** A dedicated `f2_color.h` algorithm to generate the 10-100 step color ramps based on a brand key color.

---

## 2. Fluent 2 Design Tokens (Target: `include/f2/` & `src/f2/`)
*These represent the specific design variables defined by Microsoft's Fluent 2 specifications.*

### Color System & Theming
- [ ] **Neutral Palette:** 100-step grayscale ramp tailored for layered backgrounds and text.
- [ ] **Brand Palette:** 100-step accent color ramp.
- [ ] **Semantic Tokens:** Direct mapping for `Success` (Green), `Warning` (Yellow), `Danger` (Red), and `Info` (Blue).
- [ ] **High Contrast Modes:** Map tokens to system High Contrast themes (especially critical on Windows).

### Typography
- [ ] **Font Stack:** Primary font `Segoe UI Variable`.
- [ ] **Cross-Platform Fallbacks:** Map to `system-ui`, `-apple-system`, `Roboto`, `Ubuntu` based on backend target.
- [ ] **Type Ramp Metrics:** Accurately implement Fluent 2 font sizes and line heights:
  - [ ] Caption (12px)
  - [ ] Body / Body Strong (14px)
  - [ ] Subtitle (20px)
  - [ ] Title (24px / 28px)
  - [ ] Display (32px / 40px / 68px)

### Elevation, Materials & Geometry
- [ ] **Shadow Ramps:** Map `CMPShadow` settings to Fluent 2's specific elevations (Level 2, 4, 8, 16, 64).
- [ ] **Stroke/Borders:** 1px subtle structural borders to separate layers.
- [ ] **Corner Radii Tokens:** 2px, 4px, 8px, 16px, and Pill/Circular.
- [ ] **Spacing Grid:** Enforce 4px/8px multiplier alignment in component layouts.
- [ ] **Motion Curves:** Map or add Fluent 2 easings to `cmp_anim.h`:
  - [ ] Standard (`0.0, 0.0, 0.0, 1.0` bezier approximation)
  - [ ] Decelerate
  - [ ] Accelerate

---

## 3. Fluent 2 UI Components
*These components must be built from the ground up to match Microsoft's specific interaction models, states, and visual metrics.*

### Basic Inputs & Controls
- [ ] **Button:** Standard, Primary, Subtle, Outline, and Transparent variants.
- [ ] **ToggleButton & SplitButton:** With synchronized dropdown states.
- [ ] **Checkbox:** Unchecked, Checked, Indeterminate (Mixed) states.
- [ ] **RadioButton & RadioGroup:** Circular selection controls.
- [ ] **Switch:** Horizontal sliding toggle switch.
- [ ] **Slider:** Continuous and stepped range selection.
- [ ] **SpinButton:** Number input with inline up/down arrows.
- [ ] **TextField / TextArea:** Single-line and multi-line text input with standard Fluent 2 states (Rest, Hover, Pressed, Focus, Disabled).
- [ ] **SearchBox:** Input with leading search icon and trailing clear button.

### Navigation & Commands
- [ ] **NavigationView:** Left-aligned or top-aligned app navigation (collapsed and expanded modes).
- [ ] **TabList / Pivot:** Horizontal tabbed navigation with animated indicator.
- [ ] **Breadcrumb:** Path navigation links.
- [ ] **CommandBar:** App-level toolbar with overflow menus.
- [ ] **Link:** Inline text hyperlinks.

### Surfaces, Layout & Feedback
- [ ] **Card:** Clickable and static surfaces with defined hover elevations.
- [ ] **Dialog:** Centered blocking overlays with Level 16/64 elevation.
- [ ] **Flyout / Popover:** Transient UI anchored to controls.
- [ ] **Tooltip:** Small delay-based hover labels.
- [ ] **ProgressBar & ProgressRing:** Determinate and indeterminate (spinner) loading states.
- [ ] **Badge & PresenceBadge:** Notification counts and online/offline status indicators.
- [ ] **Avatar:** Initials or image representation of a user.
- [ ] **MessageBar / Toast:** Inline alerts and transient popup notifications.

### Advanced Data Components
- [ ] **DataGrid / Table:** Column sorting, row selection, and virtualization.
- [ ] **ComboBox / Select:** Dropdown with listbox selection and auto-complete.
- [ ] **Calendar / DatePicker:** Full month grid view.
- [ ] **TreeView:** Hierarchical expandable list.

---

## 4. Deep Platform Integrations
*To truly match Microsoft's official toolkits, the framework must integrate with OS-level visual features.*

### Windows (Win32 Backend / SDL3)
- [ ] **Mica Backdrop:** Integrate `DWMWA_SYSTEMBACKDROP_TYPE` in `src/backend/win32/` to support the Mica material on Windows 11 app backgrounds.
- [ ] **Mica Alt:** Support Mica Alt for tabbed applications.
- [ ] **Acrylic:** Support Windows 10/11 Acrylic blur for transient surfaces (Flyouts, Menus).
- [ ] **Snap Layouts:** Ensure custom-drawn title bars correctly trigger the Windows 11 Snap Assist flyout on hover.
- [ ] **UI Automation (UIA):** Ensure `cmp_a11y.h` trees are correctly mapped to Windows UIA so Narrator behaves exactly like a native WinUI 3 app.

### Web (Emscripten Backend)
- [ ] **Visual Parity:** Ensure WebGL/Canvas rendering metrics precisely match Fluent UI Web React components.
- [ ] **DOM Accessibility:** Map `cmp_a11y.h` events to standard DOM ARIA attributes for full screen-reader compliance on the web.
- [ ] **Material Fallbacks:** Utilize CSS `backdrop-filter: blur()` to simulate Acrylic, and subtle tinted backgrounds to simulate Mica where native equivalents are unavailable.

### Cross-Platform (macOS, Linux, Mobile)
- [ ] **macOS Materials:** Map Mica/Acrylic concepts to `NSVisualEffectView` materials in `src/backend/cocoa/` if requested by the user theme.
- [ ] **Opaque Fallbacks:** Ensure elegant, opaque solid-color fallbacks exist for Linux (GTK4/SDL3) or older OS versions that do not support translucent layered rendering.
- [ ] **Touch Targets:** Automatically adapt component layout metrics to enforce a minimum 44x44px (iOS) or 48x48px (Android) touch target area when compiled for mobile backends, while preserving Fluent aesthetics.
