# Apple Human Interface Guidelines (Cupertino) Compliance

This document is an exhaustive checklist for achieving strict compliance with Apple's Human Interface Guidelines (HIG) across the `c-multiplatform` framework. It maps core framework capabilities to Apple's design expectations for **iOS/iPadOS**, **macOS**, **tvOS**, and **watchOS**.

Items marked `[x]` indicate overlap where the existing `core` or `m3` implementation provides the necessary foundation or directly satisfies the requirement.

---

## 🏗️ Core & Architecture Parity (Existing Framework Overlap)

Apple’s platforms require a robust, declarative, and highly responsive foundation. `c-multiplatform` already implements several subsystems that map directly to Apple's underlying UI patterns.

- [x] **Animation Engine**: Support for procedural, interruptible spring animations (`cmp_spring_init`, `cmp_spring_step`), mapping directly to `CASpringAnimation` behaviors.
- [x] **Accessibility Tree (A11y)**: Built-in semantic nodes (`CMPSemantics`, `CMPA11yNode`) ready to be bridged to `UIAccessibility` and `NSAccessibility` protocols.
- [x] **Gesture Recognizers**: Core detection for Tap, Double Tap, Long Press, and Drag, functioning similarly to `UIGestureRecognizer`.
- [x] **Environment Variables**: Contextual states (Light/Dark mode, scaling factors) via `cmp_api_env.h`, akin to `UITraitCollection` / `NSAppearance`.
- [x] **Internationalization (i18n)**: String localization (`cmp_i18n.h`) supporting localized pluralization and RTL layouts.
- [x] **Render Loop Abstraction**: Core visual layers mapping conceptually to `CALayer` and UI hierarchies (`cmp_visuals.h`, `cmp_layout.h`).
- [ ] **DisplayLink Synchronization**: Binding the core render loop directly to `CADisplayLink` (iOS/tvOS) or `CVDisplayLink` (macOS) for ProMotion (120Hz) frame pacing.

---

## 📱 1. iOS & iPadOS (Touch & Mobility)

### Navigation & Layout
- [ ] **Safe Areas**: Strict adherence to `safeAreaInsets` (Notch, Dynamic Island, Home Indicator).
- [ ] **Navigation Bar**: Collapsible headers mapping to `UINavigationBar`. Large titles that transition to inline small titles during scroll.
- [ ] **Tab Bar**: Translucent bottom navigation (`UITabBar`), with SF Symbols and badge support.
- [ ] **iPadOS Layouts**: Support for Slide Over, Split View, and Stage Manager.
- [ ] **Sidebar (iPadOS)**: Collapsible sidebar navigation mapping to `UISplitViewController` column styles.

### Gestures & Interactions
- [ ] **Edge Swipe Back**: Interactive swipe from the left edge of the screen to pop navigation controllers.
- [ ] **Pull-to-Refresh**: Rubber-band scrolling with a native-feeling spinner (`UIRefreshControl`).
- [ ] **Context Menus**: Long-press revealing a blurred background preview and a vertical list of actions (`UIContextMenuInteraction`).
- [ ] **Interactive Dismissal**: Swipe-down-to-dismiss for modal sheets.

### Modals & Sheets
- [ ] **Action Sheets**: Modals sliding from the bottom with distinct Cancel buttons and destructive actions (`UIAlertControllerStyleActionSheet`).
- [ ] **Alerts**: Center-screen alerts with standard iOS radii and font weights (`UIAlertControllerStyleAlert`).
- [ ] **Detent Sheets**: Half-screen or multi-detent sliding sheets (mapping to iOS 15+ `UISheetPresentationController.Detent`).

---

## 💻 2. macOS (Desktop & Precision)

### Windowing & Menu Bar
- [ ] **Traffic Lights**: Native integration or precise visual replication of Close, Minimize, Zoom buttons.
- [ ] **Global Menu Bar**: Bridge core actions to the macOS `NSMenu` at the top of the screen (App Name, File, Edit, View, Window, Help).
- [ ] **Translucent Sidebar**: Left-pane sidebar using the `BehindWindow` vibrancy material (`NSVisualEffectView`).
- [ ] **Toolbars**: Top window toolbars (`NSToolbar`) that align with the window control buttons seamlessly.

### Input & Hover State
- [ ] **Pointer Adaptations**: System cursor changes (`NSCursor`: pointing hand, text I-beam, resize arrows).
- [ ] **Keyboard Shortcuts**: System-wide standard modifiers (Cmd `⌘`, Option `⌥`, Shift `⇧`, Control `⌃`).
- [ ] **Hover Effects**: Distinct highlight states on buttons and list items when the mouse hovers (less prominent on iOS, critical on macOS).
- [ ] **Right-Click Menus**: Standard drop-shadow desktop context menus (`NSMenu`), omitting the large blurred background used on iOS.

### Visuals
- [ ] **Window Shadows**: Deep, diffuse drop shadows specific to macOS active/inactive window states.
- [ ] **Focus Rings**: Subtle blue or accent-colored outlines around focused text fields and inputs (`NSFocusRingType`).

---

## 📺 3. tvOS (The 10-Foot Experience)

### The Focus Engine
- [ ] **Spatial Navigation**: 2D directional navigation driven by the Siri Remote.
- [ ] **Parallax Focus**: When an item receives focus, it elevates, casts a larger shadow, and applies a 3D parallax effect based on thumb movement on the Siri Remote.
- [ ] **Focus State Scaling**: Items gracefully enlarge (e.g., 1.1x) with spring animations when focused.

### Layout & Legibility
- [ ] **Overscan Margins**: Ensure all interactive elements remain within the tvOS safe area to avoid being cut off by television bezels.
- [ ] **Typography**: Implement significantly upscaled font profiles optimized for viewing from 10+ feet away.
- [ ] **Top Shelf**: Support for passing static or dynamic imagery to the tvOS home screen top shelf.

---

## ⌚ 4. watchOS (Glanceable & Wearable)

### Input & Hardware
- [ ] **Digital Crown Integration**: Direct mapping of the physical rotating crown to `cmp_scroll.h` offsets, including detent haptics.
- [ ] **Taptic Engine**: Precise use of hardware haptics for Success, Failure, Retry, and Directional clicks (`WKHapticType`).
- [ ] **Always-On Display (AOD)**: Dimming visual components and severely reducing render loop frequency to preserve battery.

### Layout & Components
- [ ] **Edge-to-Edge**: Black backgrounds that blend seamlessly into the hardware bezel; no explicitly drawn borders.
- [ ] **Hierarchical Pagination**: Vertical, paginated scrolling or simple push-navigation tailored to tiny displays.
- [ ] **Full-Width Buttons**: Pill-shaped buttons that span the entire horizontal width of the display.
- [ ] **Complications**: Exporting static timeline data to watch faces (Circular, Rectangular, Inline).

---

## 🎨 5. Shared Cupertino Design System

### Typography
- [ ] **San Francisco (SF)**: Native font rendering using `SF Pro` (iOS/macOS/tvOS), `SF Compact` (watchOS), `SF Mono`, and `New York` (serif).
- [ ] **Dynamic Type**: Text scaling responding directly to system A11y text size sliders.
- [ ] **Optical Sizing**: Adjusting tracking (letter-spacing) automatically based on point size (e.g., tighter tracking for large Display text, looser for small Text).

### Color & Materials
- [ ] **System Colors**: Use Apple's semantic system colors (e.g., `systemBlue`, `systemRed`) that shift their precise RGB values between Light and Dark modes.
- [ ] **Vibrancy & Blur**: Implement multi-layered background blurs mapping to `UIBlurEffectStyle` (Ultra Thin, Thin, Normal, Thick).
- [ ] **Accent/Tint Color**: A single global accent color that propagates down the widget tree, tinting active interactive elements.

### Shapes & Icons
- [ ] **Continuous Corners (Squircles)**: Corner radii mapped using Apple's continuous curve formula (`kCACornerCurveContinuous`) rather than standard circular arcs to eliminate tangent breaks.
- [ ] **SF Symbols Integration**: Full mapping of `cmp_icon.h` requests to the OS-native SF Symbols library, respecting weight, scale, and multi-color rendering modes.

### Feedback
- [ ] **Rubber-Banding**: Over-scrolling lists and scroll views must resist and snap back using exact Apple friction physics.
- [ ] **Haptics (UIFeedbackGenerator)**: Selection, Impact (Light/Medium/Heavy), and Notification haptics bound to core interactions.

---

## ♿ 6. Advanced Accessibility (A11y) Integration

While `c-multiplatform` generates semantic trees natively, deep OS integration is required:
- [ ] **VoiceOver**: Announce semantic labels, traits (Button, Header, Image), and custom actions via OS bridges.
- [ ] **Switch Control**: Ensure logical, sequential focus-traversal across the entire UI tree.
- [ ] **Reduce Motion**: Hook into OS environmental flags to strip spring physics and replace screen transitions with simple cross-fades.
- [ ] **Reduce Transparency**: Provide opaque color fallbacks when the OS setting disables rendering of Vibrancy materials.
- [ ] **Increase Contrast**: Thicker borders, deeper shadows, and modified color palettes when enabled in OS settings.