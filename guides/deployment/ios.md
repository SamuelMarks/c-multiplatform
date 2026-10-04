# iOS Deployment & Testing Guide

This guide provides end-to-end instructions for building, testing, and distributing the C-Multiplatform engine on iOS natively. It covers everything from running the app in the Simulator to deploying it to a physical device over USB, and finally packaging it for TestFlight.

## Prerequisites

Before building for iOS, ensure you have the following installed on your Mac:
- **macOS** (latest recommended)
- **Xcode** (installed from the Mac App Store)
- **CMake** (`brew install cmake`)
- **Apple Developer Account** (for physical device testing and TestFlight)

---

## 1. Building the iOS Application

The iOS build leverages CMake to generate an Xcode project, linking against native iOS frameworks (`UIKit`, `Foundation`, `QuartzCore`, etc.).

### CMake Generation

To generate the Xcode project targeting iOS:

```bash
mkdir build_ios && cd build_ios
cmake .. -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=iphoneos \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED="NO"
```

To build the executable from the command line:

```bash
xcodebuild -project c-multiplatform-engine.xcodeproj -scheme ui_engine -configuration Debug -sdk iphoneos build
```

*(Note: The CMake project automatically utilizes `cmake/ios.toolchain.cmake` and configures the `MACOSX_BUNDLE` properties including the generated `Info.plist.in` and `Assets.xcassets`.)*

---

## 2. Running in the iOS Simulator

The fastest way to test the UI engine on iOS is using the Xcode iOS Simulator.

### Using Xcode (Recommended)
1. Open the generated Xcode project:
   ```bash
   open build_ios/c-multiplatform-engine.xcodeproj
   ```
2. In the top toolbar, select your application target (e.g., `ui_engine` or `example_basic`).
3. Select an iOS Simulator device from the drop-down menu (e.g., "iPhone 15 Pro").
4. Click the **Play (Run)** button or press `Cmd + R`. The Simulator will launch, and your app will run.

### Using Command Line (`simctl`)
If you prefer the CLI, you can build for the simulator and install it directly:
```bash
# 1. Build for the Simulator
cmake .. -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_ARCHITECTURES=arm64
xcodebuild -project c-multiplatform-engine.xcodeproj -scheme ui_engine -configuration Debug -sdk iphonesimulator

# 2. Boot a Simulator
xcrun simctl boot "iPhone 15 Pro"

# 3. Install and Launch
xcrun simctl install booted build_ios/Debug-iphonesimulator/ui_engine.app
xcrun simctl launch booted com.example.uiengine
```

---

## 3. Running on a Connected Device over USB

To deploy the engine to a physical iPhone or iPad, your app must be signed with a valid Apple Developer certificate.

1. Connect your iPhone/iPad to your Mac via USB (or Wi-Fi debugging).
2. Open the Xcode project:
   ```bash
   open build_ios/c-multiplatform-engine.xcodeproj
   ```
3. **Configure Code Signing**:
   - In Xcode's Project Navigator, select your target.
   - Go to the **Signing & Capabilities** tab.
   - Check **"Automatically manage signing"**.
   - Select your Apple Developer account as the **Team**. (If you haven't added an account, go to *Xcode > Settings > Accounts* to sign in).
   - Set the Bundle Identifier to your unique ID (e.g., `com.yourcompany.uiengine`).
4. Select your connected iOS device from the destination drop-down menu in the toolbar.
5. Press the **Play (Run)** button (`Cmd + R`).
6. *First Time Only:* You may need to trust your developer certificate on the device by going to **Settings > General > VPN & Device Management**, tapping your Apple ID, and selecting "Trust".

---

## 4. Packaging and Releasing for TestFlight

When you are ready to distribute your app to internal or external testers via TestFlight, use the automated build script.

The script `scripts/build_ios.sh` cleanly archives your app and generates a `.ipa` package using `xcodebuild` without requiring manual Xcode GUI intervention.

### Prerequisites for App Store Connect
- An App Record created in **App Store Connect**.
- A Distribution Certificate and a valid App Store Provisioning Profile.
- (Optional but recommended) App Store Connect API Key for automated uploading.

### Automated Archiving

Run the provided wrapper script. It expects the generated build directory and an output directory for the IPA:

```bash
# Ensure the script is executable
chmod +x scripts/build_ios.sh

# Run the archive script
./scripts/build_ios.sh build_ios export_dir
```

This script will:
1. Ensure an `exportOptions.plist` exists.
2. Run `xcodebuild archive` to create a `build.xcarchive`.
3. Run `xcodebuild -exportArchive` to sign and package the app into `export_dir/ui_engine.ipa`.

### Uploading to TestFlight

Once you have your `ui_engine.ipa` file in the `export_dir`, upload it to TestFlight.

**Option A: Using Transporter App (GUI)**
1. Download the **Transporter** app from the Mac App Store.
2. Sign in with your Apple Developer account.
3. Drag and drop the generated `ui_engine.ipa` into Transporter and click **Deliver**.

**Option B: Using Xcode Organizer (GUI)**
1. Open Xcode and go to **Window > Organizer**.
2. Select your `build.xcarchive` under the Archives tab.
3. Click **Distribute App**, select **TestFlight & App Store**, and follow the wizard.

**Option C: Using Command Line (`altool` / `xcrun`)**
To fully automate CI/CD, upload using your App Store Connect API keys:

```bash
xcrun altool --upload-app \
    --type ios \
    --file export_dir/ui_engine.ipa \
    --apiKey "YOUR_API_KEY_ID" \
    --apiIssuer "YOUR_ISSUER_ID"
```

Once uploaded and processed by Apple, the build will appear in App Store Connect under the **TestFlight** tab, ready to be distributed to your testers!

---

## 5. Getting Approved in the Apple App Store (Pure C Engine Specifics)

Apple's App Store Review guidelines are heavily biased toward apps built with Swift, SwiftUI, and standard UIKit. Because the C-Multiplatform engine utilizes a pure C codebase with a custom rendering pipeline (backed by CoreGraphics, OpenGL ES, or Metal) and interacts with iOS via direct `objc_msgSend` bindings, your application is essentially an "Opaque Canvas" to standard iOS analysis tools.

To ensure your C application is approved and doesn't trigger automated or manual rejections, you must strictly adhere to the following C-Multiplatform specific guidelines:

### 1. Defeating the "Opaque Rectangle" (Accessibility)
To Apple reviewers and automated App Store analysis tools, a pure C Metal/OpenGL application looks like a single, blank `UIView`.
- **The Risk:** Apps that provide no accessibility tree are routinely rejected under Guideline 2.1 (Performance: App Completeness) or 4.0 (Design).
- **The C-Multiplatform Solution:** You *must* ensure that the C engine's Accessibility Tree Bridging (`ui_aria.h`) is active. The engine's iOS backend (`src/ui_window_backend_ios.c`) uses the Objective-C runtime to dynamically implement the `UIAccessibilityContainer` protocol on the root view. This exposes your C-rendered UI nodes (buttons, sliders, text) as native `UIAccessibilityElement` proxies.
- **Validation:** Before submitting, open Xcode's **Accessibility Inspector** and ensure it can "see" and read your C-rendered buttons.

### 2. Native Text Input & The Software Keyboard
Do **NOT** attempt to render a custom on-screen software keyboard in C.
- **The Risk:** Apple rejects custom keyboards that bypass system AutoFill, Dictation, or password managers.
- **The C-Multiplatform Solution:** The engine's `ui_input_base.c` component is designed to bridge to a hidden native `UITextField` or `UITextInput` proxy in the iOS backend. When a user taps a C-rendered text field, the engine activates this invisible native proxy to summon the standard iOS software keyboard. Always rely on this bridge rather than raw touch-to-character mapping.

### 3. Strict Runloop & Energy Efficiency (`CADisplayLink`)
Pure C game loops (`while(1) { poll_events(); render(); }`) will cause your app to be rejected for excessive battery drain (Guideline 2.5 - Performance).
- **The C-Multiplatform Solution:** The engine hooks directly into the iOS `CADisplayLink` via pure C `objc_msgSend` bindings. Ensure your application logic relies on `ui_tick_engine.c` and properly idles (`ui_engine_is_idle()`) when there are no active animations or pointer events. If your C app burns 100% CPU drawing static screens, Apple will reject it.

### 4. Avoiding Private APIs in Pure C
Because C-Multiplatform relies on dynamic message sending (`objc_msgSend`, `sel_registerName`, `objc_getClass`) to avoid requiring `.m` or `.swift` files, you are under strict static analysis scrutiny.
- **The Risk:** App Store Connect runs static analysis on your uploaded binary. If it detects `dlsym` lookups or `sel_registerName` calls for private Apple frameworks, your binary will be instantly rejected.
- **The Rule:** Only ever pass documented, public UIKit/Foundation selectors to `sel_registerName`. Do not use runtime inspection to bypass Apple's sandboxing.

### 5. Dynamic Type & Typography Scales
Native iOS apps scale text automatically when users change their system text size (Accessibility).
- **The C-Multiplatform Solution:** The iOS backend listens for `UIContentSizeCategoryDidChangeNotification`. Ensure your application UI reacts to the C engine's `ui_typography_scale.h` updates. If your C-rendered text remains statically sized while the user has iOS "Larger Text" enabled, you risk rejection for accessibility non-compliance.

### 6. App Review Narrative & Explaining Your Architecture
Reviewers may be confused by an app that doesn't behave exactly like standard SwiftUI.
- In the **App Store Connect Review Notes**, proactively explain: *"This application utilizes a custom, highly-optimized cross-platform rendering engine. While the UI is custom-drawn via Metal/CoreGraphics, it fully implements UIAccessibility protocols for VoiceOver, uses native hidden text fields to ensure standard iOS Keyboard and AutoFill support, and perfectly respects Safe Area insets."*
- Explicitly documenting your compliance saves the reviewer from having to guess if your app is a poorly-wrapped web view.
