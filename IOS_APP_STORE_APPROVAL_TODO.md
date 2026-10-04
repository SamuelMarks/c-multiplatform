# iOS App Store Approval & Pure C Compliance Plan

Because the C-Multiplatform engine utilizes a pure C codebase with a custom rendering pipeline (backed by CoreGraphics, OpenGL ES, or Metal) and interacts with iOS via direct `objc_msgSend` bindings, your application is essentially an "Opaque Canvas" to standard iOS analysis tools.

To ensure the C application is approved and doesn't trigger automated or manual rejections, you must strictly implement and verify the following C-Multiplatform specific guidelines.

## 1. Defeating the "Opaque Rectangle" (Accessibility & VoiceOver)
- [ ] **UIAccessibilityContainer Proxying:** Verify that `ui_window_backend_ios.c` explicitly registers a `UIView` subclass utilizing `objc_allocateClassPair`.
- [ ] **Proxy Protocol Conformance:** Ensure `accessibilityElementCount`, `accessibilityElementAtIndex:`, and `indexOfAccessibilityElement:` are fully implemented via `class_addMethod` using pure C implementations.
- [ ] **VoiceOver Hit Testing:** Map touch coordinates from iOS directly into `ui_aria.h` bounding boxes to ensure VoiceOver properly announces focus changes.

## 2. Native Text Input & The Software Keyboard
- [ ] **Hidden Native Proxies:** Do not attempt to render a custom software keyboard. Implement a hidden `UITextField` or `UITextInput` proxy in `src/ui_window_backend_ios.c`.
- [ ] **Keyboard Summoning:** Bridge `ui_input_base.c` focus events to trigger `becomeFirstResponder` on the native proxy to summon the keyboard natively (ensuring AutoFill/Dictation compliance).

## 3. Strict Runloop & Energy Efficiency (`CADisplayLink`)
- [ ] **No Busy Polling:** Ensure there are zero instances of `while(1) { poll(); render(); }` which will be rejected for excessive battery drain.
- [ ] **CADisplayLink Integration:** Verify `src/ui_window_backend_ios.c` drives the render loop exclusively via `CADisplayLink`.
- [ ] **Engine Idling:** Ensure the runloop dynamically pauses when `ui_engine_is_idle()` returns true, suspending `CADisplayLink` to conserve power.

## 4. Avoiding Private APIs in Pure C
- [ ] **Public Selector Audit:** Because we rely on dynamic message sending to avoid `.m`/`.swift` files, audit all usages of `sel_registerName` and `objc_getClass` to ensure no private Apple frameworks are called.
- [ ] **Static Analysis Compliance:** Verify that `dlsym` is not used to bypass sandbox limitations to resolve undocumented selectors.

## 5. Dynamic Type & Typography Scales
- [ ] **Notification Listening:** Listen for `UIContentSizeCategoryDidChangeNotification` dynamically.
- [ ] **Engine Scaling Bridge:** Map the retrieved scale multiplier down into the C engine's `ui_typography_scale.h` logic so text resizing feels native.

## 6. App Review Narrative & Info.plist Safety
- [ ] **Privacy Keys:** Generate an exact list of all required `NS...UsageDescription` keys required by the application in `cmake/Info.plist.in`.
- [ ] **App Store Connect Notes Template:** Provide a standardized text snippet for the App Store Review Notes proactively explaining that the app utilizes a highly-optimized cross-platform rendering engine that fully implements HIG, UIAccessibility protocols, and Safe Area insets.
