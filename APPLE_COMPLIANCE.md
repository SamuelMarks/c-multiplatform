# Apple Human Interface Guidelines (Cupertino) Exhaustive Compliance Checklist

This document is a deeply exhaustive, granular checklist designed to achieve 100% compliance with Apple's Human Interface Guidelines (HIG), App Store Review Guidelines, and platform-native behaviors. It is intended to guide the `c-multiplatform` framework's development, ensuring every platform feels native, pixel-perfect, and behaviorally accurate from the UI layer down to memory management, testing, C-interoperability, and documentation.

---

## 1. Accessibility (A11y) & Inclusive Design

### 1.1 Vision & VoiceOver Integration
- [ ] **VoiceOver Labels**: Every interactive and meaningful non-interactive element provides a localized `accessibilityLabel`.
- [ ] **VoiceOver Hints**: Complex or ambiguous actions have an `accessibilityHint` describing the result of the action (e.g., "Plays the selected track").
- [ ] **VoiceOver Value**: Sliders, steppers, and progress bars accurately report their `accessibilityValue` as it changes.
- [ ] **Accessibility Traits**: All elements report the correct OS-level traits (`.button`, `.header`, `.image`, `.link`, `.searchField`, `.playsSound`, `.selected`, `.notEnabled`).
- [ ] **Element Grouping**: Logically related elements are grouped into a single focusable VoiceOver node to reduce swipe fatigue.
- [ ] **Custom Rotors**: Complex list or canvas views provide custom VoiceOver rotors to filter navigation by specific element types (e.g., "Headings", "Links").
- [ ] **Custom Actions**: Complex views utilize custom VoiceOver actions to expose swipe-actions or hidden buttons.
- [ ] **Focus Order**: The semantic tree ensures a strict logical focus order (top-to-bottom, leading-to-trailing).
- [ ] **Braille Screen Input**: Text input fields hook correctly into system APIs to support Braille screen input.
- [ ] **Audio Descriptions**: Media players default to playing secondary audio descriptive tracks if enabled in OS settings.
- [ ] **Pronunciation Assistance**: Use `accessibilityAttributedLabel` to provide phonetic pronunciation guides for unusual words or brand names.
- [ ] **Direct Touch Areas**: Specialized canvas areas (e.g., a piano keyboard) declare `.allowsDirectInteraction` to bypass standard VoiceOver swipe gestures.
- [ ] **Announcement APIs**: The framework properly fires `UIAccessibility.post(notification: .announcement, argument: "...")` for asynchronous, non-visual state changes.

### 1.2 Display, Text & Color Accommodations
- [ ] **Dynamic Type**: All text elements automatically scale their font size in response to system UI content size categories.
- [ ] **Large Text Reflow**: At accessibility text sizes (`UIContentSizeCategoryAccessibilityLarge` and above), horizontal layouts reflow into vertical stacks.
- [ ] **Target Size Scaling**: As text size increases, the surrounding UI containers and touch targets scale proportionally to prevent clipping.
- [ ] **Bold Text Setting**: All text dynamically switches to a heavier font weight when the user enables "Bold Text" in OS settings.
- [ ] **Button Shapes**: When enabled in OS settings, text-only buttons display an explicit background shape or underline.
- [ ] **Increase Contrast**: When enabled, the framework increases the opacity of blurred backgrounds, darkens separators, and utilizes the high-contrast variant of system colors.
- [ ] **Reduce Transparency**: When enabled, all vibrancy effects and blur materials (`UIBlurEffect`) fall back to solid, opaque system colors.
- [ ] **Smart Invert**: Images, video views, and dark-themed canvas areas are tagged with `accessibilityIgnoresInvertColors` to prevent looking like negatives.
- [ ] **Differentiate Without Color**: Critical states are indicated by distinct shapes, icons, or text labels, not solely by red/green color changes.
- [ ] **Contrast Ratios**: Standard text maintains a strict minimum contrast ratio of 4.5:1 against its background. Large text maintains 3:1.
- [ ] **Hover Text (macOS/iPadOS)**: UI supports showing large, high-contrast text bubbles when the user hovers over elements with the pointer (if enabled in Accessibility).

### 1.3 Motion, Cognitive & Hearing Accommodations
- [ ] **Reduce Motion**: When enabled, spring-based translation animations and parallax effects fall back to simple cross-fades.
- [ ] **Auto-Play Video Avoidance**: Respects the OS setting to disable auto-playing videos or animated GIFs.
- [ ] **Switch Control**: UI elements provide explicit focus anchors, ensuring sequential, predictable scanning behaviors.
- [ ] **Voice Control**: All interactive elements are mapped to the Voice Control grid, exposing numeric tags and phonetic labels.
- [ ] **Guided Access Compatibility**: UI doesn't break or trap users when hardware buttons are disabled via Guided Access.
- [ ] **Time Limits**: Session timeouts or temporary alerts are extended or removed if the user requires more time to read (cognitive a11y).
- [ ] **Sound Recognition/Captions**: App provides visual alternatives to all critical audio cues.

---

## 2. Privacy, Security & Data Minimization

### 2.1 Permissions & Hardware Access
- [ ] **Contextual Prompting**: Hardware permissions (Location, Camera, Mic) are requested strictly at the exact moment of use, never on app launch.
- [ ] **Purpose Strings (Info.plist)**: System permission alerts include highly specific, user-focused explanations of exactly *why* the access is needed.
- [ ] **Limited Photo Library**: Supports the iOS 14+ limited photo picker, handling dynamic state changes gracefully.
- [ ] **Approximate Location**: Apps function gracefully if the user grants "Approximate" rather than "Precise" location data.
- [ ] **App Tracking Transparency (ATT)**: App clearly explains the value proposition before presenting the system ATT prompt.
- [ ] **Pasteboard Access Banners**: Pasteboard data is only read when explicitly initiated by a user action (e.g., tapping "Paste") to avoid system privacy banners firing.
- [ ] **Microphone/Camera Indicators**: Custom UI never obscures the system's top-bar colored recording indicators (Orange/Green dots).
- [ ] **Screen Recording Prevention**: Sensitive views (e.g., DRM video, passwords) use `UITextField` secure entry hacks or `AVPlayer` protections to black out during system screen recording.

### 2.2 Authentication, Credentials & Safety
- [ ] **Sign in with Apple (SIWA)**: SIWA button is prominently displayed, strictly adhering to Apple's branding and sizing guidelines.
- [ ] **SIWA Hidden Email**: Seamlessly handles account creation and routing using the private relay email addresses generated by SIWA.
- [ ] **Face ID / Touch ID (LocalAuthentication)**: Uses `LAContext` to provide native biometrics with immediate fallback to Passcode/Password.
- [ ] **Keychain Integration**: Sensitive user credentials and tokens are securely stored in the system Secure Enclave/Keychain, never in `UserDefaults`.
- [ ] **AutoFill & Security Types**: Text fields are tagged with `textContentType` (e.g., `.username`, `.newPassword`, `.oneTimeCode`) to trigger system password managers.
- [ ] **Secure Screen Obscuring**: When the app goes into the background/app switcher, sensitive data (banking, health) is obscured with a blur or logo screen.
- [ ] **Sensitive Content Warning**: Framework integrates with system APIs to optionally blur incoming explicit photos/videos before rendering them.

---

## 3. Visual Design & Rendering Architecture

### 3.1 Color Management & Modes
- [ ] **Semantic System Colors**: The framework exclusively uses dynamic semantic colors (`systemBlue`, `label`, `secondarySystemBackground`), avoiding hardcoded hexes.
- [ ] **Dark Mode True Black**: iOS base view backgrounds utilize pure black (`#000000`) in dark mode to leverage OLED power savings.
- [ ] **Dark Mode Elevation**: iOS modal sheets utilize elevated, lighter gray colors in dark mode to indicate z-axis depth, replacing drop shadows.
- [ ] **Accent/Tint Color**: A single, global accent color propagates down the view hierarchy (`tintColor`), tinting buttons, sliders, and active states.
- [ ] **P3 Wide Color Pipeline**: The rendering pipeline natively supports 16-bit color channels and wide color (Display P3) assets.
- [ ] **Hardware Color Profiling**: Rendered colors match the current screen's ICC color profile natively, avoiding washed-out looks on newer displays.
- [ ] **EDR (Extended Dynamic Range)**: UI supports rendering HDR imagery or video highlights above the standard SDR white point.

### 3.2 Typography & Text Rendering
- [ ] **San Francisco (SF) Ecosystem**: Accurate requests to `SF Pro` (default), `SF Compact` (watchOS), `SF Mono` (code), or `New York` (serif).
- [ ] **Optical Sizing Matrix**: The engine automatically shifts from the *Text* variant to the *Display* variant of SF Pro at 20pt or larger.
- [ ] **Dynamic Tracking Formula**: Automatically tightens letter-spacing for large Display text and loosens it for small Text, replicating Apple's proprietary tracking tables.
- [ ] **Variable Fonts**: Framework supports OpenType Variable axes, allowing fluid interpolation of font weight and width.
- [ ] **System Weights**: Fully supports all 9 typographic weights (Ultralight, Thin, Light, Regular, Medium, Semibold, Bold, Heavy, Black).
- [ ] **Semantic Text Styles**: Maps strictly to: Large Title, Title 1-3, Headline, Body, Callout, Subhead, Footnote, Caption 1-2.
- [ ] **Baseline Alignment**: Text blocks strictly align their mathematical baselines across horizontal layouts, regardless of differing font sizes/weights.
- [ ] **Tabular Figures**: Numeric displays in timers or scores automatically utilize monospace tabular numerals to prevent layout jitter.
- [ ] **Line Height & Leading**: Paragraphs strictly adhere to Apple's default line heights and paragraph spacing metrics.
- [ ] **Kerning & Ligatures**: Advanced typographic features (standard ligatures, kerning pairs) are enabled by default for all text rendering.

### 3.3 Materials, Blurs & Vibrancy
- [ ] **UIBlurEffect Styles**: Maps exactingly to standard thickness levels: Ultra Thin, Thin, Regular, Thick, Prominent.
- [ ] **UIVibrancyEffect Layers**: Text and icons layered on blurred backgrounds automatically apply vibrancy, blending luma with underlying content.
- [ ] **macOS Window Materials**: Maps to specific macOS materials: `WindowBackground`, `BehindWindow` (sidebars), and `UnderWindowBackground` (scrolling content).
- [ ] **Material Transitions**: Animating between different blur styles (e.g., Thin to Thick) crossfades smoothly without rendering artifacts.

### 3.4 Iconography & SF Symbols
- [ ] **SF Symbols Native Integration**: UI icons map to the OS-native SF Symbols library, rendering cleanly at any vector scale.
- [ ] **Symbol Weight Matching**: Symbols automatically inherit the font weight of adjacent text.
- [ ] **Symbol Scale Constraints**: Supports Small, Medium, and Large symbol scales relative to the current font size.
- [ ] **Monochrome Rendering**: Supports standard single-color rendering.
- [ ] **Hierarchical Rendering**: Supports symbols containing multiple opacities of a single tint color.
- [ ] **Palette Rendering**: Supports assigning multiple independent colors to specific layers of a single symbol.
- [ ] **Multicolor Rendering**: Displays symbols in their intrinsic full-color states (e.g., weather icons).
- [ ] **Variable Color**: Supports dynamic filling of symbols based on a percentage (e.g., WiFi bars).
- [ ] **Symbol Animations**: Natively triggers iOS 17+ symbol animations (Bounce, Scale, Pulse, Replace, Variable Color loops).
- [ ] **Custom Symbols**: If custom icons are required, they are exported as annotated SF Symbol SVG templates to guarantee weight/scale parity.

### 3.5 Layout, Margins & Geometry
- [ ] **Safe Areas (Mobile)**: Strict adherence to `safeAreaInsets`, avoiding the Notch, Dynamic Island, and Home Indicator.
- [ ] **Safe Areas (TV)**: Strict adherence to overscan margins on tvOS.
- [ ] **Standard Margins**: View controllers enforce 16pt (compact screens) or 20pt (regular screens) leading/trailing layout margins.
- [ ] **Readable Content Guide**: Text-heavy views constrain width to the `readableContentGuide` preventing excessively long lines on iPads or Macs.
- [ ] **Continuous Corners (Squircles)**: Corner radii strictly utilize Apple's continuous curve formula (`kCACornerCurveContinuous`) to eliminate sharp tangent breaks.
- [ ] **Pixel Alignment**: All view geometry is rounded to exact display scale boundaries to prevent sub-pixel anti-aliasing blurring on borders and text.

---

## 4. Animation, Motion & Rendering Performance

### 4.1 Spring Physics & Timing
- [ ] **CASpringAnimation Equivalency**: All layout animations are driven by mass, stiffness, damping, and initial velocity.
- [ ] **Interruptibility**: Animations can be interrupted mid-flight; the new animation uses the current presentation value and velocity as its starting point.
- [ ] **Scrubbable Animations**: Animations can be paused, scrubbed via gestures, and reversed (equivalent to `UIViewPropertyAnimator`).
- [ ] **Gestural Velocity Hand-off**: Swipe and scroll gestures calculate exact termination velocity (points-per-second) and pass it directly into the resting spring animation.
- [ ] **Overshoot & Bounciness**: Animations naturally slightly overshoot and settle without feeling sluggish.

### 4.2 ProMotion (120Hz) & Render Loop
- [ ] **CADisplayLink Synchronization**: The render loop is bound directly to the hardware display refresh rate.
- [ ] **120Hz Unlocking**: Animations and scroll events correctly declare their desired frame rate to unlock 120Hz ProMotion on supported devices (`preferredFrameRateRange`).
- [ ] **Variable Refresh Rate (VRR)**: Static screens drop to 10Hz/24Hz to save battery; scrolling ramps instantly to 120Hz.
- [ ] **Frame Drop Thresholds**: The framework guarantees 0 dropped frames for standard scrolling and view transitions on A12+ hardware.

### 4.3 Haptics (UIFeedbackGenerator)
- [ ] **Selection Haptics**: Very light, subtle ticks triggered when scrolling through pickers or changing segment controls.
- [ ] **Impact Haptics**: Light, Medium, Heavy, Rigid, and Soft haptics mapped to physical collisions or snapping.
- [ ] **Notification Haptics**: Success, Warning, and Error haptic patterns triggered *only* after a definitive user-initiated action.
- [ ] **Audio-Haptic Sync**: Haptic feedback fires synchronously with associated system UI sounds.
- [ ] **Engine Warmup**: Haptic engines are preemptively "warmed up" (`prepare()`) just before an anticipated interaction to eliminate latency.

---

## 5. Navigation & Architectural Routing

### 5.1 Routing Models
- [ ] **Hierarchical (Navigation Bar)**: Uses standard push/pop transitions sliding horizontally. Depth is indicated by back button labels displaying the exact previous view's title.
- [ ] **Flat (Tab Bar)**: Mutually exclusive, peer categories. Tapping the currently active tab instantly pops its navigation stack back to the root view.
- [ ] **Content-Driven (Split View)**: Master-detail relationships utilizing a sidebar or list on the leading edge and content on the trailing edge.
- [ ] **Full-Screen Modals**: Used only for highly immersive tasks (photo editing, video playback, document creation) requiring hidden status bars.
- [ ] **Sheets (Modals)**: Slide from the bottom. Used for discrete tasks (creating an item, settings). The parent view recedes slightly into the background and dims.

### 5.2 Navigation Bars
- [ ] **Large Titles**: Titles default to large, bold, left-aligned text at the top of the view.
- [ ] **Scroll Collapse**: Large titles smoothly collapse into standard, centered, inline titles as the user scrolls down.
- [ ] **Appearance States**: Framework perfectly maps `standardAppearance`, `compactAppearance`, and `scrollEdgeAppearance` states.
- [ ] **Back Button Chevrons**: Uses the exact, heavily weighted Apple chevron symbol, positioned correctly with standard padding.
- [ ] **Back Button Truncation**: If the previous view's title is too long, the back button label truncates gracefully or falls back to the localized word "Back".

### 5.3 Tab Bars & Toolbars
- [ ] **Tab Bar Placement**: Fixed at the bottom on iOS. Automatically transitions to a leading Sidebar on iPadOS.
- [ ] **Tab Bar Limits**: Strictly limited to a maximum of 5 tabs on iOS. Additional tabs fall into a standard "More" tab.
- [ ] **Toolbar Placement**: Positioned at the bottom on iPhone, or integrated into the top Navigation Bar on iPad/Mac. Holds contextual actions (Share, Delete, Edit).
- [ ] **Badge Support**: Tabs support standard red circle badges with white text for notifications.
- [ ] **Sidebar Translucency**: Mac and iPad sidebars use the `BehindWindow` material, making the desktop or background views bleed through uniquely.

### 5.4 Search Architecture
- [ ] **Search Bars**: Features a centered placeholder text and magnifying glass, smoothly transitioning to leading-aligned when focused.
- [ ] **Clear Button**: Includes a circular 'x' button on the trailing edge to instantly clear input.
- [ ] **Cancel Button**: A "Cancel" button dynamically slides in from the trailing edge when active, allowing keyboard dismissal.
- [ ] **Scope Bars**: Supports segmented controls immediately below the search bar to filter search context (e.g., "All", "Unread", "Flagged").
- [ ] **Search Suggestions**: Integrates an instant-updating suggestion list overlaid on the main content while typing.

### 5.5 Deep Linking & State Restoration
- [ ] **Universal Links**: Natively handles `NSUserActivity` for Universal Links, routing the user deeply into specific views without bouncing through Safari.
- [ ] **State Restoration**: Encodes and decodes view controller state so the app returns to the exact scroll position and navigation depth after being killed by the OS.
- [ ] **Multi-Window State**: On iPadOS/macOS, perfectly isolates `UIWindowScene` states, allowing multiple independent instances of the app to restore correctly.

---

## 6. Controls & Components

### 6.1 Buttons & Toggles
- [ ] **System Buttons**: Text-only or Icon+Text. Instantly dim or reduce opacity to ~30% on touch-down without artificial delay.
- [ ] **Filled/Prominent Buttons**: Capsule-shaped or continuous rounded rectangles with a solid background and high-contrast text.
- [ ] **Tinted Buttons**: Buttons with a 15-20% opacity background of the tint color.
- [ ] **Gray Buttons**: Buttons with a subtle, translucent system gray background.
- [ ] **Close Buttons**: Small, circular, blurred gray backgrounds with a dark 'x' symbol, positioned strictly in the top-right corner of modals.
- [ ] **Toggles (Switches)**: Pill-shaped toggles. Green (or accent) when ON, translucent gray when OFF. Includes distinct spring animation and subtle impact haptic.

### 6.2 Menus & Actions (`UIMenu` equivalents)
- [ ] **Context Menus**: Long-press on an element reveals a heavily blurred background, visually lifts the target element, and displays a vertical list of actions with trailing SF Symbols.
- [ ] **Submenus**: Menus natively support infinitely nested submenus that slide out gracefully.
- [ ] **Action Sheets**: Slide up from the bottom for choosing an action. Includes a distinct, separated "Cancel" button at the very bottom.
- [ ] **Destructive Actions**: Irreversible actions in menus and sheets are strictly colored system red.
- [ ] **Mixed States**: Menu items support displaying a "mixed" or "on/off" checkmark state.
- [ ] **Deferred Elements**: Complex menus support `UIDeferredMenuElement`, showing a standard loading spinner while async options are fetched.
- [ ] **Popovers**: Floating panels with an arrow pointing directly to the originating element, automatically dismissing when the user taps outside.

### 6.3 Inputs & Selectors
- [ ] **Wheel Pickers**: Physics-based, scrolling slot-machine wheels for short lists. Includes rapid scrolling, rubber-banding, and center-line snapping haptics.
- [ ] **Inline Date Pickers**: iOS 14+ style compact calendar grid and time entry fields, replacing full-screen wheels for standard date selection.
- [ ] **Segmented Controls**: Horizontal groups of mutually exclusive buttons. The active segment's background slides to the new selection with a distinct spring animation.
- [ ] **Sliders**: Continuous tracks with a circular thumb. The track fills with the tint color on the leading side and uses translucent gray on the trailing side.
- [ ] **Steppers**: Grouped `+` and `-` buttons. Capable of continuous, accelerating firing if held down.
- [ ] **Color Pickers**: Invokes the system `UIColorPickerViewController` providing grid, spectrum, and slider color selections with eyedropper support.

### 6.4 Text Fields & Editors
- [ ] **Keyboard Types**: Automatically summons the correct OS keyboard layout (Email, URL, Number Pad, Decimal Pad, Phone Pad).
- [ ] **Return Key Labels**: Automatically adjusts the Return key label (Go, Search, Next, Done, Send) based on context.
- [ ] **Secure Entry**: Password fields use bullet characters, disable auto-correct, and temporarily show the last typed character.
- [ ] **Input Accessory Views**: Supports attaching a custom toolbar (e.g., with a "Done" button) directly above the software keyboard.
- [ ] **Auto-Capitalization**: Supports sentence, word, and all-characters capitalization behaviors.
- [ ] **Spell Check & Auto-Correct**: Native red underlines for typos and inline predictive text bubbles.
- [ ] **Rich Text Views**: Text areas support standard Apple data detectors (making phone numbers, dates, and links interactive).

---

## 7. Views, Layouts & Data Presentation

### 7.1 Lists & Tables
- [ ] **Plain Lists**: Edge-to-edge separators. Separators align exactly with the leading edge of the *text*, not the leading edge of the icon or screen.
- [ ] **Grouped Lists**: Blocks of rows with explicit section headers (all caps, small) and footers. Background uses `systemGroupedBackground`.
- [ ] **Inset Grouped Lists**: iOS 15+ style rounded blocks of rows with strict leading/trailing margins (the standard iOS Settings app look).
- [ ] **Swipe Actions**: Swiping left or right on rows reveals contextual actions (e.g., Delete, Archive) with distinct background colors.
- [ ] **Continuous Swipe Execution**: Swipe actions can be fully executed with a continuous deep swipe past a designated threshold.
- [ ] **Edit Mode**: Lists support an edit mode revealing leading-edge red minus buttons for deletion and trailing-edge grabbers for reordering.

### 7.2 Scroll Views
- [ ] **Rubber-Banding**: Scroll views resist and bounce back using precise Apple friction physics when over-scrolled past their content bounds.
- [ ] **Scroll Indicators (Scrollbars)**: Native-looking scrollbars appear *only* during active scrolling and fade out quickly.
- [ ] **Interactive Scrollbars**: Users can long-press the scroll indicator to grab it and drag rapidly through long content.
- [ ] **Deceleration Rates**: Supports `Normal` deceleration and `Fast` deceleration (typically used for paginated or specific gallery views).
- [ ] **Keyboard Dismissal**: Supports dismissing the software keyboard interactively when the user scrolls down.
- [ ] **Paging Mode**: Horizontal scroll views support snapping perfectly to page boundaries.

### 7.3 Collections, Grids & Complex Layouts
- [ ] **Flow Layouts**: Automatically reflowing grids that adjust column counts based on screen width and orientation.
- [ ] **Compositional Layouts**: Support for complex nested scrolling (e.g., horizontal scrolling rows inside a vertically scrolling list, like the App Store).
- [ ] **Orthogonal Scrolling**: Compositional lists accurately handle orthogonal scrolling sections, managing individual section scroll states.
- [ ] **Diffable Data Sources**: Layout updates are powered by hash-based diffing, providing automatic, glitch-free insert/delete/move animations.
- [ ] **Map Views**: Integration uses MapKit equivalencies, supporting native gestures, user location tracking, and Apple Maps cartography styles.
- [ ] **Web Views**: Integration uses WKWebView equivalencies, hiding browser chrome and integrating smoothly into native scroll view hierarchies.

---

## 8. User Interaction & Multi-Input Routing

### 8.1 Touch & Multi-Touch Gestures
- [ ] **Gesture Disambiguation**: Smoothly resolves conflicts between overlapping gestures (e.g., gracefully failing a horizontal table row swipe if the user is scrolling vertically).
- [ ] **Pinch-to-Zoom**: Zooms into the exact mathematical centroid of the user's fingers. Snaps to min/max scale limits with rubber-banding resistance.
- [ ] **Rotation Gestures**: Two-finger rotation tracking with optional snap-to-axis behaviors at 0, 90, 180 degrees.
- [ ] **Edge Swipe Back**: Left-edge screen swipe seamlessly controls the interactive transition of popping a navigation view, tied perfectly to finger position.
- [ ] **Multi-Touch State Machine**: UI cleanly handles 3-finger, 4-finger, and 5-finger system gesture overrides without crashing or locking state.

### 8.2 Keyboard & Trackpad (iPadOS & macOS)
- [ ] **Pointer Morphing (iPadOS)**: The circular trackpad pointer magnetically snaps to buttons, removing its outline and lifting the button element with a specular highlight.
- [ ] **Pointer Hover Regions**: Supports `.highlight`, `.lift`, and `.hover` pointer interaction styles for custom UI elements.
- [ ] **Trackpad Gestures**: Supports two-finger swipe back/forward for navigation, and two-finger double tap for smart zoom.
- [ ] **Keyboard Shortcuts**: Discoverable via holding the `Cmd` key on iPadOS. Maps all standard OS shortcuts (Cmd-C, Cmd-V, Cmd-Z, Cmd-Space, Cmd-F).
- [ ] **Menu Commands (`UICommand`)**: Defines a robust command hierarchy that populates the macOS global menu bar and iPadOS keyboard shortcut menu simultaneously.
- [ ] **Full Keyboard Navigation**: Users can navigate the entire UI via Tab, Shift-Tab, Space, Arrow keys, Enter, and Escape. Focus rings are clearly drawn.
- [ ] **Key Repeat**: Press-and-hold on keyboard arrows triggers continuous, accelerating scrolling or selection.

### 8.3 Apple Pencil & Stylus
- [ ] **Double-Tap/Squeeze (Pencil Gen 2/Pro)**: Responds to the physical double-tap/squeeze gesture on the Pencil barrel (e.g., swapping to an eraser tool, invoking tool palette).
- [ ] **Scribble**: Standard text input fields natively interpret Apple Pencil handwriting to text anywhere on the screen.
- [ ] **Pressure & Tilt**: Drawing canvases respond fluidly and with zero-latency to azimuth, altitude, and pressure variants from the hardware.
- [ ] **Hover Effect (iPad Pro)**: UI elements react to the Pencil hovering directly above the screen before making physical contact (e.g., previewing brush size).

### 8.4 Drag and Drop
- [ ] **Lift Animation**: Elements visually lift (drop shadow increases, slight scale up) when a drag begins.
- [ ] **Multi-item Drag**: Users can tap additional items while already dragging one to create a grouped "stack". A badge number appears showing the total count.
- [ ] **Drop Targets**: Valid drop targets visually highlight or expand when hovered over by dragged items.
- [ ] **Spring Loading**: Hovering over a folder or navigation link while dragging triggers a blink and automatically navigates into that destination after a short delay.

### 8.5 Game Controllers (GameController Framework)
- [ ] **Standard Navigation**: UI can be navigated naturally using D-Pad and left thumbstick, mapping perfectly to standard focus events.
- [ ] **Haptic Rumble**: Emits specific, timed rumble waveforms to Xbox/PlayStation controllers during critical interactions or game events.
- [ ] **Adaptive Triggers**: Supports configuring resistance and tension on DualSense triggers.

---

## 9. System Integration & Deep OS Hooks

### 9.1 App Extensions & Capabilities
- [ ] **Widgets (Home Screen)**: Provides read-only, glanceable widgets (Small, Medium, Large, Extra Large). Respects the strict memory limits (~30MB) for Widget Extensions.
- [ ] **Interactive Widgets (iOS 17+)**: Supports simple toggles and buttons on widgets that execute App Intents without launching the main app.
- [ ] **Live Activities**: Displays ongoing, real-time tasks (timers, deliveries, scores) on the Lock Screen.
- [ ] **Dynamic Island**: Maps Live Activities to `Compact`, `Minimal`, and `Expanded` presentation styles perfectly formatted for the Island hardware cutout.
- [ ] **Share Extension**: Registers a custom UI in the system Share Sheet for receiving links/images from other apps.
- [ ] **Action Extension**: Registers custom actions in the system Share Sheet (e.g., "Watermark Image") that operate on selected content.
- [ ] **App Clips**: Framework can compile down to an App Clip under the strict 10MB (or 50MB) limits for instantaneous QR/NFC launch.

### 9.2 OS Communications
- [ ] **Siri & App Intents**: Exposes core app capabilities to Siri and the Shortcuts app so users can build custom automations natively.
- [ ] **Share Sheet (`UIActivityViewController`)**: Used to pass text, URLs, and standard media to the OS sharing layer, completely avoiding custom share menus.
- [ ] **Handoff (Continuity)**: Broadcasts current user activity state (e.g., reading an article) so it can be resumed instantly on another logged-in Apple device via the dock.
- [ ] **Core Spotlight**: App content is indexed locally so users can search for documents, items, or states directly from the iOS/macOS system search (Spotlight).
- [ ] **Focus Modes**: Status updates respect the user's current Focus Mode (e.g., suppressing non-urgent badges while "Work" focus is active).
- [ ] **SharePlay / GroupActivities**: Hooks into FaceTime to allow synchronized media playback or collaborative canvas editing over a call.

### 9.3 Media, Audio & AVKit
- [ ] **System Media Player (`AVPlayerViewController`)**: Prefers system-provided standard video controls that support PIP, AirPlay, and subtitles out of the box.
- [ ] **Picture in Picture (PiP)**: Video playback automatically collapses into a floating PiP window when the user swipes home.
- [ ] **Now Playing Info Center**: Provides metadata (Title, Artist, Artwork) and playback progress to the OS so it displays on the Lock Screen, Apple Watch, and Control Center.
- [ ] **Remote Command Center**: Responds correctly to hardware headphone buttons, Lock Screen controls, and Siri commands (Play, Pause, Skip, Seek).
- [ ] **Spatial Audio**: Audio pipeline tags streams correctly to allow dynamic head-tracking Spatial Audio on supported AirPods.
- [ ] **Audio Session Management**: Correctly manages `AVAudioSession` categories (e.g., Ducking background audio, pausing when interrupted by a phone call).

### 9.4 Transactions, Files & Cloud
- [ ] **Apple Pay**: Uses the exact `PKPaymentButton` styling, adhering strictly to branding guidelines (no custom modifications to the Apple Pay logo or font).
- [ ] **In-App Purchases (StoreKit)**: Presents transparent subscription terms, utilizing native `StoreKit` interfaces for purchases, restorations, and cancellations.
- [ ] **Files App Integration**: Declares proper document types so internal app files can be exposed to and managed via the system Files app.
- [ ] **iCloud Key-Value Sync**: Small user preferences automatically sync across devices using `NSUbiquitousKeyValueStore`.
- [ ] **CloudKit Integration**: Larger datasets synchronize gracefully via CloudKit without blocking the main UI thread.

---

## 10. Platform-Specific Master Checklist

### 10.1 iOS (iPhone)
- [ ] **Keyboard Avoidance**: All inputs automatically scroll upward to remain visible above the software keyboard.
- [ ] **Pull-to-Refresh**: Native `UIRefreshControl` spinner behavior on lists, triggering via a rubber-band pull past the top edge.
- [ ] **Detent Sheets**: Modal sheets support snapping to `medium` (half screen) and `large` (full screen) detents.
- [ ] **Reachability**: UI doesn't break when the user triggers the system "Reachability" feature (pulling the screen down halfway).
- [ ] **Context Menu Previews**: Deep pressing lists reveals a rich UI preview of the destination before fully committing to the navigation.

### 10.2 iPadOS (iPad)
- [ ] **Multitasking Adaptability**: Layouts dynamically swap from regular to compact size classes during Split View and Slide Over, acting exactly like an iPhone app when narrow.
- [ ] **Sidebars**: Primary navigation uses a 2- or 3-column Split View layout. Sidebars collapse gracefully into buttons on narrower widths.
- [ ] **Multi-Window Support**: Allows users to tear off tabs, open multiple instances of the app simultaneously, and manage them via Stage Manager.
- [ ] **Center Stage**: Camera integration natively supports Center Stage, keeping the user in frame as they move around.

### 10.3 macOS (Mac)
- [ ] **Window Controls (Traffic Lights)**: Red, Yellow, Green buttons precisely align with the top-left corner of the window frame.
- [ ] **Global Menu Bar**: App Name, File, Edit, View, Window, Help menus are populated at the top of the screen.
- [ ] **Context Menus**: Standard drop-shadow desktop context menus, specifically *omitting* the large blurred background, scale effects, and large rounded corners used on iOS.
- [ ] **Hover States**: Distinct highlight states on list items and buttons when the mouse hovers, critical for precision non-touch interfaces.
- [ ] **Window Shadows**: Deep, diffuse, multi-layered drop shadows for active windows; lighter, flatter shadows for inactive windows.
- [ ] **Document Proxies**: Windows representing files display a proxy icon next to the title, which can be dragged to move the file.
- [ ] **Menu Bar Extras**: Background or utility states provide a system tray icon in the top right menu bar.
- [ ] **Toolbar Customization**: Top toolbars support native right-click "Customize Toolbar..." behavior, allowing users to drag/drop tools.
- [ ] **Printing**: Integrates directly with `NSPrintOperation` for standard macOS print dialogs.

### 10.4 watchOS (Apple Watch)
- [ ] **Digital Crown Integration**: Crown rotation maps exactly to scroll offsets with physical detent haptics as list items pass the center line.
- [ ] **Double Tap Gesture**: Supports the Series 9/Ultra 2 physical "Double Tap" finger gesture for the primary view action.
- [ ] **Edge-to-Edge Design**: Pure black backgrounds (`#000000`) that blend seamlessly into the physical hardware bezel. No cards, borders, or side insets.
- [ ] **Complications**: Exports rich data for all watch face templates (Circular, Rectangular, Inline, Corner, Graphic Bezel).
- [ ] **Always-On Display (AOD)**: When the wrist is down, visual components dim, sensitive data hides, and the render loop drops to 1Hz or lower.
- [ ] **Pill Buttons**: Primary actions use full-width, pill-shaped buttons placed at the bottom of views to maximize tiny touch targets.
- [ ] **Hierarchical Pagination**: Navigation heavily favors vertical, paginated scrolling or simple 1-deep push navigation.
- [ ] **Smart Stack**: App provides relevant UI snippets to the watchOS 10+ Smart Stack invoked by turning the crown from the watch face.

### 10.5 tvOS (Apple TV)
- [ ] **Focus Engine**: UI is strictly navigable via 2D spatial navigation driven exclusively by the Siri Remote. Touch screen events are completely ignored.
- [ ] **Parallax Effect**: Focused items elevate, cast a deep shadow, and slightly tilt/pan based on tiny thumb micro-movements resting on the remote trackpad.
- [ ] **Focus Scaling**: Items enlarge gracefully (e.g., 1.1x) with spring animations when they receive focus.
- [ ] **Overscan Margins**: Generous outer margins ensure all interactive elements remain visible on older televisions with physical bezel overscan.
- [ ] **Top Shelf (Carousel)**: Provides dynamic, scrollable, full-screen video/imagery to the tvOS home screen when the app is placed in the top row.
- [ ] **Top Shelf (Inset)**: Provides grids of specific content links (e.g., "Continue Watching") to the top shelf.
- [ ] **Hardware Play/Pause**: The physical Play/Pause button on the remote acts globally to control media, regardless of UI focus.
- [ ] **Menu Button**: The physical "Menu" or "Back" button always pops the navigation stack or exits the app; it is never overridden to perform custom actions.

### 10.6 visionOS (Spatial Computing)
- [ ] **Glass Materials**: Windows utilize standard OS glass background materials with specular highlights that react physically to virtual room lighting.
- [ ] **Eye Tracking (Hover)**: Elements exhibit a subtle glow or hover effect the exact moment the user *looks* at them, indicating readiness for a pinch gesture.
- [ ] **Depth & Z-Axis**: Layer hierarchy strictly utilizes the Z-axis. Modals appear physically closer to the user than the main window.
- [ ] **Ornaments**: Toolbars and controls sit slightly outside and floating in front of the main window bounds, facing the user directly.
- [ ] **Volumes vs Windows**: Uses flat Windows for 2D interfaces, bounded 3D Volumes for inspectable objects, and Immersive Spaces for full environments.
- [ ] **Immersion Levels**: Properly handles `mixed`, `progressive`, and `full` immersion styles.
- [ ] **Direct vs Indirect Touch**: Supports both reaching out and physically poking virtual buttons, and looking-and-pinching from a distance.
- [ ] **Shared Space Behavior**: App Windows are good citizens, allowing the user to place them anywhere without forcing an Immersive takeover unless explicitly requested.

### 10.7 CarPlay
- [ ] **Driving Focus**: High-contrast interfaces, massive touch targets, and absolute minimal text to reduce driver distraction.
- [ ] **CarPlay Templates**: Strictly adheres to standard CarPlay system templates (List, Grid, Map, Audio Player) to pass App Store review. No custom arbitrary UI rendering.
- [ ] **Siri Voice Integration**: All critical actions can be completed purely via voice while driving.
- [ ] **Knob Navigation**: UI is fully navigable via physical car rotary knobs (similar to tvOS Focus Engine).
- [ ] **Offline Resilience**: Maps and audio playback degrade gracefully during tunnels or cellular dead zones without throwing blocking alerts.

---

## 11. Internationalization & Localization (i18n)

### 11.1 Right-to-Left (RTL) Layouts
- [ ] **UI Mirroring**: All leading/trailing constraints perfectly flip horizontally for RTL languages (Arabic, Hebrew). Fixed left/right constraints are never used.
- [ ] **Icon Mirroring**: Directional icons (back, forward, text alignment) automatically flip. Non-directional icons (checkmarks, clocks, play buttons) do not.
- [ ] **Animation Mirroring**: Push navigation animations slide from right-to-left instead of left-to-right.

### 11.2 Text, Data Formatting & Collation
- [ ] **Dynamic Text Expansion**: UI allows text labels to expand up to 50% without clipping, accommodating highly verbose languages (e.g., German, Russian).
- [ ] **Pluralization**: Strictly uses localized `.stringsdict` files to handle all plural forms (Zero, One, Two, Few, Many, Other) per specific language grammar rules.
- [ ] **Inflection (Grammar Agreement)**: Framework supports Automatic Grammar Agreement (iOS 15+) to alter phrases based on the gender or number of a subject (e.g., in Spanish or Russian).
- [ ] **Dates & Times**: Exclusively uses system formatters so date ordering (MM/DD/YYYY vs DD/MM/YYYY) and 12h/24h time match the user's OS locale setting.
- [ ] **Numbers & Currency**: Exclusively uses system formatters so currency symbols and decimal/grouping separators match the locale.
- [ ] **Name Formatting (`PersonNameComponentsFormatter`)**: Uses system contact formatters to respect regional name ordering (e.g., Family Name first in Japanese/Korean).
- [ ] **Measurement Units**: Automatically converts and displays measurements (miles/kilometers, Fahrenheit/Celsius) based on the locale.
- [ ] **List Formatting (`ListFormatter`)**: Arrays of strings are formatted naturally (e.g., "A, B, and C" vs "A, B y C") based on locale.
- [ ] **Vertical Text**: CJK (Chinese, Japanese, Korean) text support allows for proper vertical layout rendering and ruby annotations if required.
- [ ] **Collation (Sorting)**: Lists are sorted using localized string comparison (e.g., ignoring diacritics appropriately) rather than raw ASCII/Unicode byte values.

---

## 12. C-Framework Bridging & Interoperability (For `c-multiplatform`)

Since this is a framework bridging lower-level C to Apple platforms, specific Objective-C/Swift interop guidelines must be met to feel native to Apple developers.

### 12.1 ABI Stability & Interfaces
- [ ] **Header Auditing**: C-headers are wrapped in `NS_ASSUME_NONNULL_BEGIN` and `NS_ASSUME_NONNULL_END` to ensure Swift imports them with safe Optionality.
- [ ] **Nullability Annotations**: Pointers explicitly use `_Nullable`, `_Nonnull`, or `_Null_unspecified` to inform the Swift compiler of safety contracts.
- [ ] **CF_ENUM / NS_ENUM**: C enums are strictly defined using `NS_ENUM` or `NS_CLOSED_ENUM` so Swift imports them as robust Enum types, not raw integers.
- [ ] **NS_EXTENSIBLE_STRING_ENUM**: Typedef string constants use `NS_EXTENSIBLE_STRING_ENUM` to map directly to Swift structs.

### 12.2 Swift Refinements
- [ ] **NS_REFINED_FOR_SWIFT**: Clunky C-functions or pointers are marked with `NS_REFINED_FOR_SWIFT`, allowing a cleaner Swift wrapper to be written while hiding the raw C implementation.
- [ ] **CFString Bridging**: CoreFoundation strings (`CFStringRef`) properly toll-free bridge to `NSString` and seamlessly import as native Swift `String`.
- [ ] **ARC Compatibility**: C memory allocations strictly document ownership transfer (`CFRetain`/`CFRelease`), utilizing `__attribute__((ns_returns_retained))` when returning objects to ARC.

---

## 13. Testing, Validation & Quality Assurance

### 13.1 Automated UI & Snapshot Testing
- [ ] **XCTest UI Equivalency**: The framework supports automated UI testing workflows equivalent to `XCUITest`, capable of tapping elements by their accessibility labels.
- [ ] **Snapshot Testing (Light/Dark)**: Automated snapshot tests verify every UI component renders pixel-perfect in both Light and Dark modes.
- [ ] **Snapshot Testing (Dynamic Type)**: Snapshot tests verify every UI component reflows correctly at `Large` and `AccessibilityExtraExtraExtraLarge` font sizes.
- [ ] **Snapshot Testing (RTL)**: Snapshot tests verify UI mirroring for Right-to-Left locales.
- [ ] **Animation Testing**: Test suites can disable animations globally (`UIView.setAnimationsEnabled(false)`) to speed up execution and eliminate race conditions.

### 13.2 Accessibility Auditing
- [ ] **Accessibility Inspector Validation**: The framework's rendered UI passes cleanly through the Xcode Accessibility Inspector with zero warnings for contrast or touch target size.
- [ ] **VoiceOver Audits**: A human tester or automated VoiceOver script verifies that navigating from the top-left to bottom-right of every screen is logical and unbroken.

### 13.3 Memory, Threading & Performance Profiling
- [ ] **Main Thread Rendering**: The framework guarantees that all UI updates, layer modifications, and render loop submissions occur strictly on the main thread.
- [ ] **Background Offloading**: Heavy layout calculations, image decoding, and text sizing are offloaded to background threads before hitting the main render loop.
- [ ] **Retain Cycle Prevention**: Strong reference cycles in view hierarchies and event delegates are explicitly tested for and eliminated to prevent memory leaks.
- [ ] **Zombie Object Prevention**: Event handlers and gesture recognizers safely nil-out references to destroyed views.
- [ ] **OSSignpost Telemetry**: Framework emits `os_signpost` metrics during layout and render passes, allowing precise profiling in Instruments.

---

## 14. Documentation, DX & Framework APIs

### 14.1 Developer Experience (DX)
- [ ] **Declarative APIs**: The framework provides a modern, declarative API surface mimicking SwiftUI's ease of use while maintaining UIKit's deep control.
- [ ] **Type Safety**: Enums and strongly typed structs are used for colors, fonts, and layout options instead of strings or raw integers.
- [ ] **Live Previews**: The framework architecture supports hot-reloading or Xcode Canvas-style live previews for rapid UI iteration.
- [ ] **Debug Hierarchy**: Developers can pause execution and inspect a visual 3D representation of the view hierarchy, similar to Xcode's Debug View Hierarchy.

### 14.2 Documentation & Examples
- [ ] **DocC Compliant**: All public APIs are documented using Apple's DocC format, providing rich, inline Xcode help.
- [ ] **HIG References**: API documentation explicitly references the corresponding Apple Human Interface Guidelines to educate the developer.
- [ ] **Code Snippets**: Documentation includes copy-pasteable code snippets for common tasks (e.g., setting up a TableView, configuring a Navigation Bar).
- [ ] **Sample Applications**: The repository includes fully functional sample apps replicating standard Apple apps (e.g., "Settings", "Clock", "Mail") to prove compliance.

---

## 15. Performance, Battery & System Resources

### 15.1 Battery & Thermal Impact
- [ ] **Thermal State Awareness**: The framework listens to `ProcessInfo.thermalState`. If the device gets dangerously hot (`.serious` or `.critical`), it scales down animations and frame rates.
- [ ] **Background Processing**: When the app is backgrounded, all render loops, animations, and non-essential timers are instantly paused to save battery.
- [ ] **Opaque Rendering Optimization**: Views that do not require transparency are explicitly marked as `opaque = true` to skip costly alpha blending on the GPU.

### 15.2 Network & Data Constraints
- [ ] **Low Data Mode**: The framework respects the system "Low Data Mode" flag, preventing large background downloads, auto-playing videos, or heavy image prefetching.
- [ ] **Image Caching**: Remote images are decoded asynchronously, resized to the exact display geometry, and aggressively cached in memory and on disk.
- [ ] **Core Graphics Efficiency**: Custom drawing (`drawRect` equivalents) avoids allocating massive offscreen bitmap contexts unless absolutely necessary.

---

## 16. Error Handling, Edge Cases & Resilience

### 16.1 State & Error Presentation
- [ ] **Empty States**: Lists and grids provide beautiful, informative "Empty States" with an icon, title, and call-to-action when there is no data.
- [ ] **Loading Skeletons**: Screens utilize shimmering skeleton views mimicking the final layout rather than blocking the user with a full-screen spinner.
- [ ] **Offline Resilience**: The app remains functional and navigable without a network connection, clearly indicating offline status with a subtle banner or icon.
- [ ] **Non-Blocking Errors**: Non-critical network failures use inline banners or subtle toast notifications rather than modal, center-screen Alerts that interrupt the user.

### 16.2 Data Loss Prevention
- [ ] **Undo/Redo Stack**: Forms, text editors, and complex configurations support standard Shake-to-Undo and 3-finger-swipe-left to undo.
- [ ] **Draft Preservation**: If a user hits "Cancel" on a modal sheet where they have typed data, an Action Sheet prompts them to "Save Draft" or "Discard Changes".
- [ ] **Graceful Degradation**: If a specific platform feature (e.g., ARKit, LiDAR) is unavailable on older hardware, the UI gracefully falls back to a standard alternative without crashing.

---

## 17. App Store Review Guidelines Compliance

### 17.1 Design Rejection Avoidance
- [ ] **Minimum Functionality**: The framework ensures the resulting app acts like a native application, not a wrapped website, avoiding guideline 4.2 rejections.
- [ ] **Standard Controls**: The framework does not repurpose standard controls for non-standard behaviors (e.g., using a volume slider to scroll a list).
- [ ] **Hidden Features**: The framework does not include obfuscated code, hidden Easter eggs, or remote feature toggles designed to bypass App Store Review.
- [ ] **Platform Parity**: The framework does not prompt users to use other platforms or reference Android/Windows UI paradigms (e.g., "Material Design" ripples are completely stripped out).
- [ ] **Content Guidelines**: Pre-packaged example apps contain absolutely no placeholder text ("Lorem Ipsum") or dummy data in submitted App Store screenshots.
